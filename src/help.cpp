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
! Name:         HELP
!
! Description:  Ludwig HELP facility.
!**/

#include "help.h"

#include "helpfile.h"
#include "screen.h"
#include "var.h"

#include <algorithm>

namespace {

    inline constexpr std::string_view INDEX{"0"}; // The index key.

    unsigned char to_upper(unsigned char c) {
        return std::toupper(c);
    }

}; // namespace

std::string ask_user(const std::string_view &prompt) {
    screen_writeln();
    auto reply = screen_help_prompt(prompt);
    // Note that all characters not overwritten by the user will be spaces!
    screen_writeln();
    std::ranges::transform(reply, reply.begin(), to_upper);
    return reply;
}

void help_help(const std::string &selection) {
    // The argument selects a particular part of the help file to read e.g. SD
    screen_unload();
    screen_home(true);
    std::string topic = selection.empty() ? std::string(INDEX) : selection;
    if (!helpfile_open(file_data.old_cmds)) {
        screen_write_str(3, "Can't open HELP file", 20);
        screen_writeln();
        screen_pause();
        topic.clear();
    }
    while (!topic.empty()) {
        screen_home(true);
        help_record buf;
        bool continu = helpfile_read(topic, buf);
        if (!continu) {
            screen_write_str(3, "Can't find Command or Section in HELP", 37);
            screen_writeln();
            topic.clear();
        }
        while (continu) {
            if ((buf.txt[0] == '\\') && (buf.txt[1] == '%')) {
                std::string reply = ask_user("<space> for more, <return> to exit : ");
                if (tt_controlc)
                    reply.clear();
                if (reply.empty() || reply[0] != ' ') {
                    continu = false;
                    topic = reply;
                }
            } else {
                screen_write_str(2, buf.txt.data(), buf.txt.size());
                screen_writeln();
            }
            if (continu) {
                continu = helpfile_next(buf) && buf.key == topic;
                if (!continu) {
                    topic.clear();
                }
            }
            if (tt_controlc) {
                continu = false;
            }
        }
        if (topic.empty() || topic[0] == ' ') {
            topic = ask_user("Command or Section or <return> to exit : ");
            if (!topic.empty() && (topic[0] == ' ')) {
                topic = INDEX;
            }
            if (tt_controlc)
                topic.clear();
        }
    }
}
