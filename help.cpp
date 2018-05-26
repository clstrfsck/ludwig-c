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
*/

#include "help.h"

#include "var.h"
#include "screen.h"
#include "helpfile.h"

#include <cstring>

namespace {
    const int  STS_RNF     = 98994;  // Record not found status return.
    const char INDEX[]     = "0   "; // The index key.

    char ch_toupper(char ch) {
        return std::toupper(ch);
    }
};

void ask_user(const char *prompt, key_str &reply, int &reply_len) {
    screen_writeln();
    reply.fill(' ');
    write_str pstr;
    scr_col_range plen = std::strlen(prompt);
    pstr.copy(prompt, plen);
    screen_help_prompt(pstr, plen, reply, reply_len);
    // Note that all characters not overwritten by the user will be spaces!
    screen_writeln();
    reply.apply_n(ch_toupper, reply_len);
}

void help_help(int selection_len, str_object &selection) {
    // The argument selects a particular part of the help file to read e.g. SD
//  var
//    buf : help_record;
//    reply : key_str;
//    i,len,reply_len,
//    sts : integer;
//    read_on : boolean;
//    topic     : key_str;
//    topic_len : integer;
//    continue : boolean;

    screen_unload();
    screen_home(true);
    key_str topic;
    int topic_len;
    if (selection_len == 0) {
        topic.copy(INDEX, KEY_LEN);
        topic_len = KEY_LEN;
    } else {
        if (selection_len > KEY_LEN)
            topic_len = KEY_LEN;
        else
            topic_len = selection_len;
        topic.fill(' ');
        topic.copy(selection.data(), topic_len);
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
        int sts = helpfile_read(topic, KEY_LEN, buf, KEY_LEN + WRITE_STR_LEN, len);
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
                bool read_on = (helpfile_next(buf, KEY_LEN + WRITE_STR_LEN, len) & 1) != 0;
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
                    topic.copy(INDEX, KEY_LEN);
                    topic_len = KEY_LEN;
                }
                if (tt_controlc)
                    topic_len = 0;
            }
        }
    }
}
