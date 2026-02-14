/**********************************************************************}
{                                                                      }
{            L      U   U   DDDD   W      W  IIIII   GGGG              }
{            L      U   U   D   D   W    W     I    G                  }
{            L      U   U   D   D   W ww W     I    G   GG             }
{            L      U   U   D   D    W  W      I    G    G             }
{            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              }
{                                                                      }
{**********************************************************************}
{                                                                      }
{   Copyright (C) 1981, 1987                                           }
{   Department of Computer Science, University of Adelaide, Australia  }
{   All rights reserved.                                               }
{   Reproduction of the work or any substantial part thereof in any    }
{   material form whatsoever is prohibited.                            }
{                                                                      }
{**********************************************************************/

/**
! Name:         SCREEN
!
! Description:  Map a range of lines onto the screen, or unmap the
!               screen.
!               Maintain that mapping.
!               Also SCREEN supports the HARDCOPY/BATCH mode of editing,
!               by providing methods of outputting lines and error
!               messages under these circumstances as well.
!**/

#include "screen.h"

// Global Data.
//  SCR_FRAME             -- Frame indicating which frame currently mapped.
//  SCR_TOP_LINE          -- A line_pointer indicating the first line of screen.
//  SCR_BOT_LINE          -- A line_pointer indicating the last  line of screen.
//  SCR_MSG_ROW           -- Lowest row on screen with msg on it.
//
//  LINE.SCR_ROW_NR       -- The row number of a line.  0 means 'not
//                           on screen'.
//
//  FRAME.SCR_OFFSET      -- To find the column number on the screen
//                           from the column number of a character in
//                           a line of a frame, subtract this value.

#include "line.h"
#include "var.h"
#include "vdu.h"

#include <cstring>
#include <iomanip>
#include <iostream>

enum class slide_type { slide_dont, slide_left, slide_right, slide_redraw };

enum class scroll_type { scroll_dont, scroll_forward, scroll_back, scroll_redraw };

const char PAUSE_MSG[] = "Pausing until RETURN pressed: ";
const char YNAQM_MSG[] = "Reply Y(es),N(o),A(lways),Q(uit),M(ore)";
const std::string YNAQM_CHARS(" YNAQM123456789");

void writeln(const std::string_view &message) {
    std::cout << message << std::endl;
}

void writeln(const char *message, size_t length) {
    writeln(std::string_view(message, length));
}

void writeln(const char *message) {
    writeln(message, std::strlen(message));
}

void write(const std::string_view &message) {
    std::cout.write(message.data(), message.size());
}

void write(const char *message, size_t length) {
    std::cout.write(message, length);
}

void write(const char *message) {
    write(message, std::strlen(message));
}

void write(char ch, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        std::cout << ch;
    }
}

void screen_message(const std::string_view &message) {
    /*
      Purpose  : Put a message out to the user.
      Inputs   : message: null-terminated message.
    */

    if (hangup)
        return;

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        size_t i = 0;
        do {
            screen_free_bottom_line(); // Make room for msg.
            vdu_movecurs(1, terminal_info.height);
            int j = message.size() - i;
            if (j > terminal_info.width - 1)
                j = terminal_info.width - 1;
            vdu_attr_bold();
            vdu_displaystr(j, message.data() + i, 3); // due to wrap avoidance.
            vdu_attr_normal();
            i += j;
        } while (i < message.size());
    } else {
        /* if (ludwig_mode = ludwig_mode_type::ludwig_hardcopy) */
        /*     putchar(7); */
        writeln(message);
    }
}

void screen_draw_line(line_ptr line) {
    // Draw a line if it is on the screen.

#ifdef DEBUG
    if (line->scr_row_nr == 0) {
        screen_message(DBG_INVALID_SCR_ROW_NR);
        return;
    }
#endif
    vdu_movecurs(1, line->scr_row_nr);
    int offset = scr_frame->scr_offset;
    int strlen;
    if (line->flink != nullptr) {
        strlen = line->used - offset;
    } else {
        strlen = line->len;
        offset = 0;
    }
    if (strlen <= 0) {
        vdu_cleareol();
    } else {
        if (strlen > scr_frame->scr_width)
            strlen = scr_frame->scr_width;
        vdu_displaystr(line->str->slice(offset + 1, strlen), 3);
    }
    if (line->scr_row_nr == scr_msg_row)
        scr_msg_row += 1;
}

void screen_redraw() {
    // Redraw the screen, exactly as is.

    if (scr_frame != nullptr) {
        vdu_clearscr();
        scr_msg_row = terminal_info.height + 1;
        scr_needs_fix = false;
        line_ptr line = scr_top_line;
        while (line != scr_bot_line) {
            screen_draw_line(line);
            line = line->flink;
        }
        screen_draw_line(line);
    }
}

void screen_slide_line(line_ptr line, int slide_dist, slide_type slide_state) {
    // Slide 'line' line, according to slide_dist, slide_state.
    // NOTE THIS IS REALLY A PRIVATE ROUTINE FOR SCREEN_POSITION.
    //      AND FOR                              SCREEN_SLIDE.
    // It assumes the scr_offset has been changed by slide_dist
    // and that the lines on the screen are being accordingly fixed.

    if (line->flink == nullptr)
        return; // Dont slide NULL line.

    col_offset_range offset = scr_frame->scr_offset;
    scr_col_range width = scr_frame->scr_width;

    // with line^ do
    vdu_movecurs(1, line->scr_row_nr);
    if (slide_state == slide_type::slide_left) {
        int overlap = line->used - offset;
        if (overlap > 0) {
            if (overlap > slide_dist) {
                vdu_insertchars(slide_dist);
                overlap = slide_dist;
            }
            vdu_displaystr(line->str->slice(offset + 1, overlap), 2 /*anycurs*/);
        }
    } else {
        if (offset - slide_dist < line->used) {
            int overlap = line->used /*+ 1*/ - (offset - slide_dist + width /*+ 1*/);
            if (slide_dist >= width) {
                vdu_cleareol();
                slide_dist = width;
            } else {
                vdu_deletechars(slide_dist);
                vdu_movecurs(width + 1 - slide_dist, line->scr_row_nr);
            }
            if (overlap > 0) {
                if (overlap > slide_dist)
                    overlap = slide_dist;
                vdu_displaystr(
                    line->str->slice(offset + width + 1 - slide_dist + 1, overlap), 2 /*anycurs*/
                );
            }
        }
    }
}

void screen_slide(int dist) {
    // Slide the whole screen the specified dist, -ve means left.

    if (scr_frame != nullptr) {
        if (dist != 0) {
            scr_frame->scr_offset += dist;
            slide_type s;
            if (dist < 0) {
                s = slide_type::slide_left;
                dist = -dist;
            } else {
                s = slide_type::slide_right;
            }
            line_ptr l = scr_top_line;
            while (l != nullptr) {
                screen_slide_line(l, dist, s);
                if (l == scr_bot_line)
                    l = nullptr;
                else
                    l = l->flink;
            }
        }
    }
}

