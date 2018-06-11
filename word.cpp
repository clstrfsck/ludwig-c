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
! Name:         WORD
!
! Description:  Word Processing Commands in Ludwig.
*/

#include "word.h"

#include "var.h"
#include "line.h"
#include "mark.h"
#include "text.h"
#include "screen.h"

bool word_fill(leadparam rept, int count, bool from_span) {
    /* Description:
       This routine takes the current line and moves words within
       the line so that the line fits between Left_margin & right_margin
       If necessary, it will grab stuff from the next line.
       nYF => Upset (at most) n lines after this one
       It preserves multiple spaces.
    */

    col_range start_char;
    col_range end_char;
    col_range old_end;
    int       line_count;
    int       space_to_add;
    line_ptr  this_line;
    mark_ptr  here;
    mark_ptr  there;
    mark_ptr  old_here;
    mark_ptr  old_there;
    bool      leave_dot_alone;

    bool result = false;
    leave_dot_alone = false;
    here = nullptr;
    there = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if (rept == leadparam::none) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count -= 1;
            if (this_line == nullptr)
                goto l99;
        }
        if (line_count != 0)
            goto l99;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^,line^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l99;
        // adjust the current line to the margins
        if (current_frame->dot->line->blink != nullptr) {
            if (current_frame->dot->line->blink->used != 0) {
                // this is not the first line of a paragraph so adjust to left margin
                start_char = 1;
                while (((*current_frame->dot->line->str)[start_char] == ' ') &&
                       (start_char < current_frame->dot->line->used))
                    start_char += 1;
                if ((start_char < current_frame->margin_left) && (start_char < current_frame->dot->line->used)) {
                    if (!mark_create(current_frame->dot->line, start_char, here))
                        goto l99;
                    if (!text_insert(true, 1, BLANK_STRING,
                                     current_frame->margin_left - start_char, here))
                        goto l99;
                    mark_destroy(here);
                } else {
                    // we might have to remove some spaces
                    start_char = current_frame->margin_left;
                    end_char = current_frame->margin_left;
                    if (end_char < current_frame->dot->line->used) {
                        while (((*current_frame->dot->line->str)[end_char] == ' ') &&
                               (end_char < current_frame->dot->line->used))
                            end_char += 1;
                        if (end_char > 1) {
                            if (!mark_create(current_frame->dot->line, start_char.value(), here))
                                goto l99;
                            if (!mark_create(current_frame->dot->line, end_char, there))
                                goto l99;
                            if (!text_remove(here, there))
                                goto l99;
                            mark_destroy(here);
                            mark_destroy(there);
                        }
                    }
                }
            }
        }
        if (current_frame->dot->line->used > current_frame->margin_right) {
            // We must split this line if possible
            // 1. Scan back for first non-blank
            end_char = current_frame->margin_right + 1;
            if ((*current_frame->dot->line->str)[end_char] != ' ') {
                while (((*current_frame->dot->line->str)[end_char] != ' ') &&
                       (end_char > current_frame->margin_left))
                    end_char -= 1;
                if (end_char == current_frame->margin_left)
                    goto l99;
            }
            start_char = end_char;
            while (((*current_frame->dot->line->str)[end_char] == ' ') &&
                   (end_char > current_frame->margin_left))
                end_char -= 1;
            if (end_char == current_frame->margin_left)
                goto l99; // Nothing to do
            // 2. Scan forward for first non-blank
            while ((*current_frame->dot->line->str)[start_char] == ' ')
                start_char += 1;
            // 3. Split the line at end_char and make sure the new line
            //    starts with start_char at margin_left
            //    Make sure the DOT stays on THIS LINE
            if (! mark_create(current_frame->dot->line, start_char, here))
                goto l99;
            if (current_frame->dot->col > end_char)
                if (!mark_create(current_frame->dot->line, end_char, current_frame->dot))
                    goto l99;
            if (!text_split_line(here, current_frame->margin_left, there))
                goto l99;
            if (!mark_destroy(here))
                goto l99;
            if (!mark_destroy(there))
                goto l99;
            if (rept != leadparam::pindef)
                count += 1;
        } else {
            // need to get stuff from the next line
    l3:;
            // 1. Figure out how many chars we can fit in
            space_to_add = current_frame->margin_right - current_frame->dot->line->used - 1; // Allow for 1 space
            // 2. See if we can find a word to fit
            start_char = 1;
            if ((space_to_add > 0) && (current_frame->dot->line->flink->used != 0)) {
                while ((*current_frame->dot->line->flink->str)[start_char] == ' ')
                    start_char += 1;
                end_char = start_char;
                old_end  = end_char;
                while (end_char <= current_frame->dot->line->flink->used) {
                    while ((*current_frame->dot->line->flink->str)[end_char] == ' ')
                        end_char += 1;
                    while (((*current_frame->dot->line->flink->str)[end_char] != ' ') &&
                           (end_char < current_frame->dot->line->flink->used))
                        end_char += 1;
                    if (end_char == current_frame->dot->line->flink->used)
                        end_char += 1;
                    if (space_to_add < (end_char - start_char))
                        end_char = current_frame->dot->line->flink->used + 1;
                    else
                        old_end = end_char;
                }
                if (((old_end - start_char) <= space_to_add) && (old_end != start_char)) {
                    // Hooray !! It will fit
                    old_here  = nullptr;
                    old_there = nullptr;
                    if (!mark_create(current_frame->dot->line->flink, start_char, here))
                        goto l99;
                    if (!mark_create(current_frame->dot->line->flink, start_char, old_here))
                        goto l99;
                    if (!mark_create(current_frame->dot->line->flink, old_end, there))
                        goto l99;
                    if (!mark_create(current_frame->dot->line->flink, old_end, old_there))
                        goto l99;
                    current_frame->dot->col = current_frame->dot->line->used + 2; // Allow for a space !
                    // Copy the text
                    if (!text_move(true, 1, here, there, current_frame->dot, here, there))
                        goto l99;
                    // Copy the marks
                    if (!marks_shift(current_frame->dot->line->flink, old_here->col,
                                     old_there->col - old_here->col,
                                     here->line, here->col))
                        goto l99;
                    // Now wipe out the old Grungy text
                    // First though, put the old_here and there marks back!
                    if (!mark_create(current_frame->dot->line->flink, 1, old_here))
                        goto l99;
                    if (!mark_create(current_frame->dot->line->flink, old_end, old_there))
                        goto l99;
                    if (!text_remove(old_here, old_there))
                        goto l99;
                    mark_destroy(old_here);
                    mark_destroy(old_there);
                    // Now clean up the next line
                    // If it is now empty, delete it
                    //with dot->line^ do
                    if (current_frame->dot->line->flink->used == 0) {
                        this_line = current_frame->dot->line->flink;
                        if (!marks_squeeze(current_frame->dot->line->flink, 1, current_frame->dot->line->flink->flink, 1))
                            goto l99;
                        if (!lines_extract(this_line, this_line))
                            goto l99;
                        if (!lines_destroy(this_line, this_line))
                            goto l99;
                        count -= 1;
                        if (count > 0)
                            goto l3; // Go back and have another go
                        // Put DOT here, so we can have another go if we want
                        leave_dot_alone = true;
                    }
                }
                // Now make sure the first char in this line is at LM
                //with dot->line^ do
                if ((count > 0) && (current_frame->dot->line->flink->used != 0)) {
                    start_char = 1;
                    while ((*current_frame->dot->line->flink->str)[start_char] == ' ')
                        start_char += 1;
                    if (!mark_create(current_frame->dot->line->flink, start_char, there))
                        goto l99;
                    if (start_char < current_frame->margin_left) {
                        // Must insert some chars here
                        if (!text_insert(true, 1, BLANK_STRING, current_frame->margin_left - start_char, there))
                            goto l99;
                    } else {
                        if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, here))
                            goto l99;
                        if (!text_remove(here, there))
                            goto l99;
                    }
                }
                if (here != nullptr)
                    if (!mark_destroy(here))
                        goto l99;
                if (there != nullptr)
                    if (!mark_destroy(there))
                        goto l99;
            }
        }
        count -= 1;
        if (!leave_dot_alone) {
            if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot))
                goto l99;
        }
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l99;
    }
    result = (count <= 0) || (rept == leadparam::pindef);
