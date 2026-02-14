#ifndef USER_H
#define USER_H

#include "type.h"

[[nodiscard]] bool user_key_code_to_name(key_code_range key_code, std::string &key_name);
[[nodiscard]] bool user_key_name_to_code(const std::string &key_name, key_code_range &key_code);
void user_key_initialize();
[[nodiscard]] bool user_command_introducer();
[[nodiscard]] bool user_key(const tpar_object &key, const tpar_object &strng);
[[nodiscard]] bool user_parent();
[[nodiscard]] bool user_subprocess();
[[nodiscard]] bool user_undo();

#endif
