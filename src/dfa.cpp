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
! Name:         DFA
!
! Description:  Builds the deterministic FSA for the pattern recognizer.
!**/

#include "dfa.h"

#include "screen.h"
#include "var.h"

namespace {
    struct accept_set_partition_type {
        accept_set_type accept_set_partition;
        nfa_attribute_type nfa_transition_list;
        accept_set_partition_type *flink;
        accept_set_partition_type *blink;
    };
    using partition_ptr_type = accept_set_partition_type *;
    using const_partition_ptr_type = const accept_set_partition_type *;

    template <size_t N>
    void set_range(std::bitset<N> &s, size_t from, size_t to) {
        for (auto n = from; n <= to; ++n) {
            s.set(n);
        }
    }

    nfa_set_type set_from_range(int from, int to) {
        nfa_set_type result;
        set_range(result, from, to);
        return result;
    }

    template <size_t N>
    std::bitset<N> set_difference(const std::bitset<N> &set1, const std::bitset<N> &set2) {
        auto result = set1;
        result &= ~set2;
        return result;
    }

}; // namespace

void closure_kill(nfa_attribute_type &closure) {
    state_elt_ptr_type pointer_1 = closure.equiv_list;
    while (pointer_1 != nullptr) {
        state_elt_ptr_type pointer_2 = pointer_1->next_elt;
        delete pointer_1;
        pointer_1 = pointer_2;
    }
    closure.equiv_list = nullptr;
}

void transition_kill(transition_ptr &pointer_1) {
    while (pointer_1 != nullptr) {
        transition_ptr pointer_2 = pointer_1->next_transition;
        delete pointer_1;
        pointer_1 = pointer_2;
    }
}

bool pattern_dfa_table_kill(dfa_table_ptr &pattern_ptr) {
    if (pattern_ptr != nullptr) {
        // with pattern_ptr^ do
        for (int count = 0; count <= pattern_ptr->dfa_states_used; ++count) {
            // with dfa_table[count] do
            transition_kill(pattern_ptr->dfa_table[count].transitions);
            closure_kill(pattern_ptr->dfa_table[count].nfa_attributes);
        }
        delete pattern_ptr;
        pattern_ptr = nullptr;
    }
    return true;
}

bool pattern_dfa_table_initialize(
    dfa_table_ptr &pattern_ptr, const pattern_def_type &pattern_definition
) {
    if (pattern_ptr != nullptr) {
        // with pattern_ptr^ do
        for (int count = 0; count <= pattern_ptr->dfa_states_used; ++count) {
            // with dfa_table[count] do
            transition_kill(pattern_ptr->dfa_table[count].transitions);
            closure_kill(pattern_ptr->dfa_table[count].nfa_attributes);
        }
        pattern_ptr->dfa_states_used = 0;
    } else {
        pattern_ptr = new dfa_table_object;
        // with pattern_ptr^ do
        pattern_ptr->dfa_states_used = 0;
        for (int count = 0; count <= MAX_DFA_STATE_RANGE; ++count) {
            // with dfa_table[count] do
            pattern_ptr->dfa_table[count].transitions = nullptr;
            pattern_ptr->dfa_table[count].nfa_attributes.equiv_list = nullptr;
        }
    }
    pattern_ptr->definition = pattern_definition;
    return true;
}

