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
! Name:         NEWWORD
!
! Description:  Word Processing Commands for the new Ludwig command set.
!**/

#include "newword.h"

#include "line.h"
#include "mark.h"
#include "text.h"
#include "var.h"

bool current_word(mark_ptr dot) {
    // with dot^ do
    if (dot->line->used + 2 < dot->col) {
        // check that we aren't past the last word in the para
        if (dot->line->flink == nullptr) // no more lines => end of para
            return false;
        if (dot->line->flink->used == 0) // next line blank => end of para
            return false;
        // In the middle of a paragraph so go to end of line
        dot->col = dot->line->used;
    } else if (dot->line->used < dot->col)
        dot->col = dot->line->used;
    // But were we in the blank line before a paragraph?
    if (dot->col == 0)
        return false;
    while ((dot->col > 1) && word_elements[0].test(dot->line->str->operator[](dot->col)))
        dot->col -= 1;
    if (word_elements[0].test(dot->line->str->operator[](dot->col))) {
        // we must have been somewhere on the line before the first word
        if (dot->line->blink == nullptr) // oops top of the frame reached
            return false;
        if (dot->line->blink->used == 0) // inside a paragraph break
            return false;
        if (!mark_create(dot->line->blink, dot->line->blink->used.value(), dot))
            return false;
    }
    // ASSERT: we now have dot sitting on part of a word
    word_set_range element = 0;
    while (!word_elements[element].test(dot->line->str->operator[](dot->col)))
        element += 1;
    // Now find the start of this word
    while ((dot->col > 1) && word_elements[element].test(dot->line->str->operator[](dot->col)))
        dot->col -= 1;
    if (!word_elements[element].test(dot->line->str->operator[](dot->col)))
        dot->col += 1;
    return true;
}

bool next_word(mark_ptr dot) {
    // with dot^ do
    if (dot->col > dot->line->used) {
        // check that we aren't on a blank line
        if (dot->line->used == 0)
            return false;
        // All clear so fake it that we were at the end of the last word!
        dot->col = dot->line->used;
    }
    word_set_range element = 0;
    while (!word_elements[element].test(dot->line->str->operator[](dot->col)))
        element += 1;
    while ((dot->col < dot->line->used) &&
           word_elements[element].test(dot->line->str->operator[](dot->col)))
        dot->col += 1;
    if (word_elements[element].test(dot->line->str->operator[](dot->col))) {
        if (dot->line->flink == nullptr) // no more lines
            return false;
        if (dot->line->flink->used == 0) // end of paragraph
            return false;
        if (!mark_create(dot->line->flink, 1, dot))
            return false;
    }
    while (word_elements[0].test(dot->line->str->operator[](dot->col)))
        dot->col += 1;
    return true;
}

bool previous_word(mark_ptr dot) {
    // with dot^ do
    word_set_range element = 0;
    while (!word_elements[element].test(dot->line->str->operator[](dot->col)))
        element += 1;
    while ((dot->col > 1) && word_elements[element].test(dot->line->str->operator[](dot->col)))
        dot->col -= 1;
    if (word_elements[element].test(dot->line->str->operator[](dot->col))) {
        if (dot->line->blink == nullptr) // no more lines
            return false;
        if (dot->line->blink->used == 0) // top of paragraph
            return false;
        if (!mark_create(dot->line->blink, dot->line->blink->used.value(), dot))
            return false;
    }
    if (!current_word(dot))
        return false;
    return true;
}

bool newword_advance_word(leadparam rept, int count) {
    bool result = false;
    // with current_frame^ do
    mark_ptr new_dot = nullptr;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, new_dot))
        return false;
    if (rept == leadparam::marker) {
        if (!mark_create(
                current_frame->marks[count]->line, current_frame->marks[count]->col, new_dot
            ))
            goto l98;
        rept = leadparam::nint;
        count = 0;
    }
    // If we are doing a 0AW we need to go to the current word, -nAW does this
    if ((rept == leadparam::pint) && (count == 0))
        rept = leadparam::nint;
    switch (rept) {
    case leadparam::none:
    case leadparam::plus:
    case leadparam::pint:
        {
            while (count > 0) {
                count -= 1;
                if (!next_word(new_dot))
                    goto l98;
            }
            if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                goto l98;
        }
        break;

    case leadparam::minus:
    case leadparam::nint:
        {
            count = -count;
            if (!current_word(new_dot))
                goto l98;
            while (count > 0) {
                count -= 1;
                if (!previous_word(new_dot))
                    goto l98;
            }
            if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                goto l98;
        }
        break;

    case leadparam::pindef:
        {
            // with new_dot^ do
            if (new_dot->line->used == 0) // Fail if we are on a blank line
                goto l98;
            if (new_dot->col > new_dot->line->used + 2) {
                // check that we aren't past the last word in the para
                if (new_dot->line->flink == nullptr) // no more lines => end of para
                    goto l98;
                if (new_dot->line->flink->used == 0) // next line blank => end of para
                    goto l98;
                // In the middle of a paragraph so go it end of line
                new_dot->col = new_dot->line->used;
            }
            while (next_word(new_dot)) {
                if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                    goto l98;
            }
            // now on last word of paragraph
            //*** next statement should be more sophisticated
            //    what about the right margin??
            if (new_dot->line->used + 2 > MAX_STRLENP) {
                if (!mark_create(new_dot->line, MAX_STRLENP, current_frame->dot))
                    goto l98;
            } else if (!mark_create(new_dot->line, new_dot->line->used + 2, current_frame->dot))
                goto l98;
        }
        break;

    case leadparam::nindef:
        {
            if (!current_word(new_dot))
                goto l98;
            if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                goto l98;
            while (previous_word(new_dot)) {
                if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                    goto l98;
            }
        }
        break;

    default:
        // marker Handled above
        goto l98;
    }
    result = true;
