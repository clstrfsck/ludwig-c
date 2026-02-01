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
! Name:         FRAME
!
! Description:  Creation/destruction, manipulation of Frames.
!**/

#include "frame.h"

#include "ch.h"
#include "dfa.h"
#include "line.h"
#include "mark.h"
#include "screen.h"
#include "span.h"
#include "tpar.h"
#include "user.h"
#include "var.h"
#include "vdu.h"
#include "version.h"

#include <cstring>

namespace {

    const std::string END_OF_FILE("<End of File>   ");
    const std::string NEW_VALUES("  New Values: ");

    const accept_set_type npunct() {
        // Here to ensure initialisation after ALPHA_SET et al
        static const accept_set_type NPUNCT =
            accept_set_type(ALPHA_SET).add(NUMERIC_SET).add(SPACE_SET);
        return NPUNCT;
    }
} // namespace

bool frame_edit(const std::string_view &frame_name) {
    /***************************************************************************
     *    D E S C R I P T I O N :-                                             *
     * Input   : frame_name                                                    *
     * Output  : <none>   [ Modifies current_frame ]                           *
     * Purpose : This is the \ED command.  if frame_name doesn't exist,        *
     *           then it is created.                                           *
     * Errors  : Fails if span of that name already exists, or something       *
     *           terrible happens.                                             *
     *                                                                         *
     * Notes   : This routine deals with GROUPS. When a frame is created, it   *
     *           is necessary to pass a Group_ptr to a line routine to create  *
     *           the <eop> line.  Also necessary to delve into the Group       *
     *           structure to pass the first & last lines to create the span.  *
     *           If the input name is blank then this is converted to          *
     *           automatically converted to `LUDWIG', the default name.        *
     **************************************************************************/

    const int SPN = 0x0001;
    const int FRM = 0x0002;
    const int MRK1 = 0x0004;
    const int MRK2 = 0x0008;
    const int GRP = 0x0010;
    const int DOT = 0x0020;
    int created = 0;
    std::string_view fname = frame_name.empty() ? DEFAULT_FRAME_NAME : frame_name;
    span_ptr ptr;
    span_ptr oldp;
    if (span_find(fname, ptr, oldp)) {
        if (ptr->frame != nullptr) {
            if (ptr->frame != current_frame) {
                ptr->frame->return_frame = current_frame;
                current_frame = ptr->frame;
            }
            return true;
        } else {
            screen_message(MSG_SPAN_OF_THAT_NAME_EXISTS);
            return false;
        }
    } else {
        // No Span/Frame of that name exists, create one.
        frame_ptr fptr = new frame_object;
        span_ptr sptr = new span_object;
        created |= FRM | SPN;
        group_ptr gptr;
        if (line_eop_create(fptr, gptr)) {
            // see note above
            created |= GRP;
            // with sptr^ do
            sptr->blink = oldp;
            sptr->flink = ptr;
            if (oldp == nullptr)
                first_span = sptr;
            else
                oldp->flink = sptr;
            if (ptr != nullptr)
                ptr->blink = sptr;
            sptr->name = fname;
            sptr->frame = fptr;
            sptr->mark_one = nullptr;
            sptr->mark_two = nullptr;
            sptr->code = nullptr;
            if (mark_create(gptr->first_line, 1, sptr->mark_one)) {
                // see Note above
                created |= MRK1;
                if (mark_create(gptr->last_line, 1, sptr->mark_two)) {
                    // see Note above
                    created |= MRK2;
                    fptr->dot = nullptr;
                    if (mark_create(gptr->first_line, initial_margin_left, fptr->dot))
                        created |= DOT;
                }
            }
            if (created & DOT) {
                // with fptr^ do
                fptr->first_group = gptr;
                fptr->last_group = gptr;
                // dot created above
                fptr->marks = initial_marks;
                fptr->scr_height = initial_scr_height;
                fptr->scr_width = initial_scr_width;
                fptr->scr_offset = initial_scr_offset;
                fptr->scr_dot_line = 1;
                fptr->span = sptr;
                fptr->return_frame = current_frame;
                fptr->input_count = 0;
                fptr->space_limit = file_data.space;
                fptr->space_left = file_data.space;
                fptr->text_modified = false;
                fptr->margin_left = initial_margin_left;
                fptr->margin_right = initial_margin_right;
                fptr->margin_top = initial_margin_top;
                fptr->margin_bottom = initial_margin_bottom;
                fptr->tab_stops = initial_tab_stops;
                fptr->options = initial_options;
                fptr->input_file = -1;
                fptr->output_file = -1;
                fptr->get_tpar.len = 0;
                fptr->get_tpar.con = nullptr;
                fptr->get_tpar.nxt = nullptr;
                fptr->eqs_tpar.len = 0;
                fptr->eqs_tpar.con = nullptr;
                fptr->eqs_tpar.nxt = nullptr;
                fptr->rep1_tpar.len = 0;
                fptr->rep1_tpar.con = nullptr;
                fptr->rep1_tpar.nxt = nullptr;
                fptr->rep2_tpar.len = 0;
                fptr->rep2_tpar.con = nullptr;
                fptr->rep2_tpar.nxt = nullptr;
                fptr->verify_tpar.len = 0;
                fptr->verify_tpar.con = nullptr;
                fptr->verify_tpar.nxt = nullptr;
                fptr->eqs_pattern_ptr = nullptr;
                fptr->get_pattern_ptr = nullptr;
                fptr->rep_pattern_ptr = nullptr;
                if (line_change_length(gptr->last_line, NAME_LEN + END_OF_FILE.size())) {
                    // with gptr->last_line^ do
                    //  Guaranteed that gptr->last_line->str != nullptr here if line_change_length
                    //  OK
                    gptr->last_line->str->copy_n(END_OF_FILE.data(), END_OF_FILE.size());
                    gptr->last_line->str->copy_n(
                        fname.data(), fname.size(), 1 + END_OF_FILE.size()
                    );
                    gptr->last_line->used = 0; // Special feature of the NULL line !
                    current_frame = fptr;
                    return true;
                }
            }
        }
        // Something terrible has happened..
        // This is the failure handler
        if (created & MRK1)
            mark_destroy(sptr->mark_one);
        if (created & MRK2)
            mark_destroy(sptr->mark_two);
        if (created & SPN) {
            delete sptr;
            delete fptr;
        }
        if (created & GRP)
            line_eop_destroy(gptr);
#ifdef DEBUG
        screen_message(DBG_FRAME_CREATION_FAILED);
#endif
    }
    return false;
}

