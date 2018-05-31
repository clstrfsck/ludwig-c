/**********************************************************************}
{                                                                      }
{            L      U   U   DDDD   W      W  IIIII   GGGG              }
{            L      U   U   D   D   W    W     I    G                  }
{            L      U   U   D   D   W ww W     I    G   GG             }
{            L      U   U   D   D    W  W      I    G    G             }
{            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              }
{                                                                      }
{**********************************************************************}
{                                                                      }
{   Copyright (C) 1981, 1987                                           }
{   Department of Computer Science, University of Adelaide, Australia  }
{   All rights reserved.                                               }
{   Reproduction of the work or any substantial part thereof in any    }
{   material form whatsoever is prohibited.                            }
{                                                                      }
{**********************************************************************/

/**
! Name:         USER
!
! Description:  The user commands (UC, UK, UP, US, and UU).
*/

#include "user.h"

#include "var.h"
#include "vdu.h"
#include "code.h"
#include "mark.h"
#include "span.h"
#include "text.h"
#include "tpar.h"
#include "screen.h"

namespace {
    bool special_command(commands cmd) {
        return cmd == commands::cmd_verify ||
            cmd == commands::cmd_exit_abort ||
            cmd == commands::cmd_exit_fail ||
            cmd == commands::cmd_exit_success;
    }
};

bool user_key_code_to_name(key_code_range key_code, key_name_str &key_name) {
    key_name_list_ptr[0].key_code = key_code;
    int i = nr_key_names;
    while (key_name_list_ptr[i].key_code != key_code)
        i -= 1;
    if (i != 0) {
        key_name = key_name_list_ptr[i].key_name;
        return true;
    }
    return false;
}

bool user_key_name_to_code(const key_name_str &key_name, key_code_range &key_code) {
    key_name_list_ptr[0].key_name = key_name;
    int i = nr_key_names;
    while (key_name_list_ptr[i].key_name != key_name)
        i -= 1;
    if (i != 0) {
        key_code = key_name_list_ptr[i].key_code;
        return true;
    }
    return false;
}



void user_key_initialize() {
    // WARNING - A value of 40 for key_name_len is assumed here

    //  var
    //    key_code : key_code_range;
    //    tpar : tpar_ptr;

    // Initialize terminal-defined key map table.
    key_code_range key_code;
    vdu_keyboard_init(nr_key_names, key_name_list_ptr, key_introducers, terminal_info);
    if (user_key_name_to_code(key_name_str("UP-ARROW                                "), key_code))
        lookup[key_code].command = commands::cmd_up;
    if (user_key_name_to_code(key_name_str("DOWN-ARROW                              "), key_code))
        lookup[key_code].command = commands::cmd_down;
    if (user_key_name_to_code(key_name_str("RIGHT-ARROW                             "), key_code))
        lookup[key_code].command = commands::cmd_right;
    if (user_key_name_to_code(key_name_str("LEFT-ARROW                              "), key_code))
        lookup[key_code].command = commands::cmd_left;
    if (user_key_name_to_code(key_name_str("HOME                                    "), key_code))
        lookup[key_code].command = commands::cmd_home;
    if (user_key_name_to_code(key_name_str("BACK-TAB                                "), key_code))
        lookup[key_code].command = commands::cmd_backtab;
    if (user_key_name_to_code(key_name_str("INSERT-CHAR                             "), key_code))
        lookup[key_code].command = commands::cmd_insert_char;
    if (user_key_name_to_code(key_name_str("DELETE-CHAR                             "), key_code))
        lookup[key_code].command = commands::cmd_delete_char;
    if (user_key_name_to_code(key_name_str("INSERT-LINE                             "), key_code))
        lookup[key_code].command = commands::cmd_insert_line;
    if (user_key_name_to_code(key_name_str("DELETE-LINE                             "), key_code))
        lookup[key_code].command = commands::cmd_delete_line;
    if (user_key_name_to_code(key_name_str("HELP                                    "), key_code)) {
        lookup[key_code].command = commands::cmd_help;
        tpar_ptr tpar = new tpar_object;
        //with tpar^ do
        tpar->dlm = TPD_PROMPT;
        tpar->len = 0;
        tpar->str.fill(' ');
        tpar->con = nullptr;
        tpar->nxt = nullptr;
        lookup[key_code].tpar = tpar;
    }
    if (user_key_name_to_code(key_name_str("FIND                                    "), key_code)) {
        lookup[key_code].command = commands::cmd_get;
        tpar_ptr tpar = new tpar_object;
        //with tpar^ do
        tpar->dlm = TPD_PROMPT;
        tpar->len = 0;
        tpar->str.fill(' ');
        tpar->con = nullptr;
        tpar->nxt = nullptr;
        lookup[key_code].tpar = tpar;
    }
    if (user_key_name_to_code(key_name_str("PREV-SCREEN                             "), key_code))
        lookup[key_code].command = commands::cmd_window_backward;
    if (user_key_name_to_code(key_name_str("NEXT-SCREEN                             "), key_code))
        lookup[key_code].command = commands::cmd_window_forward;
    if (user_key_name_to_code(key_name_str("PAGE-UP                                 "), key_code))
        lookup[key_code].command = commands::cmd_window_backward;
    if (user_key_name_to_code(key_name_str("PAGE-DOWN                               "), key_code))
        lookup[key_code].command = commands::cmd_window_forward;
#ifdef WINDOWCHANGE
    if (user_key_name_to_code(key_name_str("WINDOW-RESIZE-EVENT                     "), key_code))
        lookup[key_code].command = commands::cmd_resize_window;
#endif
}

