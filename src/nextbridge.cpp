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
! Name:         NEXTBRIDGE
!
! Description:  The NEXT and BRIDGE commands.
!**/

#include "nextbridge.h"

#include "var.h"
#include "mark.h"

#include <unordered_set>

namespace {
    using chset_range = prange<0, ORD_MAXCHAR>;
    using chset = prangeset<chset_range>;
};

bool nextbridge_command(int count, const tpar_object &tpar, bool bridge) {
    chset chars;
    // Form the character set.
    //with tpar do
    int i = 1;
    while (i <= tpar.len) {
        unsigned char ch1 = tpar.str[i];
        unsigned char ch2 = ch1;
        i += 1;
        if (i + 2 <= tpar.len) {
            if ((tpar.str[i] == '.') && (tpar.str[i + 1] == '.')) {
                ch2 = tpar.str[i + 2];
                i += 3;
            }
        }
        chars.add_range(ch1, ch2);
    }
    if (bridge) {
        chset old(chars);
        chars.add_range(0, ORD_MAXCHAR);
        chars.remove(old);
    }
    // Search for a character in the set.
    //with current_frame^ do
    line_ptr new_line = current_frame->dot->line;
    int new_col;
    if (count > 0) {
        new_col = current_frame->dot->col;
        if (!bridge)
            new_col += 1;
        do {
            while (new_line != nullptr) {
                //with new_line^ do
                i = new_col;
                while (i <= new_line->used) {
                    if (chars.contains(new_line->str->operator[](i))) {
                        new_col = i;
                        goto l1;
                    }
                    i += 1;
                }
                if (chars.contains(' ') && (i == new_line->used + 1)) { // Match a space at EOL.
                    new_col = i;
                    goto l1;
                }
                new_line = new_line->flink;
                new_col = 1;
            }
            return false;
    l1:;
            new_col += 1;
            count -= 1;
        } while (count != 0);
        new_col -= 1;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]))
            return false;
    } else if (count < 0) {
        new_col = current_frame->dot->col - 1;
        if (!bridge)
            new_col -= 1;
        do {
            while (new_line != nullptr) {
                //with new_line^ do
                if (new_line->used < new_col) {
                    if (chars.contains(' '))
                        goto l2;
                    new_col = new_line->used;
                }
                for (int j = new_col; j >= 1; --j) {
                    if (chars.contains(new_line->str->operator[](j))) {
                        new_col = j;
                        goto l2;
                    }
                }
                if (new_line->blink != nullptr) {
                    new_line = new_line->blink;
                    new_col = new_line->used + 1;
                } else if (bridge)
                    goto l2; // This is safe since only -1BR is allowed
                else
                    return false;
            }
    l2:;
            new_col -= 1;
            count += 1;
        } while (count != 0);
        new_col += 2;
        if (!mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]))
            return false;
    } else {
        return mark_create(current_frame->dot->line, current_frame->dot->col, current_frame->marks[MARK_EQUALS]);
    }

    // Found it, move dot to new point.
    return mark_create(new_line, new_col, current_frame->dot);
}
