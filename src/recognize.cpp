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
! Name:         RECOGNIZE
!
! Description:  The pattern matcher for EQS, GET and REPLACE.
!**/

#include "recognize.h"

#include "var.h"

namespace {
    const accept_set_type EMPTY_SET;
};

void pattern_get_input_elt(line_ptr line, char &ch, accept_set_type &input_set, col_range &column,
                           strlen_range length, bool &mark_flag, bool &end_of_line) {
    input_set.clear();
    if (length == 0) {
        ch = PATTERN_SPACE; // not 100% corrrect but OK
        input_set.add({PATTERN_BEG_LINE, PATTERN_END_LINE});
        mark_flag = true;
        end_of_line = true;
    } else {
        bool mark_found = false;
        if (!mark_flag) {      // the last time through was not a mark so look for them this time
            //with current_frame^ do
            if (column == 1) {
                input_set.add(PATTERN_BEG_LINE);
                mark_found = true;
            }
            if (column > length) {
                input_set.add(PATTERN_END_LINE);
                end_of_line = true;
                mark_found = true;
            }
            if (column == current_frame->margin_left) {
                input_set.add(PATTERN_LEFT_MARGIN);
                mark_found = true;
            }
            if (column == current_frame->margin_right) {
                input_set.add(PATTERN_RIGHT_MARGIN);
                mark_found = true;
            }
            if (column == current_frame->dot->col) {
                input_set.add(PATTERN_DOT_COLUMN);
                mark_found = true;
            }
            if (!line->marks.empty()) { // if any marks on this line
                for (mark_range mark_no = MIN_MARK_NUMBER; mark_no <= MAX_MARK_NUMBER; ++mark_no) {
                    // run through user accessible ones
                    if (current_frame->marks[mark_no] != nullptr) {
                        if ((current_frame->marks[mark_no]->line == line) && (current_frame->marks[mark_no]->col == column)) {
                            input_set.add(mark_no + PATTERN_MARKS_START);
                            mark_found = true;
                        }
                    }
                }
            }
            // mark_found is true if there is a user mark on this column.
            mark_flag = mark_found; // will not test_for a mark next time through
        }

        if (!mark_found) {
            // if there is not a mark or we have already prossesed it . then get the char
            mark_flag = false;   // will test for a mark next time through
            ch = (*line->str)[column];
            if (column <= length)
                column += 1;
        }
    }
}

bool pattern_next_state(dfa_table_ptr dfa_table_pointer, char ch,
                        const accept_set_type &input_set, bool mark_flag, dfa_state_range &state, bool &started) {
    bool found = false;
    transition_ptr transition_pointer = dfa_table_pointer->dfa_table[state].transitions;
    if (mark_flag) { // look for transitions on positionals only
        dfa_state_range aux_state;
        while ((transition_pointer != nullptr) && !found) {
            if (!transition_pointer->transition_accept_set.set_intersection(input_set).empty()) {
                found = true;
                if (transition_pointer->start_flag && !started)
                    aux_state = PATTERN_DFA_KILL;
                else
                    aux_state = transition_pointer->accept_next_state;
            } else {
                transition_pointer = transition_pointer->next_transition;
            }
        }
        if (aux_state == PATTERN_DFA_KILL)
            found = false;
        else
            state = aux_state;
    } else {
        // look for transitions on characters
        while ((transition_pointer != nullptr) && !found) {
            if (transition_pointer->transition_accept_set.contains(ch)) {
                found = true;
                if (transition_pointer->start_flag && !started)
                    state = PATTERN_DFA_KILL;
                else
                    state = transition_pointer->accept_next_state;
            } else {
                transition_pointer = transition_pointer->next_transition;
            }
        }
    }
    started = (started && dfa_table_pointer->dfa_table[state].pattern_start) ||
              (state == PATTERN_DFA_FAIL) || (state == PATTERN_DFA_KILL);
    return found;
}

bool pattern_recognize(dfa_table_ptr dfa_table_pointer, line_ptr line, col_range start_col,
                       bool &mark_flag, col_range &start_pos, col_range &finish_pos) {
    col_range line_counter = start_col;
    start_pos    = start_col;
    finish_pos   = start_col;
    dfa_state_range state = PATTERN_DFA_START;
    bool found       = false;
    bool fail        = false;
    bool started     = true;
    bool end_of_line = false;
    bool left_flag   = false;
    accept_set_type positional_set;
    char ch;
    //with dfa_table_pointer^ do
    do {
        do {
            pattern_get_input_elt(line, ch, positional_set, line_counter, line->used,
                                  mark_flag, end_of_line);
            if (pattern_next_state(dfa_table_pointer, ch, positional_set, mark_flag, state, started)) {
                if (state == PATTERN_DFA_KILL) {
                    state = PATTERN_DFA_START;
                } else if (state == PATTERN_DFA_FAIL) {
                    fail = true;
                    start_col += 1;
                    line_counter = start_col + 1;
                    state = PATTERN_DFA_START;
                }
                //with dfa_table[state] do
                if (dfa_table_pointer->dfa_table[state].left_transition) {
                    start_pos = line_counter;
                } else if (dfa_table_pointer->dfa_table[state].left_context_check) {
                    if (left_flag)
                        start_pos = line_counter - 1;
                    else
                        left_flag = true;
                }
                if (dfa_table_pointer->dfa_table[state].right_transition)
                    finish_pos = line_counter;
            }
        } while (!dfa_table_pointer->dfa_table[state].final_accept && !end_of_line);
        if (!end_of_line) {
            do {
                pattern_get_input_elt(line, ch, positional_set, line_counter, line->used,
                                      mark_flag, end_of_line);
                if (pattern_next_state(dfa_table_pointer, ch, positional_set, mark_flag, state, started)) {
                    if (state == PATTERN_DFA_KILL) {
                        found = true;
                    } else if (state == PATTERN_DFA_FAIL) {
                        fail = true;
                        start_col += 1;
                        line_counter = start_col +1;
                        state = PATTERN_DFA_START;
                    }
                    //with dfa_table[state] do
                    if (dfa_table_pointer->dfa_table[state].right_transition)
                        finish_pos = line_counter;
                }
            } while (!found && !fail && !end_of_line);
        }
    } while (!found && !end_of_line);
    if (!found) { // must also be end of line push through the white space at end of line
        bool flag = dfa_table_pointer->dfa_table[state].final_accept;
        if (pattern_next_state(dfa_table_pointer, ' ', EMPTY_SET, false, state, started)) {
                //with dfa_table[state] do
                // note end of line positional will already have been prossesed
            if ((state == PATTERN_DFA_KILL) && flag)
                found = true;
            if (dfa_table_pointer->dfa_table[state].right_transition)
                finish_pos = line->used +1;
            if (dfa_table_pointer->dfa_table[state].left_transition)
                start_pos = line->used +1;
            if (dfa_table_pointer->dfa_table[state].final_accept) {
                // does not need to be pushed to the kill state
                // as there is no more input, so there is no
                // possibility of a fail being generated
                found = true;
            }
        }
    }
    return found;
}
