#ifndef FYLE_H
#define FYLE_H

#include "type.h"

void file_name(file_ptr fp, size_t max_len, file_name_str &act_fnm);
void file_table();
void file_fix_eop(bool eof, line_ptr eop_line);

bool file_create_open(file_name_str &fn, parse_type parse, file_ptr &inputfp, file_ptr &outputfp);
bool file_close_delete(file_ptr &fp, bool delet, bool messages);
bool file_read(file_ptr fp, line_range count, bool best_try, line_ptr &first, line_ptr &last, int &actual_cnt);
bool file_write(line_ptr first_line, const_line_ptr last_line, file_ptr fp);
bool file_windthru(frame_ptr current, bool from_span);
bool file_rewind(file_ptr &fp);
bool file_page(frame_ptr current_frame, bool &exit_abort);
bool file_command(commands command, leadparam rept, int count, const_tpar_ptr tparam, bool from_span);

#endif // !defined(FYLE_H)
