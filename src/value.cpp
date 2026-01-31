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
! Name:         VALUE
!
! Description:  Initialization of global variables.
!**/

#include "value.h"

#include "const.h"
#include "type.h"
#include "var.h"

void setup_initial_values() {
    current_frame = nullptr;
    ludwig_aborted = false;
    exit_abort = false;
    hangup = false;
    edit_mode = mode_type::mode_insert;
    previous_mode = mode_type::mode_insert;
    for (auto i : file_range::iota()) {
        files[i] = nullptr;
        files_frames[i] = nullptr;
    }
    fgi_file = -1;
    fgo_file = -1;
    first_span = nullptr;
    ludwig_mode = ludwig_mode_type::ludwig_batch;
    command_introducer = '\\';
    scr_frame = nullptr;
    scr_msg_row = MAXINT;
    vdu_free_flag = false;
    exec_level = 0;

    // Set up the Free Group/Line/Mark Pools
    free_group_pool = nullptr;
    free_line_pool = nullptr;

    // Set up all the Default Default characteristics for a frame.

    for (int i = MIN_MARK_NUMBER; i <= MAX_MARK_NUMBER; ++i)
        initial_marks[i] = nullptr;
    initial_scr_height = 1;  // Set to tt_height for terminals.
    initial_scr_width = 132; // Set to tt_width  for terminals.
    initial_scr_offset = 0;
    initial_margin_left = 1;
    initial_margin_right = 132; // Set to tt_width  for terminals.
    initial_margin_top = 0;
    initial_margin_bottom = 0;
    initial_options.clear();

    // set up sets for prefixes

    // NOTE - this matches prefixcommands
    prefixes.clear();
    prefixes.add_range(commands::cmd_prefix_ast, commands::cmd_prefix_tilde);

    dflt_prompts[prompt_type::no_prompt] = "        ";
    dflt_prompts[prompt_type::char_prompt] = "Charset:";
    dflt_prompts[prompt_type::get_prompt] = "Get    :";
    dflt_prompts[prompt_type::equal_prompt] = "Equal  :";
    dflt_prompts[prompt_type::key_prompt] = "Key    :";
    dflt_prompts[prompt_type::cmd_prompt] = "Command:";
    dflt_prompts[prompt_type::span_prompt] = "Span   :";
    dflt_prompts[prompt_type::text_prompt] = "Text   :";
    dflt_prompts[prompt_type::frame_prompt] = "Frame  :";
    dflt_prompts[prompt_type::file_prompt] = "File   :";
    dflt_prompts[prompt_type::column_prompt] = "Column :";
    dflt_prompts[prompt_type::mark_prompt] = "Mark   :";
    dflt_prompts[prompt_type::param_prompt] = "Param  :";
    dflt_prompts[prompt_type::topic_prompt] = "Topic  :";
    dflt_prompts[prompt_type::replace_prompt] = "Replace:";
    dflt_prompts[prompt_type::by_prompt] = "By     :";
    dflt_prompts[prompt_type::verify_prompt] = "Verify ?";
    dflt_prompts[prompt_type::pattern_prompt] = "Pattern:";
    dflt_prompts[prompt_type::pattern_set_prompt] = "Pat Set:";

    file_data.old_cmds = true;
    file_data.entab = false;
    file_data.space = 500000;
    file_data.purge = false;
    file_data.versions = 1;
    file_data.initial.clear();

    word_elements[0] = SPACE_SET;
    /* word_elements[1]  = ALPHA_SET + NUMERIC_SET; */
    /* word_elements[2]  = PRINTABLE_SET - (word_elements[0] + word_elements[1]); */
    word_elements[1] = PRINTABLE_SET;
    word_elements[1].remove(SPACE_SET);
}

void init_cmd(
    commands cmd,
    std::initializer_list<leadparam> lps,
    equalaction eqa,
    tpcount_type tpc,
    prompt_type pnm1,
    bool tr1,
    bool mla1,
    prompt_type pnm2,
    bool tr2,
    bool mla2
) {
    cmd_attrib_rec *p = &cmd_attrib[cmd];
    p->lp_allowed.clear();
    p->lp_allowed.add(lps);
    p->eq_action = eqa;
    p->tpcount = tpc;
    tpar_attribute *t1 = &p->tpar_info[1];
    t1->prompt_name = pnm1;
    t1->trim_reply = tr1;
    t1->ml_allowed = mla1;
    tpar_attribute *t2 = &p->tpar_info[2];
    t2->prompt_name = pnm2;
    t2->trim_reply = tr2;
    t2->ml_allowed = mla2;
}

