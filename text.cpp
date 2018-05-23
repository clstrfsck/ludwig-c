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
! Name:         TEXT
!
! Description:  Text manipulation routines.
*/

#include "text.h"

#include "var.h"
#include "vdu.h"
#include "line.h"
#include "mark.h"
#include "screen.h"

col_range text_return_col(line_ptr cur_line, col_range cur_col, bool splitting) {
    //  var
    //    new_line : line_ptr;
    //    new_col  : col_range;
    //    str_1,
    //    str_2    : str_ptr;
    //    used_1,
    //    used_2   : strlen_range;

    // Calculate which col to place dot on.
    //with cur_line->group->frame^ do
    line_ptr new_line = cur_line;
    col_range new_col;
    if (cur_col >= cur_line->group->frame->margin_left)
        new_col = cur_line->group->frame->margin_left;
    else
        new_col = 1;
    if (cur_line->group->frame->options.contains(frame_options_elts::opt_auto_indent) &&
        (new_line->flink != nullptr)) {
        //with new_line^ do
        str_ptr str_1 = new_line->str;        // Aim at this line.
        strlen_range used_1 = new_line->used; // Length of this line.
        str_ptr str_2 = new_line->str;        // Aim at next iff next is not null line.
        strlen_range used_2 = new_line->used; // Len of next iff next is not null line.
        //with flink^ do
        if ((new_line->flink->flink != nullptr) && !splitting) {
            str_2  = new_line->flink->str;
            used_2 = new_line->flink->used;
        }
        while (true) {
            if (new_col <= used_1) {
                if ((*str_1)[new_col] != ' ')
                    break;
            }
            if (new_col <= used_2) {
                if ((*str_2)[new_col] != ' ')
                    break;
            } else {
                if (new_col >= used_1)
                    break;
            }
            new_col += 1;
        }
    }
    return new_col;
}

bool text_realize_null(line_ptr old_null) {
    //  var
    //    new_null : line_ptr;

    line_ptr new_null;
    if (lines_create(1, new_null, new_null)) {
        if (lines_inject(new_null, new_null, old_null)) {
            if (marks_shift(old_null, 1, MAX_STRLENP, new_null, 1)) {
                //with new_null->group->frame^ do
                new_null->group->frame->text_modified = true;
                if (mark_create(new_null->group->frame->dot->line,
                                new_null->group->frame->dot->col,
                                new_null->group->frame->marks[MARK_MODIFIED]))
                    return true;
            }
        }
    }
    return false;
}

bool text_insert(bool update_screen, int count, str_object buf, strlen_range buf_len, mark_ptr dst) {
    //  var
    //    dst_line         : line_ptr;
    //    dst_col          : col_range;
    //    i                : integer;
    //    insert_len       : 0..maxint;
    //    tail_len,
    //    len_redraw,
    //    final_len        : integer;
    //    first_col_redraw,
    //    last_col_redraw  : integer;
    //    new_col          : col_range;
    //    scr_col          : integer;

#ifdef DEBUG
    if (count < 0) {
        screen_message(DBG_REPEAT_NEGATIVE);
        return false;
    }
#endif
    size_t insert_len = count * buf_len;
    if (insert_len > 0) {
        //with dst^ do
        line_ptr dst_line = dst->line;
        col_range dst_col = dst->col;
        //with dst_line^ do
        int final_len = dst_col - 1 + insert_len;
        int tail_len  = dst_line->used + 1 - dst_col;
        if (tail_len <= 0)
            tail_len = 0;
        else
            final_len += tail_len;
        if (final_len > MAX_STRLEN)
            return false;
        if (dst_line->flink == nullptr) {
            if (!text_realize_null(dst_line))
                return false;
            dst_line = dst_line->blink;
        }
        //with dst_line^ do
        if (final_len > dst_line->len) {
            if (!line_change_length(dst_line, final_len)) {
                return false;
            }
        }
        if (!marks_shift(dst_line, dst_col, MAX_STRLENP - dst_col,
                         dst_line, dst_col + insert_len))
            return false;
        if (tail_len > 0) // to avoid subscript error when dst_col=400
            dst_line->str->copy(dst_line->str->data(dst_col + insert_len), tail_len, dst_col);
        col_range new_col = dst_col;
        for (int i = 0; i < count; ++i) {
            dst_line->str->copy(buf.data(1), buf_len, new_col);
            new_col += buf_len;
        }

        // Re-compute length of line, to remove trailing spaces.
        if (tail_len == 0) 
            dst_line->used = dst_line->str->length(' ', dst_line->len);
        else
            dst_line->used += insert_len;

        // Update screen if necessary, and it is affected. }
        if (update_screen && dst_line->scr_row_nr != 0) {
            //with (dst_line->)group->frame^ do
            // Update the VDU.
            int scr_col = dst_col - dst_line->group->frame->scr_offset;
            if (scr_col <= dst_line->group->frame->scr_width) {
                if (scr_col <= 0)
                    scr_col = 1;
                vdu_movecurs(scr_col, dst_line->scr_row_nr);
                int first_col_redraw = dst_col;
                int last_col_redraw;
                if (first_col_redraw <= dst_line->group->frame->scr_offset)
                    first_col_redraw = dst_line->group->frame->scr_offset + 1;
                if  ((scr_col + insert_len <= dst_line->group->frame->scr_width) &&
                     (scr_col + dst_line->group->frame->scr_offset <= dst_line->used)) {
                    vdu_insertchars(insert_len);
                    last_col_redraw  = first_col_redraw + insert_len - 1;
                } else {
                    last_col_redraw = dst_line->used;
                }
                if (last_col_redraw > dst_line->group->frame->scr_width + dst_line->group->frame->scr_offset)
                    last_col_redraw = dst_line->group->frame->scr_width + dst_line->group->frame->scr_offset;
                int len_redraw = last_col_redraw - first_col_redraw + 1;
                if (len_redraw > 0)
                    vdu_displaystr(len_redraw, dst_line->str->data(first_col_redraw), 0);
            }
        }
    }
    return true;
}