bool frame_kill(const std::string_view &frame_name) {
    /***************************************************************************
     *    D E S C R I P T I O N :-                                             *
     * Input   : frame_name                                                    *
     * Output  : <none>   [ Modifies current_frame ]                           *
     * Purpose : Kills the frame specified.  You cant kill frame C or OOPS!    *
     * Errors  : Fails if frame is current_frame, C or OOPS or if something    *
     *           terrible happens.                                             *
     **************************************************************************/

    span_ptr oldp;
    span_ptr sptr;
    if (!span_find(frame_name, sptr, oldp)) {
        screen_message(MSG_NO_SUCH_FRAME);
        return false;
    }
    if (sptr->frame == nullptr) {
        screen_message(MSG_NO_SUCH_FRAME);
        return false;
    }
    frame_ptr this_frame = sptr->frame;
    if ((this_frame == current_frame) || (this_frame == scr_frame) ||
        this_frame->options.contains(frame_options_elts::opt_special_frame)) {
        screen_message(MSG_CANT_KILL_FRAME);
        return false;
    }
    // with this_frame^ do
    if ((this_frame->input_file >= 0) || (this_frame->output_file >= 0)) {
        screen_message(MSG_FRAME_HAS_FILES_ATTACHED);
        return false;
    }

    // We are now free to destroy this frame
    // Step 1. -- remove all ERs back to this frame
    //            and all spans into this frame}
    oldp = first_span;
    while (oldp != nullptr) {
        sptr = oldp->flink;
        if (oldp->frame != nullptr) {
            // with oldp->frame^ do
            if (oldp->frame->return_frame == this_frame)
                oldp->frame->return_frame = nullptr;
        } else if (oldp->mark_one->line->group->frame == this_frame) {
            if (!span_destroy(oldp))
                return false;
        }
        oldp = sptr;
    }

    // with this_frame^ do
    //  Step 2. -- Destroy the Span
    //             Zap the frame ptr in the span header
    //             So that Span_Destroy doesnt crash
    this_frame->span->frame = nullptr;
    if (!span_destroy(this_frame->span))
        return false;

    // Step 3a. -- Destroy all internal lines.
    if (!mark_destroy(this_frame->dot))
        return false;
    for (int i = MIN_MARK_NUMBER; i <= MAX_MARK_NUMBER; ++i) {
        if (this_frame->marks[i] != nullptr) {
            if (!mark_destroy(this_frame->marks[i]))
                return false;
        }
    }
    line_ptr ptr2 = this_frame->last_group->last_line->blink;
    if (ptr2 != nullptr) {
        line_ptr ptr1 = this_frame->first_group->first_line;
        if (!lines_extract(ptr1, ptr2))
            return false;
        if (!lines_destroy(ptr1, ptr2))
            return false;
    }

    // Step 3b. -- Destroy the <eop> line.
    if (!line_eop_destroy(this_frame->first_group))
        return false;

    // Step 4. -- Dispose of the frame header (phew!)
    //            and any pattern tables attatched
    // with this_frame^ do
    if (!pattern_dfa_table_kill(this_frame->eqs_pattern_ptr))
        return false; // FAV
    if (!pattern_dfa_table_kill(this_frame->get_pattern_ptr))
        return false; // FAV
    if (!pattern_dfa_table_kill(this_frame->rep_pattern_ptr))
        return false; // FAV

    delete this_frame;
    return true;
}

