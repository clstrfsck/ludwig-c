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
!**/

#include "mark.h"

#include "screen.h"
#include "var.h"

size_t mark_object::allocated_marks = 0;

namespace {

    mark_ptr allocate() {
        return std::make_shared<mark_object>();
    }

    void deallocate(mark_ptr &mark) {
        mark = nullptr;
    }

    void remove_from_marks(std::list<mark_ptr> &mark_list, mark_ptr mark) {
        auto imark = std::ranges::find(mark_list, mark);
        if (imark != mark_list.end()) {
            mark_list.erase(imark);
        }
    }
} // namespace

size_t marks_allocated() {
    return mark_object::allocated_marks;
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
        mark = allocate();
        in_line->marks.push_front(mark);
        mark->line = in_line;
        mark->col = column;
    } else {
        line_ptr this_line = mark->line;
        if (this_line == in_line) {
            mark->col = column;
        } else {
            remove_from_marks(this_line->marks, mark);
            in_line->marks.push_front(mark);
            mark->line = in_line;
            mark->col = column;
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
    remove_from_marks(mark->line->marks, mark);
    deallocate(mark);
    return true;
}

bool marks_squeeze(
    const line_ptr first_line,
    col_range first_column,
    const line_ptr last_line,
    col_range last_column
) {
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
        for (auto &this_mark : last_line->marks) {
            if ((this_mark->col >= first_column) && (this_mark->col < last_column)) {
                this_mark->col = last_column;
            }
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
        for (auto &this_mark : last_line->marks) {
            this_mark->col = std::max(this_mark->col, int(last_column));
        }
        // Move marks in lines first_line..last_line-1.
        line_ptr this_line = first_line;
        while (this_line != last_line) {
            auto &marks{this_line->marks};
            for (auto it = marks.begin(); it != marks.end();) {
                auto mark{*it};
                if (mark->col >= first_column) {
                    mark->col = last_column;
                    mark->line = last_line;
                    last_line->marks.push_front(mark);
                    it = marks.erase(it);
                } else {
                    ++it;
                }
            }
            this_line = this_line->flink;
            first_column = 1;
        }
    }
    return true;
}

bool marks_shift(
    const line_ptr source_line,
    col_range source_column,
    col_width_range width,
    const line_ptr dest_line,
    col_range dest_column
) {
    /*
      Purpose  : Move all marks from the <width> columns starting at
                 <source_line,source_column> to corresponding positions
                 starting from <dest_line,dest_column>.
      Inputs   : source_line, source_column: location of the source range.
                 width: the size of the range.
                 dest_line, dest_column: location of the destination range.
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
        for (auto &mark : source_line->marks) {
            if ((mark->col >= source_column) && (mark->col <= source_end)) {
                mark->col = std::min(MAX_STRLENP, mark->col + offset);
            }
        }
    } else {
#ifdef DEBUG
        if (source_line->group->frame != dest_line->group->frame) {
            screen_message(DBG_LINES_FROM_DIFF_FRAMES);
            return false;
        }
#endif
        auto &marks{source_line->marks};
        for (auto it = marks.begin(); it != marks.end();) {
            auto mark{*it};
            if ((mark->col >= source_column) && (mark->col <= source_end)) {
                mark->line = dest_line;
                mark->col = std::min(MAX_STRLENP, mark->col + offset);
                dest_line->marks.push_front(mark);
                it = marks.erase(it);
            } else {
                ++it;
            }
        }
    }
    return true;
}
