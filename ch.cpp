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
! Name:         CH
!
! Description:  Character array handling routines.
*/

#include "ch.h"

namespace {

    template <typename T>
    int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

};

// FIXME: This could probably be improved.
int ch_compare_str(const str_object &target, strlen_range st1, strlen_range len1,
                   const str_object &text,   strlen_range st2, strlen_range len2,
                   bool exactcase, strlen_range &nch_ident) {
    int i;
    if (exactcase) {
        for (i = 0; i < len1 && i < len2; ++i) {
            if (target[st1 + i] != std::toupper(text[st2 + i]))
                break;
        }
    } else {
        for (i = 0; i < len1 && i < len2; ++i) {
            if (target[st1 + i] != text[st2 + i])
                break;
        }
    }
    nch_ident = i;
    int diff = 0;
    if (i < len1 && i < len2) {
        char ch1 = target[st1 + i];
        char ch2 = exactcase ? text[st2 + i] : std::toupper(text[st2 + i]);
        diff = sgn(ch1 - ch2);
    } else {
        diff = sgn(len1 - len2);
    }
    return diff;
}

void ch_reverse_str(const str_object &src, str_object &dst, strlen_range len) {
    std::reverse_copy(src.data(), src.data() + len, dst.data());
}

char ch_toupper(char ch) {
    return std::toupper(ch);
}

// FIXME: This could probably be improved.
bool ch_search_str(const str_object &target, strlen_range st1, strlen_range len1,
                   const str_object &text,   strlen_range st2, strlen_range len2,
                   bool exactcase, bool backwards,
                   strlen_range &found_loc) {
    str_object s;
    s.copy(text.data(st2), len2);
    if (backwards) {
        std::reverse(s.data(), s.data() + len2);
        found_loc = len2;
    } else {
        found_loc = 0;
    }
    if (!exactcase)
        s.apply_n(ch_toupper, len2);
    for (int i = 1; i <= len2 - len1 + 1; ++i) {
        if (std::equal(s.data(i), s.data(i) + len1, target.data(st1))) {
            if (backwards)
                found_loc = len2 - (i + len1) + 1;
            else
                found_loc = i - 1;
            return true;
        }
    }
    return false;
}
