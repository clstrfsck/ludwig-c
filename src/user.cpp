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
!
! $Log: user.pas,v $
! Revision 4.11  2002/07/21 02:28:03  martin
! Added suspend and shell callouts for fpc port. MPS
!
! Revision 4.10  1991/02/22 15:13:36  ludwig
! added default bindings for the X mouse functions
!
! Revision 4.9  90/09/21  12:43:26  ludwig
! Change name of IBM-PC module system to msdos (system is reserved name).
! 
! Revision 4.8  90/02/08  10:14:30  ludwig
! changed pcc conditional now that I know the syntax.
!
! Revision 4.7  90/02/05  13:45:21  ludwig
! Steven Nairn.
! Added WINDOW-RESIZE-EVENT to list of predefined keys for xwindows and
! windowchange.
!
! Revision 4.6  90/01/18  17:17:35  ludwig
! Entered into RCS at revision level 4.6.
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Jeff Blows                                           28-Jun-1989
!       Added "page-up" and "page-down" to the predefined list of key
!       names.
! 4-003 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-004 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-005 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-006 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!**/

#include "user.h"

#include "sys.h"
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

bool user_key_code_to_name(key_code_range key_code, std::string &key_name) {
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

bool user_key_name_to_code(const std::string &key_name, key_code_range &key_code) {
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
    // Initialize terminal-defined key map table.
    key_code_range key_code;
    vdu_keyboard_init(nr_key_names, key_name_list_ptr, key_introducers, terminal_info);
    if (user_key_name_to_code(std::string("UP-ARROW"), key_code))
        lookup[key_code].command = commands::cmd_up;
    if (user_key_name_to_code(std::string("DOWN-ARROW"), key_code))
        lookup[key_code].command = commands::cmd_down;
    if (user_key_name_to_code(std::string("RIGHT-ARROW"), key_code))
        lookup[key_code].command = commands::cmd_right;
    if (user_key_name_to_code(std::string("LEFT-ARROW"), key_code))
        lookup[key_code].command = commands::cmd_left;
    if (user_key_name_to_code(std::string("HOME"), key_code))
        lookup[key_code].command = commands::cmd_home;
    if (user_key_name_to_code(std::string("BACK-TAB"), key_code))
        lookup[key_code].command = commands::cmd_backtab;
    if (user_key_name_to_code(std::string("INSERT-CHAR"), key_code))
        lookup[key_code].command = commands::cmd_insert_char;
    if (user_key_name_to_code(std::string("DELETE-CHAR"), key_code))
        lookup[key_code].command = commands::cmd_delete_char;
    if (user_key_name_to_code(std::string("INSERT-LINE"), key_code))
        lookup[key_code].command = commands::cmd_insert_line;
    if (user_key_name_to_code(std::string("DELETE-LINE"), key_code))
        lookup[key_code].command = commands::cmd_delete_line;
    if (user_key_name_to_code(std::string("HELP"), key_code)) {
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
    if (user_key_name_to_code(std::string("FIND"), key_code)) {
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
    if (user_key_name_to_code(std::string("PREV-SCREEN"), key_code))
        lookup[key_code].command = commands::cmd_window_backward;
    if (user_key_name_to_code(std::string("NEXT-SCREEN"), key_code))
        lookup[key_code].command = commands::cmd_window_forward;
    if (user_key_name_to_code(std::string("PAGE-UP"), key_code))
        lookup[key_code].command = commands::cmd_window_backward;
    if (user_key_name_to_code(std::string("PAGE-DOWN"), key_code))
        lookup[key_code].command = commands::cmd_window_forward;
    if (user_key_name_to_code(std::string("WINDOW-RESIZE-EVENT"), key_code))
        lookup[key_code].command = commands::cmd_resize_window;
}

bool user_command_introducer() {
    if (!PRINTABLE_SET.contains(command_introducer)) {
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
    bool result = false;
    key_code_range key_code;
    if (key.len == 1) {
        key_code = key.str[1];
    } else {
        std::string key_name(key.str.data(), key.len);
        if (!user_key_name_to_code(key_name, key_code)) {
            screen_message(MSG_UNRECOGNIZED_KEY_NAME);
            return false;
        }
    }
    // Create a span in frame "HEAP"
    //with frame_heap^ do
    if (!mark_create(frame_heap->last_group->last_line, 1, frame_heap->span->mark_two))
        return false;
    if (!span_create(BLANK_FRAME_NAME, frame_heap->span->mark_two, frame_heap->span->mark_two))
        return false;
    span_ptr key_span;
    span_ptr old_span;
    if (span_find(BLANK_FRAME_NAME, key_span, old_span)) {
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

bool user_parent() {
    return sys_suspend();
}

bool user_subprocess() {
    return sys_shell();
}

bool user_undo() {
    screen_message(MSG_NOT_IMPLEMENTED);
    return false;
}
