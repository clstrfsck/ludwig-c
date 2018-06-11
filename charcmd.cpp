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
! Name:         CHARCMD
!
! Description:  Character Insert/Delete/Rubout commands.
*/

#include "charcmd.h"

#include "ch.h"
#include "var.h"
#include "vdu.h"
#include "mark.h"
#include "text.h"
#include "screen.h"

bool key_in_set(key_code_range key, const accept_set_type &s) {
    if (key < 0 || key > ORD_MAXCHAR)
        return false;
    return s.contains(key.value());
}

bool charcmd_insert(commands cmd, leadparam rept, int count, bool from_span) {
    bool cmd_status = false;
    if (rept == leadparam::minus)
        rept = leadparam::nint;
    count = std::abs(count);
    //with current_frame^ do
    //with dot^,line^ do
    col_range old_dot_col = current_frame->dot->col;
    strlen_range maximum;
    if (current_frame->dot->col <= current_frame->dot->line->used)
        maximum = MAX_STRLEN - current_frame->dot->line->used;
    else
        maximum = MAX_STRLEN - current_frame->dot->col;

    strlen_range inserted = 0;
    col_range eql_col;
    key_code_range key;
    do {
        bool cmd_valid = count <= maximum;
        if (cmd_valid) {
            maximum -= count;
            inserted += count;
            if (!text_insert(true, 1, BLANK_STRING, count, current_frame->dot))
                goto l9;
            //with dot^ do
            if (rept == leadparam::nint) {
                eql_col = current_frame->dot->col - count;
            } else {
                eql_col = current_frame->dot->col;
                current_frame->dot->col -= count;
            }
            cmd_status = true;
        }
        if (from_span)
            goto l9;
        if (cmd_valid)
            screen_fixup();
        else
            vdu_beep();
        key = vdu_get_key();
        if (tt_controlc)
            goto l9;
        rept = leadparam::none;
        count = 1;
        if (key_in_set(key, PRINTABLE_SET))
            cmd = commands::cmd_noop;
        else
            cmd = lookup[key].command;
    } while (cmd == commands::cmd_insert_char);
    vdu_take_back_key(key);

l9:;
    //with dot^ do
    if (tt_controlc) {
        cmd_status = false;
        current_frame->dot->col = old_dot_col;
        mark_ptr temp_mark = nullptr;
        mark_create(current_frame->dot->line, current_frame->dot->col + inserted, temp_mark);
        text_remove(current_frame->dot, temp_mark);
        mark_destroy(temp_mark);
    } else {
        if (cmd_status) {
            current_frame->text_modified = true;
            mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]);
            mark_create(current_frame->dot->line, eql_col, current_frame->marks[MARK_EQUALS]);
        }
    }
    return cmd_status || !from_span;
}

bool charcmd_delete(commands cmd, leadparam rept, int count, bool from_span) {
    bool cmd_status = false;
    //with current_frame^,dot^,line^ do
    col_range old_dot_col = current_frame->dot->col;
    str_object old_str;
    ch_fillcopy(current_frame->dot->line->str, 1, current_frame->dot->line->used, &old_str, 1, MAX_STRLEN, ' ');
    int deleted = 0;
    key_code_range key;
    do {
        bool cmd_valid = true;
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
        case leadparam::pint:
            if (count > MAX_STRLENP - current_frame->dot->col)
                cmd_valid = false;
            break;
        case leadparam::pindef:
            count = MAX_STRLENP - current_frame->dot->col;
            break;
        case leadparam::minus:
        case leadparam::nint:
            count = -count;
            if (count < current_frame->dot->col)
                current_frame->dot->col -= count;
            else
                cmd_valid = false;
            break;
        case leadparam::nindef:
            count = current_frame->dot->col - 1;
            current_frame->dot->col = 1;
            break;
        default:
            // Nothing
            break;
        }
        if (cmd_valid) {
            // Update the text of the line.
            strlen_range old_used = current_frame->dot->line->used;
            int length = (current_frame->dot->line->used + 1) - (current_frame->dot->col + count);
            if (length > 0) {
                line_ptr l = current_frame->dot->line;
                const auto &dotcol = current_frame->dot->col;
                l->str->erase(count, dotcol);
                l->str->fill_n(' ', count, l->used + 1 - count);
                l->used -= count;
            } else if (current_frame->dot->col <= current_frame->dot->line->used) {
                mark_ptr d = current_frame->dot;
                d->line->str->fill_n(' ', d->line->used + 1 - d->col, d->col);
                d->line->used = d->line->str->length(' ', d->col);
            }
            // Update the screen.
            int scr_col = current_frame->dot->col - current_frame->scr_offset;
            if ((current_frame->dot->line->scr_row_nr != 0) && (count != 0) &&
                (current_frame->dot->col <= old_used) && (scr_col <= current_frame->scr_width)) {
                if (scr_col <= 0)
                    scr_col = 1;
                vdu_movecurs(scr_col, current_frame->dot->line->scr_row_nr);
                length = current_frame->scr_width + 1 - scr_col;
                if (count < length) {
                    length = count;
                    vdu_deletechars(count);
                } else {
                    vdu_cleareol();
                }
                int first_col = current_frame->scr_offset + current_frame->scr_width + 1 - length;
                if (first_col <= current_frame->dot->line->used) { // If non-blank then redraw area.
                    vdu_movecurs(current_frame->scr_width + 1 - length, current_frame->dot->line->scr_row_nr);
                    if (length > current_frame->dot->line->used + 1 - first_col)
                        length = current_frame->dot->line->used + 1 - first_col;
                    vdu_displaystr(length, current_frame->dot->line->str->data(first_col), 3);
                }
            }
            deleted += count;
            cmd_status = true;
        }
        if (from_span)
            goto l9;
        if (cmd_valid)
            screen_fixup();
        else
            vdu_beep();
        key = vdu_get_key();
        if (tt_controlc)
            goto l9;
        rept = leadparam::none;
        count = 1;
        if (key_in_set(key, PRINTABLE_SET))
            cmd = commands::cmd_noop;
        else
            cmd = lookup[key].command;
        if ((cmd == commands::cmd_rubout) && (edit_mode == mode_type::mode_insert)) {
            // In insert_mode treat RUBOUT as \-D
            rept  = leadparam::minus;
            count = -1;
            cmd   = commands::cmd_delete_char;
        }
    } while (cmd == commands::cmd_delete_char);
    vdu_take_back_key(key);

