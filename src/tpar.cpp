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
! Name:         TPAR
!
! Description:  Tpar maintenance.
!**/

#include "tpar.h"

#include "ch.h"
#include "screen.h"
#include "span.h"
#include "sys.h"
#include "var.h"

#include <iomanip>
#include <sstream>

const size_t ENQUIRY_NUM_LEN = 20;

namespace {
    const std::string SYSTEM_NAME("C++/Linux");

    enum class vartype { unknown, terminal, frame, opsys, ludwig };

    template <typename T> std::string left_padded(std::streamsize width, const T &value) {
        std::ostringstream oss;
        oss << std::setw(width) << std::setfill(' ') << value;
        return oss.str();
    }
}; // namespace

void discard_tp(tpar_ptr tp) {
    // with tp^ do
    if (tp->nxt != nullptr)
        discard_tp(tp->nxt);
    else if (tp->con != nullptr)
        discard_tp(tp->con);
    delete tp;
}

void tpar_clean_object(tpar_object &tp_o) {
    // with tp_o do
    if (tp_o.con != nullptr)
        discard_tp(tp_o.con);
    if (tp_o.nxt != nullptr)
        discard_tp(tp_o.nxt);
    tp_o.con = nullptr;
    tp_o.nxt = nullptr;
}

void tpar_duplicate_con(const_tpar_ptr tpar, tpar_object &tp_o) {
    tp_o = *tpar;
    tp_o.nxt = nullptr;
    tpar_ptr tp2 = nullptr;
    while (tpar->con != nullptr) {
        tpar = tpar->con;
        tpar_ptr tp = new tpar_object;
        *tp = *tpar;
#ifdef DEBUG
        if (tp->nxt != nullptr)
            screen_message(DBG_NXT_NOT_NIL);
#endif
        if (tp2 == nullptr)
            tp_o.con = tp;
        else
            tp2->con = tp;
        tp2 = tp;
    }
}

void tpar_duplicate(const_tpar_ptr from_tp, tpar_ptr &to_tp) {
    if (from_tp != nullptr) {
        to_tp = new tpar_object;
        tpar_duplicate_con(from_tp, *to_tp);
        from_tp = from_tp->nxt;
        tpar_ptr tmp_tp = to_tp;
        while (from_tp != nullptr) {
            tmp_tp->nxt = new tpar_object;
            tmp_tp = tmp_tp->nxt;
            tpar_duplicate_con(from_tp, *tmp_tp);
            from_tp = from_tp->nxt;
        }
    } else {
        to_tp = nullptr;
    }
}

bool tpar_to_mark(const tpar_object &strng, int &mark) {
    // with strng do
    if (strng.len == 0) {
        screen_message(MSG_ILLEGAL_MARK_NUMBER);
        return false;
    }
    char mch = strng.str[1];
    if (mch >= '0' && mch <= '9') {
        int i = 1;
        if (!tpar_to_int(strng, i, mark))
            return false;
        if ((i <= strng.len) || ((mark < 1) || (mark > MAX_MARK_NUMBER))) {
            screen_message(MSG_ILLEGAL_MARK_NUMBER);
            return false;
        }
    } else {
        if ((strng.len > 1) || (mch != '=' && mch != '%')) {
            screen_message(MSG_ILLEGAL_MARK_NUMBER);
            return false;
        }
        if (mch == '=')
            mark = MARK_EQUALS;
        else
            mark = MARK_MODIFIED;
    }
    return true;
}

bool tpar_to_int(const tpar_object &strng, int &chpos, int &int_) {
    // with strng do
    char ch = (chpos > strng.len) ? '\0' : strng.str[chpos];
    if (ch < '0' || ch > '9') {
        screen_message(MSG_INVALID_INTEGER);
        return false;
    }
    int number = 0;
    do {
        int digit = ch - '0';
        if (number <= (MAXINT - digit) / 10) {
            number *= 10;
            number += digit;
        } else {
            screen_message(MSG_INVALID_INTEGER);
            return false;
        }
        chpos += 1;
        if (chpos > strng.len)
            ch = '\0';
        else
            ch = strng.str[chpos];
    } while (ch >= '0' && ch <= '9');
    int_ = number;
    return true;
}