bool text_overtype(bool update_screen, int count, str_object buf, strlen_range buf_len, mark_ptr &dst) {
    /*
      ! Inputs:
      !   update_screen     Do/Do not update screen. (The screen may already
      !                     have been updated by echo from the input
      !                     routines.)
      !   count             Number of copies of the text to be overtyped.
      !   buf_len           Length of the string to be overtyped.
      !   buf               The string to be overtyped.
      !   dst               A mark indicating where the overtyping is to
      !                     start.
      ! Outputs:
      !   dst               Modified to point past the insertion.
    */

    //  var
    //    i                : integer;
    //    dst_line         : line_ptr;
    //    len_on_scr,
    //    first_col_on_scr,
    //    last_col_on_scr,
    //    overtype_len,
    //    final_len        : integer;
    //    new_col          : col_range;

#ifdef DEBUG
    if (count < 0) {
        screen_message(DBG_REPEAT_NEGATIVE);
        return false;
    }
#endif
    int overtype_len = count * buf_len;
    if (overtype_len > 0) {
        //with dst^ do
        line_ptr dst_line = dst->line;
        //with dst_line^ do
        int final_len = dst->col + overtype_len - 1;
        if (final_len > MAX_STRLEN)
            return false;
        if (dst_line->flink == nullptr) {
            if (!text_realize_null(dst->line))
                return false;
            dst_line = dst_line->blink;
        }
        //with dst_line^ do
        if (final_len > dst_line->len) {
            if (!line_change_length(dst_line, final_len))
                return false;
        }
        col_range new_col = dst->col;
        for (int i = 0; i < count; ++i) {
            dst_line->str->copy(buf.data(1), buf_len, new_col);
            new_col += buf_len;
        }

        // Re-compute length of line, to remove trailing spaces/
        if (new_col > dst_line->used)
            dst_line->used = dst_line->str->length(' ', dst_line->len);

        // Update screen if necessary, and it is affected.
        if (update_screen && dst_line->scr_row_nr != 0) {
            //with (dst_line->)group->frame^ do
            // Aim first... at the first screen character changed.
            // Aim last... at PAST the last screen character changed.
            int first_col_on_scr = dst->col;
            if (first_col_on_scr <= dst_line->group->frame->scr_offset)
                first_col_on_scr = dst_line->group->frame->scr_offset + 1;
            int last_col_on_scr  = new_col;
            if (last_col_on_scr > dst_line->group->frame->scr_width + dst_line->group->frame->scr_offset)
                last_col_on_scr = dst_line->group->frame->scr_width + dst_line->group->frame->scr_offset + 1;

            // Update the VDU.
            int len_on_scr = last_col_on_scr - first_col_on_scr;
            if (len_on_scr > 0) {
                vdu_movecurs(first_col_on_scr - dst_line->group->frame->scr_offset, dst_line->scr_row_nr);
                vdu_displaystr(len_on_scr, dst_line->str->data(first_col_on_scr), 0 /*no-cleareol,no-anycurs*/);
            }
        }

        // Update the destination mark.
        dst->col += overtype_len;
    }
    return true;
}

