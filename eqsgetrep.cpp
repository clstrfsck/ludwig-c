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
! Name:         EQSGETREP
!
! Description:  The EQS, GET and REPLACE commands.
*/

#include "eqsgetrep.h"

#include "ch.h"
#include "var.h"
#include "dfa.h"
#include "mark.h"
#include "text.h"
#include "screen.h"
#include "charcmd.h"
#include "patparse.h"
#include "recognize.h"

#include <algorithm>

namespace {

    const std::string THIS_ONE("This one?");
    const std::string REPLACE_THIS_ONE("Replace this one?");

};

bool eqsgetrep_exactcase(tpar_object &target) {
    if (target.dlm != '"') {
        // only use non-exact if necessary
        for (int i = 1; i <= target.len; ++i) {
            if (ALPHA_SET.contains(target.str[i]))
                target.str[i] = std::toupper(target.str[i]);
        }
        return false;
    }
    return true;
}

bool eqsgetrep_same_pattern_def(const pattern_def_type &pattern_1, const pattern_def_type &pattern_2) {
    if ((pattern_1.length != 0) && (pattern_2.length != 0) && (pattern_1.length == pattern_2.length)) {
        for (int count = 1; count <= pattern_1.length; ++count) {
            if (pattern_1.strng[count] != pattern_2.strng[count])
                return false;
        }
        return true;
    }
    return false;
}

bool eqsgetrep_pattern_build(tpar_object tpar, dfa_table_ptr &pattern_ptr) {
    pattern_def_type pattern_definition;
    nfa_table_type   nfa_table;
    nfa_state_range  first_pattern_start;
    nfa_state_range  pattern_final_state;
    nfa_state_range  left_context_end;
    nfa_state_range  middle_context_end;
    nfa_state_range  states_used;               // DUMMY Will go
    if (pattern_parser(tpar, nfa_table, first_pattern_start, pattern_final_state,
                       left_context_end, middle_context_end, pattern_definition, states_used)) {
        bool already_built;
        if (pattern_ptr != nullptr)
            already_built =  eqsgetrep_same_pattern_def(pattern_definition, pattern_ptr->definition);
        else
            already_built = false;
        if (!already_built) {
            if (!pattern_dfa_table_initialize(pattern_ptr, pattern_definition))
                return false;
            dfa_state_range dfa_start, dfa_end; // may well go in final version
            if (!pattern_dfa_convert(nfa_table, pattern_ptr, first_pattern_start, pattern_final_state,
                                     left_context_end, middle_context_end, dfa_start, dfa_end))
                return false;
        }
    } else {
        return false;
    }
    return true;
}

bool eqsgetrep_eqs(leadparam rept, int count, tpar_object tpar, bool from_span) {
    bool success = false;
    //with current_frame^,dot^,line^ do
    if (tpar.dlm == TPD_SMART) {
        if (!eqsgetrep_pattern_build(tpar, current_frame->eqs_pattern_ptr))
            return false;
        bool      mark_flag = false;
        col_range start_col;
        col_range end_pos;
        bool      found = pattern_recognize(current_frame->eqs_pattern_ptr, current_frame->dot->line, current_frame->dot->col,
                                            mark_flag, start_col, end_pos);
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
            success = (current_frame->dot->col == start_col) && found;
            break;
        case leadparam::minus:
            success = !((current_frame->dot->col == start_col) && found);
            break;
        case leadparam::pindef:
            success = (end_pos <= current_frame->dot->line->used) && found;
            break;
        case leadparam::nindef:
            success = (end_pos >= current_frame->dot->line->used) && found;
            break;
        default:
            // Others ignores / invalid.
            // FIXME: Throw exception?
            break;
        }
        if (success && rept != leadparam::minus)
            success = mark_create(current_frame->dot->line, end_pos, current_frame->marks[MARK_EQUALS]);
    } else {
        bool exactcase = eqsgetrep_exactcase(tpar);   // Decide if exact case.
        strlen_range start_col = current_frame->dot->col;
        strlen_range length;
        if (start_col > current_frame->dot->line->used) {
            length = 0;
            start_col = 1; // in case col off end of string
        } else {
            length = current_frame->dot->line->used + 1 - current_frame->dot->col;
        }
        if (length > tpar.len)
            length = tpar.len;

        // Compare string.
        strlen_range nch_ident;
        int result = ch_compare_str(tpar.str, 1, tpar.len,
                                    *current_frame->dot->line->str, start_col, length,
                                    exactcase, nch_ident);
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
            success = (result == 0);
            break;
        case leadparam::minus:
            success = (result != 0);
            break;
        case leadparam::pindef:
            success = (result <= 0);
            break;
        case leadparam::nindef:
            success = (result >= 0);
            break;
        default:
            // FIXME: Error?
            break;
        }
        if (success && rept != leadparam::minus) {
            success = mark_create(current_frame->dot->line, current_frame->dot->col + nch_ident,
                                  current_frame->marks[MARK_EQUALS]);
        }
    }
    return success;
}

