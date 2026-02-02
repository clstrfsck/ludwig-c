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
! Name:         LINE
!
! Description:  Line manipulation commands.
!**/

#include "line.h"

#include "ch.h"
#include "screen.h"
#include "var.h"

//----------------------------------------------------------------------

// The Two Following routines maintain a Pool of free Lines/Groups.
// This should enhance the Page Fault performance by keeping all
// the Lines/Groups together in clusters.

void line_line_pool_extend() {
    for (int i = 0; i < 20; ++i) {
        line_ptr new_line = new line_hdr_object;
        new_line->flink = free_line_pool;
        free_line_pool = new_line;
    }
}

void line_group_pool_extend() {
    for (int i = 0; i < 20; ++i) {
        group_ptr new_group = new group_object;
        new_group->flink = free_group_pool;
        free_group_pool = new_group;
    }
}

bool line_eop_create(frame_ptr inframe, group_ptr &group) {
    /*
      Purpose  : Create a group containing only the EOP line.
      Inputs   : inframe: pointer to the frame to contain the new group.
      Outputs  : group: pointer to the created group.
      Bugchecks: none.
    */
    if (free_line_pool == nullptr)
        line_line_pool_extend();
    line_ptr new_line = free_line_pool;
    free_line_pool = new_line->flink;
    if (free_group_pool == nullptr)
        line_group_pool_extend();
    group_ptr new_group = free_group_pool;
    free_group_pool = new_group->flink;

    new_line->flink = nullptr;
    new_line->blink = nullptr;
    new_line->group = new_group;
    new_line->offset_nr = 0;
    new_line->marks.clear();
    new_line->str = nullptr;
    new_line->len = 0;
    new_line->used = 0;
    new_line->scr_row_nr = 0;

    new_group->flink = nullptr;
    new_group->blink = nullptr;
    new_group->frame = inframe;
    new_group->first_line = new_line;
    new_group->last_line = new_line;
    new_group->first_line_nr = 1;
    new_group->nr_lines = 1;

    group = new_group;
    return true;
}

bool line_eop_destroy(group_ptr &group) {
    /*
      Purpose  : Destroy a group containing only the EOP line.
      Inputs   : group: pointer to the group to be destroyed.
      Outputs  : group: nil.
      Bugchecks: .group_ptr is nil
                 .group has forward or backward link
                 .group contains non-eop lines
                 .line has forward or backward link
                 .line has incorrect group pointer
                 .line has incorrect line number
                 .line has marks
                 .line is on the screen
    */

#ifdef DEBUG
    if (group == nullptr) {
        screen_message(DBG_GROUP_PTR_IS_NIL);
        return false;
    }
#endif
    group_ptr this_group = group;
#ifdef DEBUG
    if (this_group->flink != nullptr || this_group->blink != nullptr) {
        screen_message(DBG_FLINK_OR_BLINK_NOT_NIL);
        return false;
    }
    if ((this_group->first_line != this_group->last_line) || (this_group->nr_lines != 1)) {
        screen_message(DBG_GROUP_HAS_LINES);
        return false;
    }
#endif
    line_ptr eop_line = group->first_line;
#ifdef DEBUG
    if ((eop_line->flink != nullptr) || (eop_line->blink != nullptr)) {
        screen_message(DBG_FLINK_OR_BLINK_NOT_NIL);
        return false;
    }
    if (eop_line->group != this_group) {
        screen_message(DBG_INVALID_GROUP_PTR);
        return false;
    }
    if (eop_line->offset_nr != 0) {
        screen_message(DBG_INVALID_OFFSET_NR);
        return false;
    }
    if (!eop_line->marks.empty()) {
        screen_message(DBG_LINE_HAS_MARKS);
        return false;
    }
    if (eop_line->scr_row_nr != 0) {
        screen_message(DBG_LINE_ON_SCREEN);
        return false;
    }
#endif
    if (eop_line->str != nullptr) {
        delete eop_line->str;
        eop_line->marks.clear();
#ifdef DEBUG
        eop_line->str = nullptr;
#endif
    }
    eop_line->flink = free_line_pool;
    free_line_pool = eop_line;
    this_group->flink = free_group_pool;
    free_group_pool = this_group;
    group = nullptr;
    return true;
}