char nextchar(const tpar_object &request, int &pos) {
    // with request do
    while ((pos < request.len) && (request.str[pos] == ' '))
        pos += 1;
    char ch;
    if ((pos > request.len) || (request.str[pos] == ' '))
        ch = '\0';
    else
        ch = request.str[pos];
    if (pos <= request.len)
        pos += 1;
    return ch;
}

bool setmemory(int sz, bool set_initial) {
    if (sz >= MAX_SPACE)
        sz = MAX_SPACE;
    if (set_initial)
        file_data.space = sz;
    // with current_frame^ do
    int used_storage = current_frame->space_limit - current_frame->space_left;
    int min_size = used_storage + 800;
    if (min_size > current_frame->space_limit)
        min_size = current_frame->space_limit;
    if (sz < min_size)
        sz = min_size;
    current_frame->space_limit = sz;
    current_frame->space_left = sz - used_storage;
    return true;
}

bool frame_setheight(int sh, bool set_initial) {
    // with current_frame^ do
    if ((sh >= 1) && (sh <= terminal_info.height)) {
        if (set_initial)
            initial_scr_height = sh;
        current_frame->scr_height = sh;
        scr_row_range band = sh / 6;
        if (set_initial)
            initial_margin_top = band;
        current_frame->margin_top = band;
        if (set_initial)
            initial_margin_bottom = band;
        current_frame->margin_bottom = band;
        return true;
    } else {
        screen_message(MSG_INVALID_SCREEN_HEIGHT);
        return false;
    }
}

bool setwidth(int wid, bool set_initial) {
    if ((wid >= 10) && (wid <= terminal_info.width)) {
        if (set_initial)
            initial_scr_width = wid;
        current_frame->scr_width = wid;
        return true;
    } else {
        screen_message(MSG_SCREEN_WIDTH_INVALID);
        return false;
    }
}

void show_options() {
    screen_unload();
    // with current_frame^ do
    screen_home(true);
    screen_write_str(0, "    Ludwig Option         Code    State");
    screen_writeln();
    screen_write_str(0, "    --------------------  ----    -----");
    screen_writeln();
    screen_writeln();
    screen_write_str(4, "Show current options  S");
    screen_writeln();
    screen_write_str(4, "Auto-indenting        I       ");
    if (current_frame->options.contains(frame_options_elts::opt_auto_indent))
        screen_write_str(0, "On");
    else
        screen_write_str(0, "Off");
    screen_writeln();
    screen_write_str(4, "New Line              N       ");
    if (current_frame->options.contains(frame_options_elts::opt_new_line))
        screen_write_str(0, "On");
    else
        screen_write_str(0, "Off");
    screen_writeln();
    screen_write_str(4, "Wrap at Right Margin  W       ");
    if (current_frame->options.contains(frame_options_elts::opt_auto_wrap))
        screen_write_str(0, "On");
    else
        screen_write_str(0, "Off");
    screen_writeln();
    screen_writeln();
    screen_pause();
    screen_home(true); // wipe out the display
}