void screen_unload() {
    // Unload the screen.

    if (scr_frame != nullptr) {
        if (scr_frame->dot->line->scr_row_nr == 0)
            scr_frame->scr_dot_line =
                (scr_frame->margin_top + scr_frame->scr_height - scr_frame->margin_bottom + 1) / 2 +
                (terminal_info.height - scr_frame->scr_height) / 2;
        else
            scr_frame->scr_dot_line = scr_frame->dot->line->scr_row_nr;
        vdu_clearscr();
        scr_msg_row = terminal_info.height + 1;
        scr_needs_fix = false;
        scr_top_line->scr_row_nr = 0;
        while (scr_top_line != scr_bot_line) {
            scr_top_line = scr_top_line->flink;
            scr_top_line->scr_row_nr = 0;
        }
        scr_frame = nullptr;
        scr_bot_line = nullptr;
        scr_top_line = nullptr;
    }
}

void screen_expand(bool init_upwards, bool init_downwards);

void screen_scroll(int count, bool expand) {
    // Scroll the screen forward or back by the specified number of lines.
    // The only limit is the final screen must contain at least one line.
    // If expand is true, the screen must keep the same number of lines.
    // Count is +ve, means scroll forward, -ve means backwards.
    // The actual method of moving is optimized into a REDRAW if "nicer".
    //
    // WARNING Screen_Position predicts the behaviour of this routine.
    //         In particular it predicts which lines this routine will not
    //         scroll off the screen. Any changes here therefore must have
    //         implications there also.

    if (scr_frame == nullptr)
        return;

    line_ptr bot_line = scr_bot_line;
    line_ptr top_line = scr_top_line;

    if (count >= 0) { // FORWARD DIRECTION.
        line_range bot_line_nr;
        if (expand) {
            if (!line_to_number(bot_line, bot_line_nr))
                return;
            int eop_line_nr =
                scr_frame->last_group->first_line_nr + scr_frame->last_group->last_line->offset_nr;
            int remaining_lines = eop_line_nr - bot_line_nr;
            if (remaining_lines < count)
                count = remaining_lines;
            scr_row_range bot_line_row = bot_line->scr_row_nr;
            int free_lines = terminal_info.height - bot_line_row;
            if (free_lines > count)
                free_lines = count;
            if (count - free_lines <= bot_line_row) {
                // Won't be redrawing so extend downwards if possible.
                for (scr_row_range row_nr = bot_line_row + 1; row_nr <= bot_line_row + free_lines;
                     ++row_nr) {
                    bot_line = bot_line->flink;
                    bot_line->scr_row_nr = row_nr;
                    screen_draw_line(bot_line);
                }
                scr_bot_line = bot_line;
                count -= free_lines;
                if (count == 0)
                    return;
            }
        }

        // Check whether or not we would be scrolling to far.

        if (count > bot_line->scr_row_nr) {
            // Would have to scroll to far, redraw instead.
            frame_ptr frame;
            if (expand) { // Remember where to reload the screen.
                frame = scr_frame;
                bot_line_nr += count;
                if (!line_from_number(scr_frame, bot_line_nr, bot_line))
                    return;
            }
            screen_unload();
            if (expand) {
                scr_frame = frame;
                scr_top_line = bot_line;
                scr_bot_line = bot_line;
                bot_line->scr_row_nr = terminal_info.height;
                screen_draw_line(bot_line);
                screen_expand(true, false);
            }
            return;
        }

        // SCROLL 'COUNT' LINES ONTO THE SCREEN.

        while (count > 0) { // Scroll lines on the
            count -= 1;     // the screen.
            vdu_scrollup(1);
            if (scr_msg_row <= terminal_info.height)
                scr_msg_row -= 1;

            if (expand) {
                bot_line->flink->scr_row_nr = bot_line->scr_row_nr;
                screen_draw_line(bot_line->flink);
                bot_line = bot_line->flink;
            } else {
                bot_line->scr_row_nr -= 1;
#ifdef DEBUG
                if (bot_line->scr_row_nr == 0) {
                    screen_message(DBG_WRONG_ROW_NR);
                    return;
                }
#endif
            }

            top_line->scr_row_nr -= 1;
            if (top_line->scr_row_nr == 0) {     // See if scrolled off.
                top_line->flink->scr_row_nr = 1; // Make next line top
                top_line = top_line->flink;      // of screen.
            }
        }
    } else {
        // BACKWARD DIRECTION.
        count = -count;
        line_range top_line_nr;
        if (expand) {
            if (!line_to_number(top_line, top_line_nr))
                return;
            int remaining_lines = top_line_nr - 1;
            if (remaining_lines < count)
                count = remaining_lines;
            scr_row_range top_line_row = top_line->scr_row_nr;
            int free_lines = top_line_row - 1;
            if (free_lines >= count)
                free_lines = count;

            if (top_line_row + count - free_lines <= terminal_info.height + 1) {
                // IT IS WORTH WHILE EXTENDING THE SCREEN.
                for (scr_row_range row_nr = top_line_row - 1; row_nr >= top_line_row - free_lines;
                     --row_nr) {
                    top_line = top_line->blink;
                    top_line->scr_row_nr = row_nr;
                    screen_draw_line(top_line);
                }
                scr_top_line = top_line;
                count -= free_lines;
                if (count == 0)
                    return;
            }
        }

        // Check whether or not to scroll.

        if (count + top_line->scr_row_nr > terminal_info.height + 1) {
            // NONE OF THE CURRENT STUFF WILL BE LEFT ON THE SCREEN.
            // REDRAW
            frame_ptr frame;
            line_ptr tmp_top_line;
            if (expand) { // Remember where to reload the screen.
                frame = scr_frame;
                top_line_nr -= count;
                if (!line_from_number(scr_frame, top_line_nr, tmp_top_line))
                    return;
            }
            screen_unload();
            if (expand) {
                scr_frame = frame;
                scr_top_line = tmp_top_line;
                scr_bot_line = tmp_top_line;
                tmp_top_line->scr_row_nr = 1 + terminal_info.height - scr_frame->scr_height;
                screen_draw_line(tmp_top_line);
                screen_expand(false, true);
            }
            return;
        }

        // SCROLL 'COUNT' LINES ONTO THE SCREEN.
        while (count > 0) { // Scroll lines on the
            count -= 1;     // the screen.
            vdu_movecurs(1, 1);
            vdu_insertlines(1);
            if (scr_msg_row <= terminal_info.height)
                scr_msg_row += 1;

            // with top_line^ do
            if (expand) {
                top_line->blink->scr_row_nr = top_line->scr_row_nr;
                screen_draw_line(top_line->blink);
                top_line = top_line->blink;
            } else {
                top_line->scr_row_nr -= 1;
            }

            // with bot_line^ do
            if (bot_line->scr_row_nr == terminal_info.height) {
                bot_line->scr_row_nr = 0;
                bot_line->blink->scr_row_nr = terminal_info.height;
                bot_line = bot_line->blink; // of screen.
            } else {
                bot_line->scr_row_nr += 1;
            }
        }
    }
    // NOW RESET THE DAMAGED SCREEN POINTERS AND LINE NUMBERS.

    scr_top_line = top_line;
    scr_bot_line = bot_line;

    scr_row_range row_nr = top_line->scr_row_nr; // Reset the row numbers.
    while (top_line != bot_line) {
        // with top_line^ do
        top_line->scr_row_nr = row_nr;
        top_line = top_line->flink;
        row_nr += 1;
    }
#ifdef DEBUG
    if (bot_line->scr_row_nr != row_nr) {
        screen_message(DBG_WRONG_ROW_NR);
        return;
    }
#endif
}

