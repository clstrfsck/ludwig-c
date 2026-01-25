#ifndef FRAME_H
#define FRAME_H

#include "type.h"

bool frame_edit(const std::string_view &frame_name);
bool frame_kill(const std::string_view &frame_name);

bool frame_setheight(int sh, bool set_initial);
bool frame_parameter(tpar_ptr tpar);

#endif
