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
! Name:         MARK
!
! Description:  Mark manipulation routines.
!
! $Log: mark.pas,v $
! Revision 4.5  1990/01/18 17:46:11  ludwig
! Entered into RCS at revision level 4.5
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-003 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-004 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-005 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!       Remove the superfluous include of system.h.
!**/

#include "mark.h"

#include "var.h"
#include "screen.h"

void mark_mark_pool_extend() {
    for (int i = 0; i < 20; ++i) {
        mark_ptr new_mark = new mark_object;
        new_mark->next = free_mark_pool;
        free_mark_pool = new_mark;
    }
}

bool mark_create(line_ptr in_line, col_range column, mark_ptr &mark) {
    /*
      Purpose  : Create or move a mark.
      Inputs   : line: the line to be marked.
                 column: the column in the line to be marked.
                 mark: nil, or a pointer to an existing mark.
      Outputs  : mark: a pointer to the created mark.
      Bugchecks: .in_line pointer is nil
    */

#ifdef DEBUG
    if (in_line == nullptr) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
#endif
    if (mark == nullptr) {
        if (free_mark_pool == nullptr)
            mark_mark_pool_extend();
        mark           = free_mark_pool;
        free_mark_pool = mark->next;

        mark->next = in_line->mark;
        mark->line = in_line;
        mark->col  = column;
        in_line->mark = mark;
    } else {
        line_ptr this_line = mark->line;
        if (this_line == in_line) {
            mark->col = column;
        } else {
            mark_ptr this_mark = this_line->mark;
            mark_ptr prev_mark = nullptr;
            while (this_mark != mark) {
                prev_mark = this_mark;
                this_mark = this_mark->next;
            }
            if (prev_mark == nullptr)
                this_line->mark = this_mark->next;
            else
                prev_mark->next = this_mark->next;
            mark->next = in_line->mark;
            mark->line = in_line;
            mark->col  = column;
            in_line->mark = mark;
        }
    }
    return true;
}

bool mark_destroy(mark_ptr &mark) {
    /*
      Purpose  : Destroy a mark.
      Inputs   : mark: the mark to be destroyed.
      Outputs  : mark: nil.
      Bugchecks: .mark pointer is nil
    */

#ifdef DEBUG
    if (mark == nullptr) {
        screen_message(DBG_MARK_PTR_IS_NIL);
        return false;
    }
#endif
    line_ptr this_line = mark->line;
    mark_ptr this_mark = this_line->mark;
    mark_ptr prev_mark = nullptr;
    while (this_mark != mark) {
        prev_mark = this_mark;
        this_mark = this_mark->next;
    }
    if (prev_mark == nullptr)
        this_line->mark = this_mark->next;
    else
        prev_mark->next = this_mark->next;
    mark->next     = free_mark_pool;
    free_mark_pool = mark;
    mark           = nullptr;
    return true;
}

