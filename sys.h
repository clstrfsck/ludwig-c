#ifndef SYS_H
#define SYS_H

// The intention is that anything that requires something that
// is not part of the C++ standard library will have an interface
// routine here that can be implemented appropriately on various
// operating systems.

#include <string>

// Initialisation and tear-down
void sys_initsig();
void sys_exit_success();
void sys_exit_failure();

// O/S query and support
bool sys_suspend();
bool sys_shell();
bool sys_istty();
bool sys_getenv(const std::string &environ, std::string &result);

#endif // !defined(SYS_H)
