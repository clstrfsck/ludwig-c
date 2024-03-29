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
! Name:         SWAP
!
! Description:  Swap Line command.
!
! $Log: swap.pas,v $
! Revision 4.5  1990/01/18 17:31:21  ludwig
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

#include "swap.h"

#include "var.h"
#include "mark.h"
#include "text.h"

bool swap_line(leadparam rept, int count) {
    // SW is implemented as a ST of the dot line to before the other line.
    bool result = false;
    mark_ptr  top_mark  = nullptr;
    mark_ptr  end_mark  = nullptr;
    mark_ptr  dest_mark = nullptr;
    //with current_frame^ do
    line_ptr  this_line = current_frame->dot->line;
    col_range dot_col   = current_frame->dot->col;
    line_ptr  next_line = this_line->flink;
    line_ptr  dest_line;
    if (next_line == nullptr)
        goto l99;
    switch (rept) {
    case leadparam::none:
    case leadparam::plus:
    case leadparam::pint:
        dest_line = next_line;
        for (int i = 1; i <= count; ++i) {
            dest_line = dest_line->flink;
            if (dest_line == nullptr)
                goto l99;
        }
        break;
    case leadparam::minus:
    case leadparam::nint:
        dest_line = this_line;
        for (int i = -1; i >= count; --i) {
            dest_line = dest_line->blink;
            if (dest_line == nullptr)
                goto l99;
        }
        break;
    case leadparam::pindef:
        dest_line = current_frame->last_group->last_line;
        break;
    case leadparam::nindef:
        dest_line = current_frame->first_group->first_line;
        break;
    case leadparam::marker:
        dest_line = current_frame->marks[count]->line;
        break;
    case leadparam::last_entry:
        // Ignore - used to support sets etc
        break;
    }
    if (!mark_create(this_line, 1, top_mark))
        goto l99;
    if (!mark_create(next_line, 1, end_mark))
        goto l99;
    if (!mark_create(dest_line, 1, dest_mark))
        goto l99;
    if (!text_move(false, 1, top_mark, end_mark, dest_mark, current_frame->dot, top_mark))
        goto l99;
    current_frame->text_modified = true;
    current_frame->dot->col = dot_col;
    if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]))
        goto l99;
    result = true;
 l99:;
    if (top_mark != nullptr)
        mark_destroy(top_mark);
    if (end_mark != nullptr)
        mark_destroy(end_mark);
    if (dest_mark != nullptr)
        mark_destroy(dest_mark);
    return result;
}