l99:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    if (there != nullptr)
        mark_destroy(there);
    return result;
}

bool word_centre(leadparam rept, int count, bool from_span) {
    /* Description:
       This routine takes the current line and moves it so that
       there are an equal number of spaces before and after the
       line, with respect to the left_margin & right_margin
       It assumes that the text already lies between Lm&Rm.
    */
    strlen_range start_char;
    int          line_count;
    int          space_to_add;
    line_ptr     this_line;
    mark_ptr     here;
    mark_ptr     there;

    bool result = false;
    here = nullptr;
    there = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if ((rept == leadparam::none) || (rept == leadparam::plus)) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count -= 1;
            if (this_line == nullptr)
                goto l2;
        }
        if (line_count != 0)
            goto l2;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l2;
        if ((current_frame->dot->line->used < current_frame->margin_left) ||
            (current_frame->dot->line->used > current_frame->margin_right))
            goto l2;
        start_char = 1;
        while ((*current_frame->dot->line->str)[start_char] == ' ')
            start_char += 1;
        if (start_char < current_frame->margin_left)
            goto l2;
        space_to_add = (current_frame->margin_right - current_frame->margin_left -
                        (current_frame->dot->line->used - start_char)) / 2 - (start_char - current_frame->margin_left);
        if (space_to_add != 0) {
            here = nullptr;
            result = mark_create(current_frame->dot->line, current_frame->margin_left, here);
            if (space_to_add > 0) {
                result = text_insert(true, 1, BLANK_STRING, space_to_add, here);
            } else {
                there = nullptr;
                result = mark_create(current_frame->dot->line, current_frame->margin_left - space_to_add, there);
                result = text_remove(here, there);
                result = mark_destroy(there);
            }
            result = mark_destroy(here);
        }
        count -= 1;
        result = mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot);
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l2;
    }
    result = (count == 0) || (rept == leadparam::pindef);
