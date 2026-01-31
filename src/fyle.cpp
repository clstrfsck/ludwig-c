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
! Name:         FYLE
!
! Description:  Open/Create, Read/Write, Close/Delete Input/Output files.
!**/

#include "fyle.h"

#include "ch.h"
#include "exec.h"
#include "filesys.h"
#include "line.h"
#include "mark.h"
#include "screen.h"
#include "tpar.h"
#include "var.h"
#include "vdu.h"

#include <algorithm>

// implementation
//   uses ch, exec, filesys, line, mark, screen, tpar, vdu;

namespace {
    const std::string BLANK_NAME("                               ");
}

void file_name(file_ptr fp, size_t max_len, file_name_str &act_fnm) {
    // Return a file's name, in the specified width.
    max_len = std::max(size_t{5}, max_len); // Minimum width
    size_t head_len;
    size_t tail_len;
    if (fp->filename.size() <= max_len) {
        head_len = fp->filename.size();
        tail_len = 0;
    } else {
        // Cut chars out the middle of the file name, insert '---'
        tail_len = (max_len - 3) / 2;
        head_len = max_len - 3 - tail_len;
    }
    if (tail_len == 0) {
        act_fnm = fp->filename.substr(0, head_len);
    } else {
        act_fnm = fp->filename.substr(0, head_len) + "---" +
                  fp->filename.substr(fp->filename.size() - tail_len, tail_len);
    }
}

void file_table() {
    // List the current files.

    screen_unload();
    screen_home(false);
    screen_write_str(0, "Usage   Mod Frame  Filename", 27);
    screen_writeln();
    screen_write_str(0, "------- --- ------ --------", 27);
    screen_writeln();
    screen_writeln();
    for (file_range file_slot : file_range::iota()) {
        if (files[file_slot] != nullptr) {
            std::string frame_name;
            if (files_frames[file_slot] != nullptr) {
                frame_name = files_frames[file_slot]->span->name;
            } else {
                frame_name.assign(BLANK_NAME, 0, 6);
            }

            if (files_frames[file_slot] != nullptr) {
                if (files[file_slot]->output_flag)
                    screen_write_str(0, "FO ", 3);
                else
                    screen_write_str(0, "FI ", 3);
            } else if (file_slot == fgi_file)
                screen_write_str(0, "FGI", 3);
            else if (file_slot == fgo_file)
                screen_write_str(0, "FGO", 3);
            else if (files[file_slot]->output_flag)
                screen_write_str(0, "FFO", 3);
            else
                screen_write_str(0, "FFI", 3);

            if (files[file_slot]->eof)
                screen_write_str(1, "EOF", 3);
            else
                screen_write_str(1, "   ", 3);

            if (files_frames[file_slot] != nullptr) {
                if (files_frames[file_slot]->text_modified)
                    screen_write_str(1, " * ", 3);
                else
                    screen_write_str(1, "   ", 3);
            } else {
                screen_write_str(1, "   ", 3);
            }

            screen_write_name_str(1, frame_name, frame_name.size());
            if (frame_name.size() > 6) {
                screen_writeln();
                screen_write_str(0, "                  ", 18);
            }

            int room;
            if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                room = terminal_info.width - 18 - 1;
            else
                room = FILE_NAME_LEN;
            file_name_str compressed_fnm;
            file_name(files[file_slot], room, compressed_fnm);
            screen_write_file_name_str(1, compressed_fnm, compressed_fnm.size());
            screen_writeln();
        }
    }
    screen_pause();
}

void file_fix_eop(bool eof, line_ptr eop_line) {
    // Here we are just going to assume that eop_line->str != nullptr
    // with eop_line^ do
    if (eof) {
        eop_line->str->copy_n("<End of File>  ", 15);
    } else {
        eop_line->str->copy_n("<Page Boundary>", 15);
    }
    if (eop_line->scr_row_nr != 0)
        screen_draw_line(eop_line);
}

