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
    code_list->code = 1;
    code_list->len = 0;
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
        // case change command ; command =  * prefix }      {start at 1}
        addlookupexp(1, 'U', commands::cmd_case_up);
        addlookupexp(2, 'L', commands::cmd_case_low);
        addlookupexp(3, 'E', commands::cmd_case_edit);

        // A prefix }    {4}
        // There aren't any in this table! }

        // B prefix }    {4}
        addlookupexp(4, 'R', commands::cmd_bridge);

        // C prefix }    {5}
        // There aren't any in this table! }

        // D prefix }    {5}
        // There aren't any in this table! }

        // E prefix }    {5}
        addlookupexp(5, 'X', commands::cmd_span_execute);
        addlookupexp(6, 'D', commands::cmd_frame_edit);
        addlookupexp(7, 'R', commands::cmd_frame_return);
        addlookupexp(8, 'N', commands::cmd_span_execute_no_recompile);
        addlookupexp(9, 'Q', commands::cmd_prefix_eq);
        addlookupexp(10, 'O', commands::cmd_prefix_eo);
        addlookupexp(11, 'K', commands::cmd_frame_kill);
        addlookupexp(12, 'P', commands::cmd_frame_parameters);

        // EO prefix }   {13}
        addlookupexp(13, 'L', commands::cmd_equal_eol);
        addlookupexp(14, 'F', commands::cmd_equal_eof);
        addlookupexp(15, 'P', commands::cmd_equal_eop);

        // EQ prefix }   {16}
        addlookupexp(16, 'S', commands::cmd_equal_string);
        addlookupexp(17, 'C', commands::cmd_equal_column);
        addlookupexp(18, 'M', commands::cmd_equal_mark);

        // F prefix - files }    {19}
        addlookupexp(19, 'S', commands::cmd_file_save);
        addlookupexp(20, 'B', commands::cmd_file_rewind);
        addlookupexp(21, 'I', commands::cmd_file_input);
        addlookupexp(22, 'E', commands::cmd_file_edit);
        addlookupexp(23, 'O', commands::cmd_file_output);
        addlookupexp(24, 'G', commands::cmd_prefix_fg);
        addlookupexp(25, 'K', commands::cmd_file_kill);
        addlookupexp(26, 'X', commands::cmd_file_execute);
        addlookupexp(27, 'T', commands::cmd_file_table);
        addlookupexp(28, 'P', commands::cmd_page);

        // FG prefix - global files }    {29}
        addlookupexp(29, 'I', commands::cmd_file_global_input);
        addlookupexp(30, 'O', commands::cmd_file_global_output);
        addlookupexp(31, 'B', commands::cmd_file_global_rewind);
        addlookupexp(32, 'K', commands::cmd_file_global_kill);
        addlookupexp(33, 'R', commands::cmd_file_read);
        addlookupexp(34, 'W', commands::cmd_file_write);

        // I prefix }    {35}
        // There aren't any in this table! }

        // K prefix }    {35}
        // There aren't any in this table! }

        // L prefix }    {35}
        // There aren't any in this table! }

        // O prefix }    {35}
        // There aren't any in this table! }

        // P prefix }    {35}
        // There aren't any in this table! }

        // S prefix - mainly spans }     {35}
        addlookupexp(35, 'A', commands::cmd_span_assign);
        addlookupexp(36, 'C', commands::cmd_span_copy);
        addlookupexp(37, 'D', commands::cmd_span_define);
        addlookupexp(38, 'T', commands::cmd_span_transfer);
        addlookupexp(39, 'W', commands::cmd_swap_line);
        addlookupexp(40, 'L', commands::cmd_split_line);
        addlookupexp(41, 'J', commands::cmd_span_jump);
        addlookupexp(42, 'I', commands::cmd_span_index);
        addlookupexp(43, 'R', commands::cmd_span_compile);

        // T prefix }    {44}
        // There aren't any in this table! }

        // TC prefix }    {44}
        // There aren't any in this table! }

        // TF prefix }    {44}
        // There aren't any in this table! }

        // U prefix - user keyboard mappings }   {44}
        addlookupexp(44, 'C', commands::cmd_user_command_introducer);
        addlookupexp(45, 'K', commands::cmd_user_key);
        addlookupexp(46, 'P', commands::cmd_user_parent);
        addlookupexp(47, 'S', commands::cmd_user_subprocess);

        // W prefix - window commands }  {48}
        addlookupexp(48, 'F', commands::cmd_window_forward);
        addlookupexp(49, 'B', commands::cmd_window_backward);
        addlookupexp(50, 'M', commands::cmd_window_middle);
        addlookupexp(51, 'T', commands::cmd_window_top);
        addlookupexp(52, 'E', commands::cmd_window_end);
        addlookupexp(53, 'N', commands::cmd_window_new);
        addlookupexp(54, 'R', commands::cmd_window_right);
        addlookupexp(55, 'L', commands::cmd_window_left);
        addlookupexp(56, 'H', commands::cmd_window_setheight);
        addlookupexp(57, 'S', commands::cmd_window_scroll);
        addlookupexp(58, 'U', commands::cmd_window_update);

        // X prefix - exit }             {59}
        addlookupexp(59, 'S', commands::cmd_exit_success);
        addlookupexp(60, 'F', commands::cmd_exit_fail);
        addlookupexp(61, 'A', commands::cmd_exit_abort);

        // Y prefix - word processing }  {62}
        addlookupexp(62, 'F', commands::cmd_line_fill);
        addlookupexp(63, 'J', commands::cmd_line_justify);
        addlookupexp(64, 'S', commands::cmd_line_squash);
        addlookupexp(65, 'C', commands::cmd_line_centre);
        addlookupexp(66, 'L', commands::cmd_line_left);
        addlookupexp(67, 'R', commands::cmd_line_right);
        addlookupexp(68, 'A', commands::cmd_word_advance);
        addlookupexp(69, 'D', commands::cmd_word_delete);

        // Z prefix - cursor commands }  {70}
        addlookupexp(70, 'U', commands::cmd_up);
        addlookupexp(71, 'D', commands::cmd_down);
        addlookupexp(72, 'R', commands::cmd_right);
        addlookupexp(73, 'L', commands::cmd_left);
        addlookupexp(74, 'H', commands::cmd_home);
        addlookupexp(75, 'C', commands::cmd_return);
        addlookupexp(76, 'T', commands::cmd_tab);
        addlookupexp(77, 'B', commands::cmd_backtab);
        addlookupexp(78, 'Z', commands::cmd_rubout);

        // ~ prefix - miscellaneous debugging commands}  {79}
        addlookupexp(79, 'V', commands::cmd_validate);
        addlookupexp(80, 'D', commands::cmd_dump);

        // sentinel }                    {81}
        addlookupexp(81, '?', commands::cmd_nosuch);

        // initialize lookupexp_ptr }
        // These magic numbers point to the start of each section in lookupexp table }
        lookupexp_ptr[commands::cmd_prefix_ast] = 1;
        lookupexp_ptr[commands::cmd_prefix_a] = 4;
        lookupexp_ptr[commands::cmd_prefix_b] = 4;
        lookupexp_ptr[commands::cmd_prefix_c] = 5;
        lookupexp_ptr[commands::cmd_prefix_d] = 5;
        lookupexp_ptr[commands::cmd_prefix_e] = 5;
        lookupexp_ptr[commands::cmd_prefix_eo] = 13;
        lookupexp_ptr[commands::cmd_prefix_eq] = 16;
        lookupexp_ptr[commands::cmd_prefix_f] = 19;
        lookupexp_ptr[commands::cmd_prefix_fg] = 29;
        lookupexp_ptr[commands::cmd_prefix_i] = 35;
        lookupexp_ptr[commands::cmd_prefix_k] = 35;
        lookupexp_ptr[commands::cmd_prefix_l] = 35;
        lookupexp_ptr[commands::cmd_prefix_o] = 35;
        lookupexp_ptr[commands::cmd_prefix_p] = 35;
        lookupexp_ptr[commands::cmd_prefix_s] = 35;
        lookupexp_ptr[commands::cmd_prefix_t] = 44;
        lookupexp_ptr[commands::cmd_prefix_tc] = 44;
        lookupexp_ptr[commands::cmd_prefix_tf] = 44;
        lookupexp_ptr[commands::cmd_prefix_u] = 44;
        lookupexp_ptr[commands::cmd_prefix_w] = 48;
        lookupexp_ptr[commands::cmd_prefix_x] = 59;
        lookupexp_ptr[commands::cmd_prefix_y] = 62;
        lookupexp_ptr[commands::cmd_prefix_z] = 70;
        lookupexp_ptr[commands::cmd_prefix_tilde] = 79;
        lookupexp_ptr[commands::cmd_nosuch] = 81;
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
        // Ast ( * ) prefix } {start at 1}
        // There aren't any in this table!

        // A prefix }    {start at 1}
        addlookupexp(1, 'C', commands::cmd_jump);
        addlookupexp(2, 'L', commands::cmd_advance);
        addlookupexp(3, 'O', commands::cmd_bridge);
        addlookupexp(4, 'P', commands::cmd_advance_paragraph);
        addlookupexp(5, 'S', commands::cmd_noop);
        addlookupexp(6, 'T', commands::cmd_next);
        addlookupexp(7, 'W', commands::cmd_word_advance);

        // B prefix }    {8}
        addlookupexp(8, 'B', commands::cmd_noop);
        addlookupexp(9, 'C', commands::cmd_noop);  // cmd_block_copy
        addlookupexp(10, 'D', commands::cmd_noop); // cmd_block_define
        addlookupexp(11, 'I', commands::cmd_noop);
        addlookupexp(12, 'K', commands::cmd_noop);
        addlookupexp(13, 'M', commands::cmd_noop); // cmd_block_transfer
        addlookupexp(14, 'O', commands::cmd_noop);

        // C prefix }    {15}
        addlookupexp(15, 'C', commands::cmd_insert_char);
        addlookupexp(16, 'L', commands::cmd_insert_line);

        // D prefix }    {17}
        addlookupexp(17, 'C', commands::cmd_delete_char);
        addlookupexp(18, 'L', commands::cmd_delete_line);
        addlookupexp(19, 'P', commands::cmd_delete_paragraph);
        addlookupexp(20, 'S', commands::cmd_noop);
        addlookupexp(21, 'W', commands::cmd_word_delete);

        // E prefix }    {22}
        addlookupexp(22, 'D', commands::cmd_frame_edit);
        addlookupexp(23, 'K', commands::cmd_frame_kill);
        addlookupexp(24, 'O', commands::cmd_prefix_eo);
        addlookupexp(25, 'P', commands::cmd_frame_parameters);
        addlookupexp(26, 'Q', commands::cmd_prefix_eq);
        addlookupexp(27, 'R', commands::cmd_frame_return);

        // EO prefix }   {28}
        addlookupexp(28, 'L', commands::cmd_equal_eol);
        addlookupexp(29, 'F', commands::cmd_equal_eof);
        addlookupexp(30, 'P', commands::cmd_equal_eop);

        // EQ prefix }   {31}
        addlookupexp(31, 'C', commands::cmd_equal_column);
        addlookupexp(32, 'L', commands::cmd_noop);
        addlookupexp(33, 'M', commands::cmd_equal_mark);
        addlookupexp(34, 'S', commands::cmd_equal_string);

        // F prefix - files }    {35}
        addlookupexp(35, 'S', commands::cmd_file_save);
        addlookupexp(36, 'B', commands::cmd_file_rewind);
        addlookupexp(37, 'E', commands::cmd_file_edit);
        addlookupexp(38, 'G', commands::cmd_prefix_fg);
        addlookupexp(39, 'I', commands::cmd_file_input);
        addlookupexp(40, 'K', commands::cmd_file_kill);
        addlookupexp(41, 'O', commands::cmd_file_output);
        addlookupexp(42, 'P', commands::cmd_page);
        addlookupexp(43, 'S', commands::cmd_noop);
        addlookupexp(44, 'T', commands::cmd_file_table);
        addlookupexp(45, 'X', commands::cmd_file_execute);

        // FG prefix - global files }    {46}
        addlookupexp(46, 'B', commands::cmd_file_global_rewind);
        addlookupexp(47, 'I', commands::cmd_file_global_input);
        addlookupexp(48, 'K', commands::cmd_file_global_kill);
        addlookupexp(49, 'O', commands::cmd_file_global_output);
        addlookupexp(50, 'R', commands::cmd_file_read);
        addlookupexp(51, 'W', commands::cmd_file_write);

        // I prefix }    {52}
        // There aren't any yet! }

        // K prefix }    {52}
        addlookupexp(52, 'B', commands::cmd_backtab);
        addlookupexp(53, 'C', commands::cmd_return);
        addlookupexp(54, 'D', commands::cmd_down);
        addlookupexp(55, 'H', commands::cmd_home);
        addlookupexp(56, 'I', commands::cmd_insert_mode);
        addlookupexp(57, 'L', commands::cmd_left);
        addlookupexp(58, 'M', commands::cmd_user_key);
        addlookupexp(59, 'O', commands::cmd_overtype_mode);
        addlookupexp(60, 'R', commands::cmd_right);
        addlookupexp(61, 'T', commands::cmd_tab);
        addlookupexp(62, 'U', commands::cmd_up);
        addlookupexp(63, 'X', commands::cmd_rubout);

        // L prefix }    {64}
        addlookupexp(64, 'R', commands::cmd_noop);
        addlookupexp(65, 'S', commands::cmd_noop);

        // O prefix }    {66}
        addlookupexp(66, 'P', commands::cmd_user_parent);
        addlookupexp(67, 'S', commands::cmd_user_subprocess);
        addlookupexp(68, 'X', commands::cmd_op_sys_command);

        // P prefix }    {69}
        addlookupexp(69, 'C', commands::cmd_position_column);
        addlookupexp(70, 'L', commands::cmd_position_line);

        // S prefix }    {71}
        addlookupexp(71, 'A', commands::cmd_span_assign);
        addlookupexp(72, 'C', commands::cmd_span_copy);
        addlookupexp(73, 'D', commands::cmd_span_define);
        addlookupexp(74, 'E', commands::cmd_span_execute_no_recompile);
        addlookupexp(75, 'J', commands::cmd_span_jump);
        addlookupexp(76, 'M', commands::cmd_span_transfer);
        addlookupexp(77, 'R', commands::cmd_span_compile);
        addlookupexp(78, 'T', commands::cmd_span_index);
        addlookupexp(79, 'X', commands::cmd_span_execute);

        // T prefix }    {80}
        addlookupexp(80, 'B', commands::cmd_split_line);
        addlookupexp(81, 'C', commands::cmd_prefix_tc);
        addlookupexp(82, 'F', commands::cmd_prefix_tf);
        addlookupexp(83, 'I', commands::cmd_insert_text);
        addlookupexp(84, 'N', commands::cmd_insert_invisible);
        addlookupexp(85, 'O', commands::cmd_overtype_text);
        addlookupexp(86, 'R', commands::cmd_noop);
        addlookupexp(87, 'S', commands::cmd_swap_line);
        addlookupexp(88, 'X', commands::cmd_execute_string);

        // TC prefix }   {89}
        addlookupexp(89, 'E', commands::cmd_case_edit);
        addlookupexp(90, 'L', commands::cmd_case_low);
        addlookupexp(91, 'U', commands::cmd_case_up);

        // TF prefix }   {92}
        addlookupexp(92, 'C', commands::cmd_line_centre);
        addlookupexp(93, 'F', commands::cmd_line_fill);
        addlookupexp(94, 'J', commands::cmd_line_justify);
        addlookupexp(95, 'L', commands::cmd_line_left);
        addlookupexp(96, 'R', commands::cmd_line_right);
        addlookupexp(97, 'S', commands::cmd_line_squash);

        // U prefix - user keyboard mappings }   {98}
        addlookupexp(98, 'C', commands::cmd_user_command_introducer);

        // W prefix - window commands }  {99}
        addlookupexp(99, 'B', commands::cmd_window_backward);
        addlookupexp(100, 'C', commands::cmd_window_middle);
        addlookupexp(101, 'E', commands::cmd_window_end);
        addlookupexp(102, 'F', commands::cmd_window_forward);
        addlookupexp(103, 'H', commands::cmd_window_setheight);
        addlookupexp(104, 'L', commands::cmd_window_left);
        addlookupexp(105, 'M', commands::cmd_window_scroll);
        addlookupexp(106, 'N', commands::cmd_window_new);
        addlookupexp(107, 'O', commands::cmd_noop);
        addlookupexp(108, 'R', commands::cmd_window_right);
        addlookupexp(109, 'S', commands::cmd_noop);
        addlookupexp(110, 'T', commands::cmd_window_top);
        addlookupexp(111, 'U', commands::cmd_window_update);

        // X prefix - exit }             {112}
        addlookupexp(112, 'A', commands::cmd_exit_abort);
        addlookupexp(113, 'F', commands::cmd_exit_fail);
        addlookupexp(114, 'S', commands::cmd_exit_success);

        // Y prefix }        {115}
        // There aren't any in this table! }

        // Z prefix }        {115}
        // There aren't any in this table! }

        // ~ prefix - miscellaneous debugging commands}  {115}
        addlookupexp(115, 'D', commands::cmd_dump);
        addlookupexp(116, 'V', commands::cmd_validate);

        // sentinel }                    {117}
        addlookupexp(117, '?', commands::cmd_nosuch);

        // initialize lookupexp_ptr }
        // These magic numbers point to the start of each section in lookupexp table }
        lookupexp_ptr[commands::cmd_prefix_ast] = 1;
        lookupexp_ptr[commands::cmd_prefix_a] = 1;
        lookupexp_ptr[commands::cmd_prefix_b] = 8;
        lookupexp_ptr[commands::cmd_prefix_c] = 15;
        lookupexp_ptr[commands::cmd_prefix_d] = 17;
        lookupexp_ptr[commands::cmd_prefix_e] = 22;
        lookupexp_ptr[commands::cmd_prefix_eo] = 28;
        lookupexp_ptr[commands::cmd_prefix_eq] = 31;
        lookupexp_ptr[commands::cmd_prefix_f] = 35;
        lookupexp_ptr[commands::cmd_prefix_fg] = 46;
        lookupexp_ptr[commands::cmd_prefix_i] = 52;
        lookupexp_ptr[commands::cmd_prefix_k] = 52;
        lookupexp_ptr[commands::cmd_prefix_l] = 64;
        lookupexp_ptr[commands::cmd_prefix_o] = 66;
        lookupexp_ptr[commands::cmd_prefix_p] = 69;
        lookupexp_ptr[commands::cmd_prefix_s] = 71;
        lookupexp_ptr[commands::cmd_prefix_t] = 80;
        lookupexp_ptr[commands::cmd_prefix_tc] = 89;
        lookupexp_ptr[commands::cmd_prefix_tf] = 92;
        lookupexp_ptr[commands::cmd_prefix_u] = 98;
        lookupexp_ptr[commands::cmd_prefix_w] = 99;
        lookupexp_ptr[commands::cmd_prefix_x] = 112;
        lookupexp_ptr[commands::cmd_prefix_y] = 115;
        lookupexp_ptr[commands::cmd_prefix_z] = 115;
        lookupexp_ptr[commands::cmd_prefix_tilde] = 115;
        lookupexp_ptr[commands::cmd_nosuch] = 117;
    }
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
    if (!file_create_open(command_line, parse_type::parse_command, files[1], files[2]))
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
    frame_oops->options.add(frame_options_elts::opt_special_frame);
    if (!frame_edit(frame_name_cmd))
        goto l99;
    frame_cmd = current_frame;
    current_frame = nullptr;
    frame_cmd->options.add(frame_options_elts::opt_special_frame);
    if (!frame_edit(frame_name_heap))
        goto l99;
    frame_heap = current_frame;
    current_frame = nullptr;
    frame_heap->options.add(frame_options_elts::opt_special_frame);
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
    if (files[1] != nullptr) {
        current_frame->input_file = 1;
        files_frames[1] = current_frame;
    }
    if (files[2] != nullptr) {
        current_frame->output_file = 2;
        files_frames[2] = current_frame;
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
