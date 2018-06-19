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
{   This file written by Martin Sandiford                              }
{                                                                      }
{**********************************************************************/

/**
! Name:        SYS_LINUX
!
! Implementation O/S support routines for Linux, likely to work OK on
! other Unix-like systems also.
*/

#include "sys.h"

#include <signal.h>
#include <unistd.h>

namespace {
    void do_exit(int status) {
        // Here would be the spot to tear-down what sys_initsig did.
        exit(status);
    }
};

bool sys_suspend() {
    return ::kill(0, SIGTSTP) == 0;
}

bool sys_shell() {
    // FIXME: Should really make this work
    return false;
}

bool sys_istty() {
    return isatty(0) && isatty(1);
}

bool sys_getenv(const std::string &environ, std::string &result) {
    char *env = ::getenv(environ.c_str());
    if (env == NULL)
        return false;
    result = env;
    return true;
}

void sys_initsig() {
    // Nothing yet
}

void sys_exit_success() {
    do_exit(EXIT_SUCCESS);
}

void sys_exit_failure() {
    do_exit(EXIT_FAILURE);
}