bool file_create_open(file_name_str &fn, parse_type parse, file_ptr &inputfp, file_ptr &outputfp) {
    // Parse fn and create I/O streams to files.

    switch (parse) {
    case parse_type::parse_command:
    case parse_type::parse_input:
    case parse_type::parse_edit:
    case parse_type::parse_stdin:
    case parse_type::parse_execute:
        {
            if (inputfp != nullptr) {
                screen_message(MSG_FILE_ALREADY_IN_USE);
                return false;
            }
            inputfp = new file_object;
            // with inputfp^ do
            inputfp->valid = false;
            inputfp->first_line = nullptr;
            inputfp->last_line = nullptr;
            inputfp->line_count = 0;
            inputfp->output_flag = false;
            inputfp->eof = false;
            inputfp->idx = 0;
            inputfp->len = 0;
            inputfp->zed = 'Z';
        }
        break;
    default:
        // Ignore here
        break;
    }
    switch (parse) {
    case parse_type::parse_command:
    case parse_type::parse_output:
    case parse_type::parse_edit:
        {
            if (outputfp != nullptr) {
                screen_message(MSG_FILE_ALREADY_IN_USE);
                return false;
            }
            outputfp = new file_object;
            // with outputfp^ do
            outputfp->valid = false;
            outputfp->first_line = nullptr;
            outputfp->last_line = nullptr;
            outputfp->line_count = 0;
            outputfp->output_flag = true;
            outputfp->eof = false;
            outputfp->idx = 0;
            outputfp->len = 0;
            outputfp->zed = 'Z';
        }
        break;
    default:
        // Ignore here
        break;
    }

    bool result = filesys_parse(fn, parse, file_data, inputfp, outputfp);
    if (inputfp != nullptr && !inputfp->valid) {
        delete inputfp;
        inputfp = nullptr;
    }
    if (outputfp != nullptr && !outputfp->valid) {
        delete (outputfp);
        outputfp = nullptr;
    }
    return result;
}

bool file_close_delete(file_ptr &fp, bool delet, bool msgs) {
    // Close a file, if it is an output file it can optionally be deleted.
    if (fp != nullptr) {
        if (filesys_close(fp, delet ? 1 : 0, msgs)) {
            // with fp^ do
            if (fp->first_line != nullptr) {                  // Dispose of any unused input lines.
                lines_destroy(fp->first_line, fp->last_line); // Ignore errors.
            }
        }
        delete fp;
        fp = nullptr;
        return true;
    }
    return false;
}

bool file_read(
    file_ptr fp, line_range count, bool best_try, line_ptr &first, line_ptr &last, int &actual_cnt
) {
    // Read a series of lines from input file.

    // with fp^ do
    //  Try to read the lines.
    if (fp->output_flag) {
        screen_message(MSG_NOT_INPUT_FILE);
        return false;
    }
    line_ptr line;
    line_ptr line_2;
    while ((count > fp->line_count) && !fp->eof) {
        // Try to read another line.
        str_object buffer(' ');
        strlen_range outlen;
        if (filesys_read(fp, buffer, outlen)) {
            if (outlen > 0)
                outlen = buffer.length(' ', outlen);
            if (!lines_create(1, line, line_2))
                return false;
            if (!line_change_length(line, outlen)) {
                lines_destroy(line, line_2);
                return false;
            }
            ch_fillcopy(&buffer, 1, outlen, line->str, 1, line->len, ' ');
            line->used = outlen;
            line->blink = fp->last_line;
            if (fp->last_line != nullptr)
                fp->last_line->flink = line;
            else
                fp->first_line = line;
            fp->last_line = line;
            fp->line_count += 1;
        } else if (!fp->eof) {
            // Something drastically wrong with the input!
            // As a TEMPORARY measure, ignore.
        }
    }

    // Check there is enough lines.
    if (fp->line_count < count) {
        if (!best_try) {
            screen_message(MSG_NOT_ENOUGH_INPUT_LEFT);
            return false;
        }
        count = fp->line_count;
    }

    // Break off the required lines.
    actual_cnt = count;
    if (count == 0) {
        first = nullptr;
        last = nullptr;
    } else if (fp->line_count == count) {
        // Give caller the whole list.
        first = fp->first_line;
        last = fp->last_line;
        fp->first_line = nullptr;
        fp->last_line = nullptr;
        fp->line_count = 0;
    } else {
        // give caller the first 'count' lines in the list.
        // find last line to be removed.
        if (count < fp->line_count / 2) {
            line = fp->first_line;
            for (int i = 2; i <= count; ++i)
                line = line->flink;
        } else {
            line = fp->last_line;
            for (int i = fp->line_count; i > count; --i)
                line = line->blink;
        }

        // Remove lines from list.
        first = fp->first_line;
        last = line;
        fp->first_line = line->flink;
        line->flink = nullptr;
        fp->first_line->blink = nullptr;
        fp->line_count -= count;
    }

    // Note we succeeded.
    return true;
}