bool set_opt(char ch, bool seton, frame_options &options) {
    // worker module to set 1 option

    switch (ch) {
    case 'S':
        show_options();
        break;
    case 'I':
        if (seton)
            options.insert(frame_options_elts::opt_auto_indent);
        else
            options.erase(frame_options_elts::opt_auto_indent);
        break;
    case 'W':
        if (seton)
            options.insert(frame_options_elts::opt_auto_wrap);
        else
            options.erase(frame_options_elts::opt_auto_wrap);
        break;
    case 'N':
        if (seton)
            options.insert(frame_options_elts::opt_new_line);
        else
            options.erase(frame_options_elts::opt_new_line);
        break;
    default:
        // No such option
        screen_message(MSG_UNKNOWN_OPTION);
        return false;
    }
    return true;
}

bool set_options(const tpar_object &request, int &pos, bool set_initial) {
    bool ok = false;
    char ch = nextchar(request, pos);
    if (ch == '(') {
        do {
            bool seton = true;
            ch = nextchar(request, pos);
            if (ch == '-') {
                seton = false;
                ch = nextchar(request, pos);
            }
            if (set_initial)
                set_opt(ch, seton, initial_options);
            ok = set_opt(ch, seton, current_frame->options);
            ch = nextchar(request, pos);
            if ((ch != ',') && (ch != ')')) {
                screen_message(MSG_SYNTAX_ERROR_IN_OPTIONS);
                return false;
            }
        } while (ok && ch != ')');
    } else {
        // single option
        bool seton = true;
        if (ch == '-') {
            seton = false;
            ch = nextchar(request, pos);
        }
        if (set_initial)
            set_opt(ch, seton, initial_options);
        ok = set_opt(ch, seton, current_frame->options);
    }
    return ok;
}

bool setcmdintr(const tpar_object &request, int &pos) {
    if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
        std::string key_name;
        int i = 0;
        bool terminate = false;
        while ((pos <= request.len) && !terminate) {
            if (request.str[pos] == ',') {
                terminate = true;
            } else {
                i += 1;
                key_name.push_back(request.str[pos]);
                pos += 1;
            }
        }
        key_code_range key_code;
        if (i == 1) {
            if (!npunct().contains(key_name[1])) {
                command_introducer = key_name[0];
                vdu_new_introducer(command_introducer);
                return true;
            } else {
                screen_message(MSG_INVALID_CMD_INTRODUCER);
            }
        } else if (user_key_name_to_code(key_name, key_code)) {
            if (key_introducers.contains(key_code)) {
                screen_message(MSG_INVALID_CMD_INTRODUCER);
            } else {
                command_introducer = key_code;
                vdu_new_introducer(command_introducer);
                return true;
            }
        } else {
            screen_message(MSG_UNRECOGNIZED_KEY_NAME);
        }
    } else {
        screen_message(MSG_SCREEN_MODE_ONLY);
    }
    return false;
}

bool set_mode(const tpar_object &request, int &pos) {
    char ch = nextchar(request, pos);
    if (ch == 'I') {
        edit_mode = mode_type::mode_insert;
    } else if (ch == 'O') {
        edit_mode = mode_type::mode_overtype;
    } else if (ch == 'C') {
        edit_mode = mode_type::mode_command;
    } else {
        screen_message(MSG_MODE_ERROR);
        // FIXME: Original always returns true,
        //        but this should probably fail.
        // return false;
    }
    return true;
}