bool lines_create(line_range line_count, line_ptr &first_line, line_ptr &last_line) {
    /*
      Purpose  : Create a linked list of lines.
      Inputs   : line_count: the number of lines to create.
      Outputs  : first_line, last_line: pointers to the created lines.
      Bugchecks: None.
    */

    line_ptr top_line = nullptr;
    line_ptr prev_line = nullptr;
    line_ptr this_line = nullptr;
    for (line_range line_nr = 1; line_nr <= line_count; ++line_nr) {
        if (free_line_pool == nullptr)
            line_line_pool_extend();
        this_line = free_line_pool;
        free_line_pool = this_line->flink;
        if (top_line == nullptr)
            top_line = this_line;
        this_line->flink = nullptr;
        this_line->blink = prev_line;
        this_line->group = nullptr;
        this_line->offset_nr = 0;
        this_line->marks.clear();
        this_line->str = nullptr;
        this_line->len = 0;
        this_line->used = 0;
        this_line->scr_row_nr = 0;
        if (prev_line != nullptr)
            prev_line->flink = this_line;
        prev_line = this_line;
    }
    first_line = top_line;
    last_line = this_line;
    return true;
}

bool lines_destroy(line_ptr &first_line, line_ptr &last_line) {
    /*
      Purpose  : Destroy a linked list of lines.
                 The lines must have been extracted from the data structure
                 before this routine is called.
      Inputs   : first_line, last_line: pointers to the lines to be destroyed.
      Outputs  : first_line, last_line: nullptr.
      Bugchecks: .first_line or last_line is nullptr
                 .first_line/last_line contains back/forward link
                 .back links not correct
                 .line has a group pointer
                 .line has a line number
                 .line has marks
                 .line is on the screen
                 .last_line is not the last line in the linked list
    */

#ifdef DEBUG
    if ((first_line == nullptr) || (last_line == nullptr)) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
    if ((first_line->blink != nullptr) || (last_line->flink != nullptr)) {
        screen_message(DBG_FLINK_OR_BLINK_NOT_NIL);
        return false;
    }
    const_line_ptr prev_line = nullptr;
#endif
    line_ptr this_line = first_line;
    while (this_line != nullptr) {
#ifdef DEBUG
        if (this_line->blink != prev_line) {
            screen_message(DBG_INVALID_BLINK);
            return false;
        }
        if (this_line->group != nullptr) {
            screen_message(DBG_INVALID_GROUP_PTR);
            return false;
        }
        if (this_line->offset_nr != 0) {
            screen_message(DBG_INVALID_OFFSET_NR);
            return false;
        }
        if (!this_line->marks.empty()) {
            screen_message(DBG_LINE_HAS_MARKS);
            return false;
        }
        if (this_line->scr_row_nr != 0) {
            screen_message(DBG_LINE_ON_SCREEN);
            return false;
        }
#endif
        if (this_line->str != nullptr) {
            delete this_line->str;
            this_line->marks.clear();
#ifdef DEBUG
            this_line->str = nullptr;
#endif
        }
#ifdef DEBUG
        this_line->len = 0;
        prev_line = this_line;
#endif
        line_ptr next_line = this_line->flink;
        this_line = next_line;
    }
#ifdef DEBUG
    if (prev_line != last_line) {
        screen_message(DBG_LAST_NOT_AT_END);
        return false;
    }
#endif
    last_line->flink = free_line_pool;
    free_line_pool = first_line;
    first_line = nullptr;
    last_line = nullptr;
    return true;
}