l2:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    if (there != nullptr)
        mark_destroy(there);
    return result;
}

bool word_justify(leadparam rept, int count, bool from_span) {
    /* Description:
       This routine takes the current line space justifies it
       between the left and right margins. It preserves multiple spaces.
    */

    col_range start_char;
    col_range end_char;
    int       holes;
    int       i;
    int       line_count;
    int       space_to_add;
    line_ptr  this_line;
    mark_ptr  here;
    double    fill_ratio;
    double    debit;

    bool result = false;
    here = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if ((rept == leadparam::none) || (rept == leadparam::plus)) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count -= 1;
            if (this_line == nullptr)
                goto l2;
        }
        if (line_count != 0)
            goto l2;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^,line^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l2;
        if (current_frame->dot->line->flink->used == 0)
            goto l1;
        if (current_frame->dot->line->used > current_frame->margin_right)
            goto l2; // Line is too long to justify

        // 1. Figure out how many spaces to add
        space_to_add = current_frame->margin_right - current_frame->dot->line->used;
        // 2. Find number of holes into which spaces are to be distributed
        start_char = current_frame->margin_left;
        while (((*current_frame->dot->line->str)[start_char] == ' ') && (start_char < current_frame->dot->line->used))
            start_char += 1;
        end_char   = start_char;  // Remember starting position
        holes = 0;
        do {
            while (((*current_frame->dot->line->str)[start_char] != ' ') && (start_char < current_frame->dot->line->used))
                start_char += 1;
            while (((*current_frame->dot->line->str)[start_char] == ' ') && (start_char < current_frame->dot->line->used))
                start_char += 1;
            holes += 1;
        } while (start_char < current_frame->dot->line->used);
        holes -= 1;
        if (holes > 0)
            fill_ratio = 1.0 * space_to_add / holes;
        debit = 0.0;
        start_char = end_char;
        for (i = 1; i <= holes; ++i) {
            // Find a hole
            while ((*current_frame->dot->line->str)[start_char] != ' ')
                start_char += 1;
            debit += fill_ratio;
            space_to_add = static_cast<int>(debit + 0.5);
            if (space_to_add > 0) {
                here = nullptr;
                if (!mark_create(current_frame->dot->line, start_char, here))
                    goto l2;
                if (!text_insert(true, 1, BLANK_STRING, space_to_add, here))
                    goto l2;
                if (!mark_destroy(here))
                    goto l2;
                debit -= space_to_add;
            }
            while ((*current_frame->dot->line->str)[start_char] == ' ')
                start_char += 1;
        }