void screen_expand(bool init_upwards, bool init_downwards) {
    // Expand a screen out to at least the frame's specified screen height.
    // Use the allowed directions to control the expansion.

    bool upwards = init_upwards;
    bool downwards = init_downwards;

    scr_row_range height = scr_frame->scr_height;
    line_ptr bot_line = scr_bot_line;
    line_ptr top_line = scr_top_line;

#ifdef DEBUG
    if (top_line->scr_row_nr == 0) {
        screen_message(DBG_TOP_LINE_NOT_DRAWN);
        return;
    }
#endif
    scr_row_range lines_on_scr = bot_line->scr_row_nr + 1 - top_line->scr_row_nr;

    while ((lines_on_scr < height) && (upwards || downwards)) {
        if (downwards) {
            downwards = false;
            scr_row_range cur_row = bot_line->scr_row_nr;
            if (bot_line->flink != nullptr) { // Add a line at bottom if poss.
                if (cur_row < terminal_info.height) {
                    downwards = true;
                    lines_on_scr += 1;                  // Count the line added.
                    bot_line = bot_line->flink;         // Step on to next new line.
                    bot_line->scr_row_nr = cur_row + 1; // Set its number.
                    screen_draw_line(bot_line);         // Draw it on the screen.
                }
            }
        }

        if (upwards) {
            upwards = false;
            scr_row_range cur_row = top_line->scr_row_nr;
            if (cur_row > 1) {
                if (top_line->blink != nullptr) { // Add a line at top if poss.
                    upwards = true;
                    lines_on_scr += 1;                  // Count the line added.
                    top_line = top_line->blink;         // Step on to next new line.
                    top_line->scr_row_nr = cur_row - 1; // Set its number.
                    screen_draw_line(top_line);         // Draw it on the screen.
                }
            }
        }
    }

    // Reset the BOT and TOP screen pointers.
    scr_bot_line = bot_line;
    scr_top_line = top_line;

    // If just expanding wasn't enough then try scrolling to get the lines.
    if (lines_on_scr < height) {
        if (init_downwards) {
            if (bot_line->flink != nullptr) {
                screen_scroll(height - lines_on_scr, true);
                lines_on_scr = scr_bot_line->scr_row_nr + 1 - scr_top_line->scr_row_nr;
            }
        }
        if (init_upwards && lines_on_scr < height) {
            line_range nr_lines;
            if (line_to_number(scr_top_line, nr_lines)) {
                if (nr_lines >= height - lines_on_scr)
                    nr_lines = height - lines_on_scr;
                screen_scroll(-nr_lines, true);
            }
        }
    }

    // Redraw the <TOP> and <BOTTOM> markers.

    // with scr_bot_line^ do
    if (scr_bot_line->flink != nullptr) {
        scr_row_range cur_row = scr_bot_line->scr_row_nr;
        if (cur_row < terminal_info.height) {
            cur_row += 1;
            vdu_movecurs(1, cur_row);
            vdu_displaystr(8, "<BOTTOM>", 3);
            if (cur_row == scr_msg_row)
                scr_msg_row += 1;
        }
    }

    if (scr_top_line->scr_row_nr > 1) {
        vdu_movecurs(1, scr_top_line->scr_row_nr - 1);
        vdu_displaystr(5, "<TOP>", 3);
    }
}

void screen_lines_extract(line_ptr first_line, line_ptr last_line) {
    // The lines specified are removed from the screen.  If the whole screen
    // is removed then it is unmapped.

    if (last_line != scr_bot_line) {
        // EXTRACTION NOT AT BOT-OF-SCR ACCOMPLISHED VIA TERMINAL H/W.
        // In our case, handled by ncurses.
        vdu_movecurs(1, first_line->scr_row_nr);
        scr_row_range count = last_line->scr_row_nr + 1 - first_line->scr_row_nr;
        vdu_deletelines(count);
        if (scr_msg_row <= terminal_info.height)
            scr_msg_row -= count;

        line_ptr line_limit = last_line->flink;
        if (first_line == scr_top_line)
            scr_top_line = line_limit;
        count = line_limit->scr_row_nr - first_line->scr_row_nr;
        do {
            // with first_line^ do
            first_line->scr_row_nr = 0;
            first_line = first_line->flink;
        } while (first_line != line_limit);
        line_limit = scr_bot_line->flink;
        do {
            // with first_line^ do
            first_line->scr_row_nr -= count;
            first_line = first_line->flink;
        } while (first_line != line_limit);
        return;
    }

    if (first_line == scr_top_line) {
        screen_unload();
    } else /* if (last_line == scr_bot_line) */ {
        const_line_ptr line_limit = first_line->blink;
        do {
            // with scr_frame^,scr_bot_line^ do
            scr_bot_line->scr_row_nr = 0;
            scr_bot_line = scr_bot_line->blink;
        } while (scr_bot_line != line_limit);
        vdu_movecurs(1, scr_bot_line->scr_row_nr + 1);
        /*             DONT RUBOUT LINES,     THUS THE LINES STAY ON THE SCREEN UNTIL */
        /*             SCREEN_FIXUP IS CALLED.  THIS ALLOWS THE TEXT TO BE USED FOR   */
        /*             OPTIMIZATION PURPOSES WHILST THE SCREEN IS BEING FIXED UP.     */
        /*     SCR_NEEDS_FIX = TRUE;                                                  */
        /* above code removed and following line added by KBN 30-Jul-1982             */
        vdu_cleareos();
        scr_msg_row = terminal_info.height + 1;
    }
}