bool groups_destroy(group_ptr &first_group, group_ptr &last_group) {
    /*
      Purpose  : Destroy a linked list of groups.
      Inputs   : first_group, last_group: pointers to the groups to be destroyed
      Outputs  : first_group, last_group: nil.
      Bugchecks: .first_group or last_group is nil
                 .first_group/last_group contains back/forward link
                 .back links not correct
                 .group has a frame pointer
                 .group contains lines
                 .last_group is not the last group in the linked list
    */

#ifdef DEBUG
    if ((first_group == nullptr) || (last_group == nullptr)) {
        screen_message(DBG_GROUP_PTR_IS_NIL);
        return false;
    }
    if ((first_group->blink != nullptr) || (last_group->flink != nullptr)) {
        screen_message(DBG_FLINK_OR_BLINK_NOT_NIL);
        return false;
    }
    const_group_ptr prev_group = nullptr;
    const_group_ptr this_group = first_group;
    while (this_group != nullptr) {
        if (this_group->blink != prev_group) {
            screen_message(DBG_INVALID_BLINK);
            return false;
        }
        if (this_group->frame != nullptr) {
            screen_message(DBG_INVALID_FRAME_PTR);
            return false;
        }
        if ((this_group->first_line != nullptr) || (this_group->last_line != nullptr) ||
            (this_group->nr_lines != 0)) {
            screen_message(DBG_GROUP_HAS_LINES);
            return false;
        }
        prev_group = this_group;
        const_group_ptr next_group = this_group->flink;
        this_group = next_group;
    }
    if (prev_group != last_group) {
        screen_message(DBG_LAST_NOT_AT_END);
        return false;
    }
#endif
    last_group->flink = free_group_pool;
    free_group_pool = first_group;
    first_group = nullptr;
    last_group = nullptr;
    return true;
}