bool text_insert_tpar(tpar_object tp, mark_ptr before_mark, mark_ptr &equals_mark) {
    /*
      ! Inputs:
      !   tp                The trailing parameter to be inserted.
      !   before_mark       Insert the text at before_mark.
      ! Outputs:
      !   before_mark       Marks the end of the inserted text.
      !   equals_mark       Marks the beginning of the inserted text.
    */
    //  var
    //    line_count, lc : integer;
    //    tmp_tp : tpar_ptr;
    //    tmp_line : line_ptr;
    //    first_line,
    //    last_line : line_ptr;
    //    discard : boolean;

    bool result = false;
    line_ptr first_line;
    line_ptr last_line;
    bool discard = false;
    // check for the simple case
    //with before_mark^ do
    if (tp.con == nullptr) {
        if (!text_insert(true, 1, tp.str, tp.len, before_mark)) {
            screen_message(MSG_NO_ROOM_ON_LINE);
            return false; // Nothing to free up here
        }
        if (!mark_create(before_mark->line, before_mark->col - tp.len, equals_mark))
            return false;
    } else {
        if (before_mark->col + tp.len > MAX_STRLEN) {
            screen_message(MSG_NO_ROOM_ON_LINE);
            return false; // Nothing to free up yet
        }
        int line_count = 0;
        tpar_ptr tmp_tp = tp.con;
        while (tmp_tp->con != nullptr) {
            line_count += 1;
            tmp_tp = tmp_tp->con;
        }
        if (tmp_tp->len + (before_mark->line->used - before_mark->col) > MAX_STRLEN) {
            screen_message(MSG_NO_ROOM_ON_LINE);
            return false; // Last chance to exit safely before we create lines
        }
        if (!lines_create(line_count, first_line, last_line)) {
            return false; // No lines created -> assumed safe to do this
        }
        discard = true;
        if (before_mark->line->flink == nullptr && !text_realize_null(before_mark->line))
            goto l99;
        if (!text_split_line(before_mark, 1, equals_mark))
            goto l99;
        if (!text_insert(true, 1, tp.str, tp.len, equals_mark))
            goto l99;
        //with equals_mark^ do
        equals_mark->col -= tp.len;
        tmp_tp = tp.con;
        line_ptr tmp_line = first_line;
        for (int lc = 0; lc < line_count; ++lc) {
            if (!line_change_length(tmp_line, tmp_tp->len))
                goto l99;
            tmp_line->str->copy(tmp_tp->str.data(1), tmp_tp->len, 1);
            tmp_line->used = tmp_line->str->length(' ', tmp_tp->len);
            tmp_tp = tmp_tp->con;
            tmp_line = tmp_line->flink;
        }
        if (line_count != 0) {
            if (!lines_inject(first_line, last_line, before_mark->line))
                goto l99;
        }
        discard = false;
        //with tmp_tp^ do
        if (!text_insert(true, 1, tmp_tp->str, tmp_tp->len, before_mark))
            return false; // Safe again now
    }
    result = true;
 l99:;
    if (discard)
        lines_destroy(first_line, last_line);
    return result;
}

bool text_intra_remove(mark_ptr mark_one, strlen_range size) {
//    var
//      buf_len         : integer;
//      ln              : line_ptr;
//      offset_p_width  : 0..maxint;
//      first_col_on_scr,
//      col_one,col_two : col_range;
//      old_used,
//      dst_len         : strlen_range;
//      distance        : col_width_range;


    //with mark_one^ do
    line_ptr ln = mark_one->line;
    col_range col_one = mark_one->col;
    col_range col_two = col_one + size;
    if (!marks_squeeze(ln, col_one, ln, col_two))
        return false;
    if (!marks_shift(ln, col_two, MAX_STRLENP + 1 - col_two, ln, col_one))
        return false;
    if (size == 0)
        return false;
    //with ln^ do
    strlen_range old_used = ln->used;
    if (col_one > old_used)
        return true;
    strlen_range dst_len = old_used + 1 - col_one;
    if (col_two <= old_used)
        ln->str->fillcopy(ln->str->data(col_two), old_used + 1 - col_two, col_one, dst_len, ' ');
    else
        ln->str->fill(' ', col_one, dst_len);
    ln->used = ln->str->length(' ', old_used);

    // Now update screen if necessary.
    if (ln->scr_row_nr == 0)
        return true;
    //with group->frame^ do
    size_t offset_p_width = ln->group->frame->scr_offset + ln->group->frame->scr_width;
    if (col_one > offset_p_width)
        return true;
    scr_col_range distance = col_two - col_one;
    col_range first_col_on_scr = col_one;
    if (col_one <= ln->group->frame->scr_offset)
        first_col_on_scr = ln->group->frame->scr_offset + 1;

    // If possible, drag any characters on screen to final place.
    if ((first_col_on_scr + distance <= offset_p_width) && (first_col_on_scr + distance <= old_used)) {
        vdu_movecurs(first_col_on_scr - ln->group->frame->scr_offset, ln->scr_row_nr);
        vdu_deletechars(distance);
        first_col_on_scr = offset_p_width + 1 - distance;
        if (first_col_on_scr > ln->used)
            return true;
    }

    // Fix the remainder of the lines appearance on the screen.
    vdu_movecurs(first_col_on_scr - ln->group->frame->scr_offset, ln->scr_row_nr);
    int buf_len;
    if (ln->used <= ln->group->frame->scr_offset + ln->group->frame->scr_width)
        buf_len = ln->used + 1 - first_col_on_scr;
    else
        buf_len = ln->group->frame->scr_offset + ln->group->frame->scr_width + 1 - first_col_on_scr;
    if (buf_len <= 0)
        vdu_cleareol();
    else
        vdu_displaystr(buf_len, ln->str->data(first_col_on_scr), 3 /*cleareol,leave cursor anywhere*/);
    return true;
}