bool set_tabs(const tpar_object &request, int &pos, bool set_initial) {
    // with current_frame^ do
    char ch = nextchar(request, pos);
    switch (ch) {
    case 'D': // default tabs
        if (set_initial)
            initial_tab_stops = DEFAULT_TAB_STOPS;
        current_frame->tab_stops = DEFAULT_TAB_STOPS;
        break;
    case 'T': // template match
        // with dot->line^ do
        if (current_frame->dot->line->used > 0) {
            bool ts = (*current_frame->dot->line->str)[1] != ' ';
            if (set_initial)
                initial_tab_stops[1] = ts;
            current_frame->tab_stops[1] = ts;
        }
        for (int i = 2; i <= current_frame->dot->line->used; ++i) {
            char chi = current_frame->dot->line->str->operator[](i);
            char chim1 = current_frame->dot->line->str->operator[](i - 1);
            if (set_initial)
                initial_tab_stops[i] = (chi != ' ') && (chim1 == ' ');
            current_frame->tab_stops[i] = (chi != ' ') && (chim1 == ' ');
        }
        for (int i = current_frame->dot->line->used; i <= MAX_STRLEN; ++i) {
            if (set_initial)
                initial_tab_stops[i] = false;
            current_frame->tab_stops[i] = false;
        }
        break;
    case 'I':
        { // insert tabs
            line_ptr first_line;
            line_ptr last_line;
            if (!lines_create(1, first_line, last_line))
                return false;
            if (!line_change_length(first_line, MAX_STRLEN))
                return false; // FIXME: is this a leak?
            // with first_line^ do
            if (set_initial) {
                for (int i = 1; i <= MAX_STRLEN; ++i) {
                    if (initial_tab_stops[i])
                        (*first_line->str)[i] = 'T';
                }
                (*first_line->str)[initial_margin_left] = 'L';
                (*first_line->str)[initial_margin_right] = 'R';
            } else {
                for (int i = 1; i < MAX_STRLEN; ++i) {
                    if (current_frame->tab_stops[i])
                        (*first_line->str)[i] = 'T';
                }
                (*first_line->str)[current_frame->margin_left] = 'L';
                (*first_line->str)[current_frame->margin_right] = 'R';
            }
            first_line->used = first_line->str->length(' ');
            if (!lines_inject(first_line, last_line, current_frame->dot->line))
                return false; // FIXME: is this a leak?
            if (!mark_create(first_line, current_frame->dot->col, current_frame->dot))
                return false;
            current_frame->text_modified = true;
            if (!mark_create(
                    first_line, current_frame->dot->col, current_frame->marks[MARK_MODIFIED]
                ))
                return false;
        }
        break;
    case 'R':
        { // Template Ruler
            // with dot^,line^ do
            int i = 1;
            bool legal = true;
            enum { LM_NONE, LM_LEFT, LM_RIGHT } last_margin = LM_NONE;
            while ((i <= current_frame->dot->line->used) && legal) {
                char chi = std::toupper(current_frame->dot->line->str->operator[](i));
                legal = (chi == 'T') || (chi == 'L') || (chi == 'R') || (chi == ' ');
                if (chi == 'L') {
                    legal = legal && (last_margin == LM_NONE);
                    last_margin = LM_LEFT;
                } else if (chi == 'R') {
                    legal = legal && (last_margin == LM_LEFT);
                    last_margin = LM_RIGHT;
                }
                i += 1;
            }
            legal = legal && (last_margin == LM_RIGHT);
            if (!legal) {
                screen_message(MSG_INVALID_RULER);
                return false;
            }
            i = 1;
            while (i <= current_frame->dot->line->used) {
                char chi = std::toupper(current_frame->dot->line->str->operator[](i));
                if (set_initial)
                    initial_tab_stops[i] = (chi != ' ');
                current_frame->tab_stops[i] = (chi != ' ');
                if (chi == 'L') {
                    if (set_initial)
                        initial_margin_left = i;
                    current_frame->margin_left = i;
                } else if (chi == 'R') {
                    if (set_initial)
                        initial_margin_right = i;
                    current_frame->margin_right = i;
                }
                i += 1;
            }
            for (int j = current_frame->dot->line->used + 1; j <= MAX_STRLEN; ++j) {
                if (set_initial)
                    initial_tab_stops[j] = false;
                current_frame->tab_stops[j] = false;
            }
            line_ptr first_line = current_frame->dot->line;
            col_range dot_col = current_frame->dot->col;
            if (!marks_squeeze(first_line, 1, first_line->flink, 1))
                return false;
            if (!lines_extract(first_line, first_line))
                return false;
            if (!lines_destroy(first_line, first_line))
                return false; // FIXME: Leak, but what to do?
            current_frame->dot->col = dot_col;
        }
        break;
    case 'S':
        if (current_frame->dot->col == MAX_STRLENP) {
            screen_message(MSG_OUT_OF_RANGE_TAB_VALUE);
            return false;
        }
        if (set_initial)
            initial_tab_stops[current_frame->dot->col] = true;
        current_frame->tab_stops[current_frame->dot->col] = true;
        break;
    case 'C':
        if (current_frame->dot->col == MAX_STRLENP) {
            screen_message(MSG_OUT_OF_RANGE_TAB_VALUE);
            return false;
        }
        if (set_initial)
            initial_tab_stops[current_frame->dot->col] = false;
        current_frame->tab_stops[current_frame->dot->col] = false;
        break;
    case '(':
        { // multi-columns specified
            tab_array temptab{};
            temptab.fill(false);
            temptab[0] = true;
            temptab[MAX_STRLENP] = true;
            do {
                int j;
                if (!tpar_to_int(request, pos, j))
                    return false;
                if ((1 <= j) && (j <= MAX_STRLEN)) {
                    temptab[j] = true;
                } else {
                    screen_message(MSG_OUT_OF_RANGE_TAB_VALUE);
                    return false;
                }
                ch = nextchar(request, pos);
                if ((ch != ',') && (ch != ')')) {
                    screen_message(MSG_BAD_FORMAT_IN_TAB_TABLE);
                    return false;
                }
            } while (ch != ')');
            if (set_initial)
                initial_tab_stops = temptab;
            current_frame->tab_stops = temptab;
        }
        break;
    default:
        screen_message(MSG_INVALID_T_OPTION);
        return false;
    }
    return true;
}