bool tpar_substitute(tpar_object &tpar, user_commands cmd, tpcount_type this_tp) {
    // with tpar do
    if (tpar.con != nullptr) {
        screen_message(MSG_SPAN_NAMES_ARE_ONE_LINE);
        return false;
    }
    // Get the Span name
    std::string name(tpar.str.data(), tpar.len);
    // and Up Case it
    std::transform(name.begin(), name.end(), name.begin(), ch_toupper);
    span_ptr span;
    span_ptr dummy;
    if (span_find(name, span, dummy)) {
        tpar.dlm = '\0';
        mark_object start_mark = *(span->mark_one);
        mark_object end_mark = *(span->mark_two);
        if (start_mark.line == end_mark.line) {
            tpar.len = end_mark.col - start_mark.col;
            int srclen;
            if (start_mark.col > start_mark.line->used) // Span in thin air
                srclen = 0;
            else if (end_mark.col > end_mark.line->used)
                srclen = end_mark.line->used - start_mark.col + 1;
            else
                srclen = tpar.len;
            // with start_mark do
            tpar.str.fillcopy(start_mark.line->str->data(start_mark.col), srclen, 1, tpar.len, ' ');
        } else if (!cmd_attrib.at(cmd).tpar_info[this_tp].ml_allowed) {
            screen_message(MSG_SPAN_MUST_BE_ONE_LINE);
            return false;
        } else {
            // copy entire span into a tpar
            // with start_mark do
            if (start_mark.col > start_mark.line->used) // Thin air span
                tpar.len = 0;
            else
                tpar.len = start_mark.line->used - start_mark.col + 1;
            tpar.str.copy(*start_mark.line->str, start_mark.col, tpar.len);
            // Anything between the start and end marks?
            tpar_ptr tmp_tp = nullptr;
            start_mark.line = start_mark.line->flink;
            while (start_mark.line != end_mark.line) {
                tpar_ptr tmp_tp_2 = new tpar_object;
                if (tmp_tp == nullptr)
                    tpar.con = tmp_tp_2;
                else
                    tmp_tp->con = tmp_tp_2;
                tmp_tp = tmp_tp_2;
                // with tmp_tp^ do
                tmp_tp->dlm = '\0';
                tmp_tp->nxt = nullptr;
                tmp_tp->con = nullptr;
                tmp_tp->len = start_mark.line->used;
                tmp_tp->str.copy(*start_mark.line->str, 1, tmp_tp->len);
                start_mark.line = start_mark.line->flink;
            }
            // with end_mark do
            //  create new tpar
            tpar_ptr tmp_tp_2 = new tpar_object;
            if (tmp_tp == nullptr)
                tpar.con = tmp_tp_2;
            else
                tmp_tp->con = tmp_tp_2;
            tmp_tp = tmp_tp_2;
            tmp_tp->dlm = '\0';
            tmp_tp->nxt = nullptr;
            tmp_tp->con = nullptr;
            tmp_tp->len = end_mark.col - 1;
            tmp_tp->str.fillcopy(end_mark.line->str->data(), end_mark.line->used, 1, tpar.len, ' ');
        }
    } else {
        screen_message(MSG_NO_SUCH_SPAN);
        return false;
    }
    return true;
}

