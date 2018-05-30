#ifndef FRAME_H
#define FRAME_H

#include "type.h"

bool frame_edit(name_str frame_name);
bool frame_kill(const name_str &frame_name);

bool frame_setheight(int sh, bool set_initial);
bool frame_parameter(tpar_ptr tpar);

#endif
