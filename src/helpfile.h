#ifndef HELPFILE_H
#define HELPFILE_H

#include "type.h"

bool helpfile_open(bool old_version);

int helpfile_read(const key_str &key, int keylen, help_record &buf, int &reclen);
int helpfile_next(help_record &buf, int &reclen);

#endif