bool eqsgetrep_dumb_get(leadparam rept, int count, tpar_object tpar, bool from_span) {
    bool result = (count == 0);
    //with current_frame^ do
    // Initialize the search variables.
    line_ptr  dot_line  = current_frame->dot->line;  // Remember initial dot.
    col_range dot_col   = current_frame->dot->col;
    bool exactcase      = eqsgetrep_exactcase(tpar); // Decide if exact case.
    line_ptr line       = dot_line;                  // Line to start on.
    strlen_range newlen = tpar.len;
    bool tail_space;
    if ((newlen > 1) && (tpar.str[newlen] == ' ')) {
        tail_space = true;
        newlen     -= 1;
    } else {
        tail_space = false;
    }
    str_object newstr;
    bool backwards;
    col_range start_col;
    strlen_range length;
    if (count < 0) {
        count     = -count;
        //with tpar do
        ch_reverse_str(tpar.str, newstr, newlen);
        backwards = true;
        start_col = 1;
        length    = current_frame->dot->col - 1;
        if (length > line->used)
            length = line->used;
    } else {
        newstr    = tpar.str;
        backwards = false;
        start_col = current_frame->dot->col;
        if (start_col > line->used)
            length = 0;
        else
            length = line->used + 1 - start_col;
    }
    // Search for target.
    while ((count > 0) && !tt_controlc) {
        bool found;
        strlen_range offset;
        if (length == 0)
            found = false;
        else
            found = ch_search_str(newstr, 1, newlen,
                                  *line->str, start_col.value(), length,
                                  exactcase, backwards, offset);
        if (found) {
            // Found AN instance, except maybe for TAIL_SPACE.
            // Check that matched string is followed by a space, or EOL space.
            if (tail_space) {
                //with line^ do
                char tail_char;
                if (start_col + offset + newlen <= line->used)
                    tail_char = (*line->str)[start_col + offset + newlen];
                else if (start_col + offset + newlen == line->used + 1) {
                    if (line->used + 1 == MAX_STRLENP) // DONT FIND VERY LAST SPACE
                        tail_char = '\0';              // BECAUSE THEN DOT WOULD NEED
                    else                               // TO GO BEYOND END OF LINE!
                        tail_char = ' ';
                } else {
                    tail_char = '\0';
                }
                if (tail_char != ' ') {
                    if (backwards)
                        start_col = start_col + offset + newlen - 1;
                    else
                        start_col += 1;
                    goto l2;
                }
            }
            // Step to end of matched string.
            start_col += offset;
            if (!backwards)
                start_col += tpar.len;
            // Check if found THE instance.
            count -= 1;
            if (count == 0) {
                // Place the dot in the right place.
                if (!mark_create(line, start_col, current_frame->dot))
                    goto l99;
                if (!from_span) {
                    str_object buffer(' ');
                    buffer.copy(THIS_ONE.data(), THIS_ONE.size());
                    switch (screen_verify(buffer, THIS_ONE.size())) {
                    case verify_response::verify_reply_always:
                    case verify_response::verify_reply_yes:
                        break;
                    case verify_response::verify_reply_quit:
                    case verify_response::verify_reply_no:
                        // Pretend we never found it.
                        count = 1;
                        if (!mark_create(dot_line, dot_col, current_frame->dot))
                            goto l99;
                        if (exit_abort)
                            goto l99;
                        else
                            goto l1;
                        break;
                    }
                }
                // Place the equals in the right place.
                if (backwards) {
                    if (!mark_create(line, start_col + tpar.len, current_frame->marks[MARK_EQUALS]))
                        goto l99;
                } else {
                    if (!mark_create(line, start_col - tpar.len, current_frame->marks[MARK_EQUALS]))
                        goto l99;
                }
                result = true;
                goto l99;
        l1:;
            }
            // Search the rest of this line.
    l2:;
            if (backwards) {
                length    = start_col - 1;
                start_col = 1;
            } else if (start_col > line->used) {
                length = 0;
            } else {
                length    = line->used + 1 - start_col;
            }
        } else {
            // No more instances on this line, move to next line.
            if (backwards)
                line = line->blink;
            else
                line = line->flink;
            if (line == nullptr)
                goto l99;
            start_col = 1;
            length = line->used;
        }
    }
l99:;
    return result;
}

