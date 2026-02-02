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
! Name:         EXEC
!
! Description:  The primitive LUDWIG commands.
!**/

#include "exec.h"

#include "arrow.h"
#include "caseditto.h"
#include "charcmd.h"
#include "code.h"
#include "eqsgetrep.h"
#include "frame.h"
#include "fyle.h"
#include "help.h"
#include "line.h"
#include "mark.h"
#include "newword.h"
#include "nextbridge.h"
#include "opsys.h"
#include "quit.h"
#include "screen.h"
#include "span.h"
#include "swap.h"
#include "text.h"
#include "tpar.h"
#include "user.h"
#include "validate.h"
#include "var.h"
#include "vdu.h"
#include "window.h"
#include "word.h"

#ifdef DEBUG
#include <iomanip>
#include <iostream>
#endif

bool exec_compute_line_range(
    frame_ptr frame, leadparam rept, int count, line_ptr &first_line, line_ptr &last_line
) {
    // This routine returns the range of lines specified by the REPT/COUNT pair.
    // It returns FALSE if the range does not exist.
    // It returns First_Line as NIL if the range is empty.
    // The range returned WILL NOT include the null line.
    // It is assumed that the mark (if any) has been checked for validity.

    bool result = false;
    // with frame^,dot^ do
    first_line = frame->dot->line;
    last_line = frame->dot->line;
    switch (rept) {
    case leadparam::none:
    case leadparam::plus:
    case leadparam::pint:
        if (count == 0)
            first_line = nullptr;
        else if (count <= 20) { // TRY TO OPTIMIZE COMMON CASE
            for (line_range line_nr = 1; line_nr < count; ++line_nr) {
                last_line = last_line->flink;
                if (last_line == nullptr)
                    goto l99;
            }
            if (last_line->flink == nullptr)
                goto l99;
        } else {
            line_range line_nr;
            if (!line_to_number(first_line, line_nr))
                goto l99;
            // FIXME: Should this be "frame" rather than "current_frame"
            if (!line_from_number(current_frame, line_nr + count - 1, last_line))
                goto l99;
            if (last_line == nullptr)
                goto l99;
            if (last_line->flink == nullptr)
                goto l99;
        }
        break;
    case leadparam::minus:
    case leadparam::nint:
        count = -count;
        last_line = frame->dot->line->blink;
        if (last_line == nullptr)
            goto l99;
        if (count <= 20) {
            for (line_range line_nr = 1; line_nr <= count; ++line_nr) {
                first_line = first_line->blink;
                if (first_line == nullptr)
                    goto l99;
            }
        } else {
            line_range line_nr;
            if (!line_to_number(last_line, line_nr))
                goto l99;
            if (count > line_nr)
                goto l99;
            line_nr = line_nr - count + 1;
            // FIXME: Not sure if "current_frame" should be "frame"
            if (!line_from_number(current_frame, line_nr, first_line))
                goto l99;
        }
        break;
    case leadparam::pindef:
        if (frame->dot->line->flink == nullptr)
            first_line = nullptr;
        else
            last_line = frame->last_group->last_line->blink;
        break;
    case leadparam::nindef:
        last_line = frame->dot->line->blink;
        if (last_line == nullptr)
            first_line = nullptr;
        else
            first_line = frame->first_group->first_line;
        break;
    case leadparam::marker:
        {
            line_ptr mark_line = frame->marks[count]->line;
            if (mark_line == first_line) // TRY TO OPTIMIZE MOST COMMON
                first_line = nullptr;    // CASES.
            else if (mark_line->flink == first_line) {
                first_line = mark_line;
                last_line = mark_line;
            } else if (mark_line->blink == first_line) {
                last_line = first_line;
            } else { // DO IT THE HARD WAY!
                line_range mark_line_nr;
                if (!line_to_number(mark_line, mark_line_nr))
                    goto l99;
                line_range line_nr;
                if (!line_to_number(frame->dot->line, line_nr))
                    goto l99;
                if (mark_line_nr < line_nr) {
                    first_line = mark_line;
                    last_line = last_line->blink;
                } else {
                    last_line = mark_line->blink;
                }
            }
        }
        break;
    default:
        // FIXME: Error?
        break;
    }
    result = true;
l99:;
    return result;
}