bool get_mar(char &ch, int &pos, const tpar_object &request, int lo_bnd, int hi_bnd, int &margin) {
    if (('0' <= ch) && (ch <= '9')) {
        pos -= 1;
        if (!tpar_to_int(request, pos, margin))
            return false;
        if ((margin < lo_bnd) || (hi_bnd < margin)) {
            screen_message(MSG_MARGIN_OUT_OF_RANGE);
            return false;
        }
        ch = nextchar(request, pos);
    }
    return true;
}

bool get_margins(
    int lo_bnd, int hi_bnd, const tpar_object &request, int &pos, int &lower, int &upper, bool lr
) {
    char ch = nextchar(request, pos);
    if (ch != '(') {
        screen_message(MSG_MARGIN_SYNTAX_ERROR);
        return false;
    }
    ch = nextchar(request, pos);
    if (ch == '.') {
        // with current_frame^ do
        if (lr)
            lower = current_frame->dot->col;
        else
            lower = current_frame->dot->line->scr_row_nr;
        ch = nextchar(request, pos);
    } else if (!get_mar(ch, pos, request, lo_bnd, hi_bnd, lower)) {
        return false;
    }
    if (ch == ',') {
        ch = nextchar(request, pos);
        if (ch == '.') {
            // with current_frame^ do
            if (lr)
                upper = current_frame->dot->col;
            else
                upper = current_frame->scr_height - current_frame->dot->line->scr_row_nr;
            ch = nextchar(request, pos);
        } else if (!get_mar(ch, pos, request, lo_bnd, hi_bnd, upper)) {
            return false;
        }
    }
    if (ch != ')') {
        screen_message(MSG_MARGIN_SYNTAX_ERROR);
        return false;
    }
    return true;
}

bool set_lrmargin(const tpar_object &request, int &pos, bool set_initial) {
    // with current_frame^ do
    int tl;
    int tr;
    if (set_initial) {
        tl = initial_margin_left;
        tr = initial_margin_right;
    } else {
        tl = current_frame->margin_left;
        tr = current_frame->margin_right;
    }
    if (!get_margins(1, MAX_STRLEN, request, pos, tl, tr, true))
        return false;
    if (tl < tr) {
        if (set_initial) {
            initial_margin_left = tl;
            initial_margin_right = tr;
        }
        current_frame->margin_left = tl;
        current_frame->margin_right = tr;
    } else {
        screen_message(MSG_LEFT_MARGIN_GE_RIGHT);
        return false;
    }
    return true;
}

bool set_tbmargin(tpar_object &request, int &pos, bool set_initial) {
    // with current_frame^ do
    int tt;
    int tb;
    if (set_initial) {
        tt = initial_margin_top;
        tb = initial_margin_bottom;
    } else {
        tt = current_frame->margin_top;
        tb = current_frame->margin_bottom;
    }
    if (!get_margins(0, current_frame->scr_height, request, pos, tt, tb, false))
        return false;
    if (tt + tb >= current_frame->scr_height) {
        screen_message(MSG_MARGIN_OUT_OF_RANGE);
        return false; // needs better message--KBN--
    }
    if (set_initial) {
        initial_margin_top = tt;
        initial_margin_bottom = tb;
    }
    current_frame->margin_top = tt;
    current_frame->margin_bottom = tb;
    return true;
}

