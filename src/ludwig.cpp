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
! Name:         LUDWIG
!
! Description:  LUDWIG startup and shutdown.   Organize the timing of
!               the session, and the other general details.
!**/

#include "exec.h"
#include "execimmed.h"
#include "filesys.h"
#include "frame.h"
#include "fyle.h"
#include "quit.h"
#include "screen.h"
#include "sys.h"
#include "user.h"
#include "value.h"
#include "var.h"
#include "vdu.h"

#include <sstream>

void prog_windup(bool set_hangup) {

    hangup = set_hangup;

    // DISABLE EVERYTHING TO DO WITH VDU'S AND INTERACTIVE USERS.
    // THIS IS BECAUSE VDU_FREE MUST HAVE BEEN INVOKED BEFORE
    // THIS EXIT HANDLER WAS, HENCE THE VDU IS NO LONGER AVAIL.

    ludwig_mode = ludwig_mode_type::ludwig_batch;
    scr_frame = nullptr;
    scr_top_line = nullptr;
    scr_bot_line = nullptr;
    tt_controlc = false;
    exit_abort = false;

    // WIND OUT EVERYTHING FOR THE USER -- Gee that's nice of us!

    quit_close_files();
}

void initialize() {
    initial_tab_stops = DEFAULT_TAB_STOPS;

    // Now create the Code Header for the compiler to use
    code_top = 0;
    code_list = new code_header;
    // with code_list^ do
    code_list->flink = code_list;
    code_list->blink = code_list;
    code_list->ref = 1;
    code_list->code = 0;
    code_list->len = 0;
}

bool start_up(int argc, char **argv) {
    const std::string_view frame_name_cmd{"COMMAND"};
    const std::string_view frame_name_oops{"OOPS"};
    const std::string_view frame_name_heap{"HEAP"};

    bool result = false;

    // Get the command line.
    std::stringstream ss;
    if (argc > 1) {
        ss << argv[1];
        for (int i = 2; i < argc; ++i)
            ss << " " << argv[i];
    }

    std::string command_line = ss.str();
    if (command_line.size() > FILE_NAME_LEN) {
        screen_message(MSG_PARAMETER_TOO_LONG);
        goto l99;
    }

    // Open the files.
    if (!file_create_open(command_line, parse_type::parse_command, files[0], files[1]))
        goto l99;

    load_command_table(file_data.old_cmds);

    // Try to get started on the terminal.  If this fails assume carry on
    // in BATCH mode.
    ludwig_mode = ludwig_mode_type::ludwig_batch;
    if (vdu_init(terminal_info, tt_controlc, tt_winchanged)) {
        initial_scr_width = terminal_info.width;
        initial_scr_height = terminal_info.height;
        initial_margin_right = terminal_info.width;
        //        if (trmflags_v_hard & tt_capabilities) {
        //            ludwig_mode = ludwig_mode_type::ludwig_hardcopy;
        //        } else
        {
            ludwig_mode = ludwig_mode_type::ludwig_screen;
            vdu_new_introducer(command_introducer);
        }
    }
    // Set the scr_msg_row as one more than the terminal height (which may
    // be zero). This avoids any need for special checks about Ludwig being
    // in Screen mode before clearing messages.

    scr_msg_row = terminal_info.height + 1;

    // Create the three automatically defined frames: OOPS, COMMAND and LUDWIG.
    // Save pointers to COMMAND & OOPS  frames for use in later frame routines.

    if (!frame_edit(frame_name_oops))
        goto l99;
    if (!frame_setheight(initial_scr_height, true))
        goto l99;
    frame_oops = current_frame;
    current_frame = nullptr;
    frame_oops->space_limit = MAX_SPACE;     // Big !
    frame_oops->space_left = MAX_SPACE - 50; // Big ! - space for <eop> line !!
    frame_oops->options.insert(frame_options_elts::opt_special_frame);
    if (!frame_edit(frame_name_cmd))
        goto l99;
    frame_cmd = current_frame;
    current_frame = nullptr;
    frame_cmd->options.insert(frame_options_elts::opt_special_frame);
    if (!frame_edit(frame_name_heap))
        goto l99;
    frame_heap = current_frame;
    current_frame = nullptr;
    frame_heap->options.insert(frame_options_elts::opt_special_frame);
    {
        if (!frame_edit(DEFAULT_FRAME_NAME))
            goto l99;
    }

    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
        screen_fixup();

    // Load the key definitions.

    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
        user_key_initialize();

    // Hook our input and output files into the current frame.

    // with current_frame^ do
    if (files[0] != nullptr) {
        current_frame->input_file = 0;
        files_frames[0] = current_frame;
    }
    if (files[1] != nullptr) {
        current_frame->output_file = 1;
        files_frames[1] = current_frame;
    }

    // Load the input file.

    if (ludwig_mode != ludwig_mode_type::ludwig_batch) {
        screen_message(MSG_COPYRIGHT_AND_LOADING_FILE);
        if (ludwig_mode == ludwig_mode_type::ludwig_screen)
            vdu_flush();
    }
    if (!file_page(current_frame, exit_abort))
        goto l99;
    if (ludwig_mode != ludwig_mode_type::ludwig_batch)
        screen_clear_msgs(false);
    if (ludwig_mode == ludwig_mode_type::ludwig_screen)
        screen_fixup();

    // Execute the user's initialization string.

    if (!file_data.initial.empty()) {
        if (ludwig_mode == ludwig_mode_type::ludwig_screen)
            vdu_flush();
        tpar_object tparam;
        // with tparam^ do
        tparam.len = file_data.initial.size();
        tparam.dlm = TPD_EXACT;
        tparam.str.copy_n(file_data.initial.data(), tparam.len);
        tparam.nxt = nullptr;
        tparam.con = nullptr;
        if (!execute(commands::cmd_file_execute, leadparam::none, 1, &tparam, true)) {
            if (exit_abort) {
                // something is wrong, but let the user continue anyway!
                if (ludwig_mode != ludwig_mode_type::ludwig_batch)
                    screen_beep();
                exit_abort = false;
            }
        }
    }

    // Set the Abort Flag now.  This will suppress spurious start-up messages
    ludwig_aborted = true;
    result = true;
l99:;
    return result;
}

int main(int argc, char **argv) {
    sys_initsig();
    value_initializations();
    initialize();               // Stuff VALUE can't do, like creating frames etc.
    if (start_up(argc, argv)) { // Parse command line, get files attached, etc.
        execute_immed();
        sys_exit_success();
    }
    if (ludwig_aborted) {
        sys_exit_failure();
    }
    sys_exit_success();
}
