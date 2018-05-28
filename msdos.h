#ifndef MSDOS_H
#define MSDOS_H

#include "type.h"

bool fpc_suspend();
bool fpc_shell();

void msdos_exit(int status);

bool cvt_int_str(int  num, str_object &strng, scr_col_range width);
bool cvt_str_int(int &num, const str_object &strng);
bool get_environment(const std::string &environ, strlen_range &reslen, str_object &result);
void init_signals();
void exit_handler(int sig);

#endif