bool text_inter_remove(mark_ptr mark_one, mark_ptr mark_two) {
//    var
//      mark_start        : mark_ptr;
//      extr_one,extr_two : line_ptr;
//      text_len          : strlen_range;
//      strng             : str_object;
//      strng_tail        : str_object;
//      delta             : integer;
//      line_one          : line_ptr;
//      col_one           : col_range;

    bool result = false;

    mark_ptr mark_start = nullptr;
    line_ptr extr_one;
    line_ptr extr_two;
    strlen_range text_len;
    str_object strng;
    str_object strng_tail;
    int delta;
    line_ptr line_one;
    col_range col_one;

    if ((mark_two->line->flink == nullptr) && (mark_one->col != 1)) {
        //with mark_one^ do
        line_one = mark_one->line;
        col_one = mark_one->col;
        extr_one = line_one->flink;
        extr_two = mark_two->line;
        if (!text_intra_remove(mark_one, MAX_STRLENP - mark_one->col))
            goto l99;
        if (!marks_squeeze(line_one, col_one, mark_two->line, mark_two->col))
            goto l99;
        if (!marks_shift(mark_two->line, mark_two->col, MAX_STRLENP + 1 - mark_two->col, line_one, col_one))
            goto l99;
        if (extr_one == extr_two)
            goto l88;               // Okay but No lines to extract.
        extr_two = extr_two->blink; // Okay but has lines to extract.
        goto l77;
    }

    // Bring the start of line_one down to replace the start of line_two.
    //with mark_one^,line^ do
    text_len = mark_one->line->used;
    if (mark_one->col <= text_len)
        text_len = mark_one->col - 1;
    strng.fillcopy(mark_one->line->str->data(1), text_len, 1, mark_one->col - 1, ' ');
    text_len = mark_one->col - 1;
    delta = mark_one->col - mark_two->col;
    if (delta < 0) {
        if (!mark_create(mark_two->line, mark_one->col, mark_start))
            goto l99;
        if (!text_intra_remove(mark_start, mark_two->col - mark_start->col))
            goto l99;
    } else if (delta > 0) {
        strng_tail.copy(strng.data(mark_two->col), delta);
        if (!text_insert(true, 1, strng_tail, delta, mark_two))
            goto l99;
        text_len -= delta;
    }
    if (!mark_create(mark_two->line, 1, mark_start))
        goto l99;
    if (text_len > 0) {
        if (!text_overtype(true, 1, strng, text_len, mark_start))
            goto l99;
    }
    //with mark_one^ do
    col_one = mark_one->col;
    extr_one = mark_one->line;
    //with mark_two^ do
    extr_two = mark_two->line->blink;
    if (!marks_squeeze(extr_one, col_one, mark_two->line, mark_two->col))
        goto l99;
    if (col_one > 1) {
        if (!marks_shift(extr_one, 1, col_one - 1, mark_two->line, 1))
            goto l99;
    }
l77:;
    if (!lines_extract(extr_one, extr_two))
        goto l99;
    if (!lines_destroy(extr_one, extr_two))
        goto l99;
l88:;
    result = true;
l99:;
    if (mark_start != nullptr)
        mark_destroy(mark_start);
    return result;
}


bool text_remove(mark_ptr mark_one, mark_ptr mark_two) {
#ifdef DEBUG
    line_range line_one_nr;
    if (!line_to_number(mark_one->line, line_one_nr))
        return false;
    line_range line_two_nr;
    if (!line_to_number(mark_two->line, line_two_nr))
        return false;
    if ((mark_one->line->group->frame != mark_two->line->group->frame) || (line_one_nr > line_two_nr)) {
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        return false;
    }
#endif
    if (mark_one->line == mark_two->line)
        return text_intra_remove(mark_one, mark_two->col - mark_one->col);
    else
        return text_inter_remove(mark_one, mark_two);
}

