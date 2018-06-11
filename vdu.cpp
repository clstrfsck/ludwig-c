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
{   Copyright (C) 2002                                                 }
{   Martin Sandiford, Adelaide, Australia                              }
{   All rights reserved.                                               }
{                                                                      }
{**********************************************************************/

/**
! Name:         VDU
!
! Description:  This module does all the complex control of the VDU type
!               screens that Ludwig demands.
!               This is the ncurses version.
*/

#include "vdu.h"

#include <cstring>
#include <unistd.h>
#include <ncurses.h>
#include <unordered_set>

namespace {
    const char BS  = 8;
    const char CR  = 13;
    const char SPC = 32;
    const char DEL = 127;

    const int OUT_M_CLEAREOL = 1;
    //const int OUT_M_ANYCURS  = 2;

    /* These min/max values define the range of "normal" keycodes */
    const int MIN_NORMAL_CODE   = 0;
    const int MAX_NORMAL_CODE   = ORD_MAXCHAR;
    /* This is how many curses keys we know about */
    const int MIN_CURSES_KEY    = KEY_MIN;
    const int MAX_CURSES_KEY    = KEY_EVENT;
    const int NUM_NCURSES_KEYS  = (MAX_CURSES_KEY - MIN_CURSES_KEY) + 1;
    const int NCURSES_SUBTRACT  = MIN_CURSES_KEY - 1;
    const int MASSAGED_MIN      = NCURSES_SUBTRACT - MAX_CURSES_KEY;
    const int NUM_CONTROL_CHARS = 33; // 0..31 and 127

    // This and "terminators" are ints so we can be a bit
    // sloppy with conversions between ranges and chars.
    const std::unordered_set<int> CONTROL_CHARS({
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x7F });

    std::unordered_set<int> terminators;
    bool vdu_setup = false;
    bool in_insert_mode = false;

    template <typename T>
    bool contains(const std::unordered_set<T> &s, const T &value) {
        return s.find(value) != s.end();
    }
}


key_code_range massage_key(int key_code) {
    if ((key_code >= MIN_NORMAL_CODE) && (key_code <= MAX_NORMAL_CODE))
        return key_code_range(key_code);
    else if ((key_code >= MIN_CURSES_KEY) && (key_code <= MAX_CURSES_KEY)) {
        if (key_code == KEY_BACKSPACE)
            return DEL;
        else
            return key_code_range(NCURSES_SUBTRACT - key_code);
    }
    return 0;
}


int unmassage_key(key_code_range key) {
    if ((key >= MIN_NORMAL_CODE) && (key <= MAX_NORMAL_CODE))
        return int(key);
    else if ((key < MIN_NORMAL_CODE) && (key >= MASSAGED_MIN))
        return NCURSES_SUBTRACT - int(key);
    return 0;
}

void vdu_movecurs(scr_col_range x, scr_row_range y) {
    ::move(y - 1, x - 1);
}


void vdu_flush(bool wait) {
    // Wait is ignored here
    ::refresh();
}

void vdu_beep() {
    if (ERR == flash())
        beep();
}

void vdu_cleareol() {
    ::clrtoeol();
    vdu_flush(false);
}

void vdu_displaystr(scr_col_range strlen, const char *str, int opts) {
    int slen = int(strlen);
    int maxlen = ::COLS - getcurx(stdscr);
    bool hitmargin;
    if (slen >= maxlen) {
        slen = maxlen;
        hitmargin = true;
    } else
        hitmargin = false;
    ::addnstr(str, slen);
    if (!hitmargin && (opts & OUT_M_CLEAREOL) != 0)
        vdu_cleareol();
    vdu_flush(false);
}

void vdu_displaych(char ch) {
    ::addch(chtype(ch));
}

void vdu_clearscr() {
    ::clear();
}

void vdu_cleareos() {
    ::clrtobot();
}

void vdu_redrawscr() {
    ::touchwin(stdscr);
    // FIXME  clearok(stdscr, true);
}

void vdu_scrollup(int n) {
    ::scrollok(stdscr, true);
    ::scrl(n);
    ::scrollok(stdscr, false);
}

void vdu_deletelines(int n, bool clear_eos) {
    ::insdelln(-n);
    /* From what I can tell, clear_eos appears to be false if it is not
     * required that the lines that have been cleared are blanked.
     * With (n)?curses, the lines are always blanked, so we ignore this
     * flag.
     * if (clear_eos && we_didnt_clear)
     *   vdu_cleareos();
     */
    vdu_flush(false);
}

void vdu_insertlines(int n) {
    ::insdelln(n);
    vdu_flush(false);
}

void vdu_insertchars(scr_col_range n) {
    int limit = int(n);
    for (int i = 0; i < limit; ++i)
        ::insch(chtype(' '));
    vdu_flush(false);
}

void vdu_deletechars(scr_col_range n) {
    int limit = int(n);
    for (int i = 0; i < limit; ++i)
        ::delch();
    vdu_flush(false);
}

void vdu_displaycrlf() {
    int y = getcury(stdscr);
    if (y == LINES - 1)
        vdu_scrollup(1);
    else
        y += 1;
    ::move(y, 0);
    vdu_flush(false);
}

void vdu_take_back_key(key_code_range key) {
    ::ungetch(unmassage_key(key));
}

void vdu_new_introducer(key_code_range key) {
    terminators = CONTROL_CHARS;
    if (key > 0) {
        int key_i = int(key);
        terminators.insert(static_cast<char>(key_i));
    }
}

key_code_range vdu_get_key() {
    vdu_flush(true);
    int raw_key;
    do {
        raw_key = ::getch();
    } while (raw_key == ERR);
    return massage_key(raw_key);
}

void vdu_get_input(const str_object &prompt, strlen_range prompt_len,
                   str_object &get, strlen_range get_len,
                   strlen_range &outlen) {
    scr_col_range plen{int(prompt_len)};
    vdu_attr_bold();
    vdu_displaystr(plen, prompt.data(), OUT_M_CLEAREOL);
    vdu_attr_normal();
    get.fill(' ');

    int maxlen = ::COLS - getcurx(stdscr);
    if (get_len > maxlen)
        get_len = maxlen;

    outlen = 0;
    key_code_range key = vdu_get_key();
    while ((get_len > 0) && (key != CR)) {
        if ((outlen > 0) && ((key == BS) || (key == DEL))) {
            get_len += 1;
            outlen -= 1;
            ::addch(BS);
            ::addch(SPC);
            ::addch(BS);
        } else {
            if ((key < 0) || contains(CONTROL_CHARS, int(key))) {
                vdu_beep();
            } else {
                get_len -= 1;
                outlen += 1;
                get[outlen] = key;
                ::addch(chtype(key));
            }
        }
        key = vdu_get_key();
    }
}

void vdu_insert_mode(bool turn_on) {
    in_insert_mode = turn_on;
}

void vdu_get_text(int str_len, str_object &str, strlen_range &outlen) {
    str.fill(' ');
    vdu_flush(false);

    outlen = 0;
    int maxlen = ::COLS - getcurx(stdscr);
    if (str_len > maxlen)
        str_len = maxlen;

    while (str_len > 0) {
        key_code_range key = vdu_get_key();
        if ((key < 0) || contains(terminators, int(key))) {
            vdu_take_back_key(key);
            str_len = 0;
        } else {
            if (in_insert_mode)
                vdu_insertchars(1);
            ::addch(chtype(key));
            ::refresh();
            outlen += 1;
            str[outlen] = key;
            str_len -= 1;
        }
    }
}


void vdu_keyboard_init(key_names_range &nr_key_names,
                       key_name_record_ptr &key_name_list_ptr,
                       accept_set_type &key_introducers,
                       terminal_info_type &terminal_info) {
    nr_key_names = NUM_CONTROL_CHARS + NUM_NCURSES_KEYS;
    key_name_list_ptr = new key_name_record[nr_key_names + 1];

    for (int i = 1; i < NUM_CONTROL_CHARS; ++i)
        key_name_list_ptr[i].key_code = key_code_range(i - 1);
    key_name_list_ptr[ 1].key_name.copy_n("CONTROL-@                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 2].key_name.copy_n("CONTROL-A                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 3].key_name.copy_n("CONTROL-B                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 4].key_name.copy_n("CONTROL-C                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 5].key_name.copy_n("CONTROL-D                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 6].key_name.copy_n("CONTROL-E                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 7].key_name.copy_n("CONTROL-F                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 8].key_name.copy_n("CONTROL-G                               ", KEY_NAME_LEN);
    key_name_list_ptr[ 9].key_name.copy_n("BACKSPACE                               ", KEY_NAME_LEN);
    key_name_list_ptr[10].key_name.copy_n("TAB                                     ", KEY_NAME_LEN);
    key_name_list_ptr[11].key_name.copy_n("LINE-FEED                               ", KEY_NAME_LEN);
    key_name_list_ptr[12].key_name.copy_n("CONTROL-K                               ", KEY_NAME_LEN);
    key_name_list_ptr[13].key_name.copy_n("CONTROL-L                               ", KEY_NAME_LEN);
    key_name_list_ptr[14].key_name.copy_n("RETURN                                  ", KEY_NAME_LEN);
    key_name_list_ptr[15].key_name.copy_n("CONTROL-N                               ", KEY_NAME_LEN);
    key_name_list_ptr[16].key_name.copy_n("CONTROL-O                               ", KEY_NAME_LEN);
    key_name_list_ptr[17].key_name.copy_n("CONTROL-P                               ", KEY_NAME_LEN);
    key_name_list_ptr[18].key_name.copy_n("CONTROL-Q                               ", KEY_NAME_LEN);
    key_name_list_ptr[19].key_name.copy_n("CONTROL-R                               ", KEY_NAME_LEN);
    key_name_list_ptr[20].key_name.copy_n("CONTROL-S                               ", KEY_NAME_LEN);
    key_name_list_ptr[21].key_name.copy_n("CONTROL-T                               ", KEY_NAME_LEN);
    key_name_list_ptr[22].key_name.copy_n("CONTROL-U                               ", KEY_NAME_LEN);
    key_name_list_ptr[23].key_name.copy_n("CONTROL-V                               ", KEY_NAME_LEN);
    key_name_list_ptr[24].key_name.copy_n("CONTROL-W                               ", KEY_NAME_LEN);
    key_name_list_ptr[25].key_name.copy_n("CONTROL-X                               ", KEY_NAME_LEN);
    key_name_list_ptr[26].key_name.copy_n("CONTROL-Y                               ", KEY_NAME_LEN);
    key_name_list_ptr[27].key_name.copy_n("CONTROL-Z                               ", KEY_NAME_LEN);
    key_name_list_ptr[28].key_name.copy_n("CONTROL-[                               ", KEY_NAME_LEN);
    key_name_list_ptr[29].key_name.copy_n("CONTROL-\\                               ", KEY_NAME_LEN);
    key_name_list_ptr[30].key_name.copy_n("CONTROL-]                               ", KEY_NAME_LEN);
    key_name_list_ptr[31].key_name.copy_n("CONTROL-^                               ", KEY_NAME_LEN);
    key_name_list_ptr[32].key_name.copy_n("CONTROL-_                               ", KEY_NAME_LEN);
    key_name_list_ptr[33].key_code = key_code_range(DEL);
    key_name_list_ptr[33].key_name.copy_n("DELETE                                  ", KEY_NAME_LEN);

    for (int i = MIN_CURSES_KEY; i <= MAX_CURSES_KEY; ++i)
        key_name_list_ptr[NUM_CONTROL_CHARS + 1 + i - MIN_CURSES_KEY].key_code = massage_key(i);
    key_name_list_ptr[NUM_CONTROL_CHARS+  1].key_name.copy_n("BREAK                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  2].key_name.copy_n("DOWN-ARROW                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  3].key_name.copy_n("UP-ARROW                                ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  4].key_name.copy_n("LEFT-ARROW                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  5].key_name.copy_n("RIGHT-ARROW                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  6].key_name.copy_n("HOME                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  7].key_name.copy_n("BACKSPACE                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  8].key_name.copy_n("FUNCTION-0                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+  9].key_name.copy_n("FUNCTION-1                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 10].key_name.copy_n("FUNCTION-2                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 11].key_name.copy_n("FUNCTION-3                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 12].key_name.copy_n("FUNCTION-4                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 13].key_name.copy_n("FUNCTION-5                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 14].key_name.copy_n("FUNCTION-6                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 15].key_name.copy_n("FUNCTION-7                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 16].key_name.copy_n("FUNCTION-8                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 17].key_name.copy_n("FUNCTION-9                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 18].key_name.copy_n("FUNCTION-10                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 19].key_name.copy_n("FUNCTION-11                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 20].key_name.copy_n("FUNCTION-12                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 21].key_name.copy_n("SHIFT-FUNCTION-1                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 22].key_name.copy_n("SHIFT-FUNCTION-2                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 23].key_name.copy_n("SHIFT-FUNCTION-3                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 24].key_name.copy_n("SHIFT-FUNCTION-4                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 25].key_name.copy_n("SHIFT-FUNCTION-5                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 26].key_name.copy_n("SHIFT-FUNCTION-6                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 27].key_name.copy_n("SHIFT-FUNCTION-7                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 28].key_name.copy_n("SHIFT-FUNCTION-8                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 29].key_name.copy_n("SHIFT-FUNCTION-9                        ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 30].key_name.copy_n("SHIFT-FUNCTION-10                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 31].key_name.copy_n("SHIFT-FUNCTION-11                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 32].key_name.copy_n("SHIFT-FUNCTION-12                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 33].key_name.copy_n("FUNCTION-25                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 34].key_name.copy_n("FUNCTION-26                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 35].key_name.copy_n("FUNCTION-27                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 36].key_name.copy_n("FUNCTION-28                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 37].key_name.copy_n("FUNCTION-29                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 38].key_name.copy_n("FUNCTION-30                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 39].key_name.copy_n("FUNCTION-31                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 40].key_name.copy_n("FUNCTION-32                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 41].key_name.copy_n("FUNCTION-33                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 42].key_name.copy_n("FUNCTION-34                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 43].key_name.copy_n("FUNCTION-35                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 44].key_name.copy_n("FUNCTION-36                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 45].key_name.copy_n("FUNCTION-37                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 46].key_name.copy_n("FUNCTION-38                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 47].key_name.copy_n("FUNCTION-39                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 48].key_name.copy_n("FUNCTION-40                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 49].key_name.copy_n("FUNCTION-41                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 50].key_name.copy_n("FUNCTION-42                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 51].key_name.copy_n("FUNCTION-43                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 52].key_name.copy_n("FUNCTION-44                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 53].key_name.copy_n("FUNCTION-45                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 54].key_name.copy_n("FUNCTION-46                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 55].key_name.copy_n("FUNCTION-47                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 56].key_name.copy_n("FUNCTION-48                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 57].key_name.copy_n("FUNCTION-49                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 58].key_name.copy_n("FUNCTION-50                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 59].key_name.copy_n("FUNCTION-51                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 60].key_name.copy_n("FUNCTION-52                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 61].key_name.copy_n("FUNCTION-53                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 62].key_name.copy_n("FUNCTION-54                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 63].key_name.copy_n("FUNCTION-55                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 64].key_name.copy_n("FUNCTION-56                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 65].key_name.copy_n("FUNCTION-57                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 66].key_name.copy_n("FUNCTION-58                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 67].key_name.copy_n("FUNCTION-59                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 68].key_name.copy_n("FUNCTION-60                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 69].key_name.copy_n("FUNCTION-61                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 70].key_name.copy_n("FUNCTION-62                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 71].key_name.copy_n("FUNCTION-63                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 72].key_name.copy_n("DELETE-LINE                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 73].key_name.copy_n("INSERT-LINE                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 74].key_name.copy_n("DELETE-CHAR                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 75].key_name.copy_n("INSERT-CHAR                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 76].key_name.copy_n("EIC                                     ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 77].key_name.copy_n("CLEAR                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 78].key_name.copy_n("CLEAR-EOS                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 79].key_name.copy_n("CLEAR-EOL                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 80].key_name.copy_n("SCROLL-FORWARD                          ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 81].key_name.copy_n("SCROLL-REVERSE                          ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 82].key_name.copy_n("PAGE-DOWN                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 83].key_name.copy_n("PAGE-UP                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 84].key_name.copy_n("SET-TAB                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 85].key_name.copy_n("CLEAR-TAB                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 86].key_name.copy_n("CLEAR-ALL-TABS                          ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 87].key_name.copy_n("SEND                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 88].key_name.copy_n("SOFT-RESET                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 89].key_name.copy_n("RESET                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 90].key_name.copy_n("PRINT                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 91].key_name.copy_n("LOWER-LEFT                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 92].key_name.copy_n("KEY-A1                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 93].key_name.copy_n("KEY-A3                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 94].key_name.copy_n("KEY-B2                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 95].key_name.copy_n("KEY-C1                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 96].key_name.copy_n("KEY-C3                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 97].key_name.copy_n("BACK-TAB                                ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 98].key_name.copy_n("BEGIN                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+ 99].key_name.copy_n("CANCEL                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+100].key_name.copy_n("CLOSE                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+101].key_name.copy_n("COMMAND                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+102].key_name.copy_n("COPY                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+103].key_name.copy_n("CREATE                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+104].key_name.copy_n("END                                     ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+105].key_name.copy_n("EXIT                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+106].key_name.copy_n("FIND                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+107].key_name.copy_n("HELP                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+108].key_name.copy_n("MARK                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+109].key_name.copy_n("MESSAGE                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+110].key_name.copy_n("MOVE                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+111].key_name.copy_n("NEXT                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+112].key_name.copy_n("OPEN                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+113].key_name.copy_n("OPTIONS                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+114].key_name.copy_n("PREVIOUS                                ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+115].key_name.copy_n("REDO                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+116].key_name.copy_n("REFERENCE                               ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+117].key_name.copy_n("REFRESH                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+118].key_name.copy_n("REPLACE                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+119].key_name.copy_n("RESTART                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+120].key_name.copy_n("RESUME                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+121].key_name.copy_n("SAVE                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+122].key_name.copy_n("SHIFT-BEGIN                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+123].key_name.copy_n("SHIFT-CANCEL                            ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+124].key_name.copy_n("SHIFT-COMMAND                           ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+125].key_name.copy_n("SHIFT-COPY                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+126].key_name.copy_n("SHIFT-CREATE                            ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+127].key_name.copy_n("SHIFT-DELETE-CHAR                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+128].key_name.copy_n("SHIFT-DELETE-LINE                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+129].key_name.copy_n("SELECT                                  ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+130].key_name.copy_n("SEND                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+131].key_name.copy_n("SHIFT-CLEAR-EOL                         ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+132].key_name.copy_n("SHIFT-EXIT                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+133].key_name.copy_n("SHIFT-FIND                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+134].key_name.copy_n("SHIFT-HELP                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+135].key_name.copy_n("SHIFT-HOME                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+136].key_name.copy_n("SHIFT-INSERT-CHAR                       ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+137].key_name.copy_n("SHIFT-LEFT                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+138].key_name.copy_n("SHIFT-MESSAGE                           ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+139].key_name.copy_n("SHIFT-MOVE                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+140].key_name.copy_n("SHIFT-NEXT                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+141].key_name.copy_n("SHIFT-OPTIONS                           ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+142].key_name.copy_n("SHIFT-PREVIOUS                          ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+143].key_name.copy_n("SHIFT-PRINT                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+144].key_name.copy_n("SHIFT-REDO                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+145].key_name.copy_n("SHIFT-REPLACE                           ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+146].key_name.copy_n("SHIFT-RIGHT                             ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+147].key_name.copy_n("SHIFT-RESUME                            ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+148].key_name.copy_n("SHIFT-SAVE                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+149].key_name.copy_n("SHIFT-SUSPEND                           ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+150].key_name.copy_n("SHIFT-UNDO                              ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+151].key_name.copy_n("SUSPEND                                 ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+152].key_name.copy_n("UNDO                                    ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+153].key_name.copy_n("MOUSE                                   ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+154].key_name.copy_n("WINDOW-RESIZE-EVENT                     ", KEY_NAME_LEN);
    key_name_list_ptr[NUM_CONTROL_CHARS+155].key_name.copy_n("SOME-OTHER-EVENT                        ", KEY_NAME_LEN);

    key_introducers.clear();

    terminal_info.name    = ::termname();
    terminal_info.namelen = std::strlen(terminal_info.name);
    terminal_info.width   = ::COLS;
    terminal_info.height  = ::LINES;
}


bool vdu_init(int outbuflen, 
              // terminal_capabilities &capabilities,
              terminal_info_type &terminal_info,
              bool &ctrl_c_flag, bool &winchange_flag) {
    // capabilities.clear();
    // capabilities.add(terminal_capabilities::trmflags_v_hard);
    terminal_info.width = 80;
    terminal_info.height = 4;
    if (isatty(0) && isatty(1)) {
        vdu_setup = ::initscr() != NULL;
        if (vdu_setup) {
            ::raw();
            ::noecho();
            ::nonl();
            ::intrflush(stdscr, false);
            ::keypad(stdscr, true);
            ::idlok(stdscr, true);
            ::idcok(stdscr, true);
            ::scrollok(stdscr, false);
            terminal_info.width = ::COLS;
            terminal_info.height = ::LINES;
            //capabilities.add({
            //        trmflags_v_clel, // Clear to end of line
            //        trmflags_v_cles, // Clear to end of screen
            //        trmflags_v_clsc, // Clear screen
            //        trmflags_v_dlch, // Delete char
            //        trmflags_v_dlln, // Delete line
            //        trmflags_v_inln, // Insert line
            //        trmflags_v_inch, // Insert char
            //        trmflags_v_scdn, // Scroll down
            //        trmflags_v_wrap  // Wrap at margins
            //});
            vdu_clearscr();
            vdu_flush(true);
            ::flushinp();
        }
        return true;
    }
    return false;
}

void vdu_free() {
    if (vdu_setup) {
        vdu_scrollup(1);
        vdu_movecurs(1, ::LINES);
        vdu_flush(true);
        ::endwin();
    }
}

void vdu_get_new_dimensions(scr_col_range &new_x, scr_row_range &new_y) {
    new_x = scr_col_range(::COLS);
    new_y = scr_row_range(::LINES);
}

void vdu_attr_bold() {
    attron(A_BOLD);
}

void vdu_attr_reverse() {
    attron(A_REVERSE);
}

void vdu_attr_normal() {
    attroff(A_BOLD);
    attroff(A_REVERSE);
}