l1:;
        count -= 1;
        if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot))
            goto l2;
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l2;
    }
    result = (count <= 0) || (rept == leadparam::pindef);
l2:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    return result;
}

bool word_squeeze(leadparam rept, int count, bool from_span) {
    /* Description:
       This routine takes the current line and removes multiple
       spaces from it.
    */

    col_range start_char;
    col_range end_char;
    int       line_count;
    line_ptr  this_line;
    mark_ptr  here;
    mark_ptr  there;

    bool result = false;
    here = nullptr;
    there = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if ((rept == leadparam::none) || (rept == leadparam::plus)) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count -= 1;
            if (this_line == nullptr)
                goto l2;
        }
        if (line_count != 0)
            goto l2;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l2;
        start_char = 1;
        while ((*current_frame->dot->line->str)[start_char] == ' ')
            start_char += 1;
        //with line^ do
        do {
            while (((*current_frame->dot->line->str)[start_char] != ' ') && (start_char < current_frame->dot->line->used))
                start_char += 1;
            if ((*current_frame->dot->line->str)[start_char] != ' ')
                goto l1; // Nothing more to do
            end_char = start_char;
            while ((*current_frame->dot->line->str)[end_char] == ' ')
                end_char += 1;
            if ((end_char - start_char) > 1) {
                here         = nullptr;
                if (!mark_create(current_frame->dot->line, start_char, here))
                    goto l2;
                there        = nullptr;
                if (!mark_create(current_frame->dot->line, end_char - 1, there))
                    goto l2;
                if (!text_remove(here, there))
                    goto l2;
                start_char   = here->col;
            } else {
                start_char = end_char;
            }
        } while (true);
l1:;
      count -= 1;
      if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot))
          goto l2;
      current_frame->text_modified = true;
      if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
          goto l2;
    }
    result = (count = 0) || (rept == leadparam::pindef);
l2:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    if (there != nullptr)
        mark_destroy(there);
    return result;
}

bool word_right(leadparam rept, int count, bool from_span) {
    /*
      Description:
      This routine takes the current line and aligns it at RM
    */

    int      line_count;
    int      space_to_add;
    mark_ptr here;
    line_ptr this_line;

    bool result = false;
    here = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if ((rept == leadparam::none) || (rept == leadparam::plus)) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count = line_count - 1;
            if (this_line == nullptr)
                goto l2;
        }
        if (line_count != 0)
            goto l2;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l2;
        if (current_frame->dot->line->used < current_frame->margin_right) {
            space_to_add = current_frame->margin_right - current_frame->dot->line->used;
            here = nullptr;
            if (!mark_create(current_frame->dot->line, 1, here))
                goto l2;
            if (!text_insert(true, 1, BLANK_STRING, space_to_add, here))
                goto l2;
            if (!mark_destroy(here))
                goto l2;
        } else {
            if (current_frame->dot->line->used != current_frame->margin_right)
                goto l2;
        }
        count -= 1;
        if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot))
            goto l2;
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l2;
    }
    result = (count == 0) || (rept == leadparam::pindef);
l2:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    return result;
}

bool word_left(leadparam rept, int count, bool from_span) {
    /* Description:
       This routine takes the current line and Aligns it at LM
    */

    int      line_count;
    int      line_start;
    mark_ptr here;
    mark_ptr there;
    line_ptr this_line;

    bool result = false;
    here = nullptr;
    there = nullptr;
    if (rept == leadparam::pindef)
        count = MAXINT;
    if ((rept == leadparam::none) || (rept == leadparam::plus)) {
        count = 1;
        rept = leadparam::pint;
    }
    if (rept == leadparam::pint) {
        line_count = count;
        this_line  = current_frame->dot->line;
        while ((line_count > 0) && (this_line->used > 0)) {
            this_line = this_line->flink;
            line_count -= 1;
            if (this_line == nullptr)
                goto l2;
        }
        if (line_count != 0)
            goto l2;
    }
    while ((count > 0) && (current_frame->dot->line->used > 0)) {
        //with current_frame^,dot^ do
        if (current_frame->dot->line->flink == nullptr) // on EOP line so abort
            goto l2;
        if (current_frame->dot->line->used > current_frame->margin_left) {
            line_start = 1;
            while ((*current_frame->dot->line->str)[line_start] == ' ')
                line_start += 1;
            if (line_start < current_frame->margin_left)
                goto l2;
            here = nullptr;
            there = nullptr;
            if (!mark_create(current_frame->dot->line, current_frame->margin_left, here))
                goto l2;
            if (!mark_create(current_frame->dot->line, line_start, there))
                goto l2;
            if (!text_remove(here, there))
                goto l2;
            if (!mark_destroy(here))
                goto l2;
            if (!mark_destroy(there))
                goto l2;
        } else {
            goto l2;
        }
        count -= 1;
        if (!mark_create(current_frame->dot->line->flink, current_frame->margin_left, current_frame->dot))
            goto l2;
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l2;
    }
    result = (count == 0) || (rept == leadparam::pindef);
l2:;
    // Clean up those temporary marks if necessary
    if (here != nullptr)
        mark_destroy(here);
    if (there != nullptr)
        mark_destroy(there);
    return result;
}