bool text_intra_move(bool copy, int count, mark_ptr mark_one, mark_ptr mark_two, mark_ptr dst, mark_ptr &new_start, mark_ptr &new_end) {
    // ASSUMES COUNT >= 1, MARKS ON SAME LINE,  MARK_ONE AT OR BEFORE _TWO.
//    var
//      full_len : strlen_range;
//      text_len : strlen_range;
//      text_str : str_object;
//      i        : integer;
//      tail_len : strlen_range;
//      col_one,col_two : col_range;
//      dst_col  : col_range;
//      dst_used : strlen_range;

    col_range col_one = mark_one->col;
    col_range col_two = mark_two->col;

    // Take COUNT copies of the source string.
    strlen_range full_len = col_two - col_one;
    str_object text_str;
    if (full_len != 0) {
        if (full_len * count > MAX_STRLEN)
            return false;
        //with mark_one->line^ do
        strlen_range text_len = full_len;
        if (col_one > mark_one->line->used)
            text_len = 0;
        else {
            if (col_two > mark_one->line->used)
                text_len = mark_one->line->used + 1 - col_one;
        }
        text_str.fillcopy(mark_one->line->str->data(col_one), text_len, 1, full_len, ' ');
        text_len = full_len;
        for (int i = 0; i < 2; ++i) {
            text_str.copy(text_str.data(), text_len, 1 + full_len);
            full_len += text_len;
            if (tt_controlc)
                return false;
        }
    }
    if (!copy) {
        //with dst^ do
        // Predict dst->col & dst->line->used after the
        // removal of the span.
        //with line^ do
        col_range dst_col  = dst->col;  // They will be the current values, unless ...
        strlen_range dst_used = dst->line->used;
        if (mark_one->line == dst->line) {
            if (dst_col > col_two)
                dst_col = dst_col - (col_two - col_one);
            else if (dst_col > col_one)
                dst_col = col_one;
            if (dst_used > col_two)
                dst_used -= col_two - col_one;
            else
                dst_used = col_one - 1;    // This is a good enough guess.
        }
        strlen_range tail_len = 0;
        if (dst_col <= dst_used)
            tail_len = dst_used + 1 - dst_col;
        if (dst_col + full_len + tail_len > MAX_STRLENP)
            return false;
        if (full_len != 0) {
            if (!text_intra_remove(mark_one, mark_two->col - mark_one->col))
                return false;
        }
    }
    if (full_len != 0) {
        if (!text_insert(true, 1, text_str, full_len, dst))
            return false;
    }
    //with dst^ do
    col_range dst_col = dst->col;
    if (!mark_create(dst->line, dst_col - full_len, new_start))
        return false;
    if (!mark_create(dst->line, dst_col,            new_end  ))
        return false;
    return true;
}