l9:;
    if (tt_controlc) {
        cmd_status = false;
        current_frame->dot->col = 1;
        text_overtype(false, 1, old_str, MAX_STRLEN, current_frame->dot);
        current_frame->dot->col = old_dot_col;
    } else if (cmd_status) {
        old_dot_col = current_frame->dot->col;
        count = MAX_STRLENP - old_dot_col;
        if (deleted > count)
            deleted = count;
        line_ptr line = current_frame->dot->line;
        marks_squeeze(line, old_dot_col, line, old_dot_col + deleted);
        marks_shift(line, old_dot_col + deleted, MAX_STRLENP - (old_dot_col + deleted)+1, line, old_dot_col);
        current_frame->text_modified = true;
        mark_create(line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]);
        if (current_frame->marks[MARK_EQUALS] != nullptr)
            mark_destroy(current_frame->marks[MARK_EQUALS]);
    }
    return cmd_status || !from_span;
}

bool charcmd_rubout(commands cmd, leadparam rept, int count, bool from_span) {
    bool cmd_status;
    if (edit_mode == mode_type::mode_insert) {
        if (rept == leadparam::pindef)
            rept = leadparam::nindef;
        else
            rept = leadparam::nint;
        cmd_status = charcmd_delete(commands::cmd_delete_char, rept, -count, from_span);
    } else {
        cmd_status = false;
        //with current_frame^,dot^ do
        col_range old_dot_col = current_frame->dot->col;
        strlen_range dot_used = current_frame->dot->line->used;
        str_object old_str;
        ch_fillcopy(current_frame->dot->line->str, 1, dot_used, &old_str, 1, MAX_STRLEN, ' ');
        key_code_range key;
        col_range eql_col;
        do {
            if (rept == leadparam::pindef)
                count = current_frame->dot->col - 1;
            bool cmd_valid = (count <= current_frame->dot->col - 1);
            if (cmd_valid) {
                eql_col = current_frame->dot->col;
                current_frame->dot->col -= count;
                if (!text_overtype(true, 1, BLANK_STRING, count, current_frame->dot))
                    goto l9;
// Comment retained for posterity
//{#if ns32000}
//{##          col = dot->col-count;   #<Multimax bug workaround#>}
//{#else}
                current_frame->dot->col -= count;
//{#endif}
                cmd_status = true;
            }
            if (from_span)
                goto l9;
            if (cmd_valid)
                screen_fixup();
            else
                vdu_beep();
            key = vdu_get_key();
            if (tt_controlc)
                goto l9;
            rept  = leadparam::none;
            count = 1;
            if (key_in_set(key, PRINTABLE_SET))
                cmd = commands::cmd_noop;
            else
                cmd = lookup[key].command;
        } while (cmd == commands::cmd_rubout);
        vdu_take_back_key(key);

l9:;
        if (tt_controlc) {
            cmd_status = false;
            current_frame->dot->col = 1;
            text_overtype(false, 1, old_str, dot_used, current_frame->dot);
            current_frame->dot->col = old_dot_col;
        } else if (cmd_status) {
            current_frame->text_modified = true;
            mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]);
            mark_create(current_frame->dot->line, eql_col, current_frame->marks[MARK_EQUALS]);
        }
    }
    return cmd_status || !from_span;
}