bool eqsgetrep_pattern_get(leadparam rept, int count, tpar_object tpar, bool from_span, bool replace_flag) {
    bool result = (count == 0);
    //with current_frame^ do
    dfa_table_ptr pattern_ptr;
    if (!replace_flag) {
        if (!eqsgetrep_pattern_build(tpar, current_frame->get_pattern_ptr))
            return result;
        pattern_ptr = current_frame->get_pattern_ptr;
    } else {
        // is a get within a replace, pattern table already exists
        pattern_ptr = current_frame->rep_pattern_ptr;
    }
    // Initialize the search variables.
    line_ptr  dot_line = current_frame->dot->line; // Remember initial dot.
    col_range dot_col  = current_frame->dot->col;
    line_ptr  line     = dot_line;                 // Line to start on.
    bool mark_flag = false;                        // search for marks at start pos
    bool backwards = count < 0;
    strlen_range start_col;
    if (backwards)                                 // Since the matcher can only search
        start_col = 1;                             // forward it must start in col 1 when
    else                                           // going backwards.
        start_col = dot_col;                       // Otherwise start at the dot.
    count = std::abs(count);
    if (start_col > line->used)
        start_col = line->used + 1;
    // Search for target.
    while ((count > 0) && !tt_controlc) {
        col_range matched_start_col;
        col_range matched_finish_col;
        if (pattern_recognize(pattern_ptr, line, start_col.value(), mark_flag,
                              matched_start_col, matched_finish_col)) {
            if (! ((line == dot_line) && (matched_finish_col >= dot_col) && backwards)) {
                // so long as not somewhere we are not allowed
                count -= 1;
                if (count == 0) {
                    // Place the dot in the right place.
                    if (backwards) {
                        if (!mark_create(line, matched_start_col, current_frame->dot))
                            goto l99;
                    } else {
                        if (!mark_create(line, matched_finish_col, current_frame->dot))
                            goto l99;
                    }
                    if (!from_span) {
                        str_object buffer;
                        buffer.copy(THIS_ONE.data(), THIS_ONE.size());
                        switch (screen_verify(buffer, THIS_ONE.size())) {
                        case verify_response::verify_reply_always:
                        case verify_response::verify_reply_yes:
                            break;
                        case verify_response::verify_reply_quit:
                        case verify_response::verify_reply_no:
                            // Pretend we never found it.
                            count = 1;
                            if (!mark_create(dot_line, dot_col, current_frame->dot))
                                goto l99;
                            if (exit_abort)
                                goto l99;
                            else
                                goto l1;
                            break;
                        }
                    }
                    // Place the equals in the right place.
                    if (backwards) {
                        if (!mark_create(line, matched_finish_col, current_frame->marks[MARK_EQUALS]))
                            goto l99;
                    } else if (!mark_create(line, matched_start_col, current_frame->marks[MARK_EQUALS]))
                        goto l99;
                    result = true;
                    goto l99;  // been there, done that, moving on
            l1:;
                } // finished code for having found it
                // Search the rest of this line.
                // found one but not the right count
                start_col = matched_finish_col;
                // search rest of line
                if (start_col == matched_start_col) // occurs if only match mark
                    mark_flag = true;               // stop it matching the mark on this col.
                if (start_col > line->used) {       // Finished this line, move on.
                    // since strictly greater than must have prosessed
                    if (backwards)
                        line = line->blink;         // the end of line & white space
                    else
                        line = line->flink;
                    if (line == nullptr)
                        goto l99;
                    mark_flag = false;
                    start_col = 1;
                }
            } else { // of we are not outside the allowed search bounds
                // we found one but outside bounds
                line = line->blink;   // must be backwards, as that is a criterion
                if (line == nullptr)
                    goto l99;   // for out of bounds
                mark_flag = false;
                start_col = 1;
            }
        } else {
            // No more instances on this line, move to next line.
            if (backwards)
                line = line->blink;
            else
                line = line->flink;
            if (line == nullptr)
                goto l99;
            mark_flag = false;
            start_col = 1;
        }
    } // of while not count > 0
    
l99:;
    return result;
}

bool eqsgetrep_get(leadparam rept, int count, tpar_object tpar, bool from_span) {
    if (tpar.dlm == TPD_SMART)
        return eqsgetrep_pattern_get(rept, count, tpar, from_span, false);
    else
        return eqsgetrep_dumb_get(rept, count, tpar, from_span);
}