void screen_lines_inject(line_ptr first_line, line_range count, line_ptr before_line) {
    // This routine is called just after LINES_INJECT has insert 'count' lines
    // just above 'before_line', the first inserted being 'first_line'.
    // LINES_INJECT has not changed any 'scr_row_nr' fields,  but has checked
    // before_line->scr_row_nr != 0,  and before_line != scr_top_line.
    //
    // The rules for this routine are (1) 'first_line' should be   on the screen
    // and (2) no more than 'scr_height' lines are to be drawn onto the screen.

    line_ptr line;
    scr_row_range row_nr;
    scr_row_range lines_above_insert;
    scr_row_range lines_below_insert;

    // HEURISTIC -- KEEP AS MANY LINES ON THE SCREEN AS POSSIBLE.
    //           -- IT IS UNLIKELY THAT SCROLLUP WILL BE USED, DONT WASTE
    //              A LOT OF CODE OPTIMIZING IT.  LET IT HAPPEN IF POSSIBLE.

    scr_row_range free_space_below = terminal_info.height - scr_bot_line->scr_row_nr;
    scr_row_range free_space_above = scr_top_line->scr_row_nr - 1;

    if ((free_space_above > 0) && (before_line != scr_top_line) &&
        (terminal_info.height > before_line->scr_row_nr - free_space_above + count)) {
        int scrollup_count = count - free_space_below;
        if (scrollup_count > 0) {

            // Scrolling the screen upwards is useful to do, because it will
            // keep some lines on the screen that would otherwise have been lost.

            if (scrollup_count > free_space_above)
                scrollup_count = free_space_above;
            vdu_scrollup(scrollup_count);
            if (scr_msg_row <= terminal_info.height)
                scr_msg_row = scr_msg_row - scrollup_count;

            // WARNING -- only the SCR_ROW_NR's up to and including BEFORE_LINE
            //         -- and SCR_BOT_LINE,
            //         -- are corrected here.  There is not need to correct the
            //         -- rest because they are about to change again.

            line = scr_top_line;
            row_nr = line->scr_row_nr - scrollup_count;
            do {
                line->scr_row_nr = row_nr;
                row_nr += 1;
                line = line->flink;
            } while (line != first_line);

            // Do the two weirdo exceptions, the order is crucial in case
            // they are the same line.
            // with scr_bot_line^ do
            scr_bot_line->scr_row_nr -= scrollup_count;
            before_line->scr_row_nr = row_nr;
        }
    }

    // The screen is now optimally placed for the insertion.  If sensible
    // extend the screen upwards.

    if ((before_line == scr_top_line) && (free_space_above > 0) &&
        /* if scr_top_line->scr_row_nr+count-free_space_above <= tt_height then */
        /* but free_space_above = scr_row_nr-1, hence the following 'weird' exp */
        (count + 1 <= terminal_info.height)) {
        row_nr = scr_top_line->scr_row_nr - 1;
        while ((row_nr > 1) && (count > 0)) {
            scr_top_line = scr_top_line->blink;
            scr_top_line->scr_row_nr = row_nr;
            screen_draw_line(scr_top_line);
            row_nr -= 1;
            count -= 1;
        }
        before_line = scr_top_line;
    }

    // Finally do the insert if one necessary.

    if (count > 0) {
        if (before_line == scr_top_line)
            scr_top_line = first_line;
        row_nr = before_line->scr_row_nr;
        vdu_movecurs(1, row_nr);
        vdu_insertlines(count);
        if (scr_msg_row <= terminal_info.height) {
            scr_msg_row = scr_msg_row + count;
            if (scr_msg_row > terminal_info.height)
                scr_msg_row = terminal_info.height + 1;
        }

        // Patch up the pointers and scr_row_nr's of lines pushed off screen.

        line = scr_bot_line;
        for (int i = line->scr_row_nr + count; i >= terminal_info.height + 1; --i) {
            // with line^ do
            if (line->scr_row_nr == 0)
                break;
            line->scr_row_nr = 0;
            line = line->blink;
        }
        if (line->scr_row_nr != 0) {

            // Lines were pushed but left on the screen.
            // Do all the lines on the screen, right up to 'first_line'.

            scr_bot_line = line;
            row_nr = line->scr_row_nr + count;
            do {
                // with line^ do
                if (line->scr_row_nr == 0) {
                    line->scr_row_nr = row_nr;
                    screen_draw_line(line);
                } else
                    line->scr_row_nr = row_nr;
                row_nr -= 1;
                line = line->blink;
            } while (line != first_line);
            line->scr_row_nr = row_nr;
            screen_draw_line(line);
        } else {

            // No lines were left on the screen.  Redraw downwards until enough
            // lines on the screen.

            scr_bot_line = first_line;
            first_line->scr_row_nr = row_nr;
            screen_draw_line(first_line);
            screen_expand(false, true);
        }
    }
}

void screen_load(line_ptr line) {
    // LUDWIG_SCREEN:
    // Map the screen into the specified frame, the line and col specified must
    // be on the screen,  it is placed in the most desirable location.
    // LUDWIG_BATCH:
    // Do nothing.
    // LUDWIG_HARDCOPY:
    // Draw scr_height lines around the dot.

    const frame_ptr frame = line->group->frame;
    switch (ludwig_mode) {
    case ludwig_mode_type::ludwig_batch:
        break;

    case ludwig_mode_type::ludwig_hardcopy:
        {
            int new_row = frame->scr_height / 2;
            while ((new_row > 0) && (line->blink != nullptr)) {
                line = line->blink;
                new_row -= 1;
            }
            const_line_ptr dot_line = frame->dot->line;
            col_range dot_col = frame->dot->col;
            new_row = 1;
            while ((new_row <= frame->scr_height) && (line != nullptr)) {
                if (new_row == 1)
                    writeln("WINDOW:");
                strlen_range buflen = 0;
                // with line^ do
                buflen = line->used;
                if (line->flink == nullptr)
                    buflen = line->len;
                if (buflen > 0)
                    writeln(line->str->slice(1, buflen));
                else
                    writeln("");
                if (line == dot_line) {
                    if (dot_col == 1)
                        writeln("<");
                    else if (dot_col == MAX_STRLENP) {
                        write(' ', MAX_STRLEN - 1);
                        writeln(">");
                    } else {
                        write(' ', dot_col - 2);
                        writeln("><");
                    }
                }
                new_row += 1;
                line = line->flink;
            }
        }
        break;

    case ludwig_mode_type::ludwig_screen:
        {
            if (scr_frame != nullptr) // A frame mapped at present.
                screen_unload();      //   Unload/clear it.
            else {
                vdu_clearscr(); //   Clear the screen anyway.
                scr_msg_row = terminal_info.height + 1;
                scr_needs_fix = false;
            }

            // with line->group->frame^ do
            int new_row = frame->scr_dot_line;
            line_range line_nr;
            if (!line_to_number(line, line_nr))
                return;
            line_range eop_line_nr =
                frame->last_group->first_line_nr + frame->last_group->nr_lines - 1;
            if ((eop_line_nr - line_nr) < (terminal_info.height - new_row))
                new_row = terminal_info.height - (eop_line_nr - line_nr);
            if (line_nr < new_row)
                new_row = line_nr;

            line->scr_row_nr = new_row;

            // Move left or right in 1/2 window chunks until DOT on screen.

            col_range dot_col = frame->dot->col;
            while ((dot_col <= frame->scr_offset) ||
                   (dot_col > frame->scr_offset + frame->scr_width)) {
                col_offset_range half_width = frame->scr_width / 2;
                if (half_width == 0)
                    half_width = 1;
                if (dot_col <= frame->scr_offset) {
                    if (frame->scr_offset > half_width)
                        frame->scr_offset -= half_width;
                    else
                        frame->scr_offset = 0;
                } else if (frame->scr_offset + half_width + frame->scr_width < MAX_STRLENP)
                    frame->scr_offset += half_width;
                else
                    frame->scr_offset = MAX_STRLENP - frame->scr_width;
            }

            // Load the screen.

            scr_frame = frame;
            scr_bot_line = line;
            scr_top_line = line;
            screen_draw_line(line);
            screen_expand(true, true);
        }
        break;
    }
}