#ifdef XXXX
  function text_inter_move : boolean;

    { ASSUMES COUNT >= 1, MARK_ONE->LINE BEFORE MARK_TWO->LINE }
    label 88,99;
    var
      first_line,
      last_line,
      next_src_line,
      next_dst_line,
      first_nicked,
      last_nicked      : line_ptr;
      line_one_nr,
      line_two_nr,
      line_dst_nr      : line_range;
      lines_required   : integer;
      last_line_length : strlen_range;
      temp_len         : strlen_range;
      text_len         : strlen_range;
      text_str         : str_object;
      i                : integer;
      line_one         : line_ptr;
      col_one          : col_range;
      line_two         : line_ptr;
      col_two          : col_range;
      dst_col          : col_range;
      dst_used         : strlen_range;
      dst_line         : line_ptr;

    begin {text_inter_move}
    text_inter_move := false;
    first_line := nil;
    with mark_one^ do begin line_one := line; col_one := col; end;
    with mark_two^ do begin line_two := line; col_two := col; end;
    if not line_to_number(line_one,line_one_nr) then goto 99;
    if not line_to_number(line_two,line_two_nr) then goto 99;

    { Verify that the insertion will work. }
    with dst^ do            { Predict dst->col & dst->line->used just before }
      begin                 { the insertion of the span.                     }
      dst_col  := col;
      dst_used := line->used;
      if not copy then
      if line->group->frame = line_one->group->frame then
        begin
        if not line_to_number(line,line_dst_nr) then goto 99;
        if (line_one_nr <= line_dst_nr) and (line_dst_nr <= line_two_nr) then
          begin
          if (line_two_nr = line_dst_nr) and (dst_col >= col_two) then
            { DST IS ON SAME LINE AS END, BUT BEYOND END }
            dst_col := col_one+dst_col-col_two
          else
          if (line_one_nr <> line_dst_nr) or (dst_col >= col_one) then
            dst_col := col_one; { DST IS INSIDE AREA TO BE REMOVED }
          temp_len := 0;
          with line_two^ do
            if col_two <= used then temp_len := used+1-col_two;
          dst_used := col_one-1+temp_len;
          end;
        end;
      if col_two <= line_two->used then
        if col_one{-1}+line_two->used{+1}-col_two > max_strlenp then
          goto 99;
      if col_one <= line_one->used then
        if dst_col{-1}+line_one->used{+1}-col_one > max_strlenp then
          goto 99;
      if dst_col <= dst_used then
        if col_two{-1}+dst_used{+1}-dst_col > max_strlen then goto 99;
      if count>1 then
      if col_one <= line_one->used then
        if col_two{-1}+line_one->used{+1}-col_one > max_strlen then goto 99;
      end;

    { Create any extra lines required. }
    lines_required := count*(line_two_nr-line_one_nr);
    if not copy then
      begin
      { Allow for lines nicked from the source! }
      lines_required := lines_required-(line_two_nr-line_one_nr-1);
      end;
    if not lines_create(lines_required,first_line,last_line) then goto 99;

    { Copy end of first line of area, TEXT_LEN and TEXT_STR keep result. }
    with line_one^ do
      begin
      text_len := 0;
      if col_one <= used then
        begin
        text_len := used+1-col_one;
        ch_move(str^,col_one,text_str,1,text_len);
        end;
      end;

    { Complete taking the COUNT copies, including nicking the interior lines }
    { if this is a Transfer operation. }
    next_dst_line := first_line;
    for i := count-1 downto 0 do
      begin
      if tt_controlc then { ABORT }
        goto 99;          {Note: Can't harm ST as count = 1 in that case}
      next_src_line := line_one->flink;
      if i=0 then                     { Last time round the loop, nick }
        begin                         { the original INTERIOR lines. }
        if not copy then
        if line_two_nr-line_one_nr > 1 then
          begin
          first_nicked := line_one->flink;
          last_nicked  := line_two->blink;
          if not marks_squeeze(first_nicked,1,last_nicked->flink,1) then
            goto 99;
          if not lines_extract(first_nicked,last_nicked) then goto 99;
{#if debug}
          if next_dst_line->flink <> nil then
            begin screen_message(dbg_internal_logic_error); goto 99; end;
{#endif}
          last_nicked->flink   := next_dst_line;
          first_nicked->blink  := next_dst_line->blink;
          if first_nicked->blink <> nil then
            first_nicked->blink->flink := first_nicked;
          next_dst_line->blink := last_nicked;
          if next_dst_line = first_line then first_line := first_nicked;
          next_src_line := line_two;
          end;
        end;

      { Copy remaining INTERIOR lines. }
      while next_src_line<>line_two do
        begin
        with next_src_line^ do
          begin
          if not line_change_length(next_dst_line,used) then goto 99;
          ch_move(str^,1,next_dst_line->str^,1,used);
          next_dst_line->used := used;
          next_src_line := flink;
          next_dst_line := next_dst_line->flink;
          end;
        end;

      { Copy the last_line, and for all but the last copy append the }
      { start of the next copy to it. }
      with next_dst_line^ do
        begin
        if i <> 0 then
          begin { PLACE START OF NEXT COPY AT END OF LINE }
          if not line_change_length(next_dst_line,col_two-1+text_len) then
            goto 99;
          ch_move(text_str,1,str^,col_two,text_len);
          end
        else
          begin { JUST MAKE LONG ENOUGH FOR THE END OF COPY. }
          if not line_change_length(next_dst_line,col_two-1) then
            goto 99;
          end;  { PLACE THE END OF THE COPY IN POSITION }
        if col_two <= next_src_line->used then
          ch_move(next_src_line->str^,1,str^,1,col_two-1)
        else
          ch_copy(next_src_line->str^,1,next_src_line->used,
                  str^,1,col_two-1,' ');
        if i <> 0 then
          used := ch_length(str^,col_two-1+text_len)
        else
          used := col_two-1;  { THIS PRESERVES KNOWLEGE OF THE LENGTH OF THE }
                              { LAST_LINE }
        next_dst_line := flink;
        end;
      end;
{#if debug}
    if next_dst_line <> nil then
      begin screen_message(dbg_internal_logic_error); goto 99; end;
{#endif}

    { If necessary, remove the original text <or what is left of it>. }
    if not copy then
      if not text_inter_remove(mark_one,mark_two) then goto 99;

    {Insert the source text into the destination text.}
    with dst^ do
      begin
      dst_line := line;
      dst_col  := col;

      { COMPLETE THE LAST LINE WITH THE REST OF THE DESTINATION LINE. }
      with last_line^ do
        begin
        last_line_length := used;
        i := dst_line->used+1-dst_col;
        if i > 0 then
          begin
          if not line_change_length(last_line,used+i) then goto 99;
          ch_move(dst_line->str^,dst_col,str^,last_line_length+1,i);
          used := ch_length(str^,last_line_length+i);
          end
        else
          used := ch_length(str^,last_line_length);
        end;
      end;

    { Special case for the NULL line and for accurate copying of }
    { one frame into another empty frame. }
    if dst_line->flink = nil then
      begin
      if (dst_col <> 1)
      or (last_line_length <> 0)
      then
        begin
        if not text_realize_null(dst_line) then goto 99;
        dst_line := dst_line->blink;
        end
      else
        begin

        { Shift last line to first line to give somewhere to place text_str.}
        if first_line <> last_line then
          begin
          first_nicked        := last_line;
          last_line           := last_line->blink;
          last_line->flink    := nil;
          first_nicked->blink := nil;
          first_nicked->flink := first_line;
          first_line->blink   := first_nicked;
          first_line          := first_nicked;
          end;

        { Place the text in the first line, and inject them all. }
        if text_len > 0 then
          begin
          if not line_change_length(first_line,text_len) then goto 99;
          ch_copy(text_str,1,text_len,
                  first_line->str^,1,first_line->len,' ');
          first_line->used := text_len;
          end;
        if not lines_inject(first_line,last_line,dst_line) then goto 99;

        { Set up variables so that the creation of the New_Start and }
        { New_End marks will be correct. }
        last_line  := dst_line;
        dst_line   := first_line;
        first_line := nil;    { To prevent their destruction. }
        goto 88;
        end;
      end;
    if not lines_inject(first_line,last_line,dst_line->flink) then goto 99;
    if not marks_shift(dst_line,dst_col,max_strlenp+1-dst_col
                      ,last_line,col_two
                      ) then goto 99;
    first_line := nil;        { To prevent their destruction. }
    with dst_line^ do
      begin
      if text_len > 0 then
        begin
        if not line_change_length(dst_line,dst_col+text_len-1) then goto 99;
        ch_copy(text_str,1,text_len,str^,dst_col,len+1-dst_col,' ');
        used := dst_col+text_len-1;
      { The following method of re-drawing the line is adequate given the }
      { relative low usage of this area of code.  The screen is optimally }
      { updated by VDU. }
        if scr_row_nr <> 0 then screen_draw_line(dst_line);
        end
      else
      if dst_col <= used then
        begin
        ch_fill(str^,dst_col,used+1-dst_col,' ');
        used := ch_length(str^,dst_col);
      { The following method of re-drawing the line is adequate given the }
      { relative low usage of this area of code.  The screen is optimally }
      { updated by VDU. }
        if scr_row_nr <> 0 then screen_draw_line(dst_line);
        end;
      end;
    { FINISHED -- AT LAST! }
   88:
    if not mark_create(dst_line,dst_col ,new_start) then goto 99;
    if not mark_create(last_line,col_two,new_end  ) then goto 99;
    text_inter_move := true;
   99:
    if first_line <> nil then { If anything wrong, Destroy any created lines.}
      if not lines_destroy(first_line,last_line) then { Ignore failures. };
    end; {text_inter_move}

function text_move (
                copy      : boolean;
                count     : integer;
                mark_one  : mark_ptr;
                mark_two  : mark_ptr;
                dst       : mark_ptr;
        var     new_start : mark_ptr;
        var     new_end   : mark_ptr)
        : boolean;

  label 99;
  var
    cmd_success : boolean;
{#if debug}
    line_one_nr,line_two_nr : line_range;
{#endif}

  begin {text_move}
{#if debug}
  text_move := false;
  if not line_to_number(mark_one->line,line_one_nr) then goto 99;
  if not line_to_number(mark_two->line,line_two_nr) then goto 99;
  if (mark_one->line->group->frame <> mark_two->line->group->frame)
  or (line_one_nr > line_two_nr)
  then begin screen_message(dbg_internal_logic_error); goto 99; end;
{#endif}
  if count > 0 then
    begin
    if mark_one->line = mark_two->line then
      cmd_success := text_intra_move
    else
      cmd_success := text_inter_move;
    if tt_controlc then goto 99;
    if not cmd_success then
      begin screen_message(msg_no_room_on_line); goto 99; end;
    {
    ! Everything has gone well, now set the modified flag for the destination
    ! frame, and the source frame, if it's a transfer.
    }
    if not copy then
      with mark_two^, line->group->frame^ do
        begin
        text_modified := true;
        if not mark_create(line,col,marks[mark_modified]) then
          goto 99;
        end;
    with new_end^, line->group->frame^ do
      begin
      text_modified := true;
      if not mark_create(line,col,marks[mark_modified]) then
        goto 99;
      end;
    end;
  text_move := true;
 99:
  end; {text_move}


function text_split_line (
                before_mark : mark_ptr;
                new_col     : integer;
        var     equals_mark : mark_ptr)
        : boolean;

  {
  ! Inputs:
  !   before_mark       Indicates where the split is to be done.
  !   new_col           The split text is to be moved to this column.
  ! Outputs:
  !   before_mark       In column new_col of the second line.
  !   equals_mark       Where before_mark used to be.
  }

  label 99;
  var
    length   : integer;
    save_col : col_range;
    shift    : integer;
    new_line : line_ptr;
    discard  : boolean;
    cost     : integer;
    equals_col  : col_range;
    equals_line : line_ptr;

  begin {text_split_line}
  text_split_line := false;
  discard := false;
  with before_mark^,line^ do
    begin
    if flink = nil then
      begin screen_message(msg_cant_split_null_line); goto 99; end;
    if new_col = 0 then new_col := text_return_col(line,col,true);
    length := used+1-col;
    if length <= 0 then length := 0
    else
      begin
      if new_col+length > max_strlenp then
        begin screen_message(msg_no_room_on_line); goto 99; end;
      end;

    { Do everything that requires additional memory allocation first. }
    if not lines_create(1,new_line,new_line) then goto 99;
    discard := true;

    { The following is a HEURISTIC calculation to decide which way to do the }
    { split. By default we are going to move the end of this line down to a  }
    { new line.                                                              }
    shift := new_col-col;
    cost := maxint;
    if (col <= used) and (scr_row_nr <> 0) then
      begin
      if shift = 0 then
        cost := col+col               { allow for move-up + erase current }
      else
      if (shift > 0) and (trmflags_v_inch in tt_capabilities) then
        cost := col+col+3*shift       { up+erase+shift }
      else
      if (shift < 0) and (trmflags_v_dlch in tt_capabilities) then
        cost := col+col-3*shift;      { up+erase+shift }
      end;

    { Do the split. }
    if 2*length < cost then
      begin                           { move end to next (new) line }
      equals_col  := col;
      equals_line := line;
      if length > 0 then
        begin
        if not line_change_length(new_line,new_col+length-1) then goto 99;
        ch_fill(new_line->str^,1,new_col-1,' ');
        ch_move(str^,col,new_line->str^,new_col,length);
        ch_fill(str^,col,length,' ');
        used := ch_length(str^,used);
        new_line->used := new_col+length-1;
        if scr_row_nr <> 0 then
          begin
          with group->frame^ do
            begin
            if used <= scr_offset then
              begin
              vdu_movecurs(1,scr_row_nr);
              vdu_cleareol;
              end
            else
            if used+1 <= scr_offset+scr_width then
              begin
              vdu_movecurs(used+1-scr_offset,scr_row_nr);
              vdu_cleareol;
              end;
            end;
          end;
        end;
      if not lines_inject(new_line,new_line,flink) then goto 99;
      discard := false;
      if not marks_shift(line,col,max_strlenp+1-col,new_line,new_col) then
        goto 99;
      end
    else
      begin                           { move front up and adjust rest }
      equals_col  := col;
      equals_line := new_line;
      if col <= used then shift := col-1 else col := used;
      if shift > 0 then
        begin
        if not line_change_length(new_line,shift) then goto 99;
        ch_move(str^,1,new_line->str^,1,shift);
        new_line->used := ch_length(new_line->str^,shift);
        end;
      if not lines_inject(new_line,new_line,line) then goto 99;
      discard := false;
      if col > 1 then
        if not marks_shift(line,1,col-1,new_line,1) then goto 99;
      shift := new_col-col;
      if shift <= 0 then
        begin
        if shift < 0 then
          begin
          col := col+shift;
          if not text_intra_remove(before_mark,-shift) then goto 99;
          end;
        if new_col > 1 then
          begin
          save_col := col;
          col := 1;
          if not text_overtype(true,1,
                               blank_string,new_col-1,before_mark) then
            goto 99;
          col := save_col;
          end;
        end
      else
        begin
        if not text_insert(true,1,blank_string,shift,before_mark)
          then goto 99;
        if new_col > 1 then
          begin
          save_col := col;
          col := 1;
          if not text_overtype(true,1,blank_string,new_col-1,before_mark)
            then goto 99;
          col := save_col;
          end;
        end;
      end;
    with group->frame^ do
      begin
      if not mark_create(line,col,marks[mark_modified]) then goto 99;
      text_modified := true;
      end;
    end;
  { FINISHED -- AT LAST }
  if not mark_create(equals_line,equals_col,equals_mark) then goto 99;
  text_split_line := true;
 99:
  if discard then if not lines_destroy(new_line,new_line) then ;
  end; {text_split_line}

{#if vms or turbop}
end.
{#endif}
#endif