bool file_write(line_ptr first_line, const_line_ptr last_line, file_ptr fp) {
    // Write a series of lines to an output file.
    // Stop when the last line output or when the next line is NIL.

    // Allow first_line=nil as a termination condition so that
    // this routine can be used in an emergency to save an edit
    // session after drastic internal corruption has happened.

    while (first_line != nullptr) {
        // with first_line^ do
        if (!filesys_write(fp, first_line->str, first_line->used))
            return false;
        if (first_line == last_line)
            return true;
        first_line = first_line->flink;
    }
    return true;
}

bool file_windthru(frame_ptr current, bool from_span) {
    // Write all the remaining input file to the output file.

    // with current^ do
    //  Check that there is something to windthru to!
    if (current->output_file < 0)
        return false;
    if (files[current->output_file] == nullptr)
        return false;
    if (current->text_modified && !from_span) {
        screen_message(MSG_WRITING_FILE);
        if (ludwig_mode == ludwig_mode_type::ludwig_screen)
            vdu_flush();
    }
    // Do any lines that have already been read in.
    line_ptr first_line = current->first_group->first_line;
    line_ptr last_line = current->last_group->last_line->blink;
    bool result = false;
    if ((first_line != nullptr) && (last_line != nullptr)) {
        if (current->text_modified) {
            if (!file_write(first_line, last_line, files[current->output_file]))
                goto l98;
        }
        if (!marks_squeeze(first_line, 1, last_line->flink, 1))
            goto l98;
        if (!lines_extract(first_line, last_line))
            goto l98;
        if (!lines_destroy(first_line, last_line))
            goto l98;
        if (current->input_file >= 0) {
            if (files[current->input_file] != nullptr)
                files[current->input_file]->line_count = 0;
        }
    }

    // So far so good, assume the rest is going to work
    result = true;
    if (current->text_modified) {
        // Only bother if we are going to keep the output
        if (current->input_file >= 0) {
            if (files[current->input_file] != nullptr) {
                if (!files[current->input_file]->eof) {
                    // Copy the file through until eof found.
                    str_object buffer;
                    strlen_range outlen;
                    while (filesys_read(files[current->input_file], buffer, outlen)) {
                        size_t buflen = outlen > 0 ? buffer.length(' ', outlen) : 0;
                        if (!filesys_write(files[current->output_file], &buffer, buflen)) {
                            // Whoops, something went wrong
                            result = false;
                            goto l98;
                        }
                    }
                }

                // Status depends on if we successfully read it all
                result = files[current->input_file]->eof;
            }
        }
    }
l98:;
    if (current->text_modified && !from_span)
        screen_clear_msgs(false);
    return result;
}