void screen_position(line_ptr new_line, col_range new_col) {
    // Position the screen so that
    //       (1) The specified line and column are on the screen.
    //       (2) At least scr_frame->scr_height lines are on the screen.
    //       (3) If possible the specified line is between the top and
    //           bottom margins.
    //       (4) No more than scr_height lines are written to the screen.

    if (new_line->group->frame != scr_frame) {
        screen_load(new_line);
        return;
    }

    // with scr_frame^ do
    col_offset_range offset = scr_frame->scr_offset;
    scr_col_range width = scr_frame->scr_width;
    scr_row_range top_margin = scr_frame->margin_top;
    scr_row_range bot_margin = scr_frame->margin_bottom;

    // Check that the specified position is not on the screen between margins.
    // The very common case is that it is, and that the screen need not be
    // moved.

    if ((new_line->scr_row_nr == 0) ||
        ((new_line->scr_row_nr - scr_top_line->scr_row_nr < top_margin) &&
         (scr_top_line->blink != nullptr)) ||
        ((scr_bot_line->scr_row_nr - new_line->scr_row_nr < bot_margin) &&
         (scr_bot_line->flink != nullptr)) ||
        (new_col <= offset) || (new_col > offset + width)) {
        // Unfortunately this is the uncommon case.
        // with scr_frame^ do
        scr_row_range height = scr_frame->scr_height;
        line_ptr bot_line = scr_bot_line;
        line_ptr top_line = scr_top_line;

        slide_type slide_state = slide_type::slide_dont; // Compute horizontal adjusting needed.
        int slide_dist = offset + 1 - new_col;
        if (slide_dist > 0) {                       // Col to left of screen.
            slide_state = slide_type::slide_redraw; // Redraw unless can slide there.
            if (offset < width / 4) {               // Heuristic, slide if sensible.
                slide_state = slide_type::slide_left;
                slide_dist = offset;
            }
        } else {
            slide_dist = new_col - (offset + width);
            if (slide_dist > 0) {                       // Col to right of screen.
                slide_state = slide_type::slide_redraw; // Redraw unless can slide there.
                if (offset > MAX_STRLENP - width / 4) { // Heuristic, slide if sensible.
                    slide_state = slide_type::slide_right;
                    slide_dist = MAX_STRLENP - (offset + width);
                }
            }
        }
        scroll_type scroll_state = scroll_type::scroll_dont;
        int scroll_dist;
        if ((slide_state != slide_type::slide_redraw) &&
            ((new_line->scr_row_nr == 0) ||
             ((new_line->scr_row_nr - scr_top_line->scr_row_nr < top_margin) &&
              (scr_top_line->blink != nullptr)) ||
             ((scr_bot_line->scr_row_nr - new_line->scr_row_nr < bot_margin) &&
              (scr_bot_line->flink != nullptr)))) {
            // Compute vertical adjusting needed.
            line_range bot_line_nr;
            line_range new_line_nr;
            line_range top_line_nr;
            if (!line_to_number(bot_line, bot_line_nr) || !line_to_number(new_line, new_line_nr) ||
                !line_to_number(top_line, top_line_nr))
                return;

            scroll_state = scroll_type::scroll_redraw;
            if ((new_line_nr < top_line_nr) ||
                ((new_line_nr < top_line_nr + top_margin) && (new_line_nr < bot_line_nr))) {
                scroll_state = scroll_type::scroll_back;
                scroll_dist = top_line_nr + top_margin - new_line_nr;
                if (scroll_dist >= top_line_nr)
                    scroll_dist = top_line_nr - 1;
            } else {
                scroll_state = scroll_type::scroll_forward;
                scroll_dist = new_line_nr - (bot_line_nr - bot_margin);
                if (scroll_dist <= 0)
                    scroll_state = scroll_type::scroll_dont;
            }
            if ((scroll_state != scroll_type::scroll_redraw) && (scroll_dist > height))
                scroll_state = scroll_type::scroll_redraw;
        }

        // At this point SLIDE_STATE, SLIDE_DIST, SCROLL_STATE and SCROLL_DIST
        // are carrying all the advice about how to get to the new position.
        // If either state has voted for a redraw, then a redraw it is, else
        // a combined scroll and slide operation is embarked on.

        if ((scroll_state == scroll_type::scroll_redraw) ||
            (slide_state == slide_type::slide_redraw))
            screen_load(new_line);
        else {
            if (slide_state != slide_type::slide_dont) {
                // Adjust the screen offset and the lines that are
                // going to be left on the screen when scrolling.

                // with scr_frame^ do
                if (slide_state == slide_type::slide_left)
                    scr_frame->scr_offset -= slide_dist;
                else
                    scr_frame->scr_offset += slide_dist;
                line_ptr line = top_line;
                switch (scroll_state) {
                case scroll_type::scroll_redraw:
                    // impossible
                    break;
                case scroll_type::scroll_dont:
                case scroll_type::scroll_forward:
                    {
                        if (scroll_state == scroll_type::scroll_dont)
                            scroll_dist = 0;

                        // Predict which lines are going to be left on the screen.
                        scr_row_range nr_rows = terminal_info.height - bot_line->scr_row_nr;
                        scr_row_range row_nr;
                        if (nr_rows >= scroll_dist)
                            row_nr = 0; // slide all the lines.
                        else            // slide all but the lines that will scroll off.
                            row_nr = scroll_dist - nr_rows;

                        // Adjust those lines that will be left on the screen.

                        do {
                            if (line->scr_row_nr > row_nr)
                                screen_slide_line(line, slide_dist, slide_state);
                            if (line != bot_line)
                                line = line->flink;
                            else
                                line = nullptr;
                        } while (line != nullptr);
                    }
                    break;

                case scroll_type::scroll_back:
                    {
                        // Decide which lines are going to be left on the screen.

                        scr_row_range nr_rows = top_line->scr_row_nr - 1;
                        scr_row_range row_nr;
                        if (nr_rows < scroll_dist) // slide all the lines
                            row_nr = terminal_info.height;
                        else // slide all the lines that won't scroll off.
                            row_nr = terminal_info.height - (scroll_dist - nr_rows);

                        // Adjust those lines that will be left on the screen.

                        do {
                            if (line->scr_row_nr <= row_nr)
                                screen_slide_line(line, slide_dist, slide_state);
                            if (line != top_line)
                                line = line->blink;
                            else
                                line = nullptr;
                        } while (line != nullptr);
                    }
                    break;
                }
            }

            switch (scroll_state) {
            case scroll_type::scroll_redraw:
                // impossible
                break;
            case scroll_type::scroll_forward:
                screen_scroll(scroll_dist, true);
                break;
            case scroll_type::scroll_back:
                screen_scroll(-scroll_dist, true);
                break;
            case scroll_type::scroll_dont:
                break;
            }
        }
    }
    screen_expand(true, true);
}