bool execute(commands command, leadparam rept, int count, tpar_ptr tparam, bool from_span) {
    bool cmd_success;
    col_range new_col;
    col_range dot_col;
    line_ptr new_line;
    line_ptr first_line;
    line_ptr last_line;
    key_code_range key;
    int i;
    int j;
    line_range line_nr;
    line_range line2_nr;
    std::string new_name;
    span_ptr new_span;
    span_ptr old_span;
    tpar_object request;
    tpar_object request2;
    mark_ptr the_mark;
    mark_ptr the_other_mark;
    mark_ptr another_mark;
    bool eq_set;         // These 3 are used for
    frame_ptr old_frame; // the setting up of
    mark_object old_dot; // the commands = behaviour
    str_object new_str;

    cmd_success = false;
    request.nxt = nullptr;
    request.con = nullptr;
    request2.nxt = nullptr;
    request2.con = nullptr;
    exec_level += 1;
    if (tt_controlc)
        goto l99;
    if (exec_level == MAX_EXEC_RECURSION) {
        screen_message(MSG_COMMAND_RECURSION_LIMIT);
        goto l99;
    }
    // with current_frame^,dot^ do
    //  Fix commands which use marks without using @ in the syntax.
    if (command == commands::cmd_mark) {
        if ((count == 0) || (std::abs(count) > MAX_USER_MARK_NUMBER)) {
            screen_message(MSG_ILLEGAL_MARK_NUMBER);
            goto l99;
        }
    } else if (command == commands::cmd_span_define) {
        if (rept == leadparam::none || rept == leadparam::pint) {
            if ((count == 0) || (count > MAX_USER_MARK_NUMBER)) {
                screen_message(MSG_ILLEGAL_MARK_NUMBER);
                goto l99;
            }
            rept = leadparam::marker;
        }
    }

    // Check the mark, assign The_Mark to the mark.
    if (rept == leadparam::marker) {
        the_mark = current_frame->marks[count];
        if (the_mark == nullptr) {
            screen_message(MSG_MARK_NOT_DEFINED);
            goto l99;
        }
    }

    // Save the current value of DOT and CURRENT_FRAME for use by equals
    old_dot = *current_frame->dot;
    old_frame = current_frame;

    // Execute the command.
    switch (command) {
    case commands::cmd_advance:
        // Establish which line to advance to.
        cmd_success =
            (rept == leadparam::pindef || rept == leadparam::nindef || rept == leadparam::marker);
        new_line = current_frame->dot->line;
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
        case leadparam::pint:
            if (count < 20) {
                while (count > 0) {
                    count -= 1;
                    new_line = new_line->flink;
                    if (new_line == nullptr)
                        goto l99;
                }
            } else {
                if (!line_to_number(new_line, line_nr))
                    goto l99;
                if (!line_from_number(current_frame, line_nr + count, new_line))
                    goto l99;
                if (new_line == nullptr)
                    goto l99;
            }
            // if flink is nullptr we are on eop-line, so fail
            if (new_line->flink == nullptr)
                goto l99;
            cmd_success = true;
            break;
        case leadparam::minus:
        case leadparam::nint:
            count = -count;
            if (count < 20) {
                while (count > 0) {
                    count -= 1;
                    new_line = new_line->blink;
                    if (new_line == nullptr)
                        goto l99;
                }
            } else {
                if (!line_to_number(new_line, line_nr))
                    goto l99;
                if (count >= line_nr)
                    goto l99;
                if (!line_from_number(current_frame, line_nr - count, new_line))
                    goto l99;
                if (new_line == nullptr)
                    goto l99;
            }
            cmd_success = true;
            break;
        case leadparam::pindef:
            new_line = current_frame->last_group->last_line;
            break;
        case leadparam::nindef:
            new_line = current_frame->first_group->first_line;
            break;
        case leadparam::marker:
            new_line = the_mark->line;
            break;
        default:
            // FIXME: Error?
            break;
        }
        if (!mark_create(
                current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]
            ))
            goto l99;
        mark_create(new_line, 1, current_frame->dot);
        break;

    case commands::cmd_bridge:
    case commands::cmd_next:
        if (tpar_get_1(tparam, command, request))
            cmd_success = nextbridge_command(count, request, command == commands::cmd_bridge);
        break;

    case commands::cmd_case_edit:
    case commands::cmd_case_low:
    case commands::cmd_case_up:
    case commands::cmd_ditto_down:
    case commands::cmd_ditto_up:
        cmd_success = caseditto_command(command, rept, count, from_span);
        break;

    case commands::cmd_delete_char:
        if (rept != leadparam::marker) {
            cmd_success = charcmd_delete(command, rept, count, from_span);
        } else {
            the_other_mark = current_frame->dot;
            if (!line_to_number(current_frame->dot->line, line_nr))
                goto l99;
            if (!line_to_number(the_mark->line, line2_nr))
                goto l99;
            if ((line_nr > line2_nr) ||
                ((line_nr == line2_nr) && (current_frame->dot->col > the_mark->col))) {
                // Reverse mark pointers to get The_Other_Mark first.
                another_mark = the_mark;
                the_mark = the_other_mark;
                the_other_mark = another_mark;
            }
            if (current_frame != frame_oops) {
                // with frame_oops^ do
                //  Make sure oops_span is okay.
                if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->span->mark_two))
                    goto l99;
                cmd_success = text_move(
                    false,                          // Dont copy,transfer
                    1,                              // One instance of
                    the_other_mark,                 // starting pos.
                    the_mark,                       // ending pos.
                    frame_oops->span->mark_two,     // destination.
                    frame_oops->marks[MARK_EQUALS], // leave at start.
                    frame_oops->dot
                ); // leave at end.
                frame_oops->text_modified = true;
                mark_create(
                    frame_oops->dot->line, frame_oops->dot->col, frame_oops->marks[MARK_MODIFIED]
                );
            } else {
                cmd_success = text_remove(
                    the_other_mark, // starting pos.
                    the_mark
                ); // ending pos.
            }
            current_frame->text_modified = true;
            mark_create(
                current_frame->dot->line,
                current_frame->dot->col,
                current_frame->marks[MARK_MODIFIED]
            );
        }
        break;

    case commands::cmd_delete_line:
        // Establish which lines to kill, this is common to K and FW cmds.
        if (!exec_compute_line_range(current_frame, rept, count, first_line, last_line))
            goto l99;
        if (first_line != nullptr) {
            dot_col = current_frame->dot->col;
            if (last_line->flink == nullptr)
                goto l99;
            if (!marks_squeeze(first_line, 1, last_line->flink, 1))
                goto l99;
            if (!lines_extract(first_line, last_line))
                goto l99;
            if (current_frame != frame_oops) {
                if (!lines_inject(first_line, last_line, frame_oops->last_group->last_line))
                    goto l99;
                // with frame_oops^ do
                if (!mark_create(first_line, 1, frame_oops->marks[MARK_EQUALS]))
                    goto l99;
                if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->dot))
                    goto l99;
                frame_oops->text_modified = true;
                mark_create(
                    frame_oops->dot->line, frame_oops->dot->col, frame_oops->marks[MARK_MODIFIED]
                );
            } else if (!lines_destroy(first_line, last_line))
                goto l99;
            current_frame->dot->col = dot_col;
            current_frame->text_modified = true;
            mark_create(
                current_frame->dot->line,
                current_frame->dot->col,
                current_frame->marks[MARK_MODIFIED]
            );
        }
        cmd_success = true;
        break;

    case commands::cmd_backtab:
    case commands::cmd_down:
    case commands::cmd_home:
    case commands::cmd_left:
    case commands::cmd_return:
    case commands::cmd_right:
    case commands::cmd_tab:
    case commands::cmd_up:
        if ((command == commands::cmd_return) && (edit_mode == mode_type::mode_insert) &&
            current_frame->options.contains(frame_options_elts::opt_new_line)) {
            if (current_frame->dot->line->flink == nullptr) {
                cmd_success = text_realize_null(current_frame->dot->line);
                cmd_success = arrow_command(command, rept, count, from_span);
            } else {
                cmd_success = execute(commands::cmd_split_line, rept, count, tparam, from_span);
            }
        } else {
            cmd_success = arrow_command(command, rept, count, from_span);
        }
        break;

