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
!               screens that Ludwig demands. This is the ncurses version.
!**/

#include "vdu.h"

#include "sys.h"

#include <algorithm>
#include <cstring>
#include <ncurses.h>
#include <unordered_set>

namespace {
    const char BS = 8;
    const char NL = 10;
    const char CR = 13;
    const char SPC = 32;
    const char DEL = 127;

    const int OUT_M_CLEAREOL = 1;
    // const int OUT_M_ANYCURS  = 2;

    /* These min/max values define the range of "normal" keycodes */
    const int MIN_NORMAL_CODE = 0;
    const int MAX_NORMAL_CODE = ORD_MAXCHAR;
    /* This is how many curses keys we know about */
    const int MIN_CURSES_KEY = KEY_MIN;
    const int MAX_CURSES_KEY = KEY_EVENT;
    const int NUM_NCURSES_KEYS = (MAX_CURSES_KEY - MIN_CURSES_KEY) + 1;
    const int NCURSES_SUBTRACT = MIN_CURSES_KEY - 1;
    const int MASSAGED_MIN = NCURSES_SUBTRACT - MAX_CURSES_KEY;
    const int NUM_CONTROL_CHARS = 33; // 0..31 and 127

    // This and "terminators" are ints so we can be a bit
    // sloppy with conversions between ranges and chars.
    const std::unordered_set<int>
        CONTROL_CHARS({0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
                       0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x7F});

    std::unordered_set<int> terminators;
    bool vdu_setup = false;
    bool in_insert_mode = false;

    template <typename T> bool contains(const std::unordered_set<T> &s, const T &value) {
        return s.find(value) != s.end();
    }

    bool *g_ctrl_c = nullptr;
    bool *g_winchange = nullptr;

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
} // namespace

void vdu_movecurs(scr_col_range x, scr_row_range y) {
    ::move(y - 1, x - 1);
}

void vdu_flush() {
    ::refresh();
}

void vdu_beep() {
    if (ERR == flash())
        beep();
}