bool user_command_introducer() {
    //  var
    //    temp : str_object;
    //    cmd_success : boolean;

    if (!printable_set.contains(command_introducer)) {
        screen_message(MSG_NONPRINTABLE_INTRODUCER);
        return false;
    } else {
        // enter command introducer into text in correct keyboard mode
        str_object temp;
        temp[1] = command_introducer;
        //with current_frame^ do
        bool cmd_success = true;
        switch (edit_mode) {
        case mode_type::mode_insert:
            cmd_success = text_insert(true, 1, temp, 1, current_frame->dot);
            break;
        case mode_type::mode_command:
            if (previous_mode == mode_type::mode_insert)
                cmd_success = text_insert(true, 1, temp, 1, current_frame->dot);
            else
                cmd_success = text_overtype(true, 1, temp, 1, current_frame->dot);
            break;
        case mode_type::mode_overtype:
            cmd_success = text_overtype(true, 1, temp, 1, current_frame->dot);
            break;
        }
        if (cmd_success) {
            current_frame->text_modified = true;
            if (!mark_create(current_frame->dot->line, current_frame->dot->col,
                             current_frame->marks[MARK_MODIFIED])) {
                cmd_success = false;
            }
        }
        return cmd_success;
    }
}

bool user_key(const tpar_object &key, const tpar_object &strng) {
    //  var
    //    i : integer;
    //    key_span, old_span : span_ptr;
    //    key_code : key_code_range;
    //    key_name : key_name_str;

    bool result = false;
    key_code_range key_code;
    if (key.len == 1) {
        key_code = key.str[1];
    } else {
        key_name_str key_name(' ');
        int use_len = key.len;
        if (use_len > KEY_NAME_LEN) {
            use_len = KEY_NAME_LEN;
            screen_message(MSG_KEY_NAME_TRUNCATED);
        }
        key_name.copy(key.str.data(), use_len);
        if (!user_key_name_to_code(key_name, key_code)) {
            screen_message(MSG_UNRECOGNIZED_KEY_NAME);
            return false;
        }
    }
    // Create a span in frame "HEAP"
    //with frame_heap^ do
    if (!mark_create(frame_heap->last_group->last_line, 1, frame_heap->span->mark_two))
        return false;
    name_str blank_frame_name(BLANK_FRAME_NAME);
    if (!span_create(blank_frame_name, frame_heap->span->mark_two, frame_heap->span->mark_two))
        return false;
    span_ptr key_span;
    span_ptr old_span;
    if (span_find(blank_frame_name, key_span, old_span)) {
        if (!text_insert_tpar(strng, key_span->mark_two, key_span->mark_one))
            goto l98;
        if (!code_compile(*key_span, true))
            goto l98;
        // discard code_ptr, if it exists, NOW!
        if (lookup[key_code].code != nullptr)
            code_discard(lookup[key_code].code);
        if (lookup[key_code].tpar != nullptr) {
            tpar_clean_object(*lookup[key_code].tpar);
            delete lookup[key_code].tpar;
            lookup[key_code].tpar = nullptr;
        }
        //with key_span^ do
        {
            code_ptr &code(key_span->code);
            if ((code->len == 2)&& (compiler_code[code->code].rep == leadparam::none) &&
                !special_command(compiler_code[code->code].op)) {
                // simple command, put directly into lookup table.
                lookup[key_code].command = compiler_code[code->code].op;
                lookup[key_code].tpar = compiler_code[code->code].tpar;
                compiler_code[code->code].tpar = nullptr;
            } else {
                lookup[key_code].command = commands::cmd_extended;
                lookup[key_code].code = code;
                code = nullptr;
            }
        }
        result = true;
    l98:;
        span_destroy(key_span);
    }
    return result;
}

#ifdef XXXX
function user_parent 
        : boolean;

  begin {user_parent}
{#if vms}
{##  user_parent = vms_attach_parent;}
{#elseif unix}
{##  user_parent = unix_suspend;}
{#elseif fpc}
  user_parent = fpc_suspend;
{#endif}
  end; {user_parent}

function user_subprocess 
        : boolean;

  begin {user_subprocess}
{#if vms}
{##  user_subprocess = vms_subprocess;}
{#elseif unix}
{##  user_subprocess = unix_shell;}
{#elseif fpc}
  user_subprocess = fpc_shell;
{#endif}
  end; {user_subprocess}

function user_undo 
        : boolean;

  begin {user_undo}
  user_undo = false;
  screen_message(msg_not_implemented);
  end; {user_undo}

{#if vms or turbop}
end.
{#endif}
#endif
 