bool find_enquiry(const std::string &name, str_object &result, strlen_range &reslen) {
    bool enquiry_result = false;
    vartype variable_type = vartype::unknown;
    std::string item;
    int i = 0;
    strlen_range len = name.size();
    while ((i < len) && (name[i] != '-')) {
        item.push_back(ch_toupper(name[i]));
        i += 1;
    }
    if (i < len && name[i] == '-') {
        i += 1;
        if (item == "TERMINAL")
            variable_type = vartype::terminal;
        else if (item == "FRAME")
            variable_type = vartype::frame;
        else if (item == "ENV")
            variable_type = vartype::opsys;
        else if (item == "LUDWIG")
            variable_type = vartype::ludwig;
        item.clear();
        while (i < len) {
            if (variable_type == vartype::opsys)
                item.push_back(name[i]);
            else
                item.push_back(std::toupper(name[i]));
            i += 1;
        }
        switch (variable_type) {
        case vartype::terminal:
            enquiry_result = true;
            if (item == "NAME") {
                reslen = terminal_info.name.size();
                result.fillcopy(terminal_info.name.data(), reslen, 1, MAX_STRLEN, ' ');
            } else if (item == "HEIGHT") {
                std::string s = left_padded(ENQUIRY_NUM_LEN, terminal_info.height);
                reslen = s.size();
                result.fillcopy(s.data(), reslen, 1, MAX_STRLEN, ' ');
            } else if (item == "WIDTH") {
                std::string s = left_padded(ENQUIRY_NUM_LEN, terminal_info.width);
                reslen = s.size();
                result.fillcopy(s.data(), reslen, 1, MAX_STRLEN, ' ');
            } else if (item == "SPEED") {
                // FIXME: Maybe we could do something better here?
                std::string s = left_padded(ENQUIRY_NUM_LEN, 0);
                reslen = s.size();
                result.fillcopy(s.data(), reslen, 1, MAX_STRLEN, ' ');
            } else {
                enquiry_result = false;
            }
            break;

        case vartype::frame:
            enquiry_result = true;
            if (item == "NAME") {
                reslen = current_frame->span->name.size();
                result.fillcopy(current_frame->span->name.data(), reslen, 1, MAX_STRLEN, ' ');
            } else if (item == "INPUTFILE") {
                if (current_frame->input_file < 0) {
                    reslen = 0;
                } else {
                    reslen = files[current_frame->input_file]->filename.size();
                    result.fillcopy(
                        files[current_frame->input_file]->filename.data(),
                        reslen,
                        1,
                        MAX_STRLEN,
                        ' '
                    );
                }
            } else if (item == "OUTPUTFILE") {
                if (current_frame->output_file < 0) {
                    reslen = 0;
                } else {
                    reslen = files[current_frame->output_file]->filename.size();
                    result.fillcopy(
                        files[current_frame->output_file]->filename.data(),
                        reslen,
                        1,
                        MAX_STRLEN,
                        ' '
                    );
                }
            } else if (item == "MODIFIED") {
                reslen = 1;
                result.fill(' ');
                if (current_frame->text_modified)
                    result[1] = 'Y';
                else
                    result[1] = 'N';
            } else {
                enquiry_result = false;
            }
            break;

        case vartype::opsys:
            {
                std::string env;
                enquiry_result = sys_getenv(item, env);
                if (enquiry_result) {
                    reslen = str_object::index_type::size();
                    if (env.size() < str_object::index_type::size())
                        reslen = env.size();
                    result.fill(' ');
                    result.copy_n(env.data(), reslen);
                }
            }
            break;

        case vartype::ludwig:
            enquiry_result = true;
            if (item == "VERSION") {
                result.fillcopy(ludwig_version.data(), ludwig_version.size(), 1, MAX_STRLEN, ' ');
                reslen = result.length(' ');
            } else if (item == "OPSYS") {
                result.fillcopy(SYSTEM_NAME.data(), SYSTEM_NAME.size(), 1, MAX_STRLEN, ' ');
                reslen = result.length(' ');
            } else if (item == "COMMAND_INTRODUCER") {
                if (!PRINTABLE_SET.test(command_introducer.value())) {
                    reslen = 0;
                    screen_message(MSG_NONPRINTABLE_INTRODUCER);
                } else {
                    reslen = 1;
                    result[1] = command_introducer;
                }
            } else if (item == "INSERT_MODE") {
                reslen = 1;
                if ((edit_mode == mode_type::mode_insert) ||
                    ((edit_mode == mode_type::mode_command) &&
                     (previous_mode == mode_type::mode_insert)))
                    result[1] = 'Y';
                else
                    result[1] = 'N';
            } else if (item == "OVERTYPE_MODE") {
                reslen = 1;
                if ((edit_mode == mode_type::mode_overtype) ||
                    ((edit_mode == mode_type::mode_command) &&
                     (previous_mode == mode_type::mode_overtype)))
                    result[1] = 'Y';
                else
                    result[1] = 'N';
            } else {
                enquiry_result = false;
            }
            break;

        case vartype::unknown:
            // Nothing to do
            break;
        }
    }
    return enquiry_result;
}

