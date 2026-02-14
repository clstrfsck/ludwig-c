#ifndef HELPFILE_H
#define HELPFILE_H

#include "type.h"

[[nodiscard]] bool helpfile_open(bool old_version);
[[nodiscard]] bool helpfile_open(const std::string_view &filename);
void helpfile_close();

[[nodiscard]] bool helpfile_read(const std::string &key, help_record &buf);
[[nodiscard]] bool helpfile_next(help_record &buf);

#endif
