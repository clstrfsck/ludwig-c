#ifndef SPAN_H
#define SPAN_H

#include "type.h"

bool span_find(const std::string &span_name, span_ptr &ptr, span_ptr &oldp);
bool span_create(const std::string &span_name, mark_ptr first_mark, mark_ptr last_mark);
bool span_destroy(span_ptr &span);
bool span_index();

#endif // !defined(SPAN_H)
