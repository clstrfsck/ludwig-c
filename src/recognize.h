#ifndef RECOGNIZE_H
#define RECOGNIZE_H

#include "type.h"

bool pattern_recognize(
    dfa_table_ptr dfa_table_pointer,
    line_ptr line,
    col_range start_col,
    bool &mark_flag,
    col_range &start_pos,
    col_range &finish_pos
);

#endif // !defined(RECOGNIZE_H)