bool word_advance_word(leadparam rept, int count, bool from_span) {
    /*
      ! This routine advances Dot to the start of the a word.
      ! Definitions:
      !   A word is a sequence of non-spaces followed by spaces.
      !   A paragraph is a sequence of words separated by one or more blank
      !   lines and is terminated by the first word of the next paragraph,
      !   or End-of-File.
      ! The semantics of the leading parameter is to advance to -
      !   0  the start of the current word.
      !   +n the start of the n'th next word.
      !   -n the start of the n'th previous word.
      !   >  the start of the first word in the next paragraph,
      !      ie. the start of the next word after the current paragraph.
      !   <  the start of the first word in the current paragraph.
    */
    line_ptr  this_line;
    col_range pos;

    bool result = false;
    if (rept == leadparam::marker) {
        screen_message(MSG_SYNTAX_ERROR);
        goto l99;
    }
    //with current_frame^ do
    pos = current_frame->dot->col;
    this_line = current_frame->dot->line;
    if (rept == leadparam::none || rept == leadparam::plus || rept == leadparam::pindef ||
        ((rept == leadparam::pint) && (count != 0))) {
        // Take care of the PINDEF case
        if (rept == leadparam::pindef) {
            // Get to the blank line between the current para. and the next
            while ((this_line->used != 0) && (this_line->flink != nullptr))
                this_line = this_line->flink;
            pos = 1;
            count = 1;
        }
        while (count > 0) {
            // Move Forwards
            // Locate the next Whitespace position
            do {
                if (pos < this_line->used) {
                    if ((*this_line->str)[pos] != ' ')
                        pos += 1;
                } else {
                    goto l1;
                }
            } while ((*this_line->str)[pos] != ' ');
    l1:;
            // Now Skip the Whitespace till we get to a Non-Space Char
            if (pos >= this_line->used) {
                // Must move onto next line
                pos = 1;
                // Go get the next line with something on it!
                do {
                    if (this_line->flink == nullptr) {
                        if (rept == leadparam::pindef)
                            goto l2; // This is close enough!
                        else
                            goto l99; // Not enough words left, FAIL!
                    }
                    this_line = this_line->flink;
                } while (this_line->used <= 0);
            }
            while ((*this_line->str)[pos] == ' ')
                pos += 1;
            // OK. We have got the start of a word.
            count -= 1;
        }
        // Fine, now move dot
l2:;
        if (!mark_create(this_line, pos, current_frame->dot))
            goto l99;
        result = true;
    } else if (rept == leadparam::nindef) {
        // Find a non blank line in this paragraph
        while ((this_line->used == 0) && (this_line->blink != nullptr))
            this_line = this_line->blink;
        // Find the blank line separating this para. from the one before
        while ((this_line->used != 0) && (this_line->blink != nullptr))
            this_line = this_line->blink;
        //Now it's time to find the first non blank}
        pos = 1;
        while (this_line->used == 0) {
            if (this_line->flink == nullptr)
                goto l99; // BAIL OUT!
            this_line = this_line->flink;
        }
        while ((*this_line->str)[pos] == ' ')
            pos += 1;
        if (!mark_create(this_line, pos, current_frame->dot))
            goto l99;
        result = true;
    } else {
        // Move Backwards
        /*
          ! OK lets now move to the start of the word to the left of where we
          ! are currently positioned, and then count off from there.
        */
        count = - count;
        if (pos > this_line->used)
            pos = this_line->used;
        do {
            /*
              ! if we are at the start of the line or on the eop-line go back to
              ! a previous line with characters on it.
            */
            if ((pos == 0) || (this_line->flink == nullptr)) {
                do {
                    if (this_line->blink == nullptr)
                        goto l99; // BAIL OUT!
                    this_line = this_line->blink;
                    pos = this_line->used;
                } while (pos <= 0);
            }
            // Skip Over any WhiteSpace
            while (((*this_line->str)[pos] == ' ') && (pos > 1))
                pos -= 1;
            if ((pos == 1) && ((*this_line->str)[1] == ' ')) {
                do {
                    if (this_line->blink == nullptr)
                        goto l99; // BAIL OUT!
                    this_line = this_line->blink;
                    pos = this_line->used;
                } while (pos <= 0);
            }
            /*
              ! ASSERTION: pos now points at a non-space character
              ! Now find the start of the word by skipping over non-whitespace
            */
            while (((*this_line->str)[pos] != ' ') && (pos > 1))
                pos -= 1;
            count -= 1;
            if (count < 0) {
                if ((*this_line->str)[pos] == ' ')
                    pos += 1; // Put Pos back onto the 1st char of the word
            } else {
                pos -= 1; // Setup to look for previous word
            }
        } while (count >= 0);
        // Fine, now move dot
        if (!mark_create(this_line, pos, current_frame->dot))
            goto l99;
        result = true;
    }
l99:;
    return result;
}

