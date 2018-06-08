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
! Name:         VALIDATE
!
! Description:  Validation of entire Ludwig data structure.
*/

#include "validate.h"

#include "var.h"
#include "screen.h"

bool validate_command() {
    /*
      Purpose  : Validate the data structure.
      Inputs   : none.
      Outputs  : none.
      Bugchecks: .lots and lots of them!
    */
    const int OOPS = 0x0001;
    const int CMD  = 0x0002;
    const int HEAP = 0x0004;

#ifdef DEBUG
    if ((current_frame == nullptr) || (frame_oops == nullptr) || (frame_cmd == nullptr) || (frame_heap == nullptr)) {
        screen_message(DBG_INVALID_FRAME_PTR);
        return false;
    }

    if (first_span == nullptr) {
        screen_message(DBG_INVALID_SPAN_PTR);
        //???
        return false;
    }

    // Validate the data structure.
    int frame_list = 0; // Bit mask OOPS, CMD, HEAP
    scr_row_range scr_row = 0;
    span_ptr this_span = first_span;
    span_ptr prev_span = nullptr;
    while (this_span != nullptr) {
        //with this_span^ do
        if (this_span->blink != prev_span) {
            screen_message(DBG_INVALID_BLINK);
            return false;
        }
        if ((this_span->mark_one == nullptr) || (this_span->mark_two == nullptr)) {
            screen_message(DBG_MARK_PTR_IS_NIL);
            return false;
        }
        if (this_span->code != nullptr) {
            if (this_span->code->ref == 0) {
                screen_message(DBG_REF_COUNT_IS_ZERO);
                return false;
            }
        }
        if (this_span->frame != nullptr) {
            frame_ptr this_frame = this_span->frame;
            if (this_frame == frame_cmd)
                frame_list |= CMD;
            else if (this_frame == frame_oops)
                frame_list |= OOPS;
            if (this_frame == frame_heap)
                frame_list |= HEAP;
            //with this_frame^ do
            if ((this_frame->first_group == nullptr) || (this_frame->last_group == nullptr)) {
                screen_message(DBG_INVALID_GROUP_PTR);
                return false;
            }
            if (this_frame->first_group->blink != nullptr) {
                screen_message(DBG_FIRST_NOT_AT_TOP);
                return false;
            }
            group_ptr end_group = this_frame->last_group->flink;
            if (end_group != nullptr) {
                screen_message(DBG_LAST_NOT_AT_END);
                return false;
            }
            group_ptr  this_group = this_frame->first_group;
            group_ptr  prev_group = nullptr;
            line_ptr   this_line  = this_frame->first_group->first_line;
            line_ptr   prev_line  = nullptr;
            line_ptr   end_line   = nullptr;
            line_range line_nr    = 1;
            while (this_group != end_group) {
                //with this_group^ do
                if (this_group->blink != prev_group) {
                    screen_message(DBG_INVALID_BLINK);
                    return false;
                }
                if (this_group->frame != this_frame) {
                    screen_message(DBG_INVALID_FRAME_PTR);
                    return false;
                }
                if ((this_group->first_line == nullptr) || (this_group->last_line == nullptr)) {
                    screen_message(DBG_LINE_PTR_IS_NIL);
                    return false;
                }
                if (this_group->first_line != this_line) {
                    screen_message(DBG_FIRST_NOT_AT_TOP);
                    return false;
                }
                group_line_range line_count = 0;
                end_line = this_group->last_line->flink;
                while (this_line != end_line) {
                    //with this_line^ do
                    if (this_line->blink != prev_line) {
                        screen_message(DBG_INVALID_BLINK);
                        return false;
                    }
                    if (this_line->group != this_group) {
                        screen_message(DBG_INVALID_GROUP_PTR);
                        return false;
                    }
                    if (this_line->offset_nr != line_count) {
                        screen_message(DBG_INVALID_OFFSET_NR);
                        return false;
                    }
                    mark_ptr this_mark = this_line->mark;
                    while (this_mark != nullptr) {
                        //with this_mark^ do
                        if (this_mark->line != this_line) {
                            screen_message(DBG_INVALID_LINE_PTR);
                            return false;
                        }
                        this_mark = this_mark->next;
                    }
                    if ((this_line->str == nullptr) && (this_line->len != 0)) {
                        screen_message(DBG_INVALID_LINE_LENGTH);
                        return false;
                    }
                    if (this_line->used > this_line->len) {
                        screen_message(DBG_INVALID_LINE_USED_LENGTH);
                        return false;
                    }
                    if (this_line->scr_row_nr != scr_row) {
                        if (this_line == scr_top_line) {
                            scr_row = this_line->scr_row_nr;
                        } else {
                            screen_message(DBG_INVALID_SCR_ROW_NR);
                            return false;
                        }
                    }
                    if (scr_row != 0) {
                        if (this_line != scr_bot_line)
                            scr_row += 1;
                        else
                            scr_row = 0;
                    }
                    line_count += 1;
                    prev_line = this_line;
                    this_line = this_line->flink;
                }
                if (this_group->last_line != prev_line) {
                    screen_message(DBG_LAST_NOT_AT_END);
                    return false;
                }
                if (this_group->first_line_nr != line_nr) {
                    screen_message(DBG_INVALID_LINE_NR);
                    return false;
                }
                if (this_group->nr_lines != line_count) {
                    screen_message(DBG_INVALID_NR_LINES);
                    return false;
                }
                line_nr = line_nr + this_group->nr_lines;
                prev_group = this_group;
                this_group = this_group->flink;
            }
            if (this_frame->first_group->first_line->blink != nullptr) {
                screen_message(DBG_FIRST_NOT_AT_TOP);
                return false;
            }
            if (end_line != nullptr) {
                screen_message(DBG_LAST_NOT_AT_END);
                return false;
            }
            if (this_frame->dot == nullptr) {
                screen_message(DBG_MARK_PTR_IS_NIL);
                return false;
            }
            if (this_frame->dot->line->group->frame != this_frame) {
                screen_message(DBG_MARK_IN_WRONG_FRAME);
                return false;
            }
            for (mark_range mark_nr = MIN_MARK_NUMBER; mark_nr <= MAX_MARK_NUMBER; ++mark_nr) {
                if (this_frame->marks[mark_nr] != nullptr) {
                    if (this_frame->marks[mark_nr]->line->group->frame != this_frame) {
                        screen_message(DBG_MARK_IN_WRONG_FRAME);
                        return false;
                    }
                }
            }
            if ((this_frame->scr_height == 0) || (this_frame->scr_height > terminal_info.height)) {
                screen_message(DBG_INVALID_SCR_PARAM);
                return false;
            }
            if ((this_frame->scr_width == 0) || (this_frame->scr_width > terminal_info.width)) {
                screen_message(DBG_INVALID_SCR_PARAM);
                return false;
            }
            if (this_frame->span != this_span) {
                screen_message(DBG_INVALID_SPAN_PTR);
                return false;
            }
            if (this_frame->margin_left >= this_frame->margin_right) {
                screen_message(MSG_LEFT_MARGIN_GE_RIGHT);
                return false;
            }
            if ((this_span->mark_one->line->group->frame != this_frame) ||
                (this_span->mark_two->line->group->frame != this_frame)) {
                screen_message(DBG_MARK_IN_WRONG_FRAME);
                return false;
            }
        } else if (this_span->mark_one->line->group->frame != this_span->mark_two->line->group->frame) {
            screen_message(DBG_MARKS_FROM_DIFF_FRAMES);
            return false;
        }
        prev_span = this_span;
        this_span = this_span->flink;
    }
    if (frame_list != (CMD | OOPS | HEAP)) {
        screen_message(DBG_NEEDED_FRAME_NOT_FOUND);
        return false;
    }
#endif
    return true;
}
