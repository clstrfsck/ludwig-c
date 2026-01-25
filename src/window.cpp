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
! Name:         WINDOW
!
! Description:  Implement the window commands.
!**/

#include "window.h"

#include "var.h"
#include "vdu.h"
#include "line.h"
#include "mark.h"
#include "frame.h"
#include "screen.h"

bool window_command(commands command, leadparam rept, int count, bool from_span) {
    bool cmd_success = false;

    //with current_frame^,dot^ do
    line_range line_nr;
    line_range line2_nr;
    line_range line3_nr;
    switch (command) {
    case commands::cmd_window_backward:
        if (!line_to_number(current_frame->dot->line, line_nr))
            goto l99;
        if (line_nr <= current_frame->scr_height * count) {
            mark_create(current_frame->first_group->first_line, current_frame->dot->col, current_frame->dot);
        } else {
            line_ptr new_line = current_frame->dot->line;
            for (int i = 1; i <= current_frame->scr_height * count; ++i)
                new_line = new_line->blink;
            if (count == 1) {
                //with line^ do
                const_line_ptr line = current_frame->dot->line;
                if (line->scr_row_nr != 0) {
                    if (line->scr_row_nr > current_frame->scr_height - current_frame->margin_bottom) {
                        screen_scroll(-2 * current_frame->scr_height + line->scr_row_nr +
                                      current_frame->margin_bottom, true);
                    } else {
                        screen_scroll(-current_frame->scr_height, true);
                    }
                }
            } else {
                screen_unload();
            }
            mark_create(new_line, current_frame->dot->col, current_frame->dot);
        }
        cmd_success = true;
        break;

    case commands::cmd_window_end:
        cmd_success = mark_create(current_frame->last_group->last_line,
                                  current_frame->dot->col, current_frame->dot);
        break;

    case commands::cmd_window_forward: {
        if (!line_to_number(current_frame->dot->line, line_nr))
            goto l99;
        //with last_group^ do
        group_ptr &last_group(current_frame->last_group);
        mark_ptr  &dot       (current_frame->dot);
        if (line_nr + current_frame->scr_height * count >
            last_group->first_line_nr + last_group->last_line->offset_nr) {
            mark_create(last_group->last_line, dot->col, dot);
        } else {
            line_ptr new_line = dot->line;
            for (int i = 1; i <= current_frame->scr_height * count; ++i)
                new_line = new_line->flink;
            if (count == 1) {
                //with line^ do
                const_line_ptr line = dot->line;
                if (line->scr_row_nr != 0) {
                    if (line->scr_row_nr <= current_frame->margin_top) {
                        screen_scroll(current_frame->scr_height + line->scr_row_nr -
                                      current_frame->margin_top - 1, true);
                    } else {
                        screen_scroll(current_frame->scr_height, true);
                    }
                }
            } else {
                screen_unload();
            }
            mark_create(new_line, dot->col, current_frame->dot);
        }
        cmd_success = true;
    }
        break;

    case commands::cmd_window_left:
        cmd_success = true;
        if (scr_frame == current_frame) {
            if (rept == leadparam::none)
                count = current_frame->scr_width / 2;
            if (current_frame->scr_offset < count)
                count = current_frame->scr_offset;
            screen_slide(-count);
            if (current_frame->scr_offset + current_frame->scr_width < current_frame->dot->col)
                current_frame->dot->col = current_frame->scr_offset + current_frame->scr_width;
        }
        break;

    case commands::cmd_window_middle:
        cmd_success = true;
        if (scr_frame == current_frame) {
            if (line_to_number(current_frame->dot->line, line_nr) &&
                line_to_number(scr_top_line, line2_nr) &&
                line_to_number(scr_bot_line, line3_nr)) {
                screen_scroll(line_nr - ((line2_nr + line3_nr) / 2), true);
            }
        }
        break;

    case commands::cmd_window_new:
        cmd_success = true;
        screen_redraw();
        break;

    case commands::cmd_window_right:
        cmd_success = true;
        if (scr_frame == current_frame) {
            if (rept == leadparam::none)
                count = current_frame->scr_width / 2;
            if (MAX_STRLENP < (current_frame->scr_offset + current_frame->scr_width) + count)
                count = MAX_STRLENP - (current_frame->scr_offset + current_frame->scr_width);
            screen_slide(count);
            if (current_frame->dot->col <= current_frame->scr_offset)
                current_frame->dot->col = current_frame->scr_offset + 1;
        }
        break;

    case commands::cmd_window_scroll:
        cmd_success = true;
        if (current_frame == scr_frame) {
            key_code_range key;
            do {
                if (rept == leadparam::pindef) {
                    count = current_frame->dot->line->scr_row_nr - 1;
                    if (count < 0)
                        count = 0;
                } else if (rept == leadparam::nindef) {
                    count = current_frame->dot->line->scr_row_nr - current_frame->scr_height;
                }
                if (rept != leadparam::none)
                    screen_scroll(count, true);
                key = 0;

                // If the dot is still visible and the command is interactive
                // then support stay-behind mode.
                if  (!from_span &&
                     (current_frame->dot->line->scr_row_nr != 0) &&
                     (current_frame->scr_offset < current_frame->dot->col) &&
                     (current_frame->dot->col <= current_frame->scr_offset + current_frame->scr_width)) {
                    if (!cmd_success) {
                        vdu_beep();
                        cmd_success = true;
                    }
                    vdu_movecurs(current_frame->dot->col - current_frame->scr_offset,
                                 current_frame->dot->line->scr_row_nr);
                    key = vdu_get_key();
                    if (tt_controlc) {
                        key = 0;
                    } else if (lookup[key].command == commands::cmd_up) {
                        rept  = leadparam::pint;
                        count = 1;
                    } else if (lookup[key].command == commands::cmd_down) {
                        rept  = leadparam::nint;
                        count = -1;
                    } else {
                        vdu_take_back_key(key);
                        key = 0;
                    }
                }
            } while (key != 0);
        }
        break;

    case commands::cmd_window_setheight:
        if (rept == leadparam::none)
            count = terminal_info.height;
        cmd_success = frame_setheight(count, false);
        break;

    case commands::cmd_window_top:
        cmd_success = mark_create(current_frame->first_group->first_line,
                                  current_frame->dot->col, current_frame->dot);
        break;

    case commands::cmd_window_update:
        cmd_success = true;
        if (ludwig_mode == ludwig_mode_type::ludwig_screen)
            screen_fixup();
        break;

    default:
        // All other commands ignored
        // FIXME: Raise error?
        break;
    }

 l99:;
    return cmd_success;
}