bool lines_inject(line_ptr first_line, line_ptr last_line, line_ptr before_line) {
    /*
      Purpose  : Inject a linked list of lines into the data structure.
      Inputs   : first_line, last_line: pointers to the lines to be injected.
               : before_line: pointer to the line before which the lines are to
                 be injected.
      Outputs  : none.
      Bugchecks: .first_line, last_line or before_line is nil
                 .first_line/last_line contains back/forward link
                 .back links not correct
                 .line has a group pointer
                 .line has a line number
                 .line has marks
                 .line is on the screen
                 .last_line is not the last line in the linked list
    */

    // Scan the lines to inserted, counting lines and checking space used.
#ifdef DEBUG
    if ((first_line == nullptr) || (last_line == nullptr) || (before_line == nullptr)) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
    if ((first_line->blink != nullptr) || (last_line->flink != nullptr)) {
        screen_message(DBG_FLINK_OR_BLINK_NOT_NIL);
        return false;
    }
#endif
    line_range nr_new_lines = 0;
    space_range space = 0;
#ifdef DEBUG
    const_line_ptr prev_line = nullptr;
#endif
    const_line_ptr this_line = first_line;
    while (this_line != nullptr) {
#ifdef DEBUG
        if (this_line->blink != prev_line) {
            screen_message(DBG_INVALID_BLINK);
            return false;
        }
        if (this_line->group != nullptr) {
            screen_message(DBG_INVALID_GROUP_PTR);
            return false;
        }
        if (this_line->offset_nr != 0) {
            screen_message(DBG_INVALID_OFFSET_NR);
            return false;
        }
        if (!this_line->marks.empty()) {
            screen_message(DBG_LINE_HAS_MARKS);
            return false;
        }
        if (this_line->scr_row_nr != 0) {
            screen_message(DBG_LINE_ON_SCREEN);
            return false;
        }
#endif
        space += this_line->len;
        nr_new_lines += 1;
#ifdef DEBUG
        prev_line = this_line;
#endif
        this_line = this_line->flink;
    }
#ifdef DEBUG
    if (prev_line != last_line) {
        screen_message(DBG_LAST_NOT_AT_END);
        return false;
    }
#endif

    /* Define some useful pointers
     *                       +-------------+   +-------------+
     *                       | top_group   |   | top_line    |
     *                       +-------------+   +-------------+
     *                           |     ^           |     ^
     * insert groups here ===>   |     |           |     |   <=== insert lines here
     *                           v     |           v     |
     *     +-------------+   +-------------+   +-------------+
     *     | this_frame  |<--| end_group   |<--| before_line |
     *     +-------------+   +-------------+   +-------------+
     */
    line_ptr top_line = before_line->blink; // can be nil
    group_ptr end_group = before_line->group;
    group_ptr top_group = end_group->blink; // can be nil
    frame_ptr this_frame = end_group->frame;

    // Determine number of free lines available in end_group and top_group.
    line_range nr_free_lines_end = MAX_GROUPLINES - end_group->nr_lines;
    line_range nr_free_lines_top;
    if (top_group != nullptr)
        nr_free_lines_top = MAX_GROUPLINES - top_group->nr_lines;
    else
        nr_free_lines_top = 0;
    line_range nr_free_lines = nr_free_lines_end + nr_free_lines_top;
    line_range line_nr = end_group->first_line_nr;

    // If insufficient free lines are available, insert some new groups.
    group_ptr adjust_group;
    if (nr_new_lines > nr_free_lines) {
        // Create a chain of new groups.
        int nr_new_groups = (nr_new_lines - nr_free_lines - 1) / MAX_GROUPLINES + 1;
        group_ptr first_group = nullptr;
        group_ptr last_group = nullptr;
        for (int group_nr = 1; group_nr <= nr_new_groups; ++group_nr) {
            if (free_group_pool == nullptr)
                line_group_pool_extend();
            group_ptr this_group = free_group_pool;
            free_group_pool = this_group->flink;
            if (first_group == nullptr)
                first_group = this_group;
            this_group->flink = nullptr;
            this_group->blink = last_group;
            this_group->frame = this_frame;
            this_group->first_line = nullptr;
            this_group->last_line = nullptr;
            this_group->first_line_nr = line_nr;
            this_group->nr_lines = 0;
            if (last_group != nullptr)
                last_group->flink = this_group;
            last_group = this_group;
        }

        // *** No more failure points follow.
        // *** Can now start putting the new stuff into the data structure.

        // Link new groups into data structure between top_group and end_group.
        last_group->flink = end_group;
        end_group->blink = last_group;
        if (top_group != nullptr) {
            top_group->flink = first_group;
            adjust_group = top_group;
        } else {
            this_frame->first_group = first_group;
            adjust_group = first_group;
        }
        first_group->blink = top_group;
    } else if (nr_new_lines > nr_free_lines_end) {
        adjust_group = top_group;
    } else {
        adjust_group = end_group;
    }

    // Insert lines into data structure between top_line and before_line.
    last_line->flink = before_line;
    before_line->blink = last_line;
    if (before_line->offset_nr == 0)
        end_group->first_line = first_line;
    if (top_line != nullptr)
        top_line->flink = first_line;
    first_line->blink = top_line;

    // Now put the data structure back together.
    line_range nr_lines_to_adjust = nr_new_lines;
    line_ptr adjust_line;
    if (nr_new_lines > nr_free_lines_end) {
        adjust_line = end_group->first_line;
        nr_lines_to_adjust = nr_lines_to_adjust + before_line->offset_nr;
        end_group->nr_lines = 0;
    } else {
        adjust_line = first_line;
        end_group->nr_lines = before_line->offset_nr;
    }
    line_ptr end_group_last_line = end_group->last_line;

    while (nr_lines_to_adjust > 0) {
        // with adjust_group^ do
        // begin
        line_range nr_lines_to_adjust_here = MAX_GROUPLINES - adjust_group->nr_lines;
        if (nr_lines_to_adjust_here > nr_lines_to_adjust)
            nr_lines_to_adjust_here = nr_lines_to_adjust;
        if (adjust_group->nr_lines == 0) {
            adjust_group->first_line = adjust_line;
            adjust_group->first_line_nr = line_nr;
        }
        for (group_line_range offset = adjust_group->nr_lines;
             offset < adjust_group->nr_lines + nr_lines_to_adjust_here;
             ++offset) {
            adjust_line->group = adjust_group;
            adjust_line->offset_nr = offset;
            adjust_line = adjust_line->flink;
        }
        adjust_group->last_line = adjust_line->blink;
        adjust_group->nr_lines += nr_lines_to_adjust_here;
        line_nr = adjust_group->first_line_nr + adjust_group->nr_lines;
        nr_lines_to_adjust -= nr_lines_to_adjust_here;
        adjust_group = adjust_group->flink;
    }
#ifdef DEBUG
    if (adjust_line != before_line) {
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        return false;
    }
#endif
    const_line_ptr next_group_first_line = end_group_last_line->flink;
    group_line_range offset = end_group->nr_lines;
    do {
        // with adjust_line^ do
        // begin
        adjust_line->offset_nr = offset;
        offset += 1;
        adjust_line = adjust_line->flink;
    } while (adjust_line != next_group_first_line);
    end_group->last_line = end_group_last_line;
    if (adjust_group == end_group) {
        end_group->first_line_nr = line_nr;
        end_group->first_line = before_line;
    }
    end_group->nr_lines = offset;

    adjust_group = end_group->flink;
    while (adjust_group != nullptr) {
        // with adjust_group^ do
        // begin
        adjust_group->first_line_nr = adjust_group->first_line_nr + nr_new_lines;
        adjust_group = adjust_group->flink;
    }

    this_frame->space_left -= space;

    // Update the screen.
    if ((before_line->scr_row_nr != 0) && (before_line != scr_top_line))
        screen_lines_inject(first_line, nr_new_lines, before_line);

    return true;
}