bool setparam(tpar_object &request) {
    int pos = 1;
    char ch = nextchar(request, pos);
    bool set_initial;
    while (ch != '\0') {
        if (ch == '$') { // setting an initial value for a new frame
            set_initial = true;
            ch = nextchar(request, pos);
        } else {
            set_initial = false;
        }
        if (nextchar(request, pos) != '=') {
            screen_message(MSG_OPTIONS_SYNTAX_ERROR);
            return false;
        }
        bool ok = false;
        int temp;
        switch (ch) {
            //      if ch in ['O', 'S', 'H', 'W', 'C', 'T', 'M', 'V', 'K'] then
        case 'O':
            ok = set_options(request, pos, set_initial);
            break;
        case 'S':
            if (tpar_to_int(request, pos, temp))
                ok = setmemory(temp, set_initial);
            break;
        case 'H':
            if (tpar_to_int(request, pos, temp))
                ok = frame_setheight(temp, set_initial);
            break;
        case 'W':
            if (tpar_to_int(request, pos, temp))
                ok = setwidth(temp, set_initial);
            break;
        case 'C':
            ok = setcmdintr(request, pos);
            break;
        case 'T':
            ok = set_tabs(request, pos, set_initial);
            break;
        case 'M':
            ok = set_lrmargin(request, pos, set_initial);
            break;
        case 'V':
            ok = set_tbmargin(request, pos, set_initial);
            break;
        case 'K':
            ok = set_mode(request, pos);
            break;
        default:
            screen_message(MSG_INVALID_PARAMETER_CODE);
            return false;
        }
        if (!ok)
            return false;
        ch = nextchar(request, pos);
        if ((ch == ',') || (ch == '\0')) {
            ch = nextchar(request, pos);
        } else {
            screen_message(MSG_SYNTAX_ERROR_IN_PARAM_CMD);
            return false;
        }
    }
    return true;
}

void display_option(char ch, bool &first) {
    if (first)
        screen_write_ch(0, '(');
    else
        screen_write_ch(0, ',');
    screen_write_ch(0, ch);
    first = false;
}

void print_options(const frame_options &options) {
    int count = 1;
    bool first = true;
    screen_write_ch(0, ' ');
    if (options.contains(frame_options_elts::opt_auto_indent)) {
        display_option('I', first);
        count += 2;
    }
    if (options.contains(frame_options_elts::opt_auto_wrap)) {
        display_option('W', first);
        count += 2;
    }
    if (options.contains(frame_options_elts::opt_new_line)) {
        display_option('N', first);
        count += 2;
    }
    if (first) {
        const char *s = "  None    ";
        screen_write_str(0, s);
        count += std::strlen(s);
    } else {
        screen_write_ch(0, ')');
        count += 1;
    }
    screen_write_str(0, "                 ", 14 - count);
}

void print_margins(int m1, int m2) {
    screen_write_str(0, " (");
    screen_write_int(m1, 1);
    screen_write_ch(0, ',');
    screen_write_int(m2, 1);
    screen_write_ch(0, ')');
    int count = 6;
    if (m1 > 99)
        count += 2;
    else if (m1 > 9)
        count += 1;
    if (m2 > 99)
        count += 2;
    else if (m2 > 9)
        count += 1;
    screen_write_str(0, "               ", 14 - count);
}

