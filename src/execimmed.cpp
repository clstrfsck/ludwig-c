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
! Name:         EXECIMMED
!
! Description:  Outermost level of command execution for LUDWIG.
!**/

#include "execimmed.h"

#include "var.h"
#include "vdu.h"
#include "code.h"
#include "exec.h"
#include "fyle.h"
#include "line.h"
#include "mark.h"
#include "quit.h"
#include "text.h"
#include "screen.h"

#include <cstring>
#include <iostream>

namespace {
    const std::string DEFAULT_SPAN_NAME("L. Wittgenstein und Sohn."); // Editors Extraordinaire

    void write(const char *message, bool flush = true) {
        std::cout << message;
        if (flush)
            std::cout.flush();
    }

    void writeln(const char *message) {
        write(message, false);
        std::cout << std::endl;
    }
};

void execute_immed() {
    scr_col_range scr_col;
    span_ptr cmd_span = new span_object;
    //with cmd_span^ do
    cmd_span->flink    = nullptr;
    cmd_span->blink    = nullptr;
    cmd_span->name     = DEFAULT_SPAN_NAME;
    cmd_span->frame    = nullptr;
    cmd_span->mark_one = nullptr;
    cmd_span->mark_two = nullptr;
    cmd_span->code     = nullptr;

    // Vector off to the appropriate main execution mode.  Each mode behaves
    // slightly differently at this level.
    switch (ludwig_mode) {
    case ludwig_mode_type::ludwig_screen: {
        bool jammed = false;
        bool cmd_success;
        while (true) {
    l2:;
            cmd_success = true;

            // MAKE SURE THE USER CAN SEE THE CURRENT DOT POSITION.
            screen_fixup();

            key_code_range key;
            if (edit_mode == mode_type::mode_command) {
                key = command_introducer;
            } else {
                // OVERTYPE/INSERTMODE IS DONE HERE AS A SPECIAL CASE.
                // THIS IS NECESSARY BECAUSE THE SCREEN IS UPDATED BY
                // VDU_GET_TEXT.
                //with current_frame^,dot^ do
                while (true) {
                    // Check for boundaries where text cannot be accepted.
                    if (jammed || (current_frame->dot->col == MAX_STRLENP)) {
                        key = vdu_get_key();
                        if (tt_controlc)
                            goto l9;
                        if (PRINTABLE_SET.contains(key) && key != command_introducer) {
                            cmd_success = false;
                            goto l9;
                        }
                        vdu_take_back_key(key);
                        break;
                    }

                    // DECIDE MAX CHARS THAT CAN BE READ.
                    scr_col_range tmp_scr_col = current_frame->dot->col - current_frame->scr_offset;
                    strlen_range input_len = MAX_STRLENP - current_frame->dot->col;
                    if (current_frame->dot->col <= current_frame->margin_right)
                        input_len = current_frame->margin_right - current_frame->dot->col + 1;
                    if (input_len > current_frame->scr_width + 1 - tmp_scr_col)
                        input_len = current_frame->scr_width + 1 - tmp_scr_col;

                    // WATCH OUT FOR NULL LINE.
                    if (current_frame->dot->line->flink == nullptr) {
                        key = vdu_get_key();
                        if (tt_controlc)
                            goto l9;
                        vdu_take_back_key(key);
                        if (PRINTABLE_SET.contains(key) && key != command_introducer) {
                            // If printing char, realize NULL, re-fix cursor.
                            if (!text_realize_null(current_frame->dot->line)) {
                                cmd_success = false;
                                goto l9;
                            }
                        }

                        // MAKE SURE THE USER CAN SEE THE CURRENT DOT POSITION.
                        screen_fixup();
                    }

                    // GET THE ECHOING TEXT
                    if (edit_mode == mode_type::mode_insert)
                        vdu_insert_mode(true);
                    str_object input_buf;
                    vdu_get_text(input_len, input_buf, input_len);
                    if (edit_mode == mode_type::mode_insert) {
                        vdu_insert_mode(false);
                        vdu_flush(); // Make sure in mode IS off!
                    }
                    if (tt_controlc)
                        goto l9;
                    if (input_len == 0)
                        break; // Simulate a continue

                    if (edit_mode == mode_type::mode_overtype)
                        cmd_success = text_overtype(false, 1, input_buf, input_len, current_frame->dot);
                    else
                        cmd_success = text_insert(false, 1, input_buf, input_len, current_frame->dot);
                    if (cmd_success) {
                        current_frame->text_modified = true;
                        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
                            cmd_success = false;
                        if (!mark_create(current_frame->dot->line, current_frame->dot->col - input_len,
                                         current_frame->marks[MARK_EQUALS]))
                            cmd_success = false;
                    } else {
                        // IF, FOR SOME REASON, THAT FAILED,  CORRECT THE VDU IMAGE OF
                        // THE LINE.  THIS IS BECAUSE VDU_GET_TEXT HAS CORRUPTED IT.
                        screen_draw_line(current_frame->dot->line);
                        goto l9;
                    }
                    if (current_frame->dot->col != current_frame->margin_right + 1) {
                        // FOLLOW THE DOT.
                        screen_position(current_frame->dot->line, current_frame->dot->col);
                        vdu_movecurs(current_frame->dot->col - current_frame->scr_offset, current_frame->dot->line->scr_row_nr);
                    } else {
                        // AT THE RIGHT MARGIN.
                        if (current_frame->options.contains(frame_options_elts::opt_auto_wrap)) {
                            // Take care of Wrap Option.
                            key = vdu_get_key();
                            if (tt_controlc)
                                goto l9;
                            if (PRINTABLE_SET.contains(key) && key != command_introducer) {
                                //with dot^ do
                                col_range col_1 = current_frame->margin_right;
                                if (key != ' ') {
                                    while ((*current_frame->dot->line->str)[col_1] != ' ' && col_1 > current_frame->margin_left)
                                        col_1 -= 1;
                                    col_range col_2 = col_1;
                                    while ((*current_frame->dot->line->str)[col_2] == ' ' && col_2 > current_frame->margin_left)
                                        col_2 -= 1;
                                    if (col_2 == current_frame->margin_left) // Line has only one word
                                        col_1 = current_frame->margin_right; // Split at right margin
                                    vdu_take_back_key(key);
                                }
                                current_frame->dot->col += 1;
                                cmd_success = text_split_line(current_frame->dot, 0, current_frame->marks[MARK_EQUALS]);
                                current_frame->dot->col += current_frame->margin_right - col_1;
                                goto l2; // Simulate break of inner loop
                            }
                            vdu_take_back_key(key);
                        } else {
                            vdu_beep();
                            current_frame->dot->col -= 1;
                            vdu_movecurs(current_frame->dot->col - current_frame->scr_offset, current_frame->dot->line->scr_row_nr);
                            jammed = true;
                        }
                    }
                } // of overtyping loop

                key = vdu_get_key(); // key is a terminator
                if (tt_controlc)
                    goto l9;

#ifdef DEBUG
                if (key >= 0) {
                    if (PRINTABLE_SET.contains(key) && key != command_introducer) {
                        screen_message(DBG_NOT_IMMED_CMD);
                        return;
                    }
                }
#endif
            }

            if (key == command_introducer) {
                if (code_compile(*cmd_span, false))
                    cmd_success = code_interpret(leadparam::none, 1, cmd_span->code, false);
                else
                    cmd_success = false;
            } else {
                //with lookup[key] do
                if (lookup[key].command == commands::cmd_extended)
                    cmd_success = code_interpret(leadparam::none, 1, lookup[key].code, true);
                else
                    cmd_success = execute(lookup[key].command, leadparam::none, 1, lookup[key].tpar, false);
            }

    l9:;
            if (tt_controlc) {
                tt_controlc = false;
                if (current_frame->dot->line->scr_row_nr != 0)
                    screen_redraw();
                else
                    screen_unload();
            } else if (!cmd_success) {
                vdu_beep();         // Complain.
                vdu_flush();        // Make sure he hears the complaint.
            } else {
                jammed = false;
            }
            exit_abort = false;
        }
    }
        break;

    case ludwig_mode_type::ludwig_hardcopy:
    case ludwig_mode_type::ludwig_batch: {
        //with cmd_span^ do
        cmd_span->mark_one = new mark_object;
        cmd_span->mark_one->line = nullptr;
        cmd_span->mark_one->col = 1;
        cmd_span->mark_two = new mark_object;
        cmd_span->mark_two->line = nullptr;
        int cmd_count;
        if (ludwig_mode == ludwig_mode_type::ludwig_hardcopy)
            cmd_count = 1;
        else
            cmd_count = MAXINT;

        // Open standard input as Ludwig command input file.
        file_name_str cmd_fnm;
        file_ptr cmd_file = nullptr;
        file_ptr dummy_fptr = nullptr;
        if (file_create_open(cmd_fnm, parse_type::parse_stdin, cmd_file, dummy_fptr)) {
            do {
                //with cmd_span^ do
                // Destroy all of cmd_span's contents.
                if (cmd_span->mark_one->line != nullptr) {
                    if (!lines_destroy(cmd_span->mark_one->line, cmd_span->mark_two->line))
                        return;
                    cmd_span->mark_one->line = nullptr;
                    cmd_span->mark_two->line = nullptr;
                }

                // If necessary, prompt.
                if (ludwig_mode == ludwig_mode_type::ludwig_hardcopy) {
                    //with current_frame->dot^ do
                    screen_load(current_frame->dot->line);
                    writeln("COMMAND: ");
                }

                // Read, compile, and execute the next lot of commands.
                int i;
                if (file_read(cmd_file, cmd_count, true, cmd_span->mark_one->line, cmd_span->mark_two->line, i)) {
                    if (cmd_span->mark_one->line != nullptr) {
                        cmd_span->mark_two->col = cmd_span->mark_two->line->used + 1;
                        if (code_compile(*cmd_span, true)) {
                            if (!code_interpret(leadparam::none, 1, cmd_span->code, true)) {
                                writeln("\aCOMMAND FAILED");
                            }
                        }
                        exit_abort = false;
                        tt_controlc = false;
                    }
                }
            } while (!cmd_file->eof);
            ludwig_aborted = false;
            quit_close_files();
        }
    }
        break;
    }
}