bool lines_extract(line_ptr first_line, line_ptr last_line) {
    /*
       Purpose  : Extract lines from the data structure.
       Inputs   : first_line, last_line: pointers to the lines to be extracted.
       Outputs  : none.
       Bugchecks: .first_line or last_line is nil
                  .last_line is eop line
                  .first and last lines in different frames
                  .first_line > last_line
                  .line has marks
    */

#ifdef DEBUG
    if ((first_line == nullptr) || (last_line == nullptr)) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
    if (last_line->flink == nullptr) {
        screen_message(DBG_LINE_IS_EOP);
        return false;
    }
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

    /* Define some useful pointers
     *                        +-------------+   +-------------+
     *                        | top_group   |<--| top_line    |
     *                        +-------------+   +-------------+
     *                                              |     ^
     *                                              v     |
     *    n.b. the group      +-------------+   +-------------+    )
     *    pointers may not    | first_group |<--| first_line  |    )
     *    be distinct         +-------------+   +-------------+    )
     *                                                ...          ) To be removed
     *                        +-------------+   +-------------+    )
     *                        | last_group  |<--| last_line   |    )
     *                        +-------------+   +-------------+    )
     *                                              |     ^
     *                                              v     |
     *      +-------------+   +-------------+   +-------------+
     *      | this_frame  |<--| end_group   |<--| end_line    |
     *      +-------------+   +-------------+   +-------------+
     */
    line_ptr top_line = first_line->blink; // can be nil
    line_ptr end_line = last_line->flink;
    // The following group pointers may not be distinct.
    group_ptr first_group = first_line->group;
    group_ptr last_group = last_line->group;
    group_ptr top_group;
    if (top_line != nullptr)
        top_group = top_line->group;
    else
        top_group = nullptr;
    group_ptr end_group = end_line->group;
    frame_ptr this_frame = end_group->frame;

    line_offset_range first_line_offset_nr = first_line->offset_nr;
    line_range first_line_nr = first_group->first_line_nr + first_line_offset_nr;
    int nr_lines_to_remove = last_group->first_line_nr + last_line->offset_nr - first_line_nr + 1;
#ifdef DEBUG
    {
        // Check that there are no marks on the lines to be removed.
        const_line_ptr this_line = first_line;
        for (line_range line_nr = 1; line_nr <= nr_lines_to_remove; ++line_nr) {
            // with this_line^ do
            // begin
            if (!this_line->marks.empty()) {
                screen_message(DBG_LINE_HAS_MARKS);
                return false;
            }
            this_line = this_line->flink;
        }
        if (this_line != last_line->flink) {
            screen_message(DBG_INTERNAL_LOGIC_ERROR);
            return false;
        }
    }
#endif

    if (this_frame == scr_frame) {
        line_ptr first_scr_line;
        if (first_line->scr_row_nr != 0)
            first_scr_line = first_line;
        else {
            // with scr_top_line^ do
            if (first_line_nr < scr_top_line->group->first_line_nr + scr_top_line->offset_nr)
                first_scr_line = scr_top_line;
            else
                goto done1;
        }
        line_ptr last_scr_line;
        if (last_line->scr_row_nr != 0)
            last_scr_line = last_line;
        else {
            // with scr_bot_line^ do
            if (last_line->group->first_line_nr + last_line->offset_nr >
                scr_bot_line->group->first_line_nr + scr_bot_line->offset_nr)
                last_scr_line = scr_bot_line;
            else
                goto done1;
        }
        screen_lines_extract(first_scr_line, last_scr_line);
    done1:;
    }

    // Unlink the lines.
    if (top_line != nullptr)
        top_line->flink = end_line;
    first_line->blink = nullptr;
    last_line->flink = nullptr;
    end_line->blink = top_line;

    // Determine the space being released by removing these lines
#ifdef DEBUG
    //   and clear their group pointers.
#endif
    space_range space = 0;
    line_ptr this_line = first_line;
    for (line_range line_nr = 1; line_nr <= nr_lines_to_remove; ++line_nr) {
        // with this_line^ do
        space += this_line->len;
#ifdef DEBUG
        this_line->group = nullptr;
        this_line->offset_nr = 0;
#endif
        this_line = this_line->flink;
    }
    this_frame->space_left += space;

    // Adjust top_group and end_group.
    if (top_group != end_group) {
        if (top_group != nullptr)
            top_group->last_line = top_line;
        end_group->first_line = end_line;
        end_group->first_line_nr = first_line_nr;
    }

    // Adjust groups below end_group.
    group_ptr this_group = end_group->flink;
    while (this_group != nullptr) {
        // with this_group^ do
        this_group->first_line_nr -= nr_lines_to_remove;
        this_group = this_group->flink;
    }

    // Adjust first_group..last_group for removed lines.
    if (first_group == top_group) {
        nr_lines_to_remove -= first_group->nr_lines - first_line_offset_nr;
        first_group->nr_lines = first_line_offset_nr;
        if (first_group != last_group)
            first_group = first_group->flink;
    }
    this_group = first_group;
    while (nr_lines_to_remove > 0) {
        // with this_group^ do
        nr_lines_to_remove = nr_lines_to_remove - this_group->nr_lines;
        this_group->nr_lines = 0;
#ifdef DEBUG
        if (nr_lines_to_remove >= 0) {
            this_group->frame = nullptr;
            this_group->first_line = nullptr;
            this_group->last_line = nullptr;
            this_group->first_line_nr = 0;
        }
#endif
        this_group = this_group->flink;
    }

    // Adjust end_group for remaining lines.
    if (nr_lines_to_remove < 0) {
        line_offset_range offset;
        if (top_group == end_group) {
            offset = first_line_offset_nr;
            end_group->nr_lines = offset - nr_lines_to_remove;
        } else {
            offset = 0;
            end_group->nr_lines = -nr_lines_to_remove;
        }
        this_line = end_line;
        for (; offset < end_group->nr_lines; ++offset) {
            // with this_line^ do
            this_line->offset_nr = offset;
            this_line = this_line->flink;
        }
    }

    // Dispose of empty groups.
    if (first_group->nr_lines == 0) {
        last_group = first_group;
        end_group = last_group->flink;
        while (end_group->nr_lines == 0) {
            last_group = end_group;
            end_group = end_group->flink;
        }
        top_group = first_group->blink; // can be nil
        if (top_group != nullptr)
            top_group->flink = end_group;
        else
            this_frame->first_group = end_group;
        first_group->blink = nullptr;
        last_group->flink = nullptr;
        end_group->blink = top_group;
        if (!groups_destroy(first_group, last_group))
            return false;
    }
    return true;
}

