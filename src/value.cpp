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
    tpar_attribute *t1 = &p->tpar_info[0];
    t1->prompt_name = pnm1;
    t1->trim_reply = tr1;
    t1->ml_allowed = mla1;
    tpar_attribute *t2 = &p->tpar_info[1];
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

void addlookupexp(int index, char ch, commands cmd) {
    lookupexp[index].extn = ch;
    lookupexp[index].command = cmd;
}

void load_command_table(bool old_version) {
    key_code_range key_code;

    for (key_code = -MAX_SPECIAL_KEYS; key_code <= -1; ++key_code)
        lookup[key_code].command = commands::cmd_noop;
    if (old_version) {
        lookup[0].command = commands::cmd_noop;
        lookup[1].command = commands::cmd_noop;
        lookup[2].command = commands::cmd_window_backward;
        lookup[3].command = commands::cmd_noop;
        lookup[4].command = commands::cmd_delete_char;
        lookup[5].command = commands::cmd_window_end;
        lookup[6].command = commands::cmd_window_forward;
        lookup[7].command = commands::cmd_do_last_command;
        lookup[8].command = commands::cmd_rubout;
        lookup[9].command = commands::cmd_tab;
        lookup[10].command = commands::cmd_down;
        lookup[11].command = commands::cmd_delete_line;
        lookup[12].command = commands::cmd_insert_line;
        lookup[13].command = commands::cmd_return;
        lookup[14].command = commands::cmd_window_new;
        lookup[15].command = commands::cmd_noop;
        lookup[16].command = commands::cmd_user_command_introducer;
        lookup[17].command = commands::cmd_noop;
        lookup[18].command = commands::cmd_right;
        lookup[19].command = commands::cmd_noop;
        lookup[20].command = commands::cmd_window_top;
        lookup[21].command = commands::cmd_up;
        lookup[22].command = commands::cmd_noop;
        lookup[23].command = commands::cmd_word_advance;
        lookup[24].command = commands::cmd_noop;
        lookup[25].command = commands::cmd_noop;
        lookup[26].command = commands::cmd_user_parent;
        lookup[27].command = commands::cmd_noop;
        lookup[28].command = commands::cmd_noop;
        lookup[29].command = commands::cmd_noop;
        lookup[30].command = commands::cmd_insert_char;
        lookup[31].command = commands::cmd_noop;
        lookup[' '].command = commands::cmd_noop;
        lookup['!'].command = commands::cmd_noop;
        lookup['"'].command = commands::cmd_ditto_up;
        lookup['#'].command = commands::cmd_noop;
        lookup['$'].command = commands::cmd_noop;
        lookup['%'].command = commands::cmd_noop;
        lookup['&'].command = commands::cmd_noop;
        lookup['\''].command = commands::cmd_ditto_down;
        lookup['('].command = commands::cmd_noop;
        lookup[')'].command = commands::cmd_noop;
        lookup['*'].command = commands::cmd_prefix_ast;
        lookup['+'].command = commands::cmd_noop;
        lookup[';'].command = commands::cmd_noop;
        lookup['-'].command = commands::cmd_noop;
        lookup['.'].command = commands::cmd_noop;
        lookup['/'].command = commands::cmd_noop;
        for (key_code = '0'; key_code <= '9'; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        lookup[':'].command = commands::cmd_noop;
        lookup[';'].command = commands::cmd_noop;
        lookup['<'].command = commands::cmd_noop;
        lookup['='].command = commands::cmd_noop;
        lookup['>'].command = commands::cmd_noop;
        lookup['?'].command = commands::cmd_insert_invisible;
        lookup['@'].command = commands::cmd_noop;
        lookup['A'].command = commands::cmd_advance;
        lookup['B'].command = commands::cmd_prefix_b;
        lookup['C'].command = commands::cmd_insert_char;
        lookup['D'].command = commands::cmd_delete_char;
        lookup['E'].command = commands::cmd_prefix_e;
        lookup['F'].command = commands::cmd_prefix_f;
        lookup['G'].command = commands::cmd_get;
        lookup['H'].command = commands::cmd_help;
        lookup['I'].command = commands::cmd_insert_text;
        lookup['J'].command = commands::cmd_jump;
        lookup['K'].command = commands::cmd_delete_line;
        lookup['L'].command = commands::cmd_insert_line;
        lookup['M'].command = commands::cmd_mark;
        lookup['N'].command = commands::cmd_next;
        lookup['O'].command = commands::cmd_overtype_text;
        lookup['P'].command = commands::cmd_noop;
        lookup['Q'].command = commands::cmd_quit;
        lookup['R'].command = commands::cmd_replace;
        lookup['S'].command = commands::cmd_prefix_s;
        lookup['T'].command = commands::cmd_noop;
        lookup['U'].command = commands::cmd_prefix_u;
        lookup['V'].command = commands::cmd_verify;
        lookup['W'].command = commands::cmd_prefix_w;
        lookup['X'].command = commands::cmd_prefix_x;
        lookup['Y'].command = commands::cmd_prefix_y;
        lookup['Z'].command = commands::cmd_prefix_z;
        lookup['['].command = commands::cmd_noop;
        lookup['\\'].command = commands::cmd_command;
        lookup[']'].command = commands::cmd_noop;
        lookup['^'].command = commands::cmd_execute_string;
        lookup['_'].command = commands::cmd_noop;
        lookup['`'].command = commands::cmd_noop;
        for (key_code = 'a'; key_code <= 'z'; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        lookup['{'].command = commands::cmd_set_margin_left;
        lookup['|'].command = commands::cmd_noop;
        lookup['}'].command = commands::cmd_set_margin_right;
#ifdef DEBUG
        lookup['~'].command = commands::cmd_prefix_tilde;
#else
        lookup['~'].command = commands::cmd_noop;
#endif
        lookup[127].command = commands::cmd_rubout;
        for (key_code = 128; key_code <= ORD_MAXCHAR; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        for (key_code = -MAX_SPECIAL_KEYS; key_code <= ORD_MAXCHAR; ++key_code) {
            // with lookup[key_code] do
            lookup[key_code].code = nullptr;
            lookup[key_code].tpar = nullptr;
        }

        // initialize lookupexp
        // case change command ; command =  * prefix }      {start at 0}
        addlookupexp(0, 'U', commands::cmd_case_up);
        addlookupexp(1, 'L', commands::cmd_case_low);
        addlookupexp(2, 'E', commands::cmd_case_edit);

        // A prefix }    {3}
        // There aren't any in this table! }

        // B prefix }    {3}
        addlookupexp(3, 'R', commands::cmd_bridge);

        // C prefix }    {4}
        // There aren't any in this table! }

        // D prefix }    {4}
        // There aren't any in this table! }

        // E prefix }    {4}
        addlookupexp(4, 'X', commands::cmd_span_execute);
        addlookupexp(5, 'D', commands::cmd_frame_edit);
        addlookupexp(6, 'R', commands::cmd_frame_return);
        addlookupexp(7, 'N', commands::cmd_span_execute_no_recompile);
        addlookupexp(8, 'Q', commands::cmd_prefix_eq);
        addlookupexp(9, 'O', commands::cmd_prefix_eo);
        addlookupexp(10, 'K', commands::cmd_frame_kill);
        addlookupexp(11, 'P', commands::cmd_frame_parameters);

        // EO prefix }   {12}
        addlookupexp(12, 'L', commands::cmd_equal_eol);
        addlookupexp(13, 'F', commands::cmd_equal_eof);
        addlookupexp(14, 'P', commands::cmd_equal_eop);

        // EQ prefix }   {15}
        addlookupexp(15, 'S', commands::cmd_equal_string);
        addlookupexp(16, 'C', commands::cmd_equal_column);
        addlookupexp(17, 'M', commands::cmd_equal_mark);

        // F prefix - files }    {18}
        addlookupexp(18, 'S', commands::cmd_file_save);
        addlookupexp(19, 'B', commands::cmd_file_rewind);
        addlookupexp(20, 'I', commands::cmd_file_input);
        addlookupexp(21, 'E', commands::cmd_file_edit);
        addlookupexp(22, 'O', commands::cmd_file_output);
        addlookupexp(23, 'G', commands::cmd_prefix_fg);
        addlookupexp(24, 'K', commands::cmd_file_kill);
        addlookupexp(25, 'X', commands::cmd_file_execute);
        addlookupexp(26, 'T', commands::cmd_file_table);
        addlookupexp(27, 'P', commands::cmd_page);

        // FG prefix - global files }    {28}
        addlookupexp(28, 'I', commands::cmd_file_global_input);
        addlookupexp(29, 'O', commands::cmd_file_global_output);
        addlookupexp(30, 'B', commands::cmd_file_global_rewind);
        addlookupexp(31, 'K', commands::cmd_file_global_kill);
        addlookupexp(32, 'R', commands::cmd_file_read);
        addlookupexp(33, 'W', commands::cmd_file_write);

        // I prefix }    {34}
        // There aren't any in this table! }

        // K prefix }    {34}
        // There aren't any in this table! }

        // L prefix }    {34}
        // There aren't any in this table! }

        // O prefix }    {34}
        // There aren't any in this table! }

        // P prefix }    {34}
        // There aren't any in this table! }

        // S prefix - mainly spans }     {34}
        addlookupexp(34, 'A', commands::cmd_span_assign);
        addlookupexp(35, 'C', commands::cmd_span_copy);
        addlookupexp(36, 'D', commands::cmd_span_define);
        addlookupexp(37, 'T', commands::cmd_span_transfer);
        addlookupexp(38, 'W', commands::cmd_swap_line);
        addlookupexp(39, 'L', commands::cmd_split_line);
        addlookupexp(40, 'J', commands::cmd_span_jump);
        addlookupexp(41, 'I', commands::cmd_span_index);
        addlookupexp(42, 'R', commands::cmd_span_compile);

        // T prefix }    {43}
        // There aren't any in this table! }

        // TC prefix }    {43}
        // There aren't any in this table! }

        // TF prefix }    {43}
        // There aren't any in this table! }

        // U prefix - user keyboard mappings }   {43}
        addlookupexp(43, 'C', commands::cmd_user_command_introducer);
        addlookupexp(44, 'K', commands::cmd_user_key);
        addlookupexp(45, 'P', commands::cmd_user_parent);
        addlookupexp(46, 'S', commands::cmd_user_subprocess);

        // W prefix - window commands }  {47}
        addlookupexp(47, 'F', commands::cmd_window_forward);
        addlookupexp(48, 'B', commands::cmd_window_backward);
        addlookupexp(49, 'M', commands::cmd_window_middle);
        addlookupexp(50, 'T', commands::cmd_window_top);
        addlookupexp(51, 'E', commands::cmd_window_end);
        addlookupexp(52, 'N', commands::cmd_window_new);
        addlookupexp(53, 'R', commands::cmd_window_right);
        addlookupexp(54, 'L', commands::cmd_window_left);
        addlookupexp(55, 'H', commands::cmd_window_setheight);
        addlookupexp(56, 'S', commands::cmd_window_scroll);
        addlookupexp(57, 'U', commands::cmd_window_update);

        // X prefix - exit }             {58}
        addlookupexp(58, 'S', commands::cmd_exit_success);
        addlookupexp(59, 'F', commands::cmd_exit_fail);
        addlookupexp(60, 'A', commands::cmd_exit_abort);

        // Y prefix - word processing }  {61}
        addlookupexp(61, 'F', commands::cmd_line_fill);
        addlookupexp(62, 'J', commands::cmd_line_justify);
        addlookupexp(63, 'S', commands::cmd_line_squash);
        addlookupexp(64, 'C', commands::cmd_line_centre);
        addlookupexp(65, 'L', commands::cmd_line_left);
        addlookupexp(66, 'R', commands::cmd_line_right);
        addlookupexp(67, 'A', commands::cmd_word_advance);
        addlookupexp(68, 'D', commands::cmd_word_delete);

        // Z prefix - cursor commands }  {69}
        addlookupexp(69, 'U', commands::cmd_up);
        addlookupexp(70, 'D', commands::cmd_down);
        addlookupexp(71, 'R', commands::cmd_right);
        addlookupexp(72, 'L', commands::cmd_left);
        addlookupexp(73, 'H', commands::cmd_home);
        addlookupexp(74, 'C', commands::cmd_return);
        addlookupexp(75, 'T', commands::cmd_tab);
        addlookupexp(76, 'B', commands::cmd_backtab);
        addlookupexp(77, 'Z', commands::cmd_rubout);

        // ~ prefix - miscellaneous debugging commands}  {78}
        addlookupexp(78, 'V', commands::cmd_validate);
        addlookupexp(79, 'D', commands::cmd_dump);

        // sentinel }                    {80}
        addlookupexp(80, '?', commands::cmd_nosuch);

        // initialize lookupexp_ptr }
        // These magic numbers point to the start/end of each section in lookupexp table }
        // (converted to 0-based indices)
        lookupexp_ptr[commands::cmd_prefix_ast] = {0, 3};
        lookupexp_ptr[commands::cmd_prefix_a] = {3, 3};
        lookupexp_ptr[commands::cmd_prefix_b] = {3, 4};
        lookupexp_ptr[commands::cmd_prefix_c] = {4, 4};
        lookupexp_ptr[commands::cmd_prefix_d] = {4, 4};
        lookupexp_ptr[commands::cmd_prefix_e] = {4, 12};
        lookupexp_ptr[commands::cmd_prefix_eo] = {12, 15};
        lookupexp_ptr[commands::cmd_prefix_eq] = {15, 18};
        lookupexp_ptr[commands::cmd_prefix_f] = {18, 28};
        lookupexp_ptr[commands::cmd_prefix_fg] = {28, 34};
        lookupexp_ptr[commands::cmd_prefix_i] = {34, 34};
        lookupexp_ptr[commands::cmd_prefix_k] = {34, 34};
        lookupexp_ptr[commands::cmd_prefix_l] = {34, 34};
        lookupexp_ptr[commands::cmd_prefix_o] = {34, 34};
        lookupexp_ptr[commands::cmd_prefix_p] = {34, 34};
        lookupexp_ptr[commands::cmd_prefix_s] = {34, 43};
        lookupexp_ptr[commands::cmd_prefix_t] = {43, 43};
        lookupexp_ptr[commands::cmd_prefix_tc] = {43, 43};
        lookupexp_ptr[commands::cmd_prefix_tf] = {43, 43};
        lookupexp_ptr[commands::cmd_prefix_u] = {43, 47};
        lookupexp_ptr[commands::cmd_prefix_w] = {47, 58};
        lookupexp_ptr[commands::cmd_prefix_x] = {58, 61};
        lookupexp_ptr[commands::cmd_prefix_y] = {61, 69};
        lookupexp_ptr[commands::cmd_prefix_z] = {69, 78};
        lookupexp_ptr[commands::cmd_prefix_tilde] = {78, 80};
        lookupexp_ptr[commands::cmd_nosuch] = {80, 81};
    } else {
        lookup[0].command = commands::cmd_noop;
        lookup[1].command = commands::cmd_noop;
        lookup[2].command = commands::cmd_window_backward;
        lookup[3].command = commands::cmd_noop;
        lookup[4].command = commands::cmd_delete_char;
        lookup[5].command = commands::cmd_window_end;
        lookup[6].command = commands::cmd_window_forward;
        lookup[7].command = commands::cmd_do_last_command;
        lookup[8].command = commands::cmd_left;
        lookup[9].command = commands::cmd_tab;
        lookup[10].command = commands::cmd_down;
        lookup[11].command = commands::cmd_delete_line;
        lookup[12].command = commands::cmd_insert_line;
        lookup[13].command = commands::cmd_return;
        lookup[14].command = commands::cmd_window_new;
        lookup[15].command = commands::cmd_noop;
        lookup[16].command = commands::cmd_user_command_introducer;
        lookup[17].command = commands::cmd_noop;
        lookup[18].command = commands::cmd_right;
        lookup[19].command = commands::cmd_noop;
        lookup[20].command = commands::cmd_window_top;
        lookup[21].command = commands::cmd_up;
        lookup[22].command = commands::cmd_noop;
        lookup[23].command = commands::cmd_word_advance;
        lookup[24].command = commands::cmd_noop;
        lookup[25].command = commands::cmd_noop;
        lookup[26].command = commands::cmd_user_parent;
        lookup[27].command = commands::cmd_noop;
        lookup[28].command = commands::cmd_noop;
        lookup[29].command = commands::cmd_noop;
        lookup[30].command = commands::cmd_insert_char;
        lookup[31].command = commands::cmd_noop;
        lookup[' '].command = commands::cmd_noop;
        lookup['!'].command = commands::cmd_noop;
        lookup['"'].command = commands::cmd_ditto_up;
        lookup['#'].command = commands::cmd_noop;
        lookup['$'].command = commands::cmd_noop;
        lookup['%'].command = commands::cmd_noop;
        lookup['&'].command = commands::cmd_noop;
        lookup['\''].command = commands::cmd_ditto_down;
        lookup['('].command = commands::cmd_noop;
        lookup[')'].command = commands::cmd_noop;
        lookup['*'].command = commands::cmd_noop;
        lookup['+'].command = commands::cmd_noop;
        lookup[';'].command = commands::cmd_noop;
        lookup['-'].command = commands::cmd_noop;
        lookup['.'].command = commands::cmd_noop;
        lookup['/'].command = commands::cmd_noop;
        for (key_code = '0'; key_code <= '9'; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        lookup[':'].command = commands::cmd_noop;
        lookup[';'].command = commands::cmd_noop;
        lookup['<'].command = commands::cmd_noop;
        lookup['='].command = commands::cmd_noop;
        lookup['>'].command = commands::cmd_noop;
        lookup['?'].command = commands::cmd_noop;
        lookup['@'].command = commands::cmd_noop;
        lookup['A'].command = commands::cmd_prefix_a;
        lookup['B'].command = commands::cmd_prefix_b;
        lookup['C'].command = commands::cmd_prefix_c;
        lookup['D'].command = commands::cmd_prefix_d;
        lookup['E'].command = commands::cmd_prefix_e;
        lookup['F'].command = commands::cmd_prefix_f;
        lookup['G'].command = commands::cmd_get;
        lookup['H'].command = commands::cmd_help;
        lookup['I'].command = commands::cmd_noop;
        lookup['J'].command = commands::cmd_noop;
        lookup['K'].command = commands::cmd_prefix_k;
        lookup['L'].command = commands::cmd_prefix_l;
        lookup['M'].command = commands::cmd_mark;
        lookup['N'].command = commands::cmd_noop;
        lookup['O'].command = commands::cmd_prefix_o;
        lookup['P'].command = commands::cmd_prefix_p;
        lookup['Q'].command = commands::cmd_quit;
        lookup['R'].command = commands::cmd_replace;
        lookup['S'].command = commands::cmd_prefix_s;
        lookup['T'].command = commands::cmd_prefix_t;
        lookup['U'].command = commands::cmd_prefix_u;
        lookup['V'].command = commands::cmd_verify;
        lookup['W'].command = commands::cmd_prefix_w;
        lookup['X'].command = commands::cmd_prefix_x;
        lookup['Y'].command = commands::cmd_noop;
        lookup['Z'].command = commands::cmd_noop;
        lookup['['].command = commands::cmd_noop;
        lookup['\\'].command = commands::cmd_command;
        lookup[']'].command = commands::cmd_noop;
        lookup['^'].command = commands::cmd_noop;
        lookup['_'].command = commands::cmd_noop;
        lookup['`'].command = commands::cmd_noop;
        for (key_code = 'a'; key_code <= 'z'; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        lookup['{'].command = commands::cmd_set_margin_left;
        lookup['|'].command = commands::cmd_noop;
        lookup['}'].command = commands::cmd_set_margin_right;
#ifdef DEBUG
        lookup['~'].command = commands::cmd_prefix_tilde;
#else
        lookup['~'].command = commands::cmd_noop;
#endif
        lookup[127].command = commands::cmd_rubout;
        for (key_code = 128; key_code <= ORD_MAXCHAR; ++key_code)
            lookup[key_code].command = commands::cmd_noop;
        for (key_code = -MAX_SPECIAL_KEYS; key_code <= ORD_MAXCHAR; ++key_code) {
            // with lookup[key_code] do
            lookup[key_code].code = nullptr;
            lookup[key_code].tpar = nullptr;
        }

        // initialize lookupexp
        // Ast ( * ) prefix } {start at 0}
        // There aren't any in this table!

        // A prefix }    {start at 0}
        addlookupexp(0, 'C', commands::cmd_jump);
        addlookupexp(1, 'L', commands::cmd_advance);
        addlookupexp(2, 'O', commands::cmd_bridge);
        addlookupexp(3, 'P', commands::cmd_advance_paragraph);
        addlookupexp(4, 'S', commands::cmd_noop);
        addlookupexp(5, 'T', commands::cmd_next);
        addlookupexp(6, 'W', commands::cmd_word_advance);

        // B prefix }    {7}
        addlookupexp(7, 'B', commands::cmd_noop);
        addlookupexp(8, 'C', commands::cmd_noop);  // cmd_block_copy
        addlookupexp(9, 'D', commands::cmd_noop); // cmd_block_define
        addlookupexp(10, 'I', commands::cmd_noop);
        addlookupexp(11, 'K', commands::cmd_noop);
        addlookupexp(12, 'M', commands::cmd_noop); // cmd_block_transfer
        addlookupexp(13, 'O', commands::cmd_noop);

        // C prefix }    {14}
        addlookupexp(14, 'C', commands::cmd_insert_char);
        addlookupexp(15, 'L', commands::cmd_insert_line);

        // D prefix }    {16}
        addlookupexp(16, 'C', commands::cmd_delete_char);
        addlookupexp(17, 'L', commands::cmd_delete_line);
        addlookupexp(18, 'P', commands::cmd_delete_paragraph);
        addlookupexp(19, 'S', commands::cmd_noop);
        addlookupexp(20, 'W', commands::cmd_word_delete);

        // E prefix }    {21}
        addlookupexp(21, 'D', commands::cmd_frame_edit);
        addlookupexp(22, 'K', commands::cmd_frame_kill);
        addlookupexp(23, 'O', commands::cmd_prefix_eo);
        addlookupexp(24, 'P', commands::cmd_frame_parameters);
        addlookupexp(25, 'Q', commands::cmd_prefix_eq);
        addlookupexp(26, 'R', commands::cmd_frame_return);

        // EO prefix }   {27}
        addlookupexp(27, 'L', commands::cmd_equal_eol);
        addlookupexp(28, 'F', commands::cmd_equal_eof);
        addlookupexp(29, 'P', commands::cmd_equal_eop);

        // EQ prefix }   {30}
        addlookupexp(30, 'C', commands::cmd_equal_column);
        addlookupexp(31, 'L', commands::cmd_noop);
        addlookupexp(32, 'M', commands::cmd_equal_mark);
        addlookupexp(33, 'S', commands::cmd_equal_string);

        // F prefix - files }    {34}
        addlookupexp(34, 'S', commands::cmd_file_save);
        addlookupexp(35, 'B', commands::cmd_file_rewind);
        addlookupexp(36, 'E', commands::cmd_file_edit);
        addlookupexp(37, 'G', commands::cmd_prefix_fg);
        addlookupexp(38, 'I', commands::cmd_file_input);
        addlookupexp(39, 'K', commands::cmd_file_kill);
        addlookupexp(40, 'O', commands::cmd_file_output);
        addlookupexp(41, 'P', commands::cmd_page);
        addlookupexp(42, 'S', commands::cmd_noop);
        addlookupexp(43, 'T', commands::cmd_file_table);
        addlookupexp(44, 'X', commands::cmd_file_execute);

        // FG prefix - global files }    {45}
        addlookupexp(45, 'B', commands::cmd_file_global_rewind);
        addlookupexp(46, 'I', commands::cmd_file_global_input);
        addlookupexp(47, 'K', commands::cmd_file_global_kill);
        addlookupexp(48, 'O', commands::cmd_file_global_output);
        addlookupexp(49, 'R', commands::cmd_file_read);
        addlookupexp(50, 'W', commands::cmd_file_write);

        // I prefix }    {51}
        // There aren't any yet! }

        // K prefix }    {51}
        addlookupexp(51, 'B', commands::cmd_backtab);
        addlookupexp(52, 'C', commands::cmd_return);
        addlookupexp(53, 'D', commands::cmd_down);
        addlookupexp(54, 'H', commands::cmd_home);
        addlookupexp(55, 'I', commands::cmd_insert_mode);
        addlookupexp(56, 'L', commands::cmd_left);
        addlookupexp(57, 'M', commands::cmd_user_key);
        addlookupexp(58, 'O', commands::cmd_overtype_mode);
        addlookupexp(59, 'R', commands::cmd_right);
        addlookupexp(60, 'T', commands::cmd_tab);
        addlookupexp(61, 'U', commands::cmd_up);
        addlookupexp(62, 'X', commands::cmd_rubout);

        // L prefix }    {63}
        addlookupexp(63, 'R', commands::cmd_noop);
        addlookupexp(64, 'S', commands::cmd_noop);

        // O prefix }    {65}
        addlookupexp(65, 'P', commands::cmd_user_parent);
        addlookupexp(66, 'S', commands::cmd_user_subprocess);
        addlookupexp(67, 'X', commands::cmd_op_sys_command);

        // P prefix }    {68}
        addlookupexp(68, 'C', commands::cmd_position_column);
        addlookupexp(69, 'L', commands::cmd_position_line);

        // S prefix }    {70}
        addlookupexp(70, 'A', commands::cmd_span_assign);
        addlookupexp(71, 'C', commands::cmd_span_copy);
        addlookupexp(72, 'D', commands::cmd_span_define);
        addlookupexp(73, 'E', commands::cmd_span_execute_no_recompile);
        addlookupexp(74, 'J', commands::cmd_span_jump);
        addlookupexp(75, 'M', commands::cmd_span_transfer);
        addlookupexp(76, 'R', commands::cmd_span_compile);
        addlookupexp(77, 'T', commands::cmd_span_index);
        addlookupexp(78, 'X', commands::cmd_span_execute);

        // T prefix }    {79}
        addlookupexp(79, 'B', commands::cmd_split_line);
        addlookupexp(80, 'C', commands::cmd_prefix_tc);
        addlookupexp(81, 'F', commands::cmd_prefix_tf);
        addlookupexp(82, 'I', commands::cmd_insert_text);
        addlookupexp(83, 'N', commands::cmd_insert_invisible);
        addlookupexp(84, 'O', commands::cmd_overtype_text);
        addlookupexp(85, 'R', commands::cmd_noop);
        addlookupexp(86, 'S', commands::cmd_swap_line);
        addlookupexp(87, 'X', commands::cmd_execute_string);

        // TC prefix }   {88}
        addlookupexp(88, 'E', commands::cmd_case_edit);
        addlookupexp(89, 'L', commands::cmd_case_low);
        addlookupexp(90, 'U', commands::cmd_case_up);

        // TF prefix }   {91}
        addlookupexp(91, 'C', commands::cmd_line_centre);
        addlookupexp(92, 'F', commands::cmd_line_fill);
        addlookupexp(93, 'J', commands::cmd_line_justify);
        addlookupexp(94, 'L', commands::cmd_line_left);
        addlookupexp(95, 'R', commands::cmd_line_right);
        addlookupexp(96, 'S', commands::cmd_line_squash);

        // U prefix - user keyboard mappings }   {97}
        addlookupexp(97, 'C', commands::cmd_user_command_introducer);

        // W prefix - window commands }  {98}
        addlookupexp(98, 'B', commands::cmd_window_backward);
        addlookupexp(99, 'C', commands::cmd_window_middle);
        addlookupexp(100, 'E', commands::cmd_window_end);
        addlookupexp(101, 'F', commands::cmd_window_forward);
        addlookupexp(102, 'H', commands::cmd_window_setheight);
        addlookupexp(103, 'L', commands::cmd_window_left);
        addlookupexp(104, 'M', commands::cmd_window_scroll);
        addlookupexp(105, 'N', commands::cmd_window_new);
        addlookupexp(106, 'O', commands::cmd_noop);
        addlookupexp(107, 'R', commands::cmd_window_right);
        addlookupexp(108, 'S', commands::cmd_noop);
        addlookupexp(109, 'T', commands::cmd_window_top);
        addlookupexp(110, 'U', commands::cmd_window_update);

        // X prefix - exit }             {111}
        addlookupexp(111, 'A', commands::cmd_exit_abort);
        addlookupexp(112, 'F', commands::cmd_exit_fail);
        addlookupexp(113, 'S', commands::cmd_exit_success);

        // Y prefix }        {114}
        // There aren't any in this table! }

        // Z prefix }        {114}
        // There aren't any in this table! }

        // ~ prefix - miscellaneous debugging commands}  {114}
        addlookupexp(114, 'D', commands::cmd_dump);
        addlookupexp(115, 'V', commands::cmd_validate);

        // sentinel }                    {116}
        addlookupexp(116, '?', commands::cmd_nosuch);

        // initialize lookupexp_ptr }
        // These magic numbers point to the start/end of each section in lookupexp table }
        // (converted to 0-based indices)
        lookupexp_ptr[commands::cmd_prefix_ast] = {0, 0};
        lookupexp_ptr[commands::cmd_prefix_a] = {0, 7};
        lookupexp_ptr[commands::cmd_prefix_b] = {7, 14};
        lookupexp_ptr[commands::cmd_prefix_c] = {14, 16};
        lookupexp_ptr[commands::cmd_prefix_d] = {16, 21};
        lookupexp_ptr[commands::cmd_prefix_e] = {21, 27};
        lookupexp_ptr[commands::cmd_prefix_eo] = {27, 30};
        lookupexp_ptr[commands::cmd_prefix_eq] = {30, 34};
        lookupexp_ptr[commands::cmd_prefix_f] = {34, 45};
        lookupexp_ptr[commands::cmd_prefix_fg] = {45, 51};
        lookupexp_ptr[commands::cmd_prefix_i] = {51, 51};
        lookupexp_ptr[commands::cmd_prefix_k] = {51, 63};
        lookupexp_ptr[commands::cmd_prefix_l] = {63, 65};
        lookupexp_ptr[commands::cmd_prefix_o] = {65, 68};
        lookupexp_ptr[commands::cmd_prefix_p] = {68, 70};
        lookupexp_ptr[commands::cmd_prefix_s] = {70, 79};
        lookupexp_ptr[commands::cmd_prefix_t] = {79, 88};
        lookupexp_ptr[commands::cmd_prefix_tc] = {88, 91};
        lookupexp_ptr[commands::cmd_prefix_tf] = {91, 97};
        lookupexp_ptr[commands::cmd_prefix_u] = {97, 98};
        lookupexp_ptr[commands::cmd_prefix_w] = {98, 111};
        lookupexp_ptr[commands::cmd_prefix_x] = {111, 114};
        lookupexp_ptr[commands::cmd_prefix_y] = {114, 114};
        lookupexp_ptr[commands::cmd_prefix_z] = {114, 114};
        lookupexp_ptr[commands::cmd_prefix_tilde] = {114, 116};
        lookupexp_ptr[commands::cmd_nosuch] = {116, 117};
    }
}

void value_initializations() {
    setup_initial_values();
    initialize_command_table_part1();
    initialize_command_table_part2();
}
