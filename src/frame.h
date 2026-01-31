#ifndef FRAME_H
#define FRAME_H

#include "type.h"

[[nodiscard]] bool frame_edit(const std::string_view &frame_name);
[[nodiscard]] bool frame_kill(const std::string_view &frame_name);

[[nodiscard]] bool frame_setheight(int sh, bool set_initial);
[[nodiscard]] bool frame_parameter(tpar_ptr tpar);

#endif