void screen_pause() {
    // Wait until user types a RETURN.  Only in SCREEN mode,  as in HARDCOPY
    // or batch there is no point in it.
    //
    // This routine DOES NOT use SCREEN_GETLINEP because then an infinite loop
    // would result FIXUP --> CLEAR_MSGS --> PAUSE --> GETLINEP --> FIXUP.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        if (scr_frame != nullptr)
            vdu_movecurs(1, 1);
        else
            vdu_displaycrlf();
        str_object buffer;
        strlen_range outlen;
        vdu_get_input(PAUSE_MSG, buffer, MAX_STRLEN, outlen);
        if (scr_top_line != nullptr) {
            if (scr_top_line->scr_row_nr == 1) {
                screen_draw_line(scr_top_line);
            } else {
                vdu_movecurs(1, 1);
                vdu_cleareol();
                if (scr_top_line->scr_row_nr == 2)
                    scr_needs_fix = true;
            }
        }
    }
}

void screen_clear_msgs(bool pause) {
    // Clear any messages off the screen.
    if (scr_msg_row <= terminal_info.height) {
        if (pause)
            screen_pause();
        if (scr_frame == nullptr) {
            vdu_clearscr();
        } else {
            vdu_movecurs(1, scr_msg_row);
            vdu_cleareos();
        }
        scr_msg_row = terminal_info.height + 1;
    }
}

void change_frame_size(frame_ptr frm, int band, int half_screen) {
    // with frm^ do
    if ((frm->scr_height == initial_scr_height) || (frm->scr_height > terminal_info.height))
        frm->scr_height = terminal_info.height;
    if ((frm->scr_width == initial_scr_width) || (frm->scr_width > terminal_info.width))
        frm->scr_width = terminal_info.width;
    // set the margins for the top and bottom
    // by default, they are 1/6 the height of the screen
    // if they are equal to initial_margin_top & bottom, then change them
    // if they are greater or equal to half the height, then change them
    if ((frm->margin_top == initial_margin_top) || (frm->margin_top >= half_screen))
        frm->margin_top = band;
    if ((frm->margin_bottom == initial_margin_bottom) || (frm->margin_bottom >= half_screen))
        frm->margin_bottom = band;
    if (frm->margin_left > terminal_info.width)
        frm->margin_left = 1;
    if ((frm->margin_right == initial_margin_right) || (frm->margin_right > terminal_info.width))
        frm->margin_right = terminal_info.width;
}

void screen_resize() {
    // The screen has changed size, so erase it, get the new size, and redraw
    // it.

    tt_winchanged = false;
    vdu_get_new_dimensions(terminal_info.width, terminal_info.height);
    scr_msg_row = terminal_info.height + 1;
    vdu_clearscr();
    // change the screen height, width, and margins of all the frames
    int band = terminal_info.height / 6;
    int half_screen = terminal_info.height / 2;
    span_ptr next_span = first_span;
    while (next_span != nullptr) {
        frame_ptr next_frame = next_span->frame;
        if (next_frame != nullptr)
            change_frame_size(next_frame, band, half_screen);
        next_span = next_span->flink;
    }
    initial_margin_right = terminal_info.width;
    initial_margin_bottom = band;
    initial_margin_top = band;
    initial_scr_width = terminal_info.width;
    initial_scr_height = terminal_info.height;

    // now actually repaint the screen with the current frame
    // I'm not terribly happy with this - there might be a memory leak in
    // here with the screen_load without doing a screen_unload. But since
    // nobody saw fit to provide any comments describing what the hell
    // screen load/unload do, it can't be too much of a problem

    // with current_frame^, dot^ do
    screen_load(current_frame->dot->line);
    scr_needs_fix = false;
    screen_expand(true, true);
    vdu_movecurs(
        current_frame->dot->col - current_frame->scr_offset, current_frame->dot->line->scr_row_nr
    );
}

void screen_fixup() {
    // Make sure that the screen is user's view of the screen is correct.

#ifdef DEBUG
    if (ludwig_mode != ludwig_mode_type::ludwig_screen) {
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        return;
    }
#endif
    if (tt_winchanged) {
        screen_resize();
    } else {
        // with current_frame^,dot^ do
        if (scr_frame != current_frame) {
            if (scr_msg_row <= terminal_info.height)
                screen_clear_msgs(true);
            screen_load(current_frame->dot->line);
        } else {
            if ((current_frame->dot->line->scr_row_nr == 0) ||
                ((current_frame->dot->line->scr_row_nr - scr_top_line->scr_row_nr <
                  current_frame->margin_top) &&
                 (scr_top_line->blink != nullptr)) ||
                ((scr_bot_line->scr_row_nr - current_frame->dot->line->scr_row_nr <
                  current_frame->margin_bottom) &&
                 (scr_bot_line->flink != nullptr)) ||
                (current_frame->dot->col <= current_frame->scr_offset) ||
                (current_frame->dot->col > current_frame->scr_offset + current_frame->scr_width)) {
                if (scr_msg_row <= terminal_info.height)
                    screen_clear_msgs(true);
                screen_position(current_frame->dot->line, current_frame->dot->col);
            } else if (scr_msg_row <= terminal_info.height) {
                // Leave screen until key press if there are messages to read.
                vdu_movecurs(
                    current_frame->dot->col - current_frame->scr_offset,
                    current_frame->dot->line->scr_row_nr
                );
                key_code_range key = vdu_get_key();
                screen_clear_msgs(false);
                if (tt_controlc)
                    return;
                vdu_take_back_key(key);
            }
        }
        scr_needs_fix = false;
        screen_expand(true, true);
        vdu_movecurs(
            current_frame->dot->col - current_frame->scr_offset,
            current_frame->dot->line->scr_row_nr
        );
    }
}