l98:;
    mark_destroy(new_dot);
    return result;
}

bool newword_delete_word(leadparam rept, int count) {
    // Delete Word deletes the same words as advance word advances over.

    bool result = false;
    // with current_frame^ do
    mark_ptr old_pos = nullptr;
    mark_ptr here = nullptr;
    mark_ptr the_other_mark = nullptr;
    line_range line_nr;
    line_range new_line_nr;
    int old_dot_col;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, old_pos))
        goto l99;
    // First Step.
    //  Get to the beginning of the word if we are in the middle of it
    if (!newword_advance_word(leadparam::pint, 0))
        goto l99;
    // ASSERTION: We are on the beginning of a word
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, here))
        goto l99;
    if (!newword_advance_word(rept, count)) {
        // Put Dot back and bail out
        if (!mark_create(old_pos->line, old_pos->col, current_frame->dot))
            goto l99;
    }
    // OK. We now wipe out everything from Dot to here
    old_dot_col = current_frame->dot->col;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, the_other_mark))
        goto l99;
    if (!line_to_number(the_other_mark->line, line_nr))
        goto l99;
    if (!line_to_number(here->line, new_line_nr))
        goto l99;
    if ((line_nr > new_line_nr) ||
        ((line_nr == new_line_nr) && (the_other_mark->col > here->col))) {
        // Reverse mark pointers to get The_Other_Mark first.
        mark_ptr another_mark = here;
        here = the_other_mark;
        the_other_mark = another_mark;
    }
    if (current_frame != frame_oops) {
        // with frame_oops^ do
        //  Make sure oops_span is okay.
        if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->span->mark_two))
            goto l99;
        result = text_move(
            false,                      // Dont copy,transfer
            1,                          // One instance of
            the_other_mark,             // starting pos.
            here,                       // ending pos.
            frame_oops->span->mark_two, // destination.
            frame_oops->marks[0],       // leave at start.
            frame_oops->dot
        ); // leave at end.
    } else {
        result = text_remove(
            the_other_mark, // starting pos.
            here
        ); // ending pos.
    }
    if (line_nr != new_line_nr)
        result = text_split_line(current_frame->dot, old_dot_col, here);
l99:;
    if (old_pos != nullptr)
        mark_destroy(old_pos);
    if (here != nullptr)
        mark_destroy(here);
    if (the_other_mark != nullptr)
        mark_destroy(the_other_mark);
    return result;
}

bool current_paragraph(mark_ptr dot) {
    line_ptr new_line = dot->line;
    col_range pos;
    if (dot->col < dot->line->used) {
        pos = dot->col;
        while ((pos > 1) && word_elements[0].test(new_line->str->operator[](pos)))
            pos -= 1;
        if (word_elements[0].test(new_line->str->operator[](pos))) {
            if (new_line->blink == nullptr)
                return false;
            else
                new_line = new_line->blink;
        }
    }
    while ((new_line->blink != nullptr) && (new_line->used == 0))
        new_line = new_line->blink;
    if (new_line->used == 0)
        return false;
    while ((new_line->blink != nullptr) && (new_line->used != 0))
        new_line = new_line->blink;
    if (new_line->used == 0)
        new_line = new_line->flink; // Oops too far!
    pos = 1;
    while (word_elements[0].test(new_line->str->operator[](pos)))
        pos += 1;
    if (!mark_create(new_line, pos, dot))
        return false;
    return true;
}

bool next_paragraph(mark_ptr dot) {
    line_ptr new_line = dot->line;
    col_range pos;
    if (dot->col < dot->line->used) {
        pos = dot->col;
        while ((pos > 1) && word_elements[0].test(new_line->str->operator[](pos)))
            pos -= 1;
        if (word_elements[0].test(new_line->str->operator[](pos))) {
            if (new_line->blink == nullptr) {
                dot->col = 1;
                while (word_elements[0].test(new_line->str->operator[](dot->col)))
                    dot->col += 1;
                return true;
            } else {
                new_line = new_line->blink;
            }
        }
    }
    while ((new_line->flink != nullptr) && (new_line->used != 0))
        new_line = new_line->flink;
    if (new_line->used != 0)
        return false;
    while ((new_line->flink != nullptr) && (new_line->used == 0))
        new_line = new_line->flink;
    if (new_line->used == 0)
        return false;
    pos = 1;
    while (word_elements[0].test(new_line->str->operator[](pos)))
        pos += 1;
    if (!mark_create(new_line, pos, dot))
        return false;
    return true;
}

