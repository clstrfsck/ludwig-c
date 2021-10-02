#ifndef LINE_H
#define LINE_H

#include "type.h"

bool line_eop_create(frame_ptr inframe, group_ptr &group);
bool line_eop_destroy(group_ptr &group);
bool lines_create(line_range line_count, line_ptr &first_line, line_ptr &last_line);
bool lines_destroy(line_ptr &first_line, line_ptr &last_line);
bool lines_inject(line_ptr first_line, line_ptr last_line, line_ptr before_line);
bool lines_extract(line_ptr first_line, line_ptr last_line);
bool line_change_length(line_ptr line, strlen_range new_length);
bool line_to_number(line_ptr line, line_range &number);
bool line_from_number(frame_ptr frame, line_range nummber, line_ptr &line);

#endif // !defined(LINE_H)