bool file_rewind(file_ptr &fp) {
    // Rewind a file.
    if (fp != nullptr) {
        // with fp^ do
        if (fp->first_line != nullptr) {
            // Dispose of any unused input lines.
            lines_destroy(fp->first_line, fp->last_line); // Ignore errors.
            fp->first_line = nullptr;
            fp->last_line = nullptr;
            fp->line_count = 0;
        }
    }
    filesys_rewind(fp); // Ignore failures due to unable to rewind SYS$INPUT, TT:, NL:, etc.
    return true;
}

bool file_page(frame_ptr current_frame, bool &exit_abort) {
    // with current_frame^,dot^ do
    line_ptr first_line;
    line_ptr last_line;
    if (!exec_compute_line_range(current_frame, leadparam::nindef, 0, first_line, last_line)) {
        screen_message(DBG_INTERNAL_LOGIC_ERROR);
        return false;
    }
    //  PAGE OUT THE STUFF ABOVE THE DOT LINE.
    if (first_line != nullptr) {
        if (current_frame->output_file >= 0 &&
            !file_write(first_line, last_line, files[current_frame->output_file])) {
            // SHOULD EXIT_ABORT, NOT JUST FAIL.
            exit_abort = true;
            return false;
        }
        if (last_line->flink == nullptr)
            return false;
        if (!marks_squeeze(first_line, 1, last_line->flink, 1))
            return false;
        if (!lines_extract(first_line, last_line))
            return false;
        if (!lines_destroy(first_line, last_line))
            return false;
    }
    //  PAGE IN THE NEW LINES
    if (current_frame->input_file < 0)
        goto l98;
    while ((current_frame->space_left * 10 > current_frame->space_limit) && !tt_controlc) {
        int i;
        if (!file_read(files[current_frame->input_file], 50, true, first_line, last_line, i))
            return false;
        current_frame->input_count += i;

        // Inject the inputted lines.
        if (first_line == nullptr)
            goto l98;
        if (!lines_inject(first_line, last_line, current_frame->last_group->last_line))
            return false;

        // IF DOT WAS ON THE NULL LINE, SHIFT IT ONTO THE FIRST LINE
        if (current_frame->dot->line->flink == nullptr) {
            if (!mark_create(first_line, current_frame->dot->col, current_frame->dot))
                return false;
        }
    }
l98:;
    if (current_frame->input_file >= 0)
        file_fix_eop(files[current_frame->input_file]->eof, current_frame->last_group->last_line);
    return true;
}

bool check_slot_allocation(slot_range slot, bool must_be_allocated, std::string &status) {
    if ((slot < 0) == must_be_allocated) {
        status = must_be_allocated ? MSG_NO_FILE_OPEN : MSG_FILE_ALREADY_OPEN;
        return false;
    }
    return true;
}

bool check_slot_usage(slot_range slot, bool must_be_in_use, std::string &status) {
    if (check_slot_allocation(slot, true, status)) {
        if ((files[slot] == nullptr) == must_be_in_use) {
            status = must_be_in_use ? MSG_NO_FILE_OPEN : MSG_FILE_ALREADY_OPEN;
        } else {
            return true;
        }
    }
    return false;
}

bool check_slot_direction(slot_range slot, bool must_be_output, std::string &status) {
    if (check_slot_usage(slot, true, status)) {
        if (files[slot]->output_flag != must_be_output) {
            status = must_be_output ? MSG_NOT_OUTPUT_FILE : MSG_NOT_INPUT_FILE;
        } else {
            return true;
        }
    }
    return false;
}

bool free_file(slot_range slot, std::string &status) {
    if (!check_slot_allocation(slot, true, status))
        return false;
    if (files_frames[slot] != nullptr) {
        // with files_frames[slot]^ do
        if (slot == files_frames[slot]->output_file) {
            files_frames[slot]->output_file = -1;
        } else {
            file_fix_eop(true, files_frames[slot]->last_group->last_line);
            files_frames[slot]->input_file = -1;
        }
        files_frames[slot] = nullptr;
    } else if (slot == fgi_file) {
        fgi_file = -1;
    } else if (slot == fgo_file) {
        fgo_file = -1;
    }
    return true;
}