bool newword_advance_paragraph(leadparam rept, int count) {
    bool result = false;
    // with current_frame^ do

    mark_ptr new_dot = nullptr;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, new_dot))
        return false;
    if (rept == leadparam::marker) {
        if (!mark_create(
                current_frame->marks[count]->line, current_frame->marks[count]->col, new_dot
            ))
            goto l98;
        rept = leadparam::nint;
        count = 0;
    }
    // If we are doing a 0AP we need to go to the current para, -nAP does this
    if ((rept == leadparam::pint) && (count == 0))
        rept = leadparam::nint;
    switch (rept) {
    case leadparam::none:
    case leadparam::plus:
    case leadparam::pint:
        {
            while (count > 0) {
                count -= 1;
                if (!next_paragraph(new_dot))
                    goto l98;
            }
            if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                goto l98;
        }
        break;

    case leadparam::minus:
    case leadparam::nint:
        {
            count = -count;
            if (!current_paragraph(new_dot))
                goto l98;
            while (count > 0) {
                count -= 1;
                if (new_dot->line->blink == nullptr)
                    goto l98;
                if (!mark_create(new_dot->line->blink, 1, new_dot))
                    goto l98;
                if (!current_paragraph(new_dot))
                    goto l98;
            }
            if (!mark_create(new_dot->line, new_dot->col, current_frame->dot))
                goto l98;
        }
        break;

    case leadparam::pindef:
        if (!mark_create(
                current_frame->last_group->last_line, current_frame->margin_left, current_frame->dot
            ))
            goto l98;
        break;

    case leadparam::nindef:
        {
            line_ptr new_line = new_dot->line;
            while ((new_line->blink != nullptr) && (new_line->used == 0))
                new_line = new_line->blink;
            if (new_line->used == 0)
                goto l98;
            // OK we know that there is a paragraph behind us, so goto
            // the top of the file and go down to the first paragraph
            new_line = current_frame->first_group->first_line;
            while (new_line->used == 0)
                new_line = new_line->flink;
            col_range pos = 1;
            while (word_elements[0].test(new_line->str->operator[](pos)))
                pos += 1;
            if (!mark_create(new_line, pos, current_frame->dot))
                goto l98;
        }
        break;

    default:
        // Others handled elsewhere (marker) or ignored.
        break;
    }
    result = true;
l98:;
    mark_destroy(new_dot);
    return result;
}

bool newword_delete_paragraph(leadparam rept, int count) {
    bool result = false;
    // with current_frame^ do
    mark_ptr old_pos = nullptr;
    mark_ptr here = nullptr;
    mark_ptr the_other_mark = nullptr;
    line_range line_nr;
    line_range new_line_nr;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, old_pos))
        goto l99;
    // Get to the beginning of the paragraph
    if (!newword_advance_paragraph(leadparam::pint, 0))
        goto l99;
    if (!mark_create(current_frame->dot->line, 1, here))
        goto l99;
    if (!newword_advance_paragraph(rept, count)) {
        // Something wrong so put dot back and abort
        mark_create(old_pos->line, old_pos->col, current_frame->dot);
        goto l99;
    }

    // Now delete all the lines between marks dot and here
    if (!mark_create(current_frame->dot->line, 1, the_other_mark))
        goto l99;
    if (!line_to_number(the_other_mark->line, line_nr))
        goto l99;
    if (!line_to_number(here->line, new_line_nr))
        goto l99;
    if (line_nr > new_line_nr) {
        // reverse marks to get the_other_mark first.
        mark_ptr another_mark = here;
        here = the_other_mark;
        the_other_mark = another_mark;
    }
    if (current_frame != frame_oops) {
        // with frame_oops^ do
        //  Make sure oops_span is okay.
        if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->span->mark_two))
            goto l99;
        result = text_move(
            false,                          // Dont copy, transfer
            1,                              // One instance of
            the_other_mark,                 // starting pos.
            here,                           // ending pos.
            frame_oops->span->mark_two,     // destination.
            frame_oops->marks[MARK_EQUALS], // leave at start.
            frame_oops->dot
        ); // leave at end.
    } else {
        result = text_remove(
            the_other_mark, // starting pos.
            here
        ); // ending pos.
    }

l99:;
    if (old_pos != nullptr)
        mark_destroy(old_pos);
    if (here != nullptr)
        mark_destroy(here);
    if (the_other_mark != nullptr)
        mark_destroy(the_other_mark);
    return result;
}