bool marks_squeeze(line_ptr first_line, col_range first_column, line_ptr last_line, col_range last_column) {
    /*
      Purpose  : Move all marks between <first_line,first_column> and
                 <last_line,last_column> to the latter position.
      Inputs   : first_line, first_column: the beginning of the range.
                 last_line, last_column: the end of the range.
      Outputs  : none.
      Bugchecks: .first or last line pointer is nil
                 .first column > last column when in same line
                 .first and last lines in different frames
                 .first line > last line
    */

#ifdef DEBUG
    if ((first_line == nullptr) || (last_line == nullptr)) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
#endif
    if (first_line == last_line) {
#ifdef DEBUG
        if (first_column > last_column) {
            screen_message(DBG_FIRST_FOLLOWS_LAST);
            return false;
        }
#endif
        mark_ptr this_mark = last_line->mark;
        while (this_mark != nullptr) {
            //with this_mark^ do
            if ((this_mark->col >= first_column) && (this_mark->col < last_column))
                this_mark->col = last_column;
            this_mark = this_mark->next;
        }
    } else {
#ifdef DEBUG
        if (first_line->group->frame != last_line->group->frame) {
            screen_message(DBG_LINES_FROM_DIFF_FRAMES);
            return false;
        }
        if (first_line->group->first_line_nr + first_line->offset_nr >
            last_line->group->first_line_nr + last_line->offset_nr) {
            screen_message(DBG_FIRST_FOLLOWS_LAST);
            return false;
        }
#endif
        // Move marks in last_line.
        mark_ptr this_mark = last_line->mark;
        while (this_mark != nullptr) {
            if (this_mark->col < last_column)
                this_mark->col = last_column;
            this_mark = this_mark->next;
        }
        // Move marks in lines first_line..last_line-1.
        line_ptr this_line = first_line;
        while (this_line != last_line) {
            this_mark = this_line->mark;
            mark_ptr prev_mark = nullptr;
            while (this_mark != nullptr) {
                // with this_mark^ do
                mark_ptr next_mark = this_mark->next;
                if (this_mark->col >= first_column) {
                    if (prev_mark == nullptr)
                        this_line->mark = this_mark->next;
                    else
                        prev_mark->next = this_mark->next;
                    this_mark->next = last_line->mark;
                    this_mark->line = last_line;
                    this_mark->col = last_column;
                    last_line->mark = this_mark;
                } else {
                    prev_mark = this_mark;
                }
                this_mark = next_mark;
            }
            this_line = this_line->flink;
            first_column = 1;
        }
    }
    return true;
}

bool marks_shift(line_ptr source_line, col_range source_column, col_width_range width,
                 line_ptr dest_line, col_range dest_column) {
    /*
      Purpose  : Move all marks from the <width> columns starting at
                 <source_line,source_column> to corresponding positions
                 starting from <dest_line,dest_column>.
      Inputs   : source_line, source_column: location of the source range.
                 width: the size of the range.
                 last_line, last_column: location of the destination range.
      Outputs  : none.
      Bugchecks: .source or dest line pointer is nil
                 .source and dest lines in different frames
                 .source ranges exceed maximum line length
    */

#ifdef DEBUG
    if ((source_line == nullptr) || (dest_line == nullptr)) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
    if (width == 0) {
        screen_message(DBG_INVALID_OFFSET_NR);
        return false;
    }
    if (source_column + width - 1 > MAX_STRLENP) {
        screen_message(DBG_INVALID_COLUMN_NUMBER);
        return false;
    }
#endif
    col_range source_end = source_column + width - 1;
    int offset = dest_column - source_column;
    if (source_line == dest_line) {
        mark_ptr this_mark = source_line->mark;
        while (this_mark != nullptr) {
            //with this_mark^ do
            if ((this_mark->col >= source_column) && (this_mark->col <= source_end)) {
                int new_col = this_mark->col + offset;
                if (new_col >= MAX_STRLENP)
                    this_mark->col = MAX_STRLENP;
                else
                    this_mark->col = new_col;
            }
            this_mark = this_mark->next;
        }
    } else {
#ifdef DEBUG
        if (source_line->group->frame != dest_line->group->frame) {
            screen_message(DBG_LINES_FROM_DIFF_FRAMES);
            return false;
        }
#endif
        mark_ptr this_mark = source_line->mark;
        mark_ptr prev_mark = nullptr;
        while (this_mark != nullptr) {
            //with this_mark^ do
            mark_ptr next_mark = this_mark->next;
            if ((this_mark->col >= source_column) && (this_mark->col <= source_end)) {
                if (prev_mark == nullptr)
                    source_line->mark = this_mark->next;
                else
                    prev_mark->next = this_mark->next;
                this_mark->next = dest_line->mark;
                this_mark->line = dest_line;
                int new_col = this_mark->col + offset;
                if (new_col >= MAX_STRLENP)
                    this_mark->col = MAX_STRLENP;
                else
                    this_mark->col = new_col;
                dest_line->mark = this_mark;
            } else {
                prev_mark = this_mark;
            }
            this_mark = next_mark;
        }
    }
    return true;
}