// FIXME: This is a brutal way to implement nested functions/procedures in C++
bool pattern_dfa_convert(
    nfa_table_type &nfa_table,
    dfa_table_ptr dfa_table_pointer,
    const nfa_state_range &nfa_start,
    nfa_state_range &nfa_end,
    nfa_state_range middle_context_start,
    nfa_state_range right_context_start,
    dfa_state_range &dfa_start,
    dfa_state_range &dfa_end
) {
    state_elt_ptr_type aux_elt;
    dfa_state_range state, current_state;
    nfa_set_type closure_set;
    nfa_attribute_type transfer_state;
    nfa_attribute_type transition_set;
    state_elt_ptr_type aux_state_ptr;
    dfa_state_range states_used;
    dfa_state_range aux_count, aux_count_2;
    transition_ptr incoming_tran_ptr, kill_tran_ptr, aux_tran_ptr;
    const_transition_ptr aux_tran_ptr_2;
    bool found;
    nfa_set_type aux_set;
    accept_set_type aux_transition_set;
    nfa_set_type mask;
    accept_set_type kill_set;
    accept_set_type intersection_set;
    partition_ptr_type partition_ptr;
    partition_ptr_type aux_partition_ptr;
    partition_ptr_type current_partition_ptr;
    partition_ptr_type follower_ptr;
    const_partition_ptr_type killer_ptr;
    partition_ptr_type insert_partition;
    const_state_elt_ptr_type aux_equiv_ptr;
    nfa_attribute_type aux_closure;

    auto epsilon_closures = [&](const nfa_attribute_type &state_set,
                                nfa_attribute_type &closure) -> bool {
        constexpr int MAX_STACK_SIZE = 50;

        std::array<nfa_state_range, MAX_STACK_SIZE + 1> stack;
        int stack_top;
        state_elt_ptr_type state_elt_ptr;
        nfa_state_range aux_state;
        bool fail_equivalent;

        auto push_stack = [&](nfa_state_range state) -> bool {
            if (stack_top < MAX_STACK_SIZE) {
                stack_top += 1;
                stack[stack_top] = state;
                closure.equiv_set.set(state);
            } else {
                screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
                return false;
            }
            return true;
        };

        stack_top = 0;
        closure.equiv_set.reset();
        closure.generator_set.reset();
        fail_equivalent = false;
        state_elt_ptr = state_set.equiv_list;
        while (state_elt_ptr != nullptr) {
            if (!push_stack(state_elt_ptr->state_elt))
                return false;
            state_elt_ptr_type kill_ptr = state_elt_ptr;
            state_elt_ptr = state_elt_ptr->next_elt;
            delete kill_ptr;
        }
        while ((stack_top != 0) && !fail_equivalent) {
            aux_state = stack[stack_top]; // pop off stack
            stack_top -= 1;
            // with nfa_table[aux_state] do
            nfa_transition_type &nta(nfa_table[aux_state]);
            if (nta.fail)
                fail_equivalent = true;
            if (nta.epsilon_out) {
                if (nta.ept.first_out != PATTERN_NULL) {
                    if (!closure.equiv_set.test(nta.ept.first_out))
                        if (!push_stack(nta.ept.first_out))
                            return false;
                }
                if (nta.ept.second_out != PATTERN_NULL) {
                    if (!closure.equiv_set.test(nta.ept.second_out))
                        if (!push_stack(nta.ept.second_out))
                            return false;
                }
            }
        }
        if (fail_equivalent) {
            closure.equiv_list = nullptr; // { Naughty }  { fix later }
            closure.equiv_set.set(PATTERN_DFA_FAIL);
        }
        return true;
    };

    auto epsilon_and_mask =
        [&](nfa_state_range state, nfa_set_type &closure, nfa_set_type &mask, bool maxim) -> bool {
        nfa_state_range aux_elt;
        nfa_attribute_type transition_set;

        state_elt_ptr_type aux_elt_ptr = new state_elt_object;
        aux_elt_ptr->next_elt = nullptr;
        aux_elt_ptr->state_elt = state;
        transition_set.equiv_list = aux_elt_ptr;
        transition_set.equiv_set.set(state);
        if (!epsilon_closures(transition_set, transition_set))
            return false;
        closure = transition_set.equiv_set;
        if (maxim) {
            aux_elt = MAX_NFA_STATE_RANGE; // the elt corr to M-C is always present
            while (!closure.test(aux_elt))
                aux_elt -= 1;
            set_range(mask, 0, aux_elt);
        } else {
            aux_elt = PATTERN_NFA_START;
            while (!closure.test(aux_elt))
                aux_elt += 1;
            set_range(mask, 0, aux_elt - 1);
        }
        return true;
    };

    auto pattern_new_dfa = [&](const nfa_attribute_type &equivalent_set,
                               dfa_state_range &state_count) -> bool {
        if (states_used < MAX_DFA_STATE_RANGE) {
            states_used += 1;
            // with dfa_table_pointer->dfa_table[states_used] do
            dfa_state_type &dts(dfa_table_pointer->dfa_table[states_used]);
            dts.nfa_attributes = equivalent_set; // gets equiv set and generator set
            dts.nfa_attributes.equiv_list = nullptr;
            for (nfa_state_range i = 0; i <= MAX_NFA_STATE_RANGE; ++i) { // build list
                if (equivalent_set.equiv_set.test(i)) {
                    state_elt_ptr_type aux_elt = new state_elt_object;
                    aux_elt->next_elt = dts.nfa_attributes.equiv_list;
                    aux_elt->state_elt = i;
                    dts.nfa_attributes.equiv_list = aux_elt;
                }
            }
            dts.transitions = nullptr;
            dts.marked = false;
            dts.pattern_start = false;
            dts.left_transition = false;
            dts.right_transition = false;
            dts.left_context_check = false;
            dts.final_accept = false;
        } else {
            screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
            return false;
        }
        state_count = states_used;
        return true;
    };

    auto pattern_add_dfa = [&](nfa_attribute_type transfer_state,
                               const accept_set_type &accept_set,
                               dfa_state_range from_state) -> bool {
        // if a DFA state with tranfer_state as its NFA_attributes does not exist
        // then create it and add a transition from from_state on the accept_set
        // to transfer_state.
        // if the DFA_state already exists then just add the transition to the
        // from_states transition list
        dfa_state_range position;

        auto dfa_search = [&](const nfa_set_type &state_head, dfa_state_range &position) -> bool {
            // finds the position in DFA_table of the state that has an NFA_equivalent
            // of state_head

            // with dfa_table_pointer^ do
            for (dfa_state_range i = 0; i <= states_used; ++i) {
                if (state_head == dfa_table_pointer->dfa_table[i].nfa_attributes.equiv_set) {
                    position = i;
                    return true;
                }
            }
            return false;
        };

        if (!dfa_search(transfer_state.equiv_set, position)) {
            // create new DFA state
            if (!pattern_new_dfa(transfer_state, position))
                return false;
        }
        // with dfa_table_pointer->dfa_table[from_state] do
        dfa_state_type &dtf(dfa_table_pointer->dfa_table[from_state]);
        transition_ptr aux_transition =
            new transition_object; // create a new transition in from_state
        // with aux_transition^ do                              // to position on input accept_elt
        aux_transition->next_transition = dtf.transitions;
        dtf.transitions = aux_transition;
        aux_transition->transition_accept_set = accept_set;
        aux_transition->accept_next_state = position;
        aux_transition->start_flag = false;
        return true;
    };

    auto unmarked_states = [&](dfa_state_range &unmarked_state) -> bool {
        for (dfa_state_range i = dfa_start; i <= states_used; ++i) {
            if (!dfa_table_pointer->dfa_table[i].marked) {
                unmarked_state = i;
                return true;
            }
        }
        return false;
    };

    auto transition_list_merge = [&](const_state_elt_ptr_type list_1,
                                     const_state_elt_ptr_type list_2) -> state_elt_ptr_type {
        // makes a copy of 2 lists and concatenates them

        state_elt_ptr_type aux_1 = nullptr;
        while (list_1 != nullptr) {
            state_elt_ptr_type aux_2 = aux_1;
            aux_1 = new state_elt_object;
            aux_1->state_elt = list_1->state_elt;
            aux_1->next_elt = aux_2;
            list_1 = list_1->next_elt;
        }
        while (list_2 != nullptr) {
            state_elt_ptr_type aux_2 = aux_1;
            aux_1 = new state_elt_object;
            aux_1->state_elt = list_2->state_elt;
            aux_1->next_elt = aux_2;
            list_2 = list_2->next_elt;
        }
        return aux_1;
    };

    auto transition_list_append = [&](state_elt_ptr_type list_1,
                                      const_state_elt_ptr_type list_2) -> state_elt_ptr_type {
        // makes a copy of list_2 and concatenates it to list_1
        // on the front

        state_elt_ptr_type aux_1 = list_1;
        while (list_2 != nullptr) {
            state_elt_ptr_type aux_2 = aux_1;
            aux_1 = new state_elt_object;
            aux_1->state_elt = list_2->state_elt;
            aux_1->next_elt = aux_2;
            list_2 = list_2->next_elt;
        }
        return aux_1;
    };

    exit_abort = true; // true in case we blow the dfa table or something
    // with dfa_table_pointer^ do
    // with dfa_table[pattern_dfa_kill] do
    dfa_state_type &dtk(dfa_table_pointer->dfa_table[PATTERN_DFA_KILL]);
    dtk.transitions = nullptr;
    dtk.marked = true;
    dtk.nfa_attributes.equiv_set.reset();
    dtk.pattern_start = false;
    dtk.left_transition = false;
    dtk.right_transition = false;
    dtk.left_context_check = false;
    dtk.final_accept = false;
    // with dfa_table[pattern_dfa_fail] do
    dfa_state_type &dtf(dfa_table_pointer->dfa_table[PATTERN_DFA_FAIL]);
    dtf.transitions = nullptr;
    dtf.marked = true;
    dtf.nfa_attributes.equiv_set.set(PATTERN_DFA_FAIL);
    dtf.pattern_start = false;
    dtf.left_transition = false;
    dtf.right_transition = false;
    dtf.left_context_check = false;
    dtf.final_accept = false;
    states_used = 1; // build initial state
    aux_elt = new state_elt_object;
    aux_elt->next_elt = nullptr;
    aux_elt->state_elt = nfa_start;
    transition_set.equiv_list = aux_elt;
    transition_set.equiv_set.set(nfa_start);
    if (!epsilon_closures(transition_set, aux_closure))
        return false;
    if (!pattern_new_dfa(aux_closure, dfa_start))
        return false;
    while (unmarked_states(current_state)) {
        if (tt_controlc) { // a reasonable place for it, gets tested once per state, = about 0.03
                           // seconds actual cp
            dfa_table_pointer->definition.length = 0; // invalidate the table
            return false; // DFA_table will be disposed on next call to DFA
        }
        kill_set.set();
        partition_ptr = nullptr;
        // with dfa_table[current_state] do
        dfa_state_type &dtc(dfa_table_pointer->dfa_table[current_state]);
        dtc.marked = true;
        aux_equiv_ptr = dtc.nfa_attributes.equiv_list;
        while (aux_equiv_ptr != nullptr) { // for transitions in equiv NFA elts
            // with nfa_table[aux_equiv_ptr->state_elt] do
            const nfa_transition_type &nta(nfa_table[aux_equiv_ptr->state_elt]);
            if (!nta.epsilon_out) { // for all SIGNIFICANT
                aux_partition_ptr = partition_ptr;
                partition_ptr = new accept_set_partition_type;
                // with partition_ptr^ do
                //  build list of transitions with accept sets
                partition_ptr->accept_set_partition = nta.epf.accept_set;
                kill_set &= ~nta.epf.accept_set;                 // update kill set
                partition_ptr->flink = aux_partition_ptr;        // link forward
                partition_ptr->blink = nullptr;                  // top of list so no blink
                if (partition_ptr->flink != nullptr)             // if there is a next one down
                    partition_ptr->flink->blink = partition_ptr; // link it back here
                partition_ptr->nfa_transition_list.equiv_list =
                    new state_elt_object; // create the NFA state
                partition_ptr->nfa_transition_list.equiv_list->next_elt = nullptr; // (only one)
                partition_ptr->nfa_transition_list.equiv_list->state_elt = nta.epf.next_state;
            }
            aux_equiv_ptr = aux_equiv_ptr->next_elt;
        }
        // OK kiddies we now have a partitionable list
        if ((partition_ptr != nullptr) && (partition_ptr->flink != nullptr)) {
            current_partition_ptr = partition_ptr;
            follower_ptr = current_partition_ptr->flink;
            while (current_partition_ptr != nullptr) {
                if (follower_ptr == current_partition_ptr)
                    follower_ptr = follower_ptr->flink;
                aux_partition_ptr = follower_ptr;
                while (aux_partition_ptr != nullptr) {
                    if (current_partition_ptr->accept_set_partition ==
                        aux_partition_ptr->accept_set_partition) {
                        // merge entrys
                        aux_state_ptr = current_partition_ptr->nfa_transition_list.equiv_list;
                        while (aux_state_ptr->next_elt != nullptr) // run to the end
                            aux_state_ptr = aux_state_ptr->next_elt;
                        aux_state_ptr->next_elt =
                            aux_partition_ptr->nfa_transition_list.equiv_list; // patch list on
                        // remove aux entry
                        // the partition being removed has no encumberences
                        // with aux_partition_ptr^ do
                        aux_partition_ptr->blink->flink = aux_partition_ptr->flink;
                        if (aux_partition_ptr->flink != nullptr)
                            aux_partition_ptr->flink->blink = aux_partition_ptr->blink;
                        killer_ptr = aux_partition_ptr;
                        if (follower_ptr == aux_partition_ptr)
                            follower_ptr = aux_partition_ptr->flink;
                        aux_partition_ptr = aux_partition_ptr->flink;
                        delete killer_ptr;
                    } else {
                        // form partition
                        intersection_set = current_partition_ptr->accept_set_partition & aux_partition_ptr->accept_set_partition;
                        if (!intersection_set.none()) { // preeety worthless if []
                            if (intersection_set == current_partition_ptr->accept_set_partition) {
                                current_partition_ptr->nfa_transition_list.equiv_list =
                                    transition_list_append(
                                        current_partition_ptr->nfa_transition_list.equiv_list,
                                        aux_partition_ptr->nfa_transition_list.equiv_list
                                    );
                                aux_partition_ptr->accept_set_partition &= ~intersection_set;
                            } else if (intersection_set ==
                                       aux_partition_ptr->accept_set_partition) {
                                aux_partition_ptr->nfa_transition_list.equiv_list =
                                    transition_list_append(
                                        aux_partition_ptr->nfa_transition_list.equiv_list,
                                        current_partition_ptr->nfa_transition_list.equiv_list
                                    );
                                current_partition_ptr->accept_set_partition &= ~intersection_set;
                            } else {
                                // need to do a full partition
                                insert_partition = new accept_set_partition_type;
                                // with insert_partition^ do
                                insert_partition->accept_set_partition = intersection_set;
                                insert_partition->flink = follower_ptr;
                                // insert above follower ptr (!= nullptr)
                                insert_partition->blink = follower_ptr->blink;
                                insert_partition->flink->blink = insert_partition;
                                insert_partition->blink->flink = insert_partition;
                                insert_partition->nfa_transition_list.equiv_list =
                                    transition_list_merge(
                                        current_partition_ptr->nfa_transition_list.equiv_list,
                                        aux_partition_ptr->nfa_transition_list.equiv_list
                                    );
                                current_partition_ptr->accept_set_partition &= ~intersection_set;
                                aux_partition_ptr->accept_set_partition &= ~intersection_set;
                            } // of full partition
                        }
                        aux_partition_ptr = aux_partition_ptr->flink;
                    } // of else
                } // of while aux_partition_ptr != nullptr
                current_partition_ptr = current_partition_ptr->flink;
            } // of while current_partition_ptr->flink != nullptr
        }
        // OK people we now have a partitioned list
        // now we use it to form DFA
        // with dfa_table[current_state] do
        // bung in the kill transitions
        dfa_state_type &dtc2(dfa_table_pointer->dfa_table[current_state]);
        dtc2.transitions = new transition_object;
        // with transitions^ do
        dtc2.transitions->accept_next_state = PATTERN_DFA_KILL;
        dtc2.transitions->start_flag = false;
        dtc2.transitions->next_transition = nullptr;
        dtc2.transitions->transition_accept_set = kill_set;
        while (partition_ptr != nullptr) {
            aux_partition_ptr = partition_ptr;
            // with aux_partition_ptr^ do
            if (!epsilon_closures(aux_partition_ptr->nfa_transition_list, transfer_state))
                return false;
            if (!pattern_add_dfa(
                    transfer_state, aux_partition_ptr->accept_set_partition, current_state
                ))
                return false;
            partition_ptr = aux_partition_ptr->flink;
            // the NFA equiv list is disposed of by epsilon_closures
            delete aux_partition_ptr;
            // we should now have no dangling objects
            // run down list , use NFA_transition_list.equiv_list to form e-c
            // to specify  state to transfer to. Then add transition
        }
    }
    // END OF DFA GENERATION
    // Now we fix it up so it will drive the recognizer

    // find all final states
    for (aux_count = 0; aux_count <= states_used; ++aux_count) {
        if (dfa_table_pointer->dfa_table[aux_count].nfa_attributes.equiv_set.test(nfa_end))
            dfa_table_pointer->dfa_table[aux_count].final_accept = true;
    }

    // start pattern flag creation
    incoming_tran_ptr = dfa_table_pointer->dfa_table[PATTERN_DFA_START].transitions;
    while (incoming_tran_ptr != nullptr) { // find all transitions out of start
        // with incoming_tran_ptr^ do
        if ((incoming_tran_ptr->accept_next_state != PATTERN_DFA_KILL) &&
            (incoming_tran_ptr->accept_next_state != PATTERN_DFA_FAIL) &&
            !dfa_table_pointer->dfa_table[incoming_tran_ptr->accept_next_state].final_accept) {
            // with dfa_table[accept_next_state] do
            dfa_state_type &dtans(
                dfa_table_pointer->dfa_table[incoming_tran_ptr->accept_next_state]
            );
            dtans.pattern_start = true;
            kill_tran_ptr = dtans.transitions; // find transition to kill state
            while ((kill_tran_ptr != nullptr) &&
                   (kill_tran_ptr->accept_next_state != PATTERN_DFA_KILL)) {
                kill_tran_ptr = kill_tran_ptr->next_transition;
                if (kill_tran_ptr != nullptr) {
                    aux_transition_set = incoming_tran_ptr->transition_accept_set & kill_tran_ptr->transition_accept_set;
                    if (!aux_transition_set.none()) {
                        aux_tran_ptr = new transition_object;
                        // with aux_tran_ptr^ do
                        aux_tran_ptr->transition_accept_set = aux_transition_set;
                        aux_tran_ptr->accept_next_state = incoming_tran_ptr->accept_next_state;
                        // point back to self all those transitions that are killed
                        // and are the same as the transitions leading in to state
                        aux_tran_ptr->next_transition = nullptr;
                        aux_tran_ptr->start_flag = true;
                        kill_tran_ptr->transition_accept_set &= ~aux_tran_ptr->transition_accept_set;
                        kill_tran_ptr->next_transition = aux_tran_ptr;
                    }
                }
            }
        }
        incoming_tran_ptr = incoming_tran_ptr->next_transition;
    }

    // find all end of left context states
    if (!epsilon_and_mask(middle_context_start, closure_set, mask, true))
        return false;
    for (aux_count = PATTERN_DFA_START; aux_count <= states_used; ++aux_count) {
        // with dfa_table[aux_count],nfa_attributes do
        dfa_state_type &dtac(dfa_table_pointer->dfa_table[aux_count]);
        if (dtac.nfa_attributes.equiv_set.test(middle_context_start) &&
            set_difference(dtac.nfa_attributes.equiv_set, mask).none())
            dtac.left_transition = true;
        aux_set = closure_set & set_from_range(middle_context_start.value(), right_context_start.value());
        for (aux_count_2 = PATTERN_DFA_START; aux_count_2 <= states_used; ++aux_count_2) {
            if (dfa_table_pointer->dfa_table[aux_count_2].left_transition) {
                // is a context start
                aux_tran_ptr_2 = dfa_table_pointer->dfa_table[aux_count_2].transitions;
                while (aux_tran_ptr_2 != nullptr) {
                    // for all members of head of context
                    state = aux_tran_ptr_2->accept_next_state;
                    if (state > aux_count_2) {
                        // stop it messing up previous contexts
                        aux_tran_ptr = dfa_table_pointer->dfa_table[state].transitions;
                        found = false; // find self transiting context head states
                        while ((aux_tran_ptr != nullptr) && !found) {
                            if (aux_tran_ptr->accept_next_state ==
                                state) // has a transition to itself
                                found = true;
                            else
                                aux_tran_ptr = aux_tran_ptr->next_transition;
                        }
                        if (found) {
                            dfa_table_pointer->dfa_table[state].left_context_check =
                                true; // assume the worst
                            for (aux_count = middle_context_start; aux_count <= right_context_start;
                                 ++aux_count) {
                                // find those NFA states that are on the front of context
                                // and are within the scope of an indefinte repetition
                                // but within the context under consideration
                                if (nfa_table[aux_count].indefinite &&
                                    aux_set.test(aux_count) &&
                                    dfa_table_pointer->dfa_table[state]
                                        .nfa_attributes.equiv_set.test(aux_count))
                                    dfa_table_pointer->dfa_table[state].left_context_check = false;
                            }
                        }
                    }
                    aux_tran_ptr_2 = aux_tran_ptr_2->next_transition;
                }
            }
        }
    }

    // find all end of middle context states
    if (!epsilon_and_mask(right_context_start, closure_set, mask, true))
        return false;
    for (aux_count = 0; aux_count <= states_used; ++aux_count) {
        // with dfa_table[aux_count],nfa_attributes do
        dfa_state_type &dtac(dfa_table_pointer->dfa_table[aux_count]);
        if (dtac.nfa_attributes.equiv_set.test(right_context_start) &&
            set_difference(dtac.nfa_attributes.equiv_set, mask).none())
            dtac.right_transition = true;
    }
    //    if states_used = pattern_dfa_start then
    //       if dfa_table[pattern_dfa_start].transitions->accept_next_state =
    //            pattern_dfa_kill         then
    //         begin screen_message( msg_pat_null_pattern ); goto 99; end;
    //
    dfa_end = states_used;                            // for debugging
    dfa_table_pointer->dfa_states_used = states_used; // most important, for disposal of things

    exit_abort = false; // set them safe again now we are finished
    return true;
}
