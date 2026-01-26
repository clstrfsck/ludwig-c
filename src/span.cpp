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
! Name:         SPAN
!
! Description:  Creation/destruction, manipulation of spans.
!**/

#include "span.h"

#include "var.h"
#include "code.h"
#include "fyle.h"
#include "line.h"
#include "mark.h"
#include "screen.h"

bool span_find(const std::string_view &span_name, span_ptr &ptr, span_ptr &oldp) {
    /****************************************************************************
     *    D E S C R I P T I O N :-                                              *
     * Input   : span_name                                                      *
     * Output  : ptr                                                            *
     * Purpose : Finds a span of the specified name.  Returns a pointer to the  *
     *           entry if the span table if found.  If the span is a frame then *
     *           reset the frame's two marks.                                   *
     * Errors  : Fails if span not found.                                       *
     ****************************************************************************/

    oldp = nullptr;
    ptr  = first_span;
    if (ptr == nullptr)
        return false;
    while (ptr->name < span_name) {
        oldp = ptr;
        ptr  = ptr->flink;
        if (ptr == nullptr)
            return false;
    }
    //with ptr^ do
    if (ptr->name == span_name) {
        if (ptr->frame != nullptr) {
            //with frame^ do
            if (!mark_create(ptr->frame->first_group->first_line, 1, ptr->mark_one) ||
                !mark_create(ptr->frame->last_group ->last_line,  1, ptr->mark_two))
                return false;
        }
        return true;
    }
    return false;
}

bool span_create(const std::string_view &span_name, mark_ptr first_mark, mark_ptr last_mark) {
    /****************************************************************************
     *    D E S C R I P T I O N :-                                              *
     * Input   : span_name, first_mark & last_mark                              *
     * Purpose : Creates a span of the specified name over the specified        *
     *           range of lines. Checks first that a span of this name doesn't  *
     *           already exist. If it does, it re-defines it                    *
     * Errors  : Fails if span already exists and is a frame                    *
     ****************************************************************************/

    span_ptr p;
    span_ptr ptr;
    span_ptr oldp;
    mark_ptr mrk1;
    mark_ptr mrk2;
    if (span_find(span_name, p, oldp)) {
        if (p->frame != nullptr) {
            screen_message(MSG_FRAME_OF_THAT_NAME_EXISTS);
            return false;
        }
        ptr = p;
        //with ptr^ do
        if (ptr->code != nullptr)
            code_discard(ptr->code);
        mrk1 = ptr->mark_one;
        mrk2 = ptr->mark_two;
    } else {
        mrk1 = nullptr;
        mrk2 = nullptr;
        ptr = new span_object;
        //with ptr^ do
        ptr->name = span_name;
        ptr->code = nullptr;
        // Now hook the span into the span structure
        if (p == nullptr) {
            ptr->flink = nullptr;
        } else {
            ptr->flink = p;
            p->blink = ptr;
        }
        if (oldp == nullptr) {
            ptr->blink = nullptr;
            first_span = ptr;
        } else {
            ptr->blink = oldp;
            oldp->flink = ptr;
        }
    }
    //with first_mark^ do
    if (!mark_create(first_mark->line, first_mark->col, mrk1))
        return false;
    //with last_mark^
    if (!mark_create(last_mark->line, last_mark->col, mrk2))
        return false;
    line_range line_nr_first;
    line_range line_nr_last;
    if (line_to_number(mrk1->line, line_nr_first) && line_to_number(mrk2->line, line_nr_last)) {
        //with ptr^ do
        ptr->frame    = nullptr;
        if ((line_nr_first < line_nr_last) ||
            ((line_nr_first == line_nr_last) && (mrk1->col < mrk2->col))) {
            // Marks are in the right order
            ptr->mark_one = mrk1;
            ptr->mark_two = mrk2;
        } else {
            // Marks are in reverse order
            ptr->mark_one = mrk2;
            ptr->mark_two = mrk1;
        }
        return true;
    }
    return false;
}