void screen_getlinep(
    const std::string_view &prompt,
    str_object &outbuf,
    strlen_range &outlen,
    tpcount_type max_tp,
    tpcount_type this_tp
) {
    outlen = 0; // THIS IS DONE BECAUSE ?_GET_INPUT MAY TREAT OUTLEN
                // AS A WORD, NOT AS A LONGWORD.
    line_ptr tmp_line;
    max_tp = std::abs(max_tp); // this is negative with some file prompts
    if (!tt_controlc) {
        if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
            prompt_region[this_tp].line_nr = 0;
            prompt_region[this_tp].redraw = nullptr;

            if (scr_top_line == nullptr)
                goto l1;
            screen_fixup();
            prompt_region[this_tp].line_nr = this_tp + 1;
            if (scr_top_line->scr_row_nr > max_tp)
                goto l1;
            if (scr_bot_line->scr_row_nr < scr_msg_row - max_tp) {
                prompt_region[this_tp].line_nr = scr_msg_row - max_tp + this_tp;
                goto l1;
            }
            tmp_line = scr_top_line;
            for (int index = scr_top_line->scr_row_nr; index <= this_tp; ++index)
                tmp_line = tmp_line->flink;
            prompt_region[this_tp].redraw = tmp_line;
            if (scr_frame->dot->line->scr_row_nr > 2)
                goto l1;

            tmp_line = scr_bot_line;
            for (int index = terminal_info.height - scr_bot_line->scr_row_nr;
                 index < max_tp - this_tp - 1;
                 ++index)
                tmp_line = tmp_line->blink;
            if (terminal_info.height - scr_bot_line->scr_row_nr > max_tp - this_tp - 1)
                tmp_line = nullptr;
            prompt_region[this_tp].redraw = tmp_line;
            prompt_region[this_tp].line_nr = terminal_info.height - max_tp + this_tp + 1;

        l1:
            if (prompt_region[this_tp].line_nr != 0)
                vdu_movecurs(1, prompt_region[this_tp].line_nr);
            vdu_get_input(prompt, outbuf, MAX_STRLEN, outlen);
            if (tt_controlc)
                goto l2;
            if (outlen == 0) {
                for (int index = this_tp + 1; index < max_tp; ++index) {
                    prompt_region[index].line_nr = 0;
                    prompt_region[index].redraw = nullptr;
                }
            }
            if ((this_tp == max_tp - 1) || (outlen == 0)) {
                for (tpcount_type index = 0; index < max_tp; ++index) {
                    if (prompt_region[index].redraw != nullptr)
                        screen_draw_line(prompt_region[index].redraw);
                    else if (prompt_region[index].line_nr != 0) {
                        vdu_movecurs(1, prompt_region[index].line_nr);
                        vdu_cleareol();
                    }
                }
            }
            vdu_flush();
        } else {
            write(prompt);
            std::string input;
            std::getline(std::cin, input);
            outbuf.copy_n(input.data(), input.size());
            outlen = input.size();
        }
    }
l2:
    if (tt_controlc)
        outlen = 0;
}

void screen_free_bottom_line() {
    // This routine assumes that the editor is in SCREEN mode.
    // This routine frees the bottom line of the screen for use by the caller.
    // The main use of the area is the outputting of messages.
#ifdef DEBUG
    if (ludwig_mode != ludwig_mode_type::ludwig_screen) {
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        return;
    }
#endif
    if (scr_frame == nullptr) { // IF SCREEN NOT MAPPED.
        vdu_displaycrlf();
        vdu_deletelines(1);
        return;
    }
    scr_needs_fix = true;
    // IF BOTTOM LINE FREE.
    if ((scr_msg_row > terminal_info.height) && (scr_bot_line->scr_row_nr < terminal_info.height)) {
        // Nothing
    } else if (scr_bot_line->scr_row_nr + 2 < scr_msg_row) { // IF ROOM BELOW BOT LINE.
                                                             // +2 because of <eos> line.
        vdu_movecurs(1, scr_bot_line->scr_row_nr + 2);
        vdu_deletelines(1);
    } else if (scr_top_line->scr_row_nr != 1) { // IF TOP LINE FREE.
        screen_scroll(1, false);
    } else {
        // with scr_bot_line^ do
        if (scr_bot_line->scr_row_nr + 1 < scr_msg_row) { // IF ROOM FOR MORE MSGS.
            vdu_movecurs(1, scr_bot_line->scr_row_nr + 1);
            vdu_deletelines(1);
        } else if ((scr_frame->dot->line != scr_top_line) && // IF DOT NOT ON TOP LINE,
                   !((scr_frame->dot->line != scr_bot_line) &&
                     (scr_bot_line->scr_row_nr ==
                          terminal_info.height))) { // AND WE CANT USE THE BOT.
            screen_scroll(1, false);
        } else if (scr_msg_row <= terminal_info.height / 2) { // 1/2 SCREEN ALREADY MSGS.
            vdu_movecurs(1, scr_msg_row);
            vdu_deletelines(1);
            return;
        } else { // CONTRACT SCREEN 1 LINE.
            scr_bot_line->scr_row_nr = 0;
            scr_bot_line = scr_bot_line->blink;
            vdu_movecurs(1, scr_msg_row - 1);
            vdu_deletelines(1);
        }
    }
    scr_msg_row -= 1;
}

verify_response screen_verify(const std::string_view &prompt) {
    // Issue a verify request to the user ... the user is to be shown the
    // current dot position.

    const int ver_height = 4;

    // with current_frame^,dot^ do
    verify_response verify = verify_response::verify_reply_quit;

    scr_row_range old_height = current_frame->scr_height;
    scr_row_range old_top_m = current_frame->margin_top;
    scr_row_range old_bot_m = current_frame->margin_bottom;
    if (old_height > ver_height) {
        current_frame->margin_top = ver_height / 2;
        current_frame->scr_height = ver_height;
        current_frame->margin_bottom = ver_height / 2;
    }

    bool use_prompt = true;
    key_code_range key;
    bool more;
    do {
        switch (ludwig_mode) {
        case ludwig_mode_type::ludwig_screen:
            {
                screen_fixup();
                vdu_attr_bold();
                if (use_prompt) {
                    screen_message(prompt);
                } else {
                    screen_message(YNAQM_MSG);
                }
                vdu_attr_normal();
                vdu_movecurs(
                    current_frame->dot->col - current_frame->scr_offset,
                    current_frame->dot->line->scr_row_nr
                );
                key = vdu_get_key();
                if (LOWER_SET.test(key.value())) {
                    key = std::toupper(key.value());
                }
                if (key == 13)
                    key = 'N'; // RETURN <=> NO
                screen_clear_msgs(false);
            }
            break;

        case ludwig_mode_type::ludwig_batch:
        case ludwig_mode_type::ludwig_hardcopy:
            {
                str_object response;
                strlen_range resp_len;
                if (use_prompt) {
                    screen_getlinep(prompt, response, resp_len, 1, 1);
                } else {
                    screen_getlinep(YNAQM_MSG, response, resp_len, 1, 1);
                }
                if (resp_len == 0)
                    key = 'N';
                else
                    key = std::toupper(response[1]);
            }
            break;
        }
        if (tt_controlc)
            goto l99;
        more = false;
        if (YNAQM_CHARS.find(key.value()) != std::string::npos) {
            switch (key.value()) {
            case ' ': // default for yes on Tops 20;CJB
            case 'Y':
                verify = verify_response::verify_reply_yes;
                break;
            case 'N':
                verify = verify_response::verify_reply_no;
                break;
            case 'A':
                verify = verify_response::verify_reply_always;
                break;
            case 'Q':
                verify = verify_response::verify_reply_quit;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'M':
                { // MORE CONTEXT PLEASE!
                    // How much is user getting now?
                    if (scr_top_line != nullptr)
                        current_frame->scr_height =
                            scr_bot_line->scr_row_nr + 1 - scr_top_line->scr_row_nr;
                    // How much more does he want?
                    if (key == 'M')
                        key = '1';
                    if (key - '0' + current_frame->scr_height < terminal_info.height)
                        current_frame->scr_height += key - '0';
                    else
                        current_frame->scr_height = terminal_info.height;
                    if (scr_top_line == nullptr)
                        screen_load(current_frame->dot->line);
                    else
                        screen_expand(true, true);
                    more = true;
                    use_prompt = true;
                }
                break;
            }
        } else {
            screen_beep();
            more = true;
            use_prompt = false;
        }
    } while (more);

l99:

    current_frame->scr_height = old_height;
    current_frame->margin_top = old_top_m;
    current_frame->margin_bottom = old_bot_m;

    if (verify == verify_response::verify_reply_quit)
        exit_abort = true;
    return verify;
}

