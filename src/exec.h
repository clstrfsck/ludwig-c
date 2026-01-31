#ifndef EXEC_H
#define EXEC_H

#include "type.h"

[[nodiscard]] bool exec_compute_line_range(
    frame_ptr frame, leadparam rept, int count, line_ptr &first_line, line_ptr &last_line
);
[[nodiscard]] bool execute(commands command, leadparam rept, int count, tpar_ptr tparam, bool from_span);

#endif // !defined(EXEC_H)
