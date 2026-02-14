#ifndef SPAN_H
#define SPAN_H

#include "type.h"

[[nodiscard]] bool span_find(const std::string_view &span_name, span_ptr &ptr, span_ptr &oldp);
[[nodiscard]] bool span_create(const std::string_view &span_name, mark_ptr first_mark, mark_ptr last_mark);
bool span_destroy(span_ptr &span);
[[nodiscard]] bool span_index();

#endif // !defined(SPAN_H)
