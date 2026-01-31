#ifndef DFA_H
#define DFA_H

#include "type.h"

[[nodiscard]] bool pattern_dfa_table_kill(dfa_table_ptr &pattern_ptr);
[[nodiscard]] bool pattern_dfa_table_initialize(
    dfa_table_ptr &pattern_ptr, const pattern_def_type &pattern_definition
);
[[nodiscard]] bool pattern_dfa_convert(
    nfa_table_type &nfa_table,
    dfa_table_ptr dfa_table_pointer,
    const nfa_state_range &nfa_start,
    nfa_state_range &nfa_end,
    nfa_state_range middle_context_start,
    nfa_state_range right_context_start,
    dfa_state_range &dfa_start,
    dfa_state_range &dfa_end
);

#endif // !defined(DFA_H)
