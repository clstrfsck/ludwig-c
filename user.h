#ifndef USER_H
#define USER_H

#include "type.h"

bool user_key_code_to_name(key_code_range key_code, key_name_str &key_name);
bool user_key_name_to_code(const key_name_str &key_name, key_code_range &key_code);
void user_key_initialize();
bool user_command_introducer();
bool user_key(const tpar_object &key, const tpar_object &strng);
bool user_parent();
bool user_subprocess();
bool user_undo();

#endif
