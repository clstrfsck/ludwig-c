#ifndef VDU_H
#define VDU_H

#include "type.h"

void vdu_movecurs(scr_col_range x, scr_row_range y);

void vdu_flush();

void vdu_beep();

void vdu_displaych(char ch);
void vdu_displaystr(scr_col_range strlen, const char *str, int opts);

void vdu_cleareol();
void vdu_cleareos();
void vdu_clearscr();

void vdu_scrollup(int n);

void vdu_deletelines(int n);
void vdu_insertlines(int n);

void vdu_insertchars(scr_col_range n);
void vdu_deletechars(scr_col_range n);

void vdu_displaycrlf();

void vdu_take_back_key(key_code_range key);

void vdu_new_introducer(key_code_range key);

key_code_range vdu_get_key();

void vdu_get_input(const std::string_view &prompt,
                   str_object &get, strlen_range get_len,
                   strlen_range &outlen);

void vdu_insert_mode(bool turn_on);

void vdu_get_text(int str_len, str_object &str, strlen_range &outlen);

void vdu_keyboard_init(key_names_range &nr_key_names,
                       std::vector<key_name_record> &key_name_list,
                       accept_set_type &key_introducers,
                       terminal_info_type &terminal_info);

bool vdu_init(terminal_info_type &terminal_info,
              bool &ctrl_c_flag, bool &winchange_flag);

void vdu_free();

void vdu_get_new_dimensions(scr_col_range &new_x, scr_row_range &new_y);

void vdu_attr_bold();
void vdu_attr_normal();

#endif // !define(VDU_H)