bool word_delete_word(leadparam rept, int count, bool from_span) {
    // Delete Word deletes the same words as advance word advances over.

    mark_ptr   old_pos;
    mark_ptr   here;
    mark_ptr   another_mark;
    mark_ptr   the_other_mark;
    line_range old_dot_col;
    line_range line_nr;
    line_range new_line_nr;

    bool result = false;
    old_pos = nullptr;
    here = nullptr;
    the_other_mark = nullptr;
    if (rept == leadparam::marker) {
        screen_message(MSG_SYNTAX_ERROR);
        goto l99;
    }
    //  with current_frame^ do
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, old_pos))
        goto l99;
    // First Step.
    //  Get to the beginning of the word if we are in the middle of it
    if (!word_advance_word(leadparam::pint, 0, from_span))
        goto l99;
    // ASSERTION: We are on the beginning of a word
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, here))
        goto l99;
    if (!word_advance_word(rept, count, from_span)) {
        // Put Dot back and bail out
        if (!mark_create(old_pos->line, old_pos->col, current_frame->dot))
            goto l99;
        goto l99;
    }
    // OK. We now wipe out everything from Dot to here
    old_dot_col = current_frame->dot->col;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, the_other_mark))
        goto l99;
    if (!line_to_number(current_frame->dot->line, line_nr))
        goto l99;
    if (!line_to_number(here->line, new_line_nr))
        goto l99;
    if ((line_nr > new_line_nr) || ((line_nr == new_line_nr) && (current_frame->dot->col > here->col))) {
        // Reverse mark pointers to get The_Other_Mark first.
        another_mark   = here;
        here       = the_other_mark;
        the_other_mark = another_mark;
    }
    if (current_frame != frame_oops) {
        //with frame_oops^ do
        // Make sure oops_span is okay.
        if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->span->mark_two))
            goto l99;
        result = text_move(false            // Dont copy,transfer
                           ,1               // One instance of
                           ,the_other_mark  // starting pos.
                           ,here            // ending pos.
                           ,frame_oops->span->mark_two // destination.
                           ,frame_oops->marks[0] // leave at start.
                           ,frame_oops->dot // leave at end.
            );
    } else {
        result = text_remove(the_other_mark   // starting pos.
                             ,here            // ending pos.
            );
    }
    if (line_nr != new_line_nr)
        result = text_split_line(current_frame->dot, old_dot_col, here);
l99:;
    // If these marks are still hanging around then zap 'em!
    if (old_pos != nullptr)
        mark_destroy(old_pos);
    if (here != nullptr)
        mark_destroy(here);
    return result;
}
