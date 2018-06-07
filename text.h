#ifndef TEXT_H
#define TEXT_H

#include "type.h"

col_range text_return_col(line_ptr cur_line, col_range cur_col, bool splitting);
bool text_realize_null(line_ptr old_null);
bool text_insert(bool update_screen, int count, str_object buf, strlen_range buf_len, mark_ptr dst);
bool text_overtype(bool update_screen, int count, str_object buf, strlen_range buf_len, mark_ptr &dst);
bool text_insert_tpar(tpar_object tp, mark_ptr before_mark, mark_ptr &equals_mark);
bool text_remove(mark_ptr mark_one, mark_ptr mark_two);
bool text_move(bool copy, int count, mark_ptr mark_one, mark_ptr mark_two, mark_ptr dst,
               mark_ptr &new_start, mark_ptr &new_end);
bool text_split_line(mark_ptr before_mark, int new_col, mark_ptr &equals_mark);

#endif // !defined(TEXT_H)