void vdu_cleareol() {
    ::clrtoeol();
    vdu_flush();
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
    vdu_flush();
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

void vdu_scrollup(int n) {
    ::scrollok(stdscr, true);
    ::scrl(n);
    ::scrollok(stdscr, false);
}

void vdu_deletelines(int n) {
    ::insdelln(-n);
    vdu_flush();
}

void vdu_insertlines(int n) {
    ::insdelln(n);
    vdu_flush();
}

void vdu_insertchars(scr_col_range n) {
    int limit = int(n);
    for (int i = 0; i < limit; ++i)
        ::insch(chtype(' '));
    vdu_flush();
}

void vdu_deletechars(scr_col_range n) {
    int limit = int(n);
    for (int i = 0; i < limit; ++i)
        ::delch();
    vdu_flush();
}

void vdu_displaycrlf() {
    int y = getcury(stdscr);
    if (y == LINES - 1)
        vdu_scrollup(1);
    else
        y += 1;
    ::move(y, 0);
    vdu_flush();
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
    vdu_flush();
    int raw_key;
    do {
        raw_key = ::getch();
    } while (raw_key == ERR);
    if (raw_key == KEY_RESIZE && g_winchange)
        *g_winchange = true;
    return massage_key(raw_key);
}

void vdu_get_input(
    const std::string_view &prompt, str_object &get, strlen_range get_len, strlen_range &outlen
) {
    auto plen = std::min(static_cast<size_t>(MAX_SCR_COLS), prompt.size());
    vdu_attr_bold();
    vdu_displaystr(plen, prompt.data(), OUT_M_CLEAREOL);
    vdu_attr_normal();
    get.fill(' ');

    int maxlen = ::COLS - getcurx(stdscr);
    if (get_len > maxlen)
        get_len = maxlen;

    outlen = 0;
    key_code_range key = vdu_get_key();
    while ((get_len > 0) && (key != CR) && (key != NL)) {
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
    vdu_flush();

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

void vdu_keyboard_init(
    key_names_range &nr_key_names,
    std::vector<key_name_record> &key_name_list,
    accept_set_type &key_introducers,
    terminal_info_type &terminal_info
) {
    nr_key_names = NUM_CONTROL_CHARS + NUM_NCURSES_KEYS;
    key_name_list.resize(nr_key_names + 1);

    for (int i = 1; i < NUM_CONTROL_CHARS; ++i)
        key_name_list[i].key_code = key_code_range(i - 1);
    key_name_list[1].key_name = "CONTROL-@";
    key_name_list[2].key_name = "CONTROL-A";
    key_name_list[3].key_name = "CONTROL-B";
    key_name_list[4].key_name = "CONTROL-C";
    key_name_list[5].key_name = "CONTROL-D";
    key_name_list[6].key_name = "CONTROL-E";
    key_name_list[7].key_name = "CONTROL-F";
    key_name_list[8].key_name = "CONTROL-G";
    key_name_list[9].key_name = "BACKSPACE";
    key_name_list[10].key_name = "TAB";
    key_name_list[11].key_name = "LINE-FEED";
    key_name_list[12].key_name = "CONTROL-K";
    key_name_list[13].key_name = "CONTROL-L";
    key_name_list[14].key_name = "RETURN";
    key_name_list[15].key_name = "CONTROL-N";
    key_name_list[16].key_name = "CONTROL-O";
    key_name_list[17].key_name = "CONTROL-P";
    key_name_list[18].key_name = "CONTROL-Q";
    key_name_list[19].key_name = "CONTROL-R";
    key_name_list[20].key_name = "CONTROL-S";
    key_name_list[21].key_name = "CONTROL-T";
    key_name_list[22].key_name = "CONTROL-U";
    key_name_list[23].key_name = "CONTROL-V";
    key_name_list[24].key_name = "CONTROL-W";
    key_name_list[25].key_name = "CONTROL-X";
    key_name_list[26].key_name = "CONTROL-Y";
    key_name_list[27].key_name = "CONTROL-Z";
    key_name_list[28].key_name = "CONTROL-[";
    key_name_list[29].key_name = "CONTROL-\\";
    key_name_list[30].key_name = "CONTROL-]";
    key_name_list[31].key_name = "CONTROL-^";
    key_name_list[32].key_name = "CONTROL-_";
    key_name_list[33].key_code = key_code_range(DEL);
    key_name_list[33].key_name = "DELETE";

    for (int i = MIN_CURSES_KEY; i <= MAX_CURSES_KEY; ++i)
        key_name_list[NUM_CONTROL_CHARS + 1 + i - MIN_CURSES_KEY].key_code = massage_key(i);
    key_name_list[NUM_CONTROL_CHARS + 1].key_name = "BREAK";
    key_name_list[NUM_CONTROL_CHARS + 2].key_name = "DOWN-ARROW";
    key_name_list[NUM_CONTROL_CHARS + 3].key_name = "UP-ARROW";
    key_name_list[NUM_CONTROL_CHARS + 4].key_name = "LEFT-ARROW";
    key_name_list[NUM_CONTROL_CHARS + 5].key_name = "RIGHT-ARROW";
    key_name_list[NUM_CONTROL_CHARS + 6].key_name = "HOME";
    key_name_list[NUM_CONTROL_CHARS + 7].key_name = "BACKSPACE";
    key_name_list[NUM_CONTROL_CHARS + 8].key_name = "FUNCTION-0";
    key_name_list[NUM_CONTROL_CHARS + 9].key_name = "FUNCTION-1";
    key_name_list[NUM_CONTROL_CHARS + 10].key_name = "FUNCTION-2";
    key_name_list[NUM_CONTROL_CHARS + 11].key_name = "FUNCTION-3";
    key_name_list[NUM_CONTROL_CHARS + 12].key_name = "FUNCTION-4";
    key_name_list[NUM_CONTROL_CHARS + 13].key_name = "FUNCTION-5";
    key_name_list[NUM_CONTROL_CHARS + 14].key_name = "FUNCTION-6";
    key_name_list[NUM_CONTROL_CHARS + 15].key_name = "FUNCTION-7";
    key_name_list[NUM_CONTROL_CHARS + 16].key_name = "FUNCTION-8";
    key_name_list[NUM_CONTROL_CHARS + 17].key_name = "FUNCTION-9";
    key_name_list[NUM_CONTROL_CHARS + 18].key_name = "FUNCTION-10";
    key_name_list[NUM_CONTROL_CHARS + 19].key_name = "FUNCTION-11";
    key_name_list[NUM_CONTROL_CHARS + 20].key_name = "FUNCTION-12";
    key_name_list[NUM_CONTROL_CHARS + 21].key_name = "SHIFT-FUNCTION-1";
    key_name_list[NUM_CONTROL_CHARS + 22].key_name = "SHIFT-FUNCTION-2";
    key_name_list[NUM_CONTROL_CHARS + 23].key_name = "SHIFT-FUNCTION-3";
    key_name_list[NUM_CONTROL_CHARS + 24].key_name = "SHIFT-FUNCTION-4";
    key_name_list[NUM_CONTROL_CHARS + 25].key_name = "SHIFT-FUNCTION-5";
    key_name_list[NUM_CONTROL_CHARS + 26].key_name = "SHIFT-FUNCTION-6";
    key_name_list[NUM_CONTROL_CHARS + 27].key_name = "SHIFT-FUNCTION-7";
    key_name_list[NUM_CONTROL_CHARS + 28].key_name = "SHIFT-FUNCTION-8";
    key_name_list[NUM_CONTROL_CHARS + 29].key_name = "SHIFT-FUNCTION-9";
    key_name_list[NUM_CONTROL_CHARS + 30].key_name = "SHIFT-FUNCTION-10";
    key_name_list[NUM_CONTROL_CHARS + 31].key_name = "SHIFT-FUNCTION-11";
    key_name_list[NUM_CONTROL_CHARS + 32].key_name = "SHIFT-FUNCTION-12";
    key_name_list[NUM_CONTROL_CHARS + 33].key_name = "FUNCTION-25";
    key_name_list[NUM_CONTROL_CHARS + 34].key_name = "FUNCTION-26";
    key_name_list[NUM_CONTROL_CHARS + 35].key_name = "FUNCTION-27";
    key_name_list[NUM_CONTROL_CHARS + 36].key_name = "FUNCTION-28";
    key_name_list[NUM_CONTROL_CHARS + 37].key_name = "FUNCTION-29";
    key_name_list[NUM_CONTROL_CHARS + 38].key_name = "FUNCTION-30";
    key_name_list[NUM_CONTROL_CHARS + 39].key_name = "FUNCTION-31";
    key_name_list[NUM_CONTROL_CHARS + 40].key_name = "FUNCTION-32";
    key_name_list[NUM_CONTROL_CHARS + 41].key_name = "FUNCTION-33";
    key_name_list[NUM_CONTROL_CHARS + 42].key_name = "FUNCTION-34";
    key_name_list[NUM_CONTROL_CHARS + 43].key_name = "FUNCTION-35";
    key_name_list[NUM_CONTROL_CHARS + 44].key_name = "FUNCTION-36";
    key_name_list[NUM_CONTROL_CHARS + 45].key_name = "FUNCTION-37";
    key_name_list[NUM_CONTROL_CHARS + 46].key_name = "FUNCTION-38";
    key_name_list[NUM_CONTROL_CHARS + 47].key_name = "FUNCTION-39";
    key_name_list[NUM_CONTROL_CHARS + 48].key_name = "FUNCTION-40";
    key_name_list[NUM_CONTROL_CHARS + 49].key_name = "FUNCTION-41";
    key_name_list[NUM_CONTROL_CHARS + 50].key_name = "FUNCTION-42";
    key_name_list[NUM_CONTROL_CHARS + 51].key_name = "FUNCTION-43";
    key_name_list[NUM_CONTROL_CHARS + 52].key_name = "FUNCTION-44";
    key_name_list[NUM_CONTROL_CHARS + 53].key_name = "FUNCTION-45";
    key_name_list[NUM_CONTROL_CHARS + 54].key_name = "FUNCTION-46";
    key_name_list[NUM_CONTROL_CHARS + 55].key_name = "FUNCTION-47";
    key_name_list[NUM_CONTROL_CHARS + 56].key_name = "FUNCTION-48";
    key_name_list[NUM_CONTROL_CHARS + 57].key_name = "FUNCTION-49";
    key_name_list[NUM_CONTROL_CHARS + 58].key_name = "FUNCTION-50";
    key_name_list[NUM_CONTROL_CHARS + 59].key_name = "FUNCTION-51";
    key_name_list[NUM_CONTROL_CHARS + 60].key_name = "FUNCTION-52";
    key_name_list[NUM_CONTROL_CHARS + 61].key_name = "FUNCTION-53";
    key_name_list[NUM_CONTROL_CHARS + 62].key_name = "FUNCTION-54";
    key_name_list[NUM_CONTROL_CHARS + 63].key_name = "FUNCTION-55";
    key_name_list[NUM_CONTROL_CHARS + 64].key_name = "FUNCTION-56";
    key_name_list[NUM_CONTROL_CHARS + 65].key_name = "FUNCTION-57";
    key_name_list[NUM_CONTROL_CHARS + 66].key_name = "FUNCTION-58";
    key_name_list[NUM_CONTROL_CHARS + 67].key_name = "FUNCTION-59";
    key_name_list[NUM_CONTROL_CHARS + 68].key_name = "FUNCTION-60";
    key_name_list[NUM_CONTROL_CHARS + 69].key_name = "FUNCTION-61";
    key_name_list[NUM_CONTROL_CHARS + 70].key_name = "FUNCTION-62";
    key_name_list[NUM_CONTROL_CHARS + 71].key_name = "FUNCTION-63";
    key_name_list[NUM_CONTROL_CHARS + 72].key_name = "DELETE-LINE";
    key_name_list[NUM_CONTROL_CHARS + 73].key_name = "INSERT-LINE";
    key_name_list[NUM_CONTROL_CHARS + 74].key_name = "DELETE-CHAR";
    key_name_list[NUM_CONTROL_CHARS + 75].key_name = "INSERT-CHAR";
    key_name_list[NUM_CONTROL_CHARS + 76].key_name = "EIC";
    key_name_list[NUM_CONTROL_CHARS + 77].key_name = "CLEAR";
    key_name_list[NUM_CONTROL_CHARS + 78].key_name = "CLEAR-EOS";
    key_name_list[NUM_CONTROL_CHARS + 79].key_name = "CLEAR-EOL";
    key_name_list[NUM_CONTROL_CHARS + 80].key_name = "SCROLL-FORWARD";
    key_name_list[NUM_CONTROL_CHARS + 81].key_name = "SCROLL-REVERSE";
    key_name_list[NUM_CONTROL_CHARS + 82].key_name = "PAGE-DOWN";
    key_name_list[NUM_CONTROL_CHARS + 83].key_name = "PAGE-UP";
    key_name_list[NUM_CONTROL_CHARS + 84].key_name = "SET-TAB";
    key_name_list[NUM_CONTROL_CHARS + 85].key_name = "CLEAR-TAB";
    key_name_list[NUM_CONTROL_CHARS + 86].key_name = "CLEAR-ALL-TABS";
    key_name_list[NUM_CONTROL_CHARS + 87].key_name = "SEND";
    key_name_list[NUM_CONTROL_CHARS + 88].key_name = "SOFT-RESET";
    key_name_list[NUM_CONTROL_CHARS + 89].key_name = "RESET";
    key_name_list[NUM_CONTROL_CHARS + 90].key_name = "PRINT";
    key_name_list[NUM_CONTROL_CHARS + 91].key_name = "LOWER-LEFT";
    key_name_list[NUM_CONTROL_CHARS + 92].key_name = "KEY-A1";
    key_name_list[NUM_CONTROL_CHARS + 93].key_name = "KEY-A3";
    key_name_list[NUM_CONTROL_CHARS + 94].key_name = "KEY-B2";
    key_name_list[NUM_CONTROL_CHARS + 95].key_name = "KEY-C1";
    key_name_list[NUM_CONTROL_CHARS + 96].key_name = "KEY-C3";
    key_name_list[NUM_CONTROL_CHARS + 97].key_name = "BACK-TAB";
    key_name_list[NUM_CONTROL_CHARS + 98].key_name = "BEGIN";
    key_name_list[NUM_CONTROL_CHARS + 99].key_name = "CANCEL";
    key_name_list[NUM_CONTROL_CHARS + 100].key_name = "CLOSE";
    key_name_list[NUM_CONTROL_CHARS + 101].key_name = "COMMAND";
    key_name_list[NUM_CONTROL_CHARS + 102].key_name = "COPY";
    key_name_list[NUM_CONTROL_CHARS + 103].key_name = "CREATE";
    key_name_list[NUM_CONTROL_CHARS + 104].key_name = "END";
    key_name_list[NUM_CONTROL_CHARS + 105].key_name = "EXIT";
    key_name_list[NUM_CONTROL_CHARS + 106].key_name = "FIND";
    key_name_list[NUM_CONTROL_CHARS + 107].key_name = "HELP";
    key_name_list[NUM_CONTROL_CHARS + 108].key_name = "MARK";
    key_name_list[NUM_CONTROL_CHARS + 109].key_name = "MESSAGE";
    key_name_list[NUM_CONTROL_CHARS + 110].key_name = "MOVE";
    key_name_list[NUM_CONTROL_CHARS + 111].key_name = "NEXT";
    key_name_list[NUM_CONTROL_CHARS + 112].key_name = "OPEN";
    key_name_list[NUM_CONTROL_CHARS + 113].key_name = "OPTIONS";
    key_name_list[NUM_CONTROL_CHARS + 114].key_name = "PREVIOUS";
    key_name_list[NUM_CONTROL_CHARS + 115].key_name = "REDO";
    key_name_list[NUM_CONTROL_CHARS + 116].key_name = "REFERENCE";
    key_name_list[NUM_CONTROL_CHARS + 117].key_name = "REFRESH";
    key_name_list[NUM_CONTROL_CHARS + 118].key_name = "REPLACE";
    key_name_list[NUM_CONTROL_CHARS + 119].key_name = "RESTART";
    key_name_list[NUM_CONTROL_CHARS + 120].key_name = "RESUME";
    key_name_list[NUM_CONTROL_CHARS + 121].key_name = "SAVE";
    key_name_list[NUM_CONTROL_CHARS + 122].key_name = "SHIFT-BEGIN";
    key_name_list[NUM_CONTROL_CHARS + 123].key_name = "SHIFT-CANCEL";
    key_name_list[NUM_CONTROL_CHARS + 124].key_name = "SHIFT-COMMAND";
    key_name_list[NUM_CONTROL_CHARS + 125].key_name = "SHIFT-COPY";
    key_name_list[NUM_CONTROL_CHARS + 126].key_name = "SHIFT-CREATE";
    key_name_list[NUM_CONTROL_CHARS + 127].key_name = "SHIFT-DELETE-CHAR";
    key_name_list[NUM_CONTROL_CHARS + 128].key_name = "SHIFT-DELETE-LINE";
    key_name_list[NUM_CONTROL_CHARS + 129].key_name = "SELECT";
    key_name_list[NUM_CONTROL_CHARS + 130].key_name = "SEND";
    key_name_list[NUM_CONTROL_CHARS + 131].key_name = "SHIFT-CLEAR-EOL";
    key_name_list[NUM_CONTROL_CHARS + 132].key_name = "SHIFT-EXIT";
    key_name_list[NUM_CONTROL_CHARS + 133].key_name = "SHIFT-FIND";
    key_name_list[NUM_CONTROL_CHARS + 134].key_name = "SHIFT-HELP";
    key_name_list[NUM_CONTROL_CHARS + 135].key_name = "SHIFT-HOME";
    key_name_list[NUM_CONTROL_CHARS + 136].key_name = "SHIFT-INSERT-CHAR";
    key_name_list[NUM_CONTROL_CHARS + 137].key_name = "SHIFT-LEFT";
    key_name_list[NUM_CONTROL_CHARS + 138].key_name = "SHIFT-MESSAGE";
    key_name_list[NUM_CONTROL_CHARS + 139].key_name = "SHIFT-MOVE";
    key_name_list[NUM_CONTROL_CHARS + 140].key_name = "SHIFT-NEXT";
    key_name_list[NUM_CONTROL_CHARS + 141].key_name = "SHIFT-OPTIONS";
    key_name_list[NUM_CONTROL_CHARS + 142].key_name = "SHIFT-PREVIOUS";
    key_name_list[NUM_CONTROL_CHARS + 143].key_name = "SHIFT-PRINT";
    key_name_list[NUM_CONTROL_CHARS + 144].key_name = "SHIFT-REDO";
    key_name_list[NUM_CONTROL_CHARS + 145].key_name = "SHIFT-REPLACE";
    key_name_list[NUM_CONTROL_CHARS + 146].key_name = "SHIFT-RIGHT";
    key_name_list[NUM_CONTROL_CHARS + 147].key_name = "SHIFT-RESUME";
    key_name_list[NUM_CONTROL_CHARS + 148].key_name = "SHIFT-SAVE";
    key_name_list[NUM_CONTROL_CHARS + 149].key_name = "SHIFT-SUSPEND";
    key_name_list[NUM_CONTROL_CHARS + 150].key_name = "SHIFT-UNDO";
    key_name_list[NUM_CONTROL_CHARS + 151].key_name = "SUSPEND";
    key_name_list[NUM_CONTROL_CHARS + 152].key_name = "UNDO";
    key_name_list[NUM_CONTROL_CHARS + 153].key_name = "MOUSE";
    key_name_list[NUM_CONTROL_CHARS + 154].key_name = "WINDOW-RESIZE-EVENT";
    key_name_list[NUM_CONTROL_CHARS + 155].key_name = "SOME-OTHER-EVENT";

    key_introducers.reset();

    terminal_info.name = ::termname();
    terminal_info.width = ::COLS;
    terminal_info.height = ::LINES;
}

bool vdu_init(terminal_info_type &terminal_info, bool &ctrl_c_flag, bool &winchange_flag) {
    g_ctrl_c = &ctrl_c_flag;
    g_winchange = &winchange_flag;
    terminal_info.width = 80;
    terminal_info.height = 4;
    if (sys_istty()) {
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
            vdu_clearscr();
            vdu_flush();
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
        vdu_flush();
        ::endwin();
    }
}

void vdu_get_new_dimensions(scr_col_range &new_x, scr_row_range &new_y) {
    ::endwin();
    ::refresh();
    new_x = scr_col_range(::COLS);
    new_y = scr_row_range(::LINES);
}

void vdu_attr_bold() {
    attron(A_BOLD);
}

void vdu_attr_normal() {
    attroff(A_BOLD);
    attroff(A_REVERSE);
}