bool tpar_enquire(tpar_object &tpar) {
    // with tpar do
    tpar.dlm = '\0';
    std::string name(tpar.str.data(), tpar.len);
    if (find_enquiry(name, tpar.str, tpar.len)) {
        return true;
    } else {
        screen_message(MSG_UNKNOWN_ITEM);
        exit_abort = true;
    }
    return false;
}

bool tpar_analyse(user_commands cmd, tpar_object &tran, int depth, tpcount_type this_tp) {
    if (depth > MAX_TPAR_RECURSION) {
        screen_message(MSG_TPAR_TOO_DEEP);
        return false;
    }
    // with tran do
    if (tran.dlm != TPD_SMART && tran.dlm != TPD_EXACT && tran.dlm != TPD_LIT) {
        bool ended = false;
        do {
            char delim = tran.dlm; // Save copy of delimiter in case of recursive call.
            if (tran.con == nullptr) {
                if (tran.len > 1) {
                    char ts1 = tran.str[1];
                    if ((ts1 == tran.str[tran.len]) &&
                        (ts1 == TPD_SPAN || ts1 == TPD_PROMPT || ts1 == TPD_ENVIRONMENT ||
                         ts1 == TPD_SMART || ts1 == TPD_EXACT || ts1 == TPD_LIT)) {
                        // nested delimiters
                        tran.dlm = ts1;
                        tran.len -= 2;
                        tran.str.erase(1, 1);
                        if (!tpar_analyse(cmd, tran, depth + 1, this_tp))
                            return false;
                    }
                }
            } else {
                tpar_ptr tmp_tp = tran.con;
                while (tmp_tp->con != nullptr)
                    tmp_tp = tmp_tp->con;
                if ((tran.len != 0) && (tmp_tp->len != 0)) {
                    char ts1 = tran.str[1];
                    if ((ts1 == tmp_tp->str[tmp_tp->len]) &&
                        (ts1 == TPD_SPAN || ts1 == TPD_PROMPT || ts1 == TPD_ENVIRONMENT ||
                         ts1 == TPD_SMART || ts1 == TPD_EXACT || ts1 == TPD_LIT)) {
                        // nested delimiters
                        tran.dlm = ts1;
                        tran.len -= 1;
                        tran.str.erase(1, 1);
                        tmp_tp->len -= 1;
                        if (!tpar_analyse(cmd, tran, depth + 1, this_tp))
                            return false;
                    }
                }
            }
            if (delim == TPD_SPAN) {
                if (!tpar_substitute(tran, cmd, this_tp))
                    return false;
            } else if (delim == TPD_ENVIRONMENT) {
                if (file_data.old_cmds) {
                    screen_message(MSG_RESERVED_TPD);
                    return false;
                } else {
                    if (!tpar_enquire(tran))
                        return false;
                }
            } else if (delim == TPD_PROMPT) {
                str_object buffer;
                if (ludwig_mode != ludwig_mode_type::ludwig_batch) {
                    if (cmd == commands::cmd_verify) {
                        verify_response verify_reply;
                        if (tran.len == 0) {
                            // with cmd_attrib[cmd].tpar_info[this_tp] do
                            const auto &prompt{
                                dflt_prompts.at(cmd_attrib.at(cmd).tpar_info[this_tp].prompt_name)
                            };
                            verify_reply = screen_verify(prompt);
                        } else {
                            // FIXME: messy cast
                            const std::string_view prompt{
                                tran.str.data(), static_cast<size_t>(tran.len)
                            };
                            verify_reply = screen_verify(prompt);
                        }
                        switch (verify_reply) {
                        case verify_response::verify_reply_yes:
                            tran.str[1] = 'Y';
                            break;
                        case verify_response::verify_reply_no:
                            tran.str[1] = 'N';
                            break;
                        case verify_response::verify_reply_always:
                            tran.str[1] = 'A';
                            break;
                        case verify_response::verify_reply_quit:
                            tran.str[1] = 'Q';
                            break;
                        }
                        tran.len = 1;
                    } else if (tran.len == 0) {
                        // change first str and len with cmd values
                        // with cmd_attrib[cmd].tpar_info[this_tp] do
                        const auto &prompt{
                            dflt_prompts.at(cmd_attrib.at(cmd).tpar_info[this_tp].prompt_name)
                        };
                        screen_getlinep(
                            prompt, tran.str, tran.len, cmd_attrib.at(cmd).tpcount, this_tp
                        );
                    } else {
                        if (tran.con != nullptr) {
                            screen_message(MSG_PROMPTS_ARE_ONE_LINE);
                            return false;
                        } else {
                            // FIXME: messy cast
                            const std::string_view prompt{
                                tran.str.data(), static_cast<size_t>(tran.len)
                            };
                            screen_getlinep(
                                prompt, tran.str, tran.len, cmd_attrib.at(cmd).tpcount, this_tp
                            );
                        }
                    }
                    tran.dlm = '\0';
                } else {
                    screen_message(MSG_INTERACTIVE_MODE_ONLY);
                    return false;
                }
            } else {
                ended = true;
            }
        } while (!ended && !tt_controlc);
    }
    return !tt_controlc;
}