bool get_free_slot(slot_range &new_slot, slot_range file_slot, std::string &status) {
    slot_range slot = 0;
    while ((slot < MAX_FILES) && ((files[slot] != nullptr) || (slot == file_slot)))
        slot += 1;
    if (slot >= MAX_FILES) {
        status = MSG_NO_MORE_FILES_ALLOWED;
        return false;
    }
    new_slot = slot;
    return true;
}

bool get_file_name(const_tpar_ptr tparam, file_name_str &fnm, commands command) {
    tpar_object tp_file_name;
    // with tp_file_name do
    tp_file_name.con = nullptr;
    tp_file_name.nxt = nullptr;
    if (!tpar_get_1(tparam, command, tp_file_name))
        return false;
    // with tp_file_name do
    fnm = std::string(tp_file_name.str.data(), tp_file_name.len);
    tpar_clean_object(tp_file_name);
    return true;
}

bool file_command(
    commands command, leadparam rept, int count, const_tpar_ptr tparam, bool from_span
) {
    // with current_frame^ do
    //  Fudge some of the commands that accept rept = minus.
    commands saved_cmd = command;
    if ((rept == leadparam::minus) && (command != commands::cmd_file_write)) {
        saved_cmd = command;
        command = commands::cmd_file_close;
    }

    // Perform the operation.
    std::string status;
    slot_range file_slot = -1;
    file_name_str fnm;
    bool result = false;
    switch (command) {
    case commands::cmd_file_input:
        {
            if (!check_slot_allocation(current_frame->input_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            if (!get_file_name(tparam, fnm, command))
                goto l99;
            file_ptr dummy_fptr = nullptr;
            if (!file_create_open(fnm, parse_type::parse_input, files[file_slot], dummy_fptr))
                goto l99;
            current_frame->input_file = file_slot;
            files_frames[file_slot] = current_frame;
            if (!from_span) {
                screen_message(MSG_LOADING_FILE);
                if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                    vdu_flush();
            }
            file_page(current_frame, exit_abort);

            // Clean up the LOADING message.
            if (!from_span)
                screen_clear_msgs(false);
        }
        break;

    case commands::cmd_file_global_input:
        {
            if (!check_slot_allocation(fgi_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            if (files[file_slot] == nullptr) {
                if (!get_file_name(tparam, fnm, command))
                    goto l99;
                file_ptr dummy_fptr = nullptr;
                if (!file_create_open(fnm, parse_type::parse_input, files[file_slot], dummy_fptr))
                    goto l99;
            }
            fgi_file = file_slot;
        }
        break;

    case commands::cmd_file_edit:
        {
            if (!check_slot_allocation(current_frame->input_file, false, status))
                goto l99;
            if (!check_slot_allocation(current_frame->output_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            slot_range file_slot_2;
            if (!get_free_slot(file_slot_2, file_slot, status))
                goto l99;

            if (!get_file_name(tparam, fnm, command))
                goto l99;
            if (!file_create_open(
                    fnm, parse_type::parse_edit, files[file_slot], files[file_slot_2]
                ))
                goto l99;
            current_frame->input_file = file_slot;
            files_frames[file_slot] = current_frame;
            current_frame->output_file = file_slot_2;
            files_frames[file_slot_2] = current_frame;
            if (!from_span) {
                screen_message(MSG_LOADING_FILE);
                if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                    vdu_flush();
            }
            file_page(current_frame, exit_abort);
            // Clean up the LOADING message.
            if (!from_span)
                screen_clear_msgs(false);
        }
        break;

    case commands::cmd_file_execute:
        {
            if (!check_slot_allocation(current_frame->input_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            if (!get_file_name(tparam, fnm, command))
                goto l99;
            file_ptr dummy_fptr = nullptr;
            if (!file_create_open(fnm, parse_type::parse_execute, files[file_slot], dummy_fptr))
                goto l99;
            current_frame->input_file = file_slot;
            files_frames[file_slot] = current_frame;
            file_page(current_frame, exit_abort);
            // Clean up the LOADING message.
            if (!free_file(file_slot, status))
                goto l99;
            if (!file_close_delete(files[file_slot], true, false))
                goto l99;
        }
        break;

    case commands::cmd_file_close:
        {
            switch (saved_cmd) {
            case /*-FI*/ commands::cmd_file_input:
                file_slot = current_frame->input_file;
                break;
            case /*-FO*/ commands::cmd_file_output:
                file_slot = current_frame->output_file;
                break;
            case /*-FGI*/ commands::cmd_file_global_input:
                file_slot = fgi_file;
                break;
            case /*-FGO*/ commands::cmd_file_global_output:
                file_slot = fgo_file;
                break;
            case /*-FE*/ commands::cmd_file_edit:
                file_slot = current_frame->input_file;
                break;
            default: /* Nothing */
                break;
            }
            if (saved_cmd == commands::cmd_file_output || saved_cmd == commands::cmd_file_edit) {
                if (!file_windthru(current_frame, from_span))
                    goto l99;
                /*
                  ! Update the screen now so that the file closed messages
                  ! remain visible.
                */
                screen_fixup();
            }
            if (!free_file(file_slot, status))
                goto l99;
            if (saved_cmd == commands::cmd_file_global_input ||
                saved_cmd == commands::cmd_file_global_output) {
                if (!file_close_delete(files[file_slot], false, true))
                    goto l99;
            } else {
                if (!file_close_delete(
                        files[file_slot],
                        !current_frame->text_modified,
                        current_frame->text_modified || !files[file_slot]->output_flag
                    ))
                    goto l99;
            }
            if (saved_cmd == commands::cmd_file_edit) {
                file_slot = current_frame->output_file;
                if (!free_file(file_slot, status))
                    goto l99;
                if (!file_close_delete(
                        files[file_slot],
                        !current_frame->text_modified,
                        current_frame->text_modified
                    ))
                    goto l99;
            }
            if (saved_cmd == commands::cmd_file_output || saved_cmd == commands::cmd_file_edit)
                current_frame->text_modified = false;
        }
        break;

    case commands::cmd_file_kill:
        {
            file_slot = current_frame->output_file;
            if (!free_file(file_slot, status))
                goto l99;
            if (!file_close_delete(files[file_slot], true, true))
                goto l99;
        }
        break;

    case commands::cmd_file_global_kill:
        {
            file_slot = fgo_file;
            if (!free_file(file_slot, status))
                goto l99;
            if (!file_close_delete(files[file_slot], true, true))
                goto l99;
        }
        break;

    case commands::cmd_file_output:
        {
            if (!check_slot_allocation(current_frame->output_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            if (!get_file_name(tparam, fnm, command))
                goto l99;
            if (current_frame->input_file >= 0) {
                if (!file_create_open(
                        fnm,
                        parse_type::parse_output,
                        files[current_frame->input_file],
                        files[file_slot]
                    ))
                    goto l99;
            } else {
                file_ptr dummy_fptr = nullptr;
                if (!file_create_open(fnm, parse_type::parse_output, dummy_fptr, files[file_slot]))
                    goto l99;
            }
            current_frame->output_file = file_slot;
            files_frames[file_slot] = current_frame;
        }
        break;

    case commands::cmd_file_global_output:
        {
            if (!check_slot_allocation(fgo_file, false, status))
                goto l99;
            if (!get_free_slot(file_slot, file_slot, status))
                goto l99;
            if (files[file_slot] == nullptr) {
                if (!get_file_name(tparam, fnm, command))
                    goto l99;
                file_ptr dummy_fptr = nullptr;
                if (!file_create_open(fnm, parse_type::parse_output, dummy_fptr, files[file_slot]))
                    goto l99;
            }
            fgo_file = file_slot;
        }
        break;

    case commands::cmd_file_read:
        {
            if (!check_slot_allocation(fgi_file, true, status))
                goto l99;
            int lines_to_read = count;
            if (rept == leadparam::pindef)
                lines_to_read = MAXINT;
            line_ptr first;
            line_ptr last;
            int i;
            if (!file_read(
                    files[fgi_file], lines_to_read, rept == leadparam::pindef, first, last, i
                ))
                goto l99;
            if (first != nullptr) {
                if (!lines_inject(first, last, current_frame->dot->line))
                    goto l99;
                if (!mark_create(first, 1, current_frame->marks[MARK_EQUALS]))
                    goto l99;
                current_frame->text_modified = true;
                if (!mark_create(last->flink, 1, current_frame->marks[MARK_MODIFIED]))
                    goto l99;
                if (!mark_create(last->flink, 1, current_frame->dot))
                    goto l99;
            }
        }
        break;

    case commands::cmd_file_write:
        {
            if (!check_slot_allocation(fgo_file, true, status))
                goto l99;
            // Establish which lines to write.
            if (!from_span) {
                screen_message(MSG_WRITING_FILE);
                if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                    vdu_flush();
            }
            line_ptr first;
            line_ptr last;
            if (!exec_compute_line_range(current_frame, rept, count, first, last))
                goto l99;
            if (first != nullptr) {
                if (!file_write(first, last, files[fgo_file]))
                    goto l99;
            }
            if (!from_span)
                screen_clear_msgs(false);
        }
        break;

    case commands::cmd_file_rewind:
        {
            if (!check_slot_direction(current_frame->input_file, false, status))
                goto l99;
            if (!file_rewind(files[current_frame->input_file]))
                goto l99;
            if (!from_span) {
                screen_message(MSG_LOADING_FILE);
                if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                    vdu_flush();
            }
            file_page(current_frame, exit_abort);
            // Clean up the LOADING message.
            if (!from_span)
                screen_clear_msgs(false);
        }
        break;

    case commands::cmd_file_global_rewind:
        {
            if (!check_slot_direction(fgi_file, false, status))
                goto l99;
            if (!file_rewind(files[fgi_file]))
                goto l99;
        }
        break;

    case commands::cmd_file_save:
        {
            if (current_frame->output_file < 0) {
                status = MSG_NO_OUTPUT;
                goto l99;
            }
            if (!current_frame->text_modified) {
                if (!from_span) {
                    screen_message(MSG_NOT_MODIFIED);
                    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                        vdu_flush();
                }
                result = true;
                goto l99;
            }
            if (!from_span) {
                screen_message(MSG_SAVING_FILE);
                if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                    vdu_flush();
            }
            int lines_written = files[current_frame->output_file]->l_counter;
            line_ptr first = current_frame->first_group->first_line;
            line_ptr last = current_frame->last_group->last_line->blink;
            // If the frame is empty, last = nullptr
            if (last != nullptr) {
                if (!file_write(first, last, files[current_frame->output_file]))
                    goto l99;
            }
            file_ptr dummy_fptr;
            if (current_frame->input_file >= 0)
                dummy_fptr = files[current_frame->input_file];
            else
                dummy_fptr = nullptr;
            if (!filesys_save(dummy_fptr, files[current_frame->output_file], lines_written))
                goto l99;
            line_range nr_lines;
            if (last == nullptr)
                nr_lines = 0;
            else if (!line_to_number(last, nr_lines))
                nr_lines = 0;
            current_frame->input_count = files[current_frame->output_file]->l_counter + nr_lines;
            if (current_frame->input_file >= 0)
                files[current_frame->input_file]->l_counter = current_frame->input_count;
            current_frame->text_modified = false;
        }
        break;

    default:
        // Nothing
        break;
    }

    result = true;

l99:;
    if (!status.empty())
        screen_message(status);
    return result;
}