bool line_change_length(line_ptr line, strlen_range new_length) {
    /*
      Purpose  : Change the length of the allocated text of a line.
      Inputs   : line: pointer to the line to be adjusted.
                 new_length: the new length of the text.
      Outputs  : none.
      Bugchecks: .line is nil
    */

#ifdef DEBUG
    if (line == nullptr) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
#endif
    // with line^ do
    str_ptr new_str;
    if (new_length > 0) {
        // Quantize the length to get some slack..
        if (new_length < MAX_STRLEN - 10)
            new_length = (new_length / 10 + 1) * 10;
        else
            new_length = MAX_STRLEN;
        // Create a new str_object and copy the text from the old one.
        // FIXME: for now, all strings are the same size
        new_str = new str_object(' ');
        if (new_str == nullptr) {
            screen_message(MSG_EXCEEDED_DYNAMIC_MEMORY);
            return false;
        }
        ch_fillcopy(line->str, 1, line->len, new_str, 1, new_length, ' ');
    } else {
        new_str = nullptr;
    }
    // Dispose the old str_object.
    if (line->str != nullptr)
        delete line->str;
    // Update the amount of free space available in the frame.
    if (line->group != nullptr)
        line->group->frame->space_left += line->len - new_length;

    // Change line to refer to the new str_object.
    line->str = new_str;
    line->len = new_length;
    // FIXME: Don't see how this has changed: used  = ch_length(str^, len);
    return true;
}