void trim(tpar_object &request) {
    if (request.len > 0) {
        // Find first non-blank character
        int i = 0;
        do {
            i += 1;
        } while (request.str[i] == ' ' && i != request.len);
        // with request do
        request.len -= i - 1;
        if (request.len > 0) {
            request.str.erase(i - 1, 1);
            request.str.apply_n(ch_toupper, request.len);
        }
        if (request.len >= 0 && size_t(request.len) < str_object::index_type::size())
            request.str.fill(' ', request.len + 1);
    }
}

bool tpar_get_1(const_tpar_ptr tpar, user_commands cmd, tpar_object &tran) {
#ifdef DEBUG
    if (tpar == nullptr) {
        screen_message(DBG_TPAR_NIL);
        return false;
    }
#endif
    tpar_duplicate_con(tpar, tran);

    if (tpar_analyse(cmd, tran, 1, 0)) {
        if (cmd_attrib.at(cmd).tpar_info[0].trim_reply)
            trim(tran);
        return true;
    }
    return false;
}

bool tpar_get_2(const_tpar_ptr tpar, user_commands cmd, tpar_object &trn1, tpar_object &trn2) {
#ifdef DEBUG
    if (tpar == nullptr) {
        screen_message(DBG_TPAR_NIL);
        return false;
    }
    if (tpar->nxt == nullptr) {
        screen_message(DBG_TPAR_NIL);
        return false;
    }
#endif

    tpar_duplicate_con(tpar, trn1);
    tpar_duplicate_con(tpar->nxt, trn2);

    if (!tpar_analyse(cmd, trn1, 1, 0))
        return false;
    if (trn1.len != 0) {
        if (!tpar_analyse(cmd, trn2, 1, 1))
            return false;
    }
    if (cmd_attrib.at(cmd).tpar_info[0].trim_reply)
        trim(trn1);
    if (cmd_attrib.at(cmd).tpar_info[1].trim_reply)
        trim(trn2);
    return true;
}
