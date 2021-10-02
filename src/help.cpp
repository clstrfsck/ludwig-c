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
!
! $Log: help.pas,v $
! Revision 4.6  1990/01/18 18:07:12  ludwig
! Entered into RCS at revision level 4.6
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Kelvin B. Nicolle                                     4-May-1987
!       Change the indent of the displayed help text from 3 to 2.
! 4-003 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-004 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-005 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-006 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!**/

#include "help.h"

#include "ch.h"
#include "var.h"
#include "screen.h"
#include "helpfile.h"

#include <cstring>

namespace {

    const int  STS_RNF     = 98994;  // Record not found status return.
    const char INDEX[]     = "0   "; // The index key.

};

void ask_user(const char *prompt, key_str &reply, int &reply_len) {
    screen_writeln();
    reply.fill(' ');
    write_str pstr;
    scr_col_range plen = std::strlen(prompt);
    pstr.copy_n(prompt, plen);
    screen_help_prompt(pstr, plen, reply, reply_len);
    // Note that all characters not overwritten by the user will be spaces!
    screen_writeln();
    reply.apply_n(ch_toupper, reply_len);
}

void help_help(const std::string &selection) {
    // The argument selects a particular part of the help file to read e.g. SD
    screen_unload();
    screen_home(true);
    key_str topic;
    int topic_len;
    if (selection.empty()) {
        topic.copy_n(INDEX, KEY_LEN);
        topic_len = KEY_LEN;
    } else {
        if (selection.size() > KEY_LEN)
            topic_len = KEY_LEN;
        else
            topic_len = selection.size();
        topic.fill(' ');
        topic.copy_n(selection.data(), topic_len);
    }
    if (!helpfile_open(file_data.old_cmds)) {
        screen_write_str(3, "Can't open HELP file", 20);
        screen_writeln();
        screen_pause();
        topic_len = 0;
    }
    while (topic_len != 0) {
        screen_home(true);
        // Note: the topic is space padded to key_len characters.
        int len;
        help_record buf;
        int sts = helpfile_read(topic, KEY_LEN, buf, len);
        if (sts == STS_RNF) {
            screen_write_str(3, "Can't find Command or Section in HELP", 37);
            screen_writeln();
        }
        bool continu = (sts & 1) != 0; // Was: odd(sts)
        if (!continu)
            topic.fill(' ');
        while (continu) {
            if ((buf.txt[1] == '\\') && (buf.txt[2] == '%')) {
                key_str reply;
                int reply_len;
                ask_user("<space> for more, <return> to exit : ", reply, reply_len);
                if (tt_controlc)
                    reply_len = 0;
                if ((reply_len == 0) || (reply[1] != ' ')) {
                    continu = false;
                    topic = reply;
                    topic_len = reply_len;
                }
            } else {
                screen_write_str(2, buf.txt.data(), len); // len was len-key_len for c version
                screen_writeln();
            }
            if (continu) {
                bool read_on = (helpfile_next(buf, len) & 1) != 0;
                if (read_on) { // check key still valid
                    for (int i = 1; i <= KEY_LEN; ++i) {
                        if (buf.key[i] != topic[i])
                            read_on = false;
                    }
                }
                if (!read_on) {
                    continu = false;
                    topic.fill(' ');
                }
            }
            if (tt_controlc) {
                topic_len = 0;
                continu = false;
            }
        }
        if (topic_len != 0) {
            if (topic[1] == ' ') {
                ask_user("Command or Section or <return> to exit : ", topic, topic_len);
                if ((topic_len != 0) && (topic[1] == ' ')) {
                    topic.copy_n(INDEX, KEY_LEN);
                    topic_len = KEY_LEN;
                }
                if (tt_controlc)
                    topic_len = 0;
            }
        }
    }
}
