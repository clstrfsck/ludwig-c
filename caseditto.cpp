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
! Name:         CASEDITTO
!
! Description:  The Case change and Ditto commands.
*/

#include "caseditto.h"

#include "var.h"
#include "vdu.h"
#include "mark.h"
#include "text.h"
#include "screen.h"

#include <unordered_set>

const std::string LETTERS_S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
const std::unordered_set<char> LETTERS(std::begin(LETTERS_S), std::end(LETTERS_S));

char char_toupper(char ch) {
    return std::toupper(ch);
}
     
char char_tolower(char ch) {
    return std::tolower(ch);
}

bool key_is_lower(key_code_range key) {
    if (key < 0)
        return false;
    return lower_set.contains(key.value());
}

bool caseditto_command(commands command, leadparam rept, int count, bool from_span) {
    //  label 9, 99;
    //  var
    //    cmd_status,
    //    cmd_valid   : boolean;
    //    old_dot_col,
    //    first_col,
    //    new_col     : col_range;
    //    old_str,
    //    new_str     : str_object;
    //    command_set : set of commands;
    //    other_line  : line_ptr;
    //    key,
    //    key_up      : key_code_range;
    //    ch          : char;
    //    i           : integer;
    //    insert      : boolean;

    bool cmd_status = false;
    bool insert = (command == commands::cmd_ditto_up || command == commands::cmd_ditto_down) &&
        ((edit_mode == mode_type::mode_insert) ||
         ((edit_mode == mode_type::mode_command) && (previous_mode == mode_type::mode_insert)));
    //with current_frame^,dot^ do
    // Remember current line.
    col_range old_dot_col = current_frame->dot->col;

    str_object old_str;
    old_str.fillcopy(current_frame->dot->line->str->data(),
                     current_frame->dot->line->used, 1, MAX_STRLEN, ' ');
    penumset<commands> command_set;
    line_ptr other_line;
    switch (command) {
    case commands::cmd_case_up:
    case commands::cmd_case_low:
    case commands::cmd_case_edit: {
        command_set.add({commands::cmd_case_up, commands::cmd_case_low, commands::cmd_case_edit});
        other_line = current_frame->dot->line;
    }
        break;
    case commands::cmd_ditto_up:
    case commands::cmd_ditto_down: {
        if (insert && (rept == leadparam::minus || rept == leadparam::nint || rept == leadparam::nindef)) {
            screen_message(MSG_NOT_ALLOWED_IN_INSERT_MODE);
            return false;
        }
        command_set.add({commands::cmd_ditto_up, commands::cmd_ditto_down});
    }
        break;
    default:
        // Ignored - probably should be an error
        break;
    }

    col_range first_col;
    col_range new_col;
    key_code_range key;
    do {
        if (command == commands::cmd_ditto_up)
            other_line  = current_frame->dot->line->blink;
        else if (command == commands::cmd_ditto_down)
            other_line  = current_frame->dot->line->flink;
        bool cmd_valid = true;
        if (other_line != nullptr) {
            //with other_line^ do
            switch (rept) {
            case leadparam::none:
            case leadparam::plus:
            case leadparam::pint: {
                if ((count != 0) && (current_frame->dot->col + count > other_line->used + 1))
                    cmd_valid = false;
                first_col = current_frame->dot->col;
                new_col   = current_frame->dot->col + count;
            }
                break;
            case leadparam::pindef: {
                count     = other_line->used + 1 - current_frame->dot->col;
                if (count < 0)
                    cmd_valid = false;
                first_col = current_frame->dot->col;
                new_col   = other_line->used + 1;
            }
                break;
            case leadparam::minus:
            case leadparam::nint: {
                count     = -count;
                if (count >= current_frame->dot->col)
                    cmd_valid = false;
                else
                    first_col = current_frame->dot->col - count;
                new_col   = first_col;
            }
                break;
            case leadparam::nindef: {
                count     = current_frame->dot->col - 1;
                first_col = 1;
                new_col   = 1;
            }
                break;
            default:
                // Ignore others
                break;
            }
        } else {
            cmd_valid = false;
        }

        // Carry out the command.
        if (cmd_valid) {
            //with other_line^ do
            int i = other_line->used + 1 - first_col;
            str_object new_str;
            if (i <= 0)
                new_str.fill_n(' ', count);
            else
                new_str.fillcopy(other_line->str->data(first_col), i, 1, count, ' ');
            switch (command) {
            case commands::cmd_case_up:
                new_str.apply_n(char_toupper, count);
                break;
            case commands::cmd_case_low:
                new_str.apply_n(char_tolower, count);
                break;
            case commands::cmd_case_edit: {
                char ch;
                if ((1 < first_col) && (first_col <= other_line->used))
                    ch = other_line->str->operator[](first_col - 1);
                else
                    ch = ' ';
                for (int i = 0; i < count; ++i) {
                    if (LETTERS.find(ch) != LETTERS.end())
                        ch = std::tolower(new_str[i]);
                    else
                        ch = std::toupper(new_str[i]);
                    new_str[i] = ch;
                }
            }
                break;
            case commands::cmd_ditto_up:
            case commands::cmd_ditto_down:
                // No massaging required.
                break;
            default:
                // Others ignored.
                break;
            }
            current_frame->dot->col = first_col;
            if (insert) {
                if (!text_insert(true, 1, new_str, count, current_frame->dot))
                    goto l9;
            } else {
                if (!text_overtype(true, 1, new_str, count, current_frame->dot))
                    goto l9;
            }
            // Reposition dot.
            current_frame->dot->col = new_col;
            cmd_status = true;
        }
        if (from_span)
            goto l9;
        if (cmd_valid)
            screen_fixup();
        else
            vdu_beep();
        key = vdu_get_key();
        if (tt_controlc)
            goto l9;
        key_code_range key_up;
        if (key_is_lower(key))
            key_up = key - 32; // Uppercase it!
        else
            key_up = key;
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
        case leadparam::pint:
        case leadparam::pindef:
            rept = leadparam::plus;
            count = +1;
            break;
        case leadparam::minus:
        case leadparam::nint:
        case leadparam::nindef:
            rept = leadparam::minus;
            count = -1;
            break;
        default:
            // Nothing
            break;
        }
        if (command == commands::cmd_ditto_up || command == commands::cmd_ditto_down)
            command = lookup[key_up].command;
        else if (key_up == 'E')
            command = commands::cmd_case_edit;
        else if (key_up == 'L')
            command = commands::cmd_case_low;
        else if (key_up == 'U')
            command = commands::cmd_case_up;
        else
            command = commands::cmd_noop;
    } while (command_set.contains(command));
    vdu_take_back_key(key);

 l9:;
    if (tt_controlc) {
        cmd_status = false;
        current_frame->dot->col = 1;
        text_overtype(false, 1, old_str, MAX_STRLEN, current_frame->dot);
        current_frame->dot->col = old_dot_col;
    } else if (cmd_status) {
        current_frame->text_modified = true;
        mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]);
        mark_create(current_frame->dot->line, old_dot_col, current_frame->marks[MARK_EQUALS]);
    }
    return cmd_status || !from_span;
}