#ifdef DEBUG
    case commands::cmd_dump:
        first_line = scr_frame->first_group->first_line;
        vdu_movecurs(1, terminal_info.height);
        vdu_flush();
        std::cout << "DUMP sr ln" << std::endl;
        while (first_line != nullptr) {
            if (first_line == scr_top_line)
                std::cout << "SCR_TOP_LINE:" << std::endl;
            if (first_line == scr_bot_line)
                std::cout << "SCR_BOT_LINE:" << std::endl;
            if (first_line == scr_frame->dot->line)
                std::cout << "DOT         :" << std::endl;
            // with first_line^ do
            if (line_to_number(first_line, line_nr))
                std::cout << "     " << std::setw(2) << first_line->scr_row_nr << " "
                          << std::setw(2) << line_nr << std::endl;
            else
                std::cout << "Line to number failed" << std::endl;
            first_line = first_line->flink;
        }
        cmd_success = true;
        break;
#endif

    case commands::cmd_equal_column:
        i = 1; // Start of column number, j receives column number.
        if (tpar_get_1(tparam, command, request)) {
            if (tpar_to_int(request, i, j)) {
                switch (rept) {
                case leadparam::none:
                case leadparam::plus:
                    cmd_success = (current_frame->dot->col == j);
                    break;
                case leadparam::minus:
                    cmd_success = (current_frame->dot->col != j);
                    break;
                case leadparam::pindef:
                    cmd_success = (current_frame->dot->col >= j);
                    break;
                case leadparam::nindef:
                    cmd_success = (current_frame->dot->col <= j);
                    break;
                default:
                    // FIXME: Error?
                    break;
                }
            }
        }
        break;

    case commands::cmd_equal_eol:
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
            cmd_success = (current_frame->dot->col == current_frame->dot->line->used + 1);
            break;
        case leadparam::minus:
            cmd_success = (current_frame->dot->col != current_frame->dot->line->used + 1);
            break;
        case leadparam::pindef:
            cmd_success = (current_frame->dot->col >= current_frame->dot->line->used + 1);
            break;
        case leadparam::nindef:
            cmd_success = (current_frame->dot->col <= current_frame->dot->line->used + 1);
            break;
        default:
            // FIXME: Error?
            break;
        }
        break;

    case commands::cmd_equal_eop:
    case commands::cmd_equal_eof:
        cmd_success = (current_frame->dot->line->flink == nullptr);
        if (command == commands::cmd_equal_eof) {
            if (current_frame->input_file >= 0) {
                if (!files[current_frame->input_file]->eof)
                    cmd_success = false;
            }
        }
        if (rept == leadparam::minus)
            cmd_success = !cmd_success;
        break;

    case commands::cmd_equal_mark:
        if (!tpar_get_1(tparam, command, request))
            goto l99;
        if (!tpar_to_mark(request, j))
            goto l99;
        if (current_frame->marks[j] != nullptr) {
            switch (rept) {
            case leadparam::none:
            case leadparam::plus:
            case leadparam::minus:
                if ((current_frame->marks[j]->line == current_frame->dot->line) &&
                    (current_frame->marks[j]->col == current_frame->dot->col))
                    cmd_success = true;
                if (rept == leadparam::minus)
                    cmd_success = !cmd_success;
                break;
            case leadparam::pindef:
            case leadparam::nindef:
                if (current_frame->marks[j]->line == current_frame->dot->line) {
                    if (rept == leadparam::pindef)
                        cmd_success = (current_frame->dot->col >= current_frame->marks[j]->col);
                    else
                        cmd_success = (current_frame->dot->col <= current_frame->marks[j]->col);
                } else if (line_to_number(current_frame->dot->line, line_nr) &&
                           line_to_number(current_frame->marks[j]->line, line2_nr)) {
                    if (rept == leadparam::pindef)
                        cmd_success = (line_nr >= line2_nr);
                    else
                        cmd_success = (line_nr <= line2_nr);
                }
                break;
            default:
                // FIXME: Error?
                break;
            }
        }
        break;

    case commands::cmd_equal_string:
        if (tpar_get_1(tparam, command, request)) {
            if (request.len == 0) {
                // If didnt specify, use default.
                request = current_frame->eqs_tpar;
                if (request.len == 0) {
                    screen_message(MSG_NO_DEFAULT_STR);
                    goto l99;
                }
            } else {
                current_frame->eqs_tpar = request; // If did specify, save for next time.
            }
        }
        cmd_success = eqsgetrep_eqs(rept, request);
        break;

    case commands::cmd_do_last_command:
    case commands::cmd_execute_string:
        if (current_frame == frame_cmd) {
            screen_message(MSG_NOT_WHILE_EDITING_CMD);
            goto l99;
        }
        // with frame_cmd^ do
        if (command == commands::cmd_execute_string) {
            if (!tpar_get_1(tparam, command, request))
                goto l99;

            frame_cmd->return_frame = current_frame;
            current_frame = frame_cmd;

            // Zap frame COMMANDs current contents.
            first_line = frame_cmd->first_group->first_line;
            last_line = frame_cmd->last_group->last_line->blink;
            if (last_line != nullptr) {
                if (!marks_squeeze(first_line, 1, last_line->flink, 1))
                    goto l99;
                if (!lines_extract(first_line, last_line))
                    goto l99;
                if (!lines_destroy(first_line, last_line))
                    goto l99;
            }

            // Insert the new tpar into frame COMMAND.
            if (!text_insert_tpar(request, frame_cmd->dot, frame_cmd->marks[MARK_EQUALS]))
                goto l99;

            current_frame = current_frame->return_frame;
        }

        // Recompile and execute frame COMMAND.
        // First we look it up, mainly to reset the end-of-span marks.
        // This is an expensive way of doing it, but this is not too freq.
        // an operation.
        if (span_find(frame_cmd->span->name, new_span, old_span)) {
            if (!code_compile(*frame_cmd->span, true))
                goto l99;
            cmd_success = code_interpret(rept, count, frame_cmd->span->code, true);
        }
        break;

    case commands::cmd_file_input:
    case commands::cmd_file_output:
    case commands::cmd_file_edit:
    case commands::cmd_file_read:
    case commands::cmd_file_write:
    case commands::cmd_file_rewind:
    case commands::cmd_file_kill:
    case commands::cmd_file_save:
    case commands::cmd_file_global_input:
    case commands::cmd_file_global_output:
    case commands::cmd_file_global_rewind:
    case commands::cmd_file_global_kill:
        cmd_success = file_command(command, rept, count, tparam, from_span);
        break;

    case commands::cmd_file_execute:
        if (current_frame == frame_cmd) {
            screen_message(MSG_NOT_WHILE_EDITING_CMD);
            goto l99;
        }
        if (tpar_get_1(tparam, command, request)) {
            tpar_object new_tparam{request};
            // with frame_cmd^ do
            frame_cmd->return_frame = current_frame;
            current_frame = frame_cmd;
            // Zap frame COMMANDs current contents.
            first_line = frame_cmd->first_group->first_line;
            last_line = frame_cmd->last_group->last_line->blink;
            if (last_line != nullptr) {
                if (!marks_squeeze(first_line, 1, last_line->flink, 1))
                    goto l99;
                if (!lines_extract(first_line, last_line))
                    goto l99;
                if (!lines_destroy(first_line, last_line))
                    goto l99;
            }
            if (file_command(commands::cmd_file_execute, leadparam::none, 0, &new_tparam, false)) {
                current_frame = current_frame->return_frame;
                //
                //! Recompile and execute frame COMMAND
                //! First we look it up, mainly to reset the end-of-span marks.
                //! This is an expensive way of doing it, but this is not too
                //! frequent an operation.
                //
                if (span_find(frame_cmd->span->name, new_span, old_span)) {
                    if (code_compile(*frame_cmd->span, true))
                        cmd_success = code_interpret(rept, count, frame_cmd->span->code, true);
                }
            } else {
                current_frame = current_frame->return_frame;
            }
        }
        break;

    case commands::cmd_file_table:
        file_table();
        cmd_success = true;
        break;

    case commands::cmd_frame_edit:
        if (tpar_get_1(tparam, command, request)) {
            // with request do
            new_name = request.str.slice(1, request.len);
            cmd_success = frame_edit(new_name);
        }
        break;

    case commands::cmd_frame_kill:
        if (tpar_get_1(tparam, command, request)) {
            // with request do
            new_name = request.str.slice(1, request.len);
            cmd_success = frame_kill(new_name);
        }
        break;

    case commands::cmd_frame_parameters:
        cmd_success = frame_parameter(tparam);
        break;

    case commands::cmd_frame_return:
        for (i = 1; i <= count; ++i) {
            // with current_frame^ do
            if (current_frame->return_frame == nullptr) {
                current_frame = old_frame;
                goto l99;
            }
            current_frame = current_frame->return_frame;
        }
        cmd_success = true;
        break;

    case commands::cmd_get:
        if (tpar_get_1(tparam, command, request)) {
            if (request.len == 0) {
                // If didnt specify, use default.
                request = current_frame->get_tpar;
                if (request.len == 0) {
                    screen_message(MSG_NO_DEFAULT_STR);
                    goto l99;
                }
            } else {
                current_frame->get_tpar = request; // If did specify, save for next time.
            }
            cmd_success = eqsgetrep_get(count, request, from_span);
        }
        break;

    case commands::cmd_help:
        if (ludwig_mode == ludwig_mode_type::ludwig_batch) {
            screen_message(MSG_INTERACTIVE_MODE_ONLY);
            goto l99;
        }
        if (tpar_get_1(tparam, command, request)) {
            // with request do
            help_help(std::string(request.str.slice(1, request.len)));
            cmd_success = true; // Never Fails.
        }
        break;

    case commands::cmd_insert_char:
        cmd_success = charcmd_insert(command, rept, count, from_span);
        break;

    case commands::cmd_insert_line:
        if (count != 0) {
            cmd_success = lines_create(std::abs(count), first_line, last_line);
            if (cmd_success)
                cmd_success = lines_inject(first_line, last_line, current_frame->dot->line);
            if (cmd_success) {
                if (count > 0) {
                    cmd_success = mark_create(
                        current_frame->dot->line,
                        current_frame->dot->col,
                        current_frame->marks[MARK_EQUALS]
                    );
                    cmd_success =
                        mark_create(first_line, current_frame->dot->col, current_frame->dot);
                } else {
                    cmd_success = mark_create(
                        first_line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]
                    );
                }
                current_frame->text_modified = true;
                mark_create(
                    current_frame->dot->line,
                    current_frame->dot->col,
                    current_frame->marks[MARK_MODIFIED]
                );
            }
        } else {
            cmd_success = mark_create(
                current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]
            );
        }
        break;

    case commands::cmd_insert_mode:
        edit_mode = mode_type::mode_insert;
        cmd_success = true;
        break;

    case commands::cmd_insert_text:
        if (file_data.old_cmds && !from_span) {
            if (rept == leadparam::none) {
                edit_mode = mode_type::mode_insert;
                cmd_success = true;
            } else {
                screen_message(MSG_SYNTAX_ERROR);
            }
        } else if (tpar_get_1(tparam, command, request)) {
            // with request do
            if (request.con == nullptr) {
                cmd_success =
                    text_insert(true, count, request.str, request.len, current_frame->dot);
                if (cmd_success && (count * request.len != 0)) {
                    current_frame->text_modified = true;
                    cmd_success = mark_create(
                        current_frame->dot->line,
                        current_frame->dot->col,
                        current_frame->marks[MARK_MODIFIED]
                    );
                }
            } else {
                for (i = 1; i <= count; ++i) {
                    if (text_insert_tpar(
                            request, current_frame->dot, current_frame->marks[MARK_EQUALS]
                        ))
                        goto l99;
                }
                current_frame->text_modified = true;
                cmd_success = mark_create(
                    current_frame->dot->line,
                    current_frame->dot->col,
                    current_frame->marks[MARK_MODIFIED]
                );
            }
        }
        break;

    case commands::cmd_insert_invisible:
        if (ludwig_mode != ludwig_mode_type::ludwig_screen)
            goto l99;
        // with line^ do
        if (current_frame->dot->col > current_frame->dot->line->used)
            i = MAX_STRLENP - current_frame->dot->col;
        else
            i = MAX_STRLEN - current_frame->dot->line->used;
        if (rept == leadparam::pindef)
            count = i;
        if (count > i)
            goto l99;
        new_str = BLANK_STRING;
        i = 0;
        while (i < count) {
            key = vdu_get_key();
            if (tt_controlc)
                goto l99;
            if (PRINTABLE_SET.test(key)) {
                i += 1;
                new_str[i] = key;
            } else if (key == 13) {
                if (rept == leadparam::pindef)
                    count = i;
                else
                    i = count;
            } else {
                vdu_beep();
            }
        }
        cmd_success = text_insert(true, 1, new_str, count, current_frame->dot);
        if (cmd_success && (count != 0)) {
            current_frame->text_modified = true;
            if (!mark_create(
                    current_frame->dot->line,
                    current_frame->dot->col,
                    current_frame->marks[MARK_MODIFIED]
                ))
                cmd_success = false;
        }
        break;

    case commands::cmd_jump:
        switch (rept) {
        case leadparam::none:
        case leadparam::plus:
        case leadparam::pint:
            if (current_frame->dot->col + count > MAX_STRLENP)
                goto l99;
            break;
        case leadparam::minus:
        case leadparam::nint:
            if (current_frame->dot->col <= -count)
                goto l99;
            break;
        case leadparam::pindef:
            if (current_frame->dot->col > current_frame->dot->line->used + 1)
                goto l99;
            count = 1 + current_frame->dot->line->used - current_frame->dot->col;
            break;
        case leadparam::nindef:
            count = 1 - current_frame->dot->col;
            break;
        case leadparam::marker:
            if (!mark_create(the_mark->line, the_mark->col, current_frame->dot))
                goto l99;
            count = 0;
            break;
        default:
            // FIXME: Error?
            break;
        }
        current_frame->dot->col += count;
        cmd_success = true;
        break;

    case commands::cmd_line_centre:
        cmd_success = word_centre(rept, count);
        break;

    case commands::cmd_line_fill:
        cmd_success = word_fill(rept, count);
        break;

    case commands::cmd_line_justify:
        cmd_success = word_justify(rept, count);
        break;

    case commands::cmd_line_squash:
        cmd_success = word_squeeze(rept, count);
        break;

    case commands::cmd_line_left:
        cmd_success = word_left(rept, count);
        break;

    case commands::cmd_line_right:
        cmd_success = word_right(rept, count);
        break;

    case commands::cmd_word_advance:
        if (file_data.old_cmds)
            cmd_success = word_advance_word(rept, count);
        else
            cmd_success = newword_advance_word(rept, count);
        break;

    case commands::cmd_word_delete:
        if (file_data.old_cmds)
            cmd_success = word_delete_word(rept, count);
        else
            cmd_success = newword_delete_word(rept, count);
        break;

    case commands::cmd_advance_paragraph:
        cmd_success = newword_advance_paragraph(rept, count);
        break;

    case commands::cmd_delete_paragraph:
        cmd_success = newword_delete_paragraph(rept, count);
        break;

    case commands::cmd_mark:
        if (count < 0) {
            if (current_frame->marks[-count] != nullptr)
                cmd_success = mark_destroy(current_frame->marks[-count]);
            else
                cmd_success = true;
        } else {
            cmd_success = mark_create(
                current_frame->dot->line, current_frame->dot->col, current_frame->marks[count]
            );
        }
        break;

    case commands::cmd_noop:
        // Nothing to do, as one might expect.
        break;

    case commands::cmd_command:
        if (rept == leadparam::minus) {
            if (edit_mode != mode_type::mode_command) {
                previous_mode = edit_mode;
                edit_mode = mode_type::mode_command;
            } else {
                goto l99;
            }
        } else {
            if (edit_mode == mode_type::mode_command)
                edit_mode = previous_mode;
            else {
                if (ludwig_mode != ludwig_mode_type::ludwig_screen) {
                    screen_message(MSG_SCREEN_MODE_ONLY);
                    goto l99;
                }
                cmd_success = user_command_introducer();
            }
        }
        cmd_success = true;
        break;

    case commands::cmd_overtype_mode:
        edit_mode = mode_type::mode_overtype;
        cmd_success = true;
        break;

    case commands::cmd_overtype_text:
        if (file_data.old_cmds && !from_span) {
            if (rept == leadparam::none) {
                edit_mode = mode_type::mode_overtype;
                cmd_success = true;
            } else {
                screen_message(MSG_SYNTAX_ERROR);
            }
        } else if (tpar_get_1(tparam, command, request)) {
            // with request do
            cmd_success = text_overtype(true, count, request.str, request.len, current_frame->dot);
            if (cmd_success && (count * request.len != 0)) {
                current_frame->text_modified = true;
                if (!mark_create(
                        current_frame->dot->line,
                        current_frame->dot->col,
                        current_frame->marks[MARK_MODIFIED]
                    ))
                    cmd_success = false;
            }
        }
        break;

    case commands::cmd_page:
        if (!from_span) {
            screen_message(MSG_PAGING);
            if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                vdu_flush();
        }
        cmd_success = file_page(current_frame, exit_abort);
        // Clean up the PAGING message.
        if (!from_span)
            screen_clear_msgs(false);
        break;

    case commands::cmd_op_sys_command:
        if (tpar_get_1(tparam, command, request)) {
            if (!opsys_command(request, first_line, last_line, i))
                goto l99;
            if (first_line != nullptr) {
                if (!lines_inject(first_line, last_line, current_frame->dot->line))
                    goto l99;
                if (!mark_create(first_line, 1, current_frame->marks[MARK_EQUALS]))
                    goto l99;
                current_frame->text_modified = true;
                if (!mark_create(last_line->flink, 1, current_frame->marks[MARK_MODIFIED]))
                    goto l99;
                if (!mark_create(last_line->flink, 1, current_frame->dot))
                    goto l99;
                cmd_success = true;
            }
        }
        break;

    case commands::cmd_position_column:
        if (count > MAX_STRLEN)
            goto l99;
        current_frame->dot->col = count;
        cmd_success = true;
        break;

    case commands::cmd_position_line:
        if (!line_from_number(current_frame, count, new_line))
            goto l99;
        if (new_line == nullptr)
            goto l99;
        cmd_success = true;
        if (!mark_create(
                current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]
            ))
            goto l99;
        mark_create(new_line, 1, current_frame->dot);
        break;

    case commands::cmd_quit:
        cmd_success = quit_command();
        break;

    case commands::cmd_replace:
        if (tpar_get_2(tparam, command, request, request2)) {
            if (request.len == 0) { // If didnt specify, use default.
                if (current_frame->rep1_tpar.len == 0) {
                    screen_message(MSG_NO_DEFAULT_STR);
                    goto l99;
                }
            } else {
                current_frame->rep1_tpar = request; // If did specify, save for next time.
                tpar_clean_object(current_frame->rep2_tpar);
                current_frame->rep2_tpar = request2;
                request2.con = nullptr;
            }
            cmd_success = eqsgetrep_rep(
                rept, count, current_frame->rep1_tpar, current_frame->rep2_tpar, from_span
            );
        }
        break;

    case commands::cmd_rubout:
        cmd_success = charcmd_rubout(command, rept, count, from_span);
        break;

    case commands::cmd_set_margin_left:
        if (rept == leadparam::minus) {
            current_frame->margin_left = initial_margin_left;
        } else {
            if (current_frame->dot->col >= current_frame->margin_right) {
                screen_message(MSG_LEFT_MARGIN_GE_RIGHT);
                goto l99;
            }
            current_frame->margin_left = current_frame->dot->col;
        }
        cmd_success = true;
        break;

    case commands::cmd_set_margin_right:
        if (rept == leadparam::minus) {
            current_frame->margin_right = initial_margin_right;
        } else {
            if (current_frame->dot->col <= current_frame->margin_left) {
                screen_message(MSG_LEFT_MARGIN_GE_RIGHT);
                goto l99;
            }
            current_frame->margin_right = current_frame->dot->col;
        }
        cmd_success = true;
        break;

    case commands::cmd_span_jump:
    case commands::cmd_span_compile:
    case commands::cmd_span_copy:
    case commands::cmd_span_define:
    case commands::cmd_span_execute:
    case commands::cmd_span_execute_no_recompile:
    case commands::cmd_span_transfer:
        if (tpar_get_1(tparam, command, request)) {
            // with request do
            new_name = request.str.slice(1, request.len);
            switch (command) {
            case commands::cmd_span_define:
                if (rept == leadparam::minus) {
                    if (span_find(new_name, new_span, old_span))
                        cmd_success = span_destroy(new_span);
                    else
                        screen_message(MSG_NO_SUCH_SPAN);
                } else {
                    cmd_success = span_create(new_name, the_mark, current_frame->dot);
                }
                break;

            case commands::cmd_span_jump:
                if (span_find(new_name, new_span, old_span)) {
                    // with new_span^ do
                    if (rept == leadparam::minus) {
                        // with mark_one^ do
                        new_col = new_span->mark_one->col;
                        new_line = new_span->mark_one->line;
                    } else {
                        // with mark_two^ do
                        new_col = new_span->mark_two->col;
                        new_line = new_span->mark_two->line;
                    }
                    if (new_line->group->frame == current_frame) {
                        cmd_success = mark_create(
                            current_frame->dot->line,
                            current_frame->dot->col,
                            current_frame->marks[MARK_EQUALS]
                        );
                        if (cmd_success)
                            cmd_success = mark_create(new_line, new_col, current_frame->dot);
                    } else {
                        // with new_line->group->frame^ do
                        frame_ptr fr = new_line->group->frame;
                        if (frame_edit(fr->span->name)) {
                            if (fr->marks[MARK_EQUALS] != nullptr)
                                mark_destroy(fr->marks[MARK_EQUALS]);
                            cmd_success = mark_create(new_line, new_col, fr->dot);
                        }
                    }
                } else {
                    screen_message(MSG_NO_SUCH_SPAN);
                }
                break;

            case commands::cmd_span_copy:
            case commands::cmd_span_transfer:
                if (span_find(new_name, new_span, old_span)) {
                    // with new_span^ do
                    cmd_success = text_move(
                        command == commands::cmd_span_copy,
                        count,
                        new_span->mark_one,
                        new_span->mark_two,
                        current_frame->dot,                // Dest.
                        current_frame->marks[MARK_EQUALS], // New_Start.
                        current_frame->dot
                    ); // New_End.
                    if ((command == commands::cmd_span_transfer) && (new_span->frame == nullptr) &&
                        cmd_success) {
                        mark_create(
                            current_frame->marks[MARK_EQUALS]->line,
                            current_frame->marks[MARK_EQUALS]->col,
                            new_span->mark_one
                        );
                        mark_create(
                            current_frame->dot->line, current_frame->dot->col, new_span->mark_two
                        );
                    }
                } else {
                    screen_message(MSG_NO_SUCH_SPAN);
                }
                break;

            case commands::cmd_span_compile:
            case commands::cmd_span_execute:
            case commands::cmd_span_execute_no_recompile:
                if (span_find(new_name, new_span, old_span)) {
                    if ((new_span->code == nullptr) ||
                        (command != commands::cmd_span_execute_no_recompile)) {
                        if (!code_compile(*new_span, true))
                            goto l99;
                    }
                    if (command == commands::cmd_span_compile)
                        cmd_success = true;
                    else
                        cmd_success = code_interpret(rept, count, new_span->code, true);
                } else {
                    screen_message(MSG_NO_SUCH_SPAN);
                }
                break;

            default:
                // FIXME: Error?
                break;
            }
        }
        break;

    case commands::cmd_span_index:
        cmd_success = span_index();
        break;

    case commands::cmd_span_assign:
        if (!tpar_get_2(tparam, command, request, request2))
            goto l99;
        if (request.len == 0)
            goto l99;
        // with request do
        new_name = request.str.slice(1, request.len);
        if (span_find(new_name, new_span, old_span)) {
            // Grunge the old one
            if (new_span == frame_oops->span) {
                // with frame_oops->span^ do
                if (!text_remove(frame_oops->span->mark_one, frame_oops->span->mark_two))
                    goto l99;
            } else {
                // with frame_oops^ do
                //  Make sure oops_span is okay.
                if (!mark_create(frame_oops->last_group->last_line, 1, frame_oops->span->mark_two))
                    goto l99;
                if (!text_move(
                        false, // Dont copy,transfer
                        1,     // One instance of
                        new_span->mark_one,
                        new_span->mark_two,
                        frame_oops->span->mark_two,     // destination.
                        frame_oops->marks[MARK_EQUALS], // leave at start.
                        frame_oops->dot
                    )) // leave at end.
                    goto l99;
            }
        } else {
            // Create a span in frame "HEAP"
            // with frame_heap^ do
            if (!mark_create(frame_heap->last_group->last_line, 1, frame_heap->span->mark_two))
                goto l99;
            if (!span_create(new_name, frame_heap->span->mark_two, frame_heap->span->mark_two))
                goto l99;
            if (!span_find(new_name, new_span, old_span))
                goto l99;
        }
        // Phew. Thats done. Now NEW_SPAN is an empty span
        // with request2 do
        //  begin
        /* We have not decided what 3sa should do yet.
        !           if rept in [none, pindef] then
        !             count = len
        !           else
        !             if rept in [plus, pint] then
        !               begin
        !               if count > max_strlen then
        !                 goto l99;
        !               if count > len then
        !                 begin <Fiddle>
        !                 ch_fill(str,len+1,count-len,' ');
        !                 len = count;
        !                 end
        !               else
        !                 len = count;
        !               end
        !             else
        !               begin <Take the last "Count" Chars.>
        !               if -count > max_strlen then
        !                 goto l99;
        !               if -count > len then
        !                 begin <Must put some spaces in>
        !                 ch_copy(str,1,len,str,1-count-len,len,' ');
        !                 ch_fill(str,1,-count-len,' ');
        !                 len = -count;
        !                 end
        !               else
        !                 begin <Just move the string>
        !                 ch_copy(str,len+count+1,-count,str,1,-count,' ');
        !                 len = -count;
        !                 end;
        !               end;
        */
        // Now copy the tpar into the span.
        if (!text_insert_tpar(request2, new_span->mark_two, new_span->mark_one))
            goto l99;
        // with new_span->mark_two^, line->group->frame^ do
        {
            frame_ptr fr = new_span->mark_two->line->group->frame;
            fr->text_modified = true;
            cmd_success = mark_create(
                new_span->mark_two->line, new_span->mark_two->col, fr->marks[MARK_MODIFIED]
            );
        }
        break;

    case commands::cmd_split_line:
        if (current_frame->dot->line->flink == nullptr) {
            if (!text_realize_null(current_frame->dot->line))
                goto l99;
        }
        cmd_success = text_split_line(current_frame->dot, 0, current_frame->marks[MARK_EQUALS]);
        break;

    case commands::cmd_swap_line:
        cmd_success = swap_line(rept, count);
        break;

    case commands::cmd_user_command_introducer:
        if (ludwig_mode != ludwig_mode_type::ludwig_screen) {
            screen_message(MSG_SCREEN_MODE_ONLY);
            goto l99;
        }
        cmd_success = user_command_introducer();
        break;

    case commands::cmd_user_key:
        if (ludwig_mode != ludwig_mode_type::ludwig_screen) {
            screen_message(MSG_SCREEN_MODE_ONLY);
            goto l99;
        }
        if (tpar_get_2(tparam, command, request, request2)) {
            if (request.len == 0)
                cmd_success = false;
            else
                cmd_success = user_key(request, request2);
        }
        break;

    case commands::cmd_user_parent:
        if (ludwig_mode == ludwig_mode_type::ludwig_batch) {
            screen_message(MSG_INTERACTIVE_MODE_ONLY);
            goto l99;
        }
        cmd_success = user_parent();
        break;

    case commands::cmd_user_subprocess:
        if (ludwig_mode == ludwig_mode_type::ludwig_batch) {
            screen_message(MSG_INTERACTIVE_MODE_ONLY);
            goto l99;
        }
        cmd_success = user_subprocess();
        break;

    case commands::cmd_user_undo:
        cmd_success = user_undo();
        break;

    case commands::cmd_window_backward:
    case commands::cmd_window_end:
    case commands::cmd_window_forward:
    case commands::cmd_window_left:
    case commands::cmd_window_middle:
    case commands::cmd_window_new:
    case commands::cmd_window_right:
    case commands::cmd_window_scroll:
    case commands::cmd_window_setheight:
    case commands::cmd_window_top:
    case commands::cmd_window_update:
        cmd_success = window_command(command, rept, count, from_span);
        break;

    case commands::cmd_resize_window:
        screen_resize();
        cmd_success = true;
        break;

#ifdef DEBUG
    case commands::cmd_validate:
        cmd_success = validate_command();
        break;
#endif

    case commands::cmd_block_define:
    case commands::cmd_block_transfer:
    case commands::cmd_block_copy:
        screen_message(MSG_NOT_IMPLEMENTED);
        break;

    default:
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        break;
    }

    if (cmd_success) {
        // with old_frame^,old_dot do
        switch (cmd_attrib.at(command).eq_action) {
        case equalaction::eqold:
            eq_set = mark_create(old_dot.line, old_dot.col, old_frame->marks[MARK_EQUALS]);
            break;
        case equalaction::eqdel:
            eq_set = (old_frame->marks[MARK_EQUALS] == nullptr) ||
                     mark_destroy(old_frame->marks[MARK_EQUALS]);
            break;
        case equalaction::eqnil:
            eq_set = true;
            break;
        }
        if (!eq_set)
            screen_message(MSG_EQUALS_NOT_SET);
    }
l99:;
    tpar_clean_object(request);
    tpar_clean_object(request2);
    exec_level -= 1;
    return cmd_success;
}
