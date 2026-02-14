#ifndef OPSYS_H
#define OPSYS_H

#include "type.h"

[[nodiscard]] bool opsys_command(const tpar_object &command, line_ptr &first, line_ptr &last, int &actual_cnt);

#endif // !defined(OPSYS_H)