bool eqsgetrep_rep(leadparam rept, int count, tpar_object tpar, tpar_object tpar2, bool from_span) {
    int getcount;
    leadparam getrept;
    int length;
    int delta;
    col_range start_col;
    mark_ptr old_dot = nullptr;
    mark_ptr old_equals = nullptr;
    bool okay;
    bool result = false;
    //with current_frame^ do
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, old_dot))
        goto l99;
    if (current_frame->marks[MARK_EQUALS] != nullptr) {
        if (!mark_create(current_frame->marks[MARK_EQUALS]->line, current_frame->marks[MARK_EQUALS]->col, old_equals))
            goto l99;
    }
    if (tpar.dlm == TPD_SMART) {
        if (!eqsgetrep_pattern_build(tpar, current_frame->rep_pattern_ptr))
            goto l99;
    }
    getrept  = leadparam::pint;
    getcount = 1;
    if (rept == leadparam::minus || rept == leadparam::nindef || rept == leadparam::nint) {
        getrept  = leadparam::nint;
        getcount = -1;
    }
    if (rept == leadparam::pindef || rept == leadparam::nindef)
        count = MAXINT;
    else if (count < 0)
        count = -count;
    while (count > 0) {
        do {
            okay = true;
            if (tt_controlc || exit_abort)
                goto l1;
            if (tpar.dlm == TPD_SMART) {
                if (!eqsgetrep_pattern_get(getrept, getcount, tpar, true, true))
                    goto l1;
            } else if (!eqsgetrep_dumb_get(getrept, getcount, tpar, true))
                goto l1;
            if (tt_controlc || exit_abort)
                goto l1;
            if (!from_span) {
                str_object buffer;
                buffer.copy(REPLACE_THIS_ONE.data(), REPLACE_THIS_ONE.size());
                switch (screen_verify(buffer, REPLACE_THIS_ONE.size())) {
                case verify_response::verify_reply_always:
                    from_span = true;
                    break;
                case verify_response::verify_reply_yes:
                    break;
                case verify_response::verify_reply_quit:
                case verify_response::verify_reply_no:
                    okay = false;
                    break;
                }
            }
        } while (!okay);
        length = current_frame->marks[MARK_EQUALS]->col - current_frame->dot->col;
        // If EQUALS < DOT then REVERSE THEM.
        if (length < 0) {
            current_frame->dot->col = current_frame->marks[MARK_EQUALS]->col;
            current_frame->marks[MARK_EQUALS]->col = current_frame->dot->col-length;
            length = -length;
        }
        if (tpar2.con == nullptr) {
            start_col = current_frame->dot->col;
            // Make exactly right size for replacement text.
            delta = length - tpar2.len;
            if (delta > 0) {
                if (current_frame->dot->col + delta > current_frame->dot->line->used + 1)
                    delta = current_frame->dot->line->used + 1 - current_frame->dot->col;
                if (delta > 0) {
                    if (!charcmd_delete(commands::cmd_delete_char, leadparam::pint, delta, true))
                        goto l99;
                }
            } else if (delta < 0) {
                if (!charcmd_insert(commands::cmd_insert_char, leadparam::pint, -delta, true))
                    goto l99;
                current_frame->dot->col = start_col;
            }
            // Overtype the replacement text and place DOT and EQUALS correctly.
            if (!text_overtype(true, 1, tpar2.str, tpar2.len, current_frame->dot))
                goto l99;
            //with dot^ do
            if (getcount > 0) {
                if (!mark_create(current_frame->dot->line, start_col, current_frame->marks[MARK_EQUALS]))
                    goto l99;
            } else {
                if (!mark_create(current_frame->dot->line, start_col + tpar2.len, current_frame->marks[MARK_EQUALS]))
                    goto l99;
                current_frame->dot->col = start_col;
            }
        } else {
            if (!charcmd_delete(commands::cmd_delete_char, leadparam::pint, length, true))
                goto l99;
            if (!text_insert_tpar(tpar2, current_frame->dot, current_frame->marks[MARK_EQUALS]))
                goto l99;
        }
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, old_dot))
            goto l99;
        if (!mark_create(current_frame->marks[MARK_EQUALS]->line, current_frame->marks[MARK_EQUALS]->col, old_equals))
            goto l99;
        current_frame->text_modified = true;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
            goto l99;
        // Decrement the count remaining.
        count -= 1;
    }
l1:;
    if (!mark_create(old_dot->line, old_dot->col, current_frame->dot))
        goto l99;
    if (!mark_destroy(old_dot))
        goto l99;
    if (old_equals != nullptr) {
        if (!mark_create(old_equals->line, old_equals->col, current_frame->marks[MARK_EQUALS]))
            goto l99;
        if (!mark_destroy(old_equals))
            goto l99;
    } else if (current_frame->marks[MARK_EQUALS] != nullptr) {
        if (!mark_destroy(current_frame->marks[MARK_EQUALS]))
            goto l99;
    }
    result = (count == 0) || rept == leadparam::pindef || rept == leadparam::nindef;
l99:;
    // Just in case these marks are still hanging around ...
    if (old_dot != nullptr)
        mark_destroy(old_dot);
    if (old_equals != nullptr)
        mark_destroy(old_equals);
    return result;
}
