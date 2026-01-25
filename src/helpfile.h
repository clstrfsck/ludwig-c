#ifndef HELPFILE_H
#define HELPFILE_H

#include "type.h"

bool helpfile_open(bool old_version);
bool helpfile_open(const std::string_view &filename);
void helpfile_close();

bool helpfile_read(const key_str &key, help_record &buf);
bool helpfile_next(help_record &buf);

#endif