bool frame_parameter(tpar_ptr tpar) {
    bool result = false;
    // with request do
    tpar->nxt = nullptr;
    tpar->con = nullptr;
    tpar_object request;
    if (!tpar_get_1(tpar, commands::cmd_frame_parameters, request))
        goto l99;
    if (request.len > 0) {
        result = setparam(request);
        goto l99;
    }
    // else display parameters and stats
    screen_unload();
    screen_home(true);
    // with current_frame^ do
    do {
        screen_home(false); // Dont clear the screen here !
        screen_write_str(0, " Ludwig ");
        for (int i = 0; i < 8; ++i)
            screen_write_ch(0, LUDWIG_VERSION[i]);
        screen_write_str(5, "Parameters      Frame: ");
        screen_write_name_str(0, current_frame->span->name, NAME_LEN);
        screen_writeln_clel();
        screen_write_str(0, " ===============     ==========      =====");
        screen_writeln_clel();
        screen_writeln_clel();
        screen_write_str(3, "Unused  memory available in frame    =");
        screen_write_int(current_frame->space_left, 9);
        screen_writeln_clel();
        screen_write_str(3, "The number of lines in this frame    =");
        line_range temp;
        if (line_to_number(current_frame->last_group->last_line, temp))
            temp -= 1;
        else
            temp = 0;
        screen_write_int(temp, 9);
        screen_writeln_clel();
        screen_write_str(3, "Lines read from input file so far    =");
        screen_write_int(current_frame->input_count, 9);
        screen_writeln_clel();
        screen_write_str(3, "Current Line number in this frame    =");
        if (!line_to_number(current_frame->dot->line, temp))
            temp = 0;
        screen_write_int(temp, 9);
        screen_writeln_clel();
        screen_writeln_clel();
        screen_write_str(9, "Parameters");
        screen_write_str(41, "Defaults");
        screen_writeln_clel();
        screen_write_str(9, "----------");
        screen_write_str(41, "--------");
        screen_writeln_clel();
        screen_write_str(3, "Keyboard Mode                      K =");
        switch (edit_mode) {
        case mode_type::mode_overtype:
            screen_write_str(1, "Overtype Mode");
            break;
        case mode_type::mode_insert:
            screen_write_str(1, "Insert Mode");
            break;
        case mode_type::mode_command:
            screen_write_str(1, "Command Mode");
            break;
        }
        screen_writeln_clel();
        if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
            screen_write_str(3, "Command introducer                 C = ");
            std::string key_name;
            if (user_key_code_to_name(command_introducer, key_name)) {
                screen_write_str(0, key_name.data(), key_name.size());
            } else {
                screen_write_ch(0, command_introducer);
            }
            screen_writeln_clel();
        }
        screen_write_str(3, "Maximum memory available in frame  S =");
        screen_write_int(current_frame->space_limit, 9);
        screen_write_str(5, "  --  ");
        screen_write_int(file_data.space, 9);
        screen_writeln_clel();
        screen_write_str(3, "Screen height  (lines displayed)   H =");
        screen_write_int(current_frame->scr_height, 9);
        screen_write_str(5, "  --  ");
        screen_write_int(initial_scr_height, 9);
        screen_writeln_clel();
        screen_write_str(3, "Screen width   (characters)        W =");
        screen_write_int(current_frame->scr_width, 9);
        screen_write_str(5, "  --  ");
        screen_write_int(initial_scr_width, 9);
        screen_writeln_clel();
        screen_write_str(3, "Editing options                    O =");
        print_options(current_frame->options);
        screen_write_str(0, "  --  ");
        print_options(initial_options);
        screen_writeln_clel();
        screen_write_str(3, "Horizontal margins                 M =");
        print_margins(current_frame->margin_left, current_frame->margin_right);
        screen_write_str(0, "  --  ");
        print_margins(initial_margin_left, initial_margin_right);
        screen_writeln_clel();
        screen_write_str(3, "Vertical margins                   V =");
        print_margins(current_frame->margin_top, current_frame->margin_bottom);
        screen_write_str(0, "  --  ");
        print_margins(initial_margin_top, initial_margin_bottom);
        screen_writeln_clel();
        screen_write_str(3, "Tab settings                       T =");
        screen_writeln_clel();
        for (int i = 1; i <= current_frame->scr_width; ++i) { // ### Scr_width is wrong
            if (i == current_frame->margin_left)
                screen_write_ch(0, 'L');
            else if (i == current_frame->margin_right)
                screen_write_ch(0, 'R');
            else if (current_frame->tab_stops[i])
                screen_write_ch(0, 'T');
            else
                screen_write_ch(0, ' ');
        }
        screen_writeln();
        screen_writeln_clel();
        screen_getlinep(NEW_VALUES, request.str, request.len, 1, 1);
        if (request.len > 0) {
            request.str.apply_n(ch_toupper, request.len);
            if (!setparam(request))
                screen_beep();
        }
    } while (!tt_controlc && request.len != 0);
    result = true;
l99:;
    tpar_clean_object(request);
    return result;
}
