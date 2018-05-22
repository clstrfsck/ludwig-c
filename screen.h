#ifndef SCREEN_H
#define SCREEN_H

#include "type.h"

void screen_message(const msg_str &message);
void screen_message(const char *message);
void screen_str_message(const str_object &message);

void screen_draw_line(line_ptr line);
void screen_redraw();
void screen_slide(int dist);
void screen_unload();
void screen_scroll(int count, bool expand);
void screen_lines_extract(line_ptr first_line, line_ptr last_line);
void screen_lines_inject(line_ptr first_line, line_range count, line_ptr before_line);
void screen_load(line_ptr line, col_range col);
void screen_position(line_ptr new_line, col_range new_col);
void screen_pause();
void screen_clear_msgs(bool pause);
void screen_fixup();
void screen_getlinep(const str_object &prompt, strlen_range prompt_len,
                     str_object &outbuf, strlen_range &outlen,
                     tpcount_type max_tp, tpcount_type this_tp);
void screen_free_bottom_line();
verify_response screen_verify(const str_object &prompt, strlen_range prompt_len);
void screen_beep();
void screen_home(bool clear);
void screen_write_int(int int_, size_t width);
void screen_write_ch(scr_col_range indent, char ch);
void screen_write_str(scr_col_range indent, const char *str);
void screen_write_name_str(scr_col_range indent, const name_str &str, size_t width);
void screen_write_file_name_str(scr_col_range indent, const file_name_str &str, size_t width);
void screen_writeln();
void screen_writeln_clel();
void screen_help_prompt(const write_str &prompt, strlen_range prompt_len,
                        key_str &reply, int &reply_len);
void screen_resize();

#endif // !defined(SCREEN_H)
