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
! Name:        MSDOS
!
! Implementation O/S support routines originally for MS-DOS, now for
! Linux / Unix.
*/

#include "msdos.h"

#include <string>
#include <cstring>
#include <algorithm>

#include <sys/types.h>
#include <signal.h>

namespace {
    template <class R>
    std::string to_string(const parray<char, R> &strng) {
        return std::string(strng.data(), strng.length(' '));
    }
}

bool fpc_suspend() {
    return ::kill(0, SIGTSTP) == 0;
}

bool fpc_shell() {
    // FIXME: Should really make this work
    return false;
}

void msdos_exit(int status) {
    exit(status);
}

bool cvt_int_str(int num, str_object &strng, scr_col_range width) {
    std::string s = std::to_string(num);

    // FIXME: Not sure if we should throw exception
    // if s.size() > width ?
    // Check for negative, as we do an unsigned/signed compare next.
    if (width < 0)
        return false;
    if (s.size() > static_cast<size_t>(width))
        return false;

    strng.fill(' ');
    size_t to_copy = std::min(size_t(width), s.size());
    size_t offset = 1 + width - to_copy;
    for (size_t i = 0; i < to_copy; ++i) {
        strng[offset + i] = s[i];
    }
    strng[offset + to_copy] = '\0'; // NUL terminated
    return true;
}

bool cvt_str_int(int &num, const str_object &strng) {
    std::string s = to_string(strng);
    while (!s.empty() && std::isspace(s[0])) {
        s.erase(0, 1);
    }
    size_t converted = 0;
    num = std::stoi(s, &converted);
    return converted > 0;
}

bool get_environment(const std::string &environ, strlen_range &reslen, str_object &result) {
    char *env = ::getenv(environ.c_str());
    if (env == NULL)
        return false;
    size_t len = std::strlen(env);
    reslen = len;
    result.copy(env, len);
    return true;
}

void init_signals() {
    // Nothing yet
}

void exit_handler(int sig) {
    // Nothing yet
}
