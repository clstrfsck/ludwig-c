#ifndef MARK_H
#define MARK_H

#include "type.h"

bool mark_create(line_ptr in_line, col_range column, mark_ptr &mark);
bool mark_destroy(mark_ptr &mark);
bool marks_squeeze(
    const line_ptr first_line,
    col_range first_column,
    const line_ptr last_line,
    col_range last_column
);
bool marks_shift(
    const line_ptr source_line,
    col_range source_column,
    col_width_range width,
    const line_ptr dest_line,
    col_range dest_column
);

// Visible for testing
size_t marks_allocated();

#endif // !define(MARK_H)
