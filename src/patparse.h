#ifndef PATPARSE_H
#define PATPARSE_H

#include "type.h"

bool pattern_parser(
    tpar_object &pattern,
    nfa_table_type &nfa_table,
    nfa_state_range &first_pattern_start,
    nfa_state_range &pattern_final_state,
    nfa_state_range &left_context_end,
    nfa_state_range &middle_context_end,
    pattern_def_type &pattern_definition,
    nfa_state_range &states_used
);

#endif // !defined(PATPARSE_H)