bool span_destroy(span_ptr &span) {
    /****************************************************************************
     *    D E S C R I P T I O N :-                                              *
     * Input   : span                                                           *
     * Output  : span set to nil                                                *
     * Purpose : Destroys the specified span.                                   *
     * Errors  : Fails if span is not destroyed.                                *
     ****************************************************************************/
    //with span^ do
    if (span->frame != nullptr) {
        screen_message(MSG_CANT_KILL_FRAME);
        return false;
    }
    if (span->code != nullptr)
        code_discard(span->code);
    if (span->blink != nullptr)
        span->blink->flink = span->flink;
    else
        first_span = span->flink;
    if (span->flink != nullptr)
        span->flink->blink = span->blink;
    if (mark_destroy(span->mark_one)) {
        if (mark_destroy(span->mark_two)) {
            delete span;
            span = nullptr;
            return true;
        }
    }
    return false;
}

bool span_index() {
    /****************************************************************************
     *    D E S C R I P T I O N :-                                              *
     * Input   : <none>                                                         *
     * Output  : <none>                                                         *
     * Purpose : Displays the list of spans. This is the \SI command.           *
     * Errors  : Fails something terrible happens.                              *
     ****************************************************************************/

    screen_unload();
    screen_home(true);
    span_ptr p = first_span;
    bool have_span = false;
    screen_writeln();
    screen_write_str(0, "Spans", 5);
    screen_writeln();
    screen_write_str(0, "=====", 5);
    screen_writeln();
    int line_count = 3;
    while (p != nullptr) {
        if (p->frame == nullptr) {
            have_span = true;
            if (line_count > terminal_info.height - 2) {
                screen_pause();
                screen_home(true);
                screen_writeln();
                screen_write_str(0, "Spans", 5);
                screen_writeln();
                screen_write_str(0, "=====", 5);
                screen_writeln();
                line_count = 3;
            }
            screen_write_name_str(0, p->name, NAME_LEN);
            screen_write_str(0, " : ", 3);
            //with p^,mark_one^,line^ do
            std::string span_start;
            bool continu = p->mark_one->line != p->mark_two->line;
            if (p->mark_one->col <= p->mark_one->line->used) {
                if (!continu) {
                    continu = p->mark_two->col - p->mark_one->col > NAME_LEN;
                    size_t to_copy = std::min(NAME_LEN, p->mark_two->col - p->mark_one->col);
                    span_start.assign(p->mark_one->line->str->data(p->mark_one->col), to_copy);
                } else {
                    size_t to_copy = std::min(NAME_LEN, p->mark_one->line->used + 1 - p->mark_one->col);
                    span_start.assign(p->mark_one->line->str->data(p->mark_one->col), to_copy);
                }
            }
            screen_write_name_str(0, span_start, NAME_LEN);
            if (continu)
                screen_write_str(1, "...", 3);
            screen_writeln();
            line_count += 1;
        }
        p = p->flink;
    }
    if (!have_span) {
        screen_write_str(10, "<none>", 6);
        screen_writeln();
        line_count += 1;
    }
    bool first_time = true;
    int  old_count  = line_count;
    p          = first_span;
    line_count = terminal_info.height;
    while (p != nullptr) {
        if (p->frame != nullptr) {
            if (line_count > terminal_info.height - 2) {
                if (!first_time) {
                    screen_pause();
                    screen_home(true);
                }
                screen_writeln();
                screen_write_str(0, "Frames", 6);
                screen_writeln();
                screen_write_str(0, "======", 6);
                screen_writeln();
                if (first_time) {
                    line_count = old_count + 3;
                    first_time = false;
                } else {
                    line_count = 3;
                }
            }
            screen_write_name_str(0, p->name, NAME_LEN);
            screen_writeln();
            line_count += 1;
            //with p->frame^ do
            file_name_str fyl_nam;
            if (p->frame->input_file != 0) {
                screen_write_str(0, "  Input:  ", 10);
                file_name(files[p->frame->input_file], 70, fyl_nam);
                screen_write_file_name_str(0, fyl_nam, fyl_nam.size());
                screen_writeln();
                line_count += 1;
            }
            if (p->frame->output_file != 0) {
                screen_write_str(0, "  Output: ", 10);
                file_name(files[p->frame->output_file], 70, fyl_nam);
                screen_write_file_name_str(0, fyl_nam, fyl_nam.size());
                screen_writeln();
                line_count += 1;
            }
        }
        p = p->flink;
    }
    screen_pause();
    return true; // It can never fail....
}
