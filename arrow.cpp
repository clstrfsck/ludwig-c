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
! Name:         ARROW
!
! Description:  The arrow key, TAB, and BACKTAB commands.
!
! $Log: arrow.pas,v $
! Revision 4.5  1990/01/18 18:30:47  ludwig
! Entered into RCS at revision level 4.5
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-003 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-004 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-005 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!**/

#include "arrow.h"

#include "var.h"
#include "vdu.h"
#include "line.h"
#include "mark.h"
#include "text.h"
#include "screen.h"

const penumset<commands> ARROW_COMMANDS{
    commands::cmd_return, commands::cmd_home,
        commands::cmd_tab, commands::cmd_backtab,
        commands::cmd_left, commands::cmd_right,
        commands::cmd_down, commands::cmd_up};

bool arrow_command(commands command, leadparam rept, int count, bool from_span) {
    bool cmd_status = false;
    mark_object new_eql;
    //with current_frame^ do
    mark_object old_dot = *(current_frame->dot);
    //with last_group^ do
    line_range eop_line_nr = current_frame->last_group->first_line_nr +
        current_frame->last_group->last_line->offset_nr;

    key_code_range key;
    while (true) {
        bool cmd_valid = false;
        if (ARROW_COMMANDS.contains(command)) {
            switch (command) {
            case commands::cmd_return: {
                cmd_valid = true;
                new_eql = *(current_frame->dot);
                //with dot^ do
                line_ptr  dot_line = current_frame->dot->line;
                col_range dot_col  = current_frame->dot->col;
                for (int counter = 1; counter <= count; ++counter) {
                    if (tt_controlc)
                        goto l9;
                    if (dot_line->flink == nullptr) {
                        if (!text_realize_null(dot_line))
                            goto l9;
                        eop_line_nr = eop_line_nr + 1;
                        dot_line = dot_line->blink;
                        if (counter == 1)
                            new_eql.line = dot_line;
                    }
                    dot_col  = text_return_col(dot_line, dot_col, false);
                    dot_line = dot_line->flink;
                }
                if (!mark_create(dot_line, dot_col, current_frame->dot))
                    goto l9;
            }
                break;

            case commands::cmd_home: {
                cmd_valid = true;
                new_eql = *(current_frame->dot);
                if (current_frame == scr_frame) {
                    if (!mark_create(scr_top_line, current_frame->scr_offset + 1, current_frame->dot))
                        goto l9;
                }
            }
                break;

            case commands::cmd_tab:
            case commands::cmd_backtab: {
                col_width_range new_col = current_frame->dot->col;
                int step;
                if (command == commands::cmd_tab) {
                    if (new_col == MAX_STRLENP)
                        goto l1;
                    step = 1;
                } else {
                    step = -1;
                }
                for (int counter = 1; counter <= count; ++counter) {
                    do {
                        new_col = new_col + step;
                    } while (!current_frame->tab_stops[new_col] &&
                             (new_col != current_frame->margin_left) &&
                             (new_col != current_frame->margin_right));
                    if ((new_col == 0) || (new_col == MAX_STRLENP))
                        goto l1;
                }
                cmd_valid = true;
                new_eql = *(current_frame->dot);
                current_frame->dot->col = new_col;
                l1:;
            }
                break;

            case commands::cmd_left: {
                //with dot^ do
                switch (rept) {
                case leadparam::none:
                case leadparam::plus:
                case leadparam::pint:
                    if (current_frame->dot->col - count >= 1) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        current_frame->dot->col -= count;
                    }
                    break;
		case leadparam::pindef:
                    if (current_frame->dot->col >= current_frame->margin_left) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        current_frame->dot->col = current_frame->margin_left;
                    }
                    break;
                default:
                    // Nothing
                    break;
                }
            }
                break;

            case commands::cmd_right: {
                //with dot^ do
                switch (rept) {
		case leadparam::none:
                case leadparam::plus:
                case leadparam::pint:
                    if (current_frame->dot->col + count <= MAX_STRLENP) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        current_frame->dot->col += count;
                    }
                    break;
		case leadparam::pindef:
                    if (current_frame->dot->col <= current_frame->margin_right) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        current_frame->dot->col = current_frame->margin_right;
                    }
                    break;
                default:
                    // Nothing
                    break;
                }
            }
                break;

            case commands::cmd_down: {
                line_ptr dot_line = current_frame->dot->line;
                line_range line_nr;
                if (!line_to_number(dot_line, line_nr))
                    goto l9;
                switch (rept) {
                case leadparam::none:
                case leadparam::plus:
                case leadparam::pint:
                    if (line_nr + count <= eop_line_nr) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        if (count < MAX_GROUPLINES / 2) {
                            for (int counter = 1; counter <= count; ++counter)
                                dot_line = dot_line->flink;
                        } else {
                            if (!line_from_number(current_frame, line_nr + count, dot_line))
                                goto l9;
                        }
                    }
                    break;
                case leadparam::pindef: {
                    cmd_valid = true;
                    new_eql = *(current_frame->dot);
                    dot_line = current_frame->last_group->last_line;
                }
                    break;
                default:
                    // Nothing
                    break;
                }
                if (!mark_create(dot_line, current_frame->dot->col, current_frame->dot))
                    goto l9;
            }
                break;

            case commands::cmd_up: {
                line_ptr dot_line = current_frame->dot->line;
                line_range line_nr;
                if (!line_to_number(dot_line, line_nr))
                    goto l9;
                switch (rept) {
                case leadparam::none:
                case leadparam::plus:
                case leadparam::pint:
                    if (line_nr - count > 0) {
                        cmd_valid = true;
                        new_eql = *(current_frame->dot);
                        if (count < MAX_GROUPLINES / 2) {
                            for (int counter = 1; counter <= count; ++counter)
                                dot_line = dot_line->blink;
                        } else {
                            if (!line_from_number(current_frame, line_nr - count, dot_line))
                                goto l9;
                        }
                    }
                    break;
                case leadparam::pindef: {
                    cmd_valid = true;
                    new_eql = *(current_frame->dot);
                    dot_line = current_frame->first_group->first_line;
                }
                    break;
                default:
                    // Nothing
                    break;
                }
                if (!mark_create(dot_line, current_frame->dot->col, current_frame->dot))
                    goto l9;
            }
                break;
            default:
                // Apparently there are 143 ignored enumeration values,
                // according to clang++
                break;
            }
        } else {
            vdu_take_back_key(key);
            goto l9;
        }

        if (cmd_valid)
            cmd_status = true;
        if (from_span)
            goto l9;
        screen_fixup();
        if (!cmd_valid ||
            ((command == commands::cmd_down) && (rept != leadparam::pindef) &&
             (current_frame->dot->line->flink == nullptr)))
            vdu_beep();
        key = vdu_get_key();
        if (tt_controlc)
            goto l9;
        rept    = leadparam::none;
        count   = 1;
        command = lookup[key].command;
        if ((command == commands::cmd_return) && (edit_mode == mode_type::mode_insert))
            command = commands::cmd_split_line;
    }

 l9:;
    if (tt_controlc) {
        mark_create(old_dot.line, old_dot.col, current_frame->dot);
    } else {
        // Define Equals.
        if (cmd_status) {
            mark_create(new_eql.line, new_eql.col, current_frame->marks[MARK_EQUALS]);
            if ((command == commands::cmd_down) &&
                (rept != leadparam::pindef) && (current_frame->dot->line->flink == nullptr))
                cmd_status = false;
        }
    }
    return cmd_status || !from_span;
}
