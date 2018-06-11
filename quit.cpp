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
! Name:         QUIT
!
! Description:  Quit Ludwig
*/

#include "quit.h"

#include "var.h"
#include "vdu.h"
#include "fyle.h"
#include "mark.h"
#include "const.h"
#include "msdos.h"
#include "screen.h"

namespace {
    const std::string NO_OUTPUT_FILE_MSG("This frame has no output file--are you sure you want to QUIT? ");
};

bool quit_command() {
    //with current_frame^ do
    if (ludwig_mode != ludwig_mode_type::ludwig_batch) {
        span_ptr new_span = first_span;
        while (new_span != nullptr) {
            if (new_span->frame != nullptr) {
                //with new_span->frame^ do
                if (new_span->frame->text_modified &&
                    new_span->frame->output_file == 0 &&
                    new_span->frame->input_file != 0) {
                    current_frame = new_span->frame;
                    //with marks[mark_modified]^ do
                    mark_ptr mm = new_span->frame->marks[MARK_MODIFIED];
                    mark_create(mm->line, mm->col, new_span->frame->dot);
                    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
                        screen_fixup();
                    screen_beep();
                    str_object tmp_buffer(' ');
                    tmp_buffer.copy_n(NO_OUTPUT_FILE_MSG.data(), NO_OUTPUT_FILE_MSG.size());
                    switch (screen_verify(tmp_buffer, NO_OUTPUT_FILE_MSG.size())) {
                    case verify_response::verify_reply_yes:
                        // Nothing to do here.
                        break;
                    case verify_response::verify_reply_always:
                        goto l2;
                    case verify_response::verify_reply_no:
                    case verify_response::verify_reply_quit:
                        exit_abort = true;
                        return false;
                    }
                }
            }
            new_span = new_span->flink;
        }
    }
l2:;
    screen_unload();
    if (ludwig_mode != ludwig_mode_type::ludwig_batch)
        screen_message(MSG_QUITTING);
    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
        vdu_flush(false);
    ludwig_aborted = false;
    quit_close_files();
    exit_handler(0);
    exit(NORMAL_EXIT);
    return true; // Given the exit above, this shouldn't happen
}


bool do_frame(frame_ptr f) {
    //with f^ do
    if (f->output_file == 0)
        return true;
    if (files[f->output_file] == nullptr)
        return true;
    // Wind out and close the associated input file.
    if (!file_windthru(f, true))
        return false;
    if (f->input_file != 0) {
	if (files[f->input_file] != nullptr) {
            if (!file_close_delete(files[f->input_file], false, true))
                return false;
            f->input_file = 0;
        }
    }
    // Close the output file.
    bool result = true;
    if (!ludwig_aborted) {
	result = file_close_delete(files[f->output_file], !f->text_modified, f->text_modified);
    }
    f->output_file = 0;
    return result;
}

void quit_close_files() {
    // THIS ROUTINE DOES BOTH THE NORMAL "Q" COMMAND, AND ALSO IS CALLED AS PART
    // OF THE LUDWIG "PROG_WINDUP" SEQUENCE.  THUS BY TYPING "^Y EXIT" USERS MAY
    // SAFELY ABORT LUDWIG AND NOT LOSE ANY FILES.
    span_ptr next_span = first_span;
    while (next_span != nullptr) {
        frame_ptr next_frame = next_span->frame;
        if (next_frame != nullptr) {
            if (!do_frame(next_frame))
                goto l99;
        }
        next_span = next_span->flink;
    }

    // Close all remaining files.
    if (!ludwig_aborted) {
        for (file_range file_index = 1; file_index <= MAX_FILES; ++file_index) {
            if (files[file_index] != nullptr) {
                if (!file_close_delete(files[file_index], false, true))
                    goto l99;
            }
        }
    }
l99:;
    // Now free up the VDU, thus re-setting anything we have changed
    if (!vdu_free_flag) {     // Has it been called already?
        vdu_free();
        vdu_free_flag = true; // Well it has now
        ludwig_mode = ludwig_mode_type::ludwig_batch;
    }
    if (ludwig_aborted) {
        screen_message(MSG_NOT_RENAMED);
        screen_message(MSG_ABORT);
    }
}
