#ifndef FILESYS_H
#define FILESYS_H

#include "type.h"

bool filesys_create_open(file_ptr fyle, file_ptr related_file, bool ordinary_open);
bool filesys_close(file_ptr fyle, int action, bool msgs);

bool filesys_read(file_ptr fyle, str_object &buffer, strlen_range &outlen);
bool filesys_rewind(file_ptr fyle);
bool filesys_write(file_ptr fyle, str_object &buffer, strlen_range bufsiz);

bool filesys_save(file_ptr i_fyle, file_ptr o_file, int copy_lines);

bool filesys_parse(file_name_str &command_line, parse_type parse,
                   file_data_type &file_data, file_ptr &input_file, file_ptr &output_file);

#endif