bool line_to_number(line_ptr line, line_range &number) {
    /*
      Purpose  : Determine the line number of a given line.
      Inputs   : line: the line whose number is to be determined.
      Outputs  : number: the line number.
      Bugchecks: .line is nil
    */
#ifdef DEBUG
    if (line == nullptr) {
        screen_message(DBG_LINE_PTR_IS_NIL);
        return false;
    }
#endif
    number = line->group->first_line_nr + line->offset_nr;
    return true;
}

bool line_from_number(frame_ptr frame, line_range number, line_ptr &line) {
    /*
      Purpose  : Find the line with a given line number in a given frame.
      Inputs   : frame: the frame to search.
                 number: the line number to search for.
      Outputs  : line: a pointer to the found line, or nil.
      Bugchecks: .frame pointer is nil
                 .line pointer is nil
    */

#ifdef DEBUG
    if (frame == nullptr) {
        screen_message(DBG_FRAME_PTR_IS_NIL);
        return false;
    }
    if (number <= 0) {
        screen_message(DBG_INVALID_LINE_NR);
        return false;
    }
#endif
    group_ptr this_group = frame->last_group;
    if (number >= this_group->first_line_nr + this_group->nr_lines)
        line = nullptr;
    else {
        while (this_group->first_line_nr > number)
            this_group = this_group->blink;
        line_ptr this_line = this_group->first_line;
        for (line_range line_nr = 1; line_nr <= number - this_group->first_line_nr; ++line_nr)
            this_line = this_line->flink;
        line = this_line;
    }
    return true;
}