void initialize_command_table_part1() {
    init_cmd(
        commands::cmd_noop,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_up,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_down,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_right,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_left,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_home,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_return,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_tab,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_backtab,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_rubout,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_jump,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_advance,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_position_column,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_position_line,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_op_sys_command,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::cmd_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_forward,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_backward,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_right,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_left,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_scroll,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_top,
        {leadparam::none},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_end,
        {leadparam::none},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_new,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_middle,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_setheight,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_window_update,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_get,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::nint},
        equalaction::eqnil,
        1,
        prompt_type::get_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_next,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::nint},
        equalaction::eqnil,
        1,
        prompt_type::char_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_bridge,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::char_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_replace,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        2,
        prompt_type::replace_prompt,
        false,
        false,
        prompt_type::by_prompt,
        false,
        true
    );
    init_cmd(
        commands::cmd_equal_string,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pindef, leadparam::nindef},
        equalaction::eqnil,
        1,
        prompt_type::equal_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_equal_column,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pindef, leadparam::nindef},
        equalaction::eqnil,
        1,
        prompt_type::column_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_equal_mark,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pindef, leadparam::nindef},
        equalaction::eqnil,
        1,
        prompt_type::mark_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_equal_eol,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pindef, leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_equal_eop,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_equal_eof,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_overtype_mode,
        {leadparam::none},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_insert_mode,
        {leadparam::none},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_overtype_text,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        1,
        prompt_type::text_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_insert_text,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        1,
        prompt_type::text_prompt,
        false,
        true,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_type_text,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqold,
        1,
        prompt_type::text_prompt,
        false,
        true,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_insert_line,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::nint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_insert_char,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::nint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_insert_invisible,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_delete_line,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqdel,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_delete_char,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
}

void initialize_command_table_part2() {
    init_cmd(
        commands::cmd_swap_line,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqdel,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_split_line,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_ditto_up,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_ditto_down,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_case_up,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_case_low,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_case_edit,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_set_margin_left,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_set_margin_right,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_fill,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_justify,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_squash,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_centre,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_left,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_line_right,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_word_advance,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_word_delete,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_advance_paragraph,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_delete_paragraph,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_define,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::marker},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_transfer,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_copy,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_compile,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_jump,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_index,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_assign,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef},
        equalaction::eqnil,
        2,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::text_prompt,
        false,
        true
    );
    init_cmd(
        commands::cmd_block_define,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::marker},
        equalaction::eqnil,
        1,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_block_transfer,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_block_copy,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        1,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_frame_kill,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::frame_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_frame_edit,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::frame_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_frame_return,
        {leadparam::none, leadparam::plus, leadparam::pint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_execute,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_span_execute_no_recompile,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        1,
        prompt_type::span_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_frame_parameters,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::param_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_input,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_output,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_edit,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_read,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_write,
        {leadparam::none,
         leadparam::plus,
         leadparam::minus,
         leadparam::pint,
         leadparam::nint,
         leadparam::pindef,
         leadparam::nindef,
         leadparam::marker},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_close,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_rewind,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_kill,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_execute,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_save,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_table,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_global_input,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_global_output,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        1,
        prompt_type::file_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_global_rewind,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_file_global_kill,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_user_command_introducer,
        {leadparam::none},
        equalaction::eqold,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_user_key,
        {leadparam::none},
        equalaction::eqnil,
        2,
        prompt_type::key_prompt,
        true,
        false,
        prompt_type::cmd_prompt,
        false,
        true
    );
    init_cmd(
        commands::cmd_user_parent,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_user_subprocess,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_user_undo,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_help,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::topic_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_verify,
        {leadparam::none},
        equalaction::eqnil,
        1,
        prompt_type::verify_prompt,
        true,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_command,
        {leadparam::none, leadparam::plus, leadparam::minus},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_mark,
        {leadparam::none, leadparam::plus, leadparam::minus, leadparam::pint, leadparam::nint},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_page,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_quit,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_dump,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_validate,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_execute_string,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        1,
        prompt_type::cmd_prompt,
        false,
        true,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_do_last_command,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_extended,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_exit_abort,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_exit_fail,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_exit_success,
        {leadparam::none, leadparam::plus, leadparam::pint, leadparam::pindef},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_pattern_dummy_pattern,
        {},
        equalaction::eqnil,
        1,
        prompt_type::pattern_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_pattern_dummy_text,
        {},
        equalaction::eqnil,
        1,
        prompt_type::text_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
    init_cmd(
        commands::cmd_resize_window,
        {leadparam::none},
        equalaction::eqnil,
        0,
        prompt_type::no_prompt,
        false,
        false,
        prompt_type::no_prompt,
        false,
        false
    );
}

void value_initializations() {
    setup_initial_values();
    initialize_command_table_part1();
    initialize_command_table_part2();
}