void screen_beep() {
    // Beep the terminal bell.
    switch (ludwig_mode) {
    case ludwig_mode_type::ludwig_screen:
        vdu_beep();
        break;
    case ludwig_mode_type::ludwig_hardcopy:
        write('\007', 1);
        break;
    case ludwig_mode_type::ludwig_batch:
        writeln("LUDWIG RINGS TERMINAL BELL <beep>!");
        break;
    }
}

void screen_home(bool clear) {
    // If screen editing home the cursor, otherwise do a 'nice' equivalent.
    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        if (clear) {
            vdu_clearscr();
            scr_msg_row = terminal_info.height + 1;
        } else
            vdu_movecurs(1, 1);
    } else {
        writeln("");
        writeln("");
    }
}

void screen_write_int(int intv, scr_col_range width) {
    // Write an integer at the current cursor position, or to the output file.

    std::string s = std::to_string(intv);
    if (width <= 0)
        return;
    if (s.size() < static_cast<size_t>(width)) {
        std::string pad(width - s.size(), ' ');
        s = pad + s;
    }
    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (char ch : s)
            vdu_displaych(ch);
    } else {
        write(s.data(), s.size());
    }
}

void screen_write_ch(scr_col_range indent, char ch) {
    // Write a character at the current cursor position, or to the output file.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (int i = 0; i < indent; ++i)
            vdu_displaych(' ');
        vdu_displaych(ch);
    } else {
        write(' ', indent);
        write(ch, 1);
    }
}

void screen_write_str(scr_col_range indent, const std::string_view &str) {
    // Write a string at the current cursor position, or to the output file.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (int i = 0; i < indent; ++i)
            vdu_displaych(' ');
        for (size_t i = 0; i < str.size(); ++i)
            vdu_displaych(str[i]);
    } else {
        write(' ', indent);
        write(str);
    }
}

void screen_write_str(scr_col_range indent, const char *str, scr_col_range width) {
    // Write a string at the current cursor position, or to the output file.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (int i = 0; i < indent; ++i)
            vdu_displaych(' ');
        for (int i = 0; i < width; ++i)
            vdu_displaych(str[i]);
    } else {
        write(' ', indent);
        write(str, width);
    }
}

void screen_write_str(scr_col_range indent, const char *str) {
    screen_write_str(indent, str, std::strlen(str));
}

void screen_write_name_str(scr_col_range indent, const std::string &str, scr_col_range width) {
    // Write a name string at the current cursor position, or to the output file.
    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (int i = 0; i < indent; ++i)
            vdu_displaych(' ');
        for (int i = 0; i < width; ++i) {
            // FIXME: Cast should be removed here and prange fixed to use
            // signed or unsigned int as appropriate for the range
            if (std::string::size_type(i) < str.size())
                vdu_displaych(str[i]);
            else
                vdu_displaych(' ');
        }
    } else {
        write(' ', indent);
        for (int i = 0; i < width; ++i) {
            // FIXME: Cast should be removed here and prange fixed to use
            // signed or unsigned int as appropriate for the range
            if (std::string::size_type(i) < str.size())
                write(str[i], 1);
            else
                write(' ', 1);
        }
    }
}

void screen_write_file_name_str(scr_col_range indent, const file_name_str &str, size_t width) {
    // Write a file name at the current cursor position, or to the output file.
    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        for (int i = 0; i < indent; ++i)
            vdu_displaych(' ');
        for (size_t i = 0; i < width; ++i) {
            if (i < str.size() && std::isprint(str[i]))
                vdu_displaych(str[i]);
            else
                vdu_displaych(' ');
        }
    } else {
        write(' ', indent);
        for (size_t i = 0; i < width; ++i) {
            if (i < str.size())
                write(str[i], 1);
            else
                write(' ', 1);
        }
    }
}

void screen_writeln() {
    // Write a CRLF to the output file.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
        vdu_displaycrlf();
    else
        writeln("");
}

void screen_writeln_clel() {
    // Write a CLEAR_EOL, CRLF to the output file.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        vdu_cleareol();
        vdu_displaycrlf();
    } else
        writeln("");
}

std::string screen_help_prompt(const std::string_view &prompt) {
    std::string reply;
    switch (ludwig_mode) {
    case ludwig_mode_type::ludwig_screen:
    case ludwig_mode_type::ludwig_hardcopy:
        {
            if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                vdu_attr_bold();
            screen_write_str(0, prompt.data(), prompt.size());
            if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                vdu_attr_normal();
            bool terminated = false;
            do {
                key_code_range key = vdu_get_key();
                if (key == 13)
                    terminated = true;
                else if (key == 127) {
                    if (!reply.empty()) {
                        reply.pop_back();
                        vdu_displaych(char(8));
                        vdu_displaych(' ');
                        vdu_displaych(char(8));
                    }
                } else if (PRINTABLE_SET.test(int(key))) {
                    vdu_displaych(char(key));
                    reply.push_back(char(key));
                    terminated = (key == ' ') || (reply.size() == KEY_LEN);
                }
            } while (!terminated);
            screen_writeln();
        }
        break;

    case ludwig_mode_type::ludwig_batch:
        reply.clear();
        break;
    }
    return reply;
}
