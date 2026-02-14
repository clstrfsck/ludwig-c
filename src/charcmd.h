#ifndef CHARCMD_H
#define CHARCMD_H

#include "type.h"

[[nodiscard]] bool charcmd_insert(commands cmd, leadparam rept, int count, bool from_span);
[[nodiscard]] bool charcmd_delete(commands cmd, leadparam rept, int count, bool from_span);
[[nodiscard]] bool charcmd_rubout(commands cmd, leadparam rept, int count, bool from_span);

#endif
