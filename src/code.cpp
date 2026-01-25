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
! Name:         CODE
!
! Description:  Ludwig compiler and interpreter.
!**/

#include "code.h"

#include "var.h"
#include "vdu.h"
#include "exec.h"
#include "line.h"
#include "mark.h"
#include "tpar.h"
#include "frame.h"
#include "screen.h"

#include <cstring>

namespace {
    const penumset<commands> INTERP_CMDS({commands::cmd_pcjump, commands::cmd_exitto,
                commands::cmd_failto, commands::cmd_iterate, commands::cmd_exit_success,
                commands::cmd_exit_fail, commands::cmd_exit_abort, commands::cmd_extended,
                commands::cmd_verify, commands::cmd_noop});

    const accept_set_type &punct() {
        // Here to ensure initialised after PRINTABLE_SET et al
        static const accept_set_type PUNCT = accept_set_type(PRINTABLE_SET)
                                             .remove(ALPHA_SET)
                                             .remove(NUMERIC_SET)
                                             .remove(SPACE_SET);
        return PUNCT;
    }

    template <class R>
    void assign(parray<char, R>& dst, const char *src) {
        dst.fillcopy(src, std::strlen(src),
                     parray<char, R>::index_type::min(),
                     parray<char, R>::index_type::size(), ' ');
    }

    struct parse_state {
        // No copying
        parse_state() = default;
        parse_state(const parse_state&) = delete;
        parse_state &operator=(const parse_state&) = delete;
        std::string    status;
        key_code_range key;
        bool           eoln;          // Used to signal end of line
        int            pc;            // This is always an offset from code_top
        code_idx       code_base;     // Base in code array for new code
        mark_object    currentpoint;
        mark_object    startpoint;
        mark_object    endpoint;
        int            verify_count;
        bool           from_span;
    };
}

void code_discard(code_ptr &code_head) {
    // This routine releases the specified code and compacts the code array.
    // The code_head is set to NIL.

    //with code_head^ do
    code_head->ref -= 1;
    if (code_head->ref == 0) {
        code_idx start = code_head->code.value();
        code_idx size  = code_head->len;

        for (code_idx source = start; source < start + size; ++source) {
            //with compiler_code[source] do
            if (compiler_code[source].code != nullptr)
                code_discard(compiler_code[source].code);
            if (compiler_code[source].tpar != nullptr)
                tpar_clean_object(*compiler_code[source].tpar);
        }
        for (code_idx source = start + size; source <= code_top; ++source) {
            compiler_code[source - size] = compiler_code[source];
        }
        code_top -= size;

        code_ptr link = code_head->blink;
        while (link != code_list) {
            //with link^ do
            link->code -= size;
            link = link->blink;
        }

        code_head->flink->blink = code_head->blink;
        code_head->blink->flink = code_head->flink;

        delete code_head;
        code_head = nullptr;
    }
}

void error(parse_state &ps, const char *err_text) {
    // Inserts an error message into the span where it was detected.

    ps.status = MSG_SYNTAX_ERROR;
    if (ps.from_span) {
        // If possible, backup the current point one character.
        //with currentpoint do
        if (ps.currentpoint.line != ps.startpoint.line) {
            if (ps.currentpoint.col > 1) {
                ps.currentpoint.col -= 1;
            } else {
                ps.currentpoint.line = ps.currentpoint.line->blink;
                ps.currentpoint.col  = ps.currentpoint.line->used + 1;
                if (ps.currentpoint.col > 1)
                    ps.currentpoint.col -= 1;
                if (ps.currentpoint.line == ps.startpoint.line) {
                    if (ps.currentpoint.col < ps.startpoint.col)
                        ps.currentpoint.col = ps.startpoint.col;
                }
            }
        } else if (ps.currentpoint.col > ps.startpoint.col) {
            ps.currentpoint.col -= 1;
        }

        // Insert the error message into the span.
        if (ludwig_mode == ludwig_mode_type::ludwig_screen) {
            if (!frame_edit(ps.currentpoint.line->group->frame->span->name))
                return;
            //with current_frame^ do
            if (current_frame->marks[0] != nullptr) {
                if (!mark_destroy(current_frame->marks[0]))
                    return;
            }
            line_ptr e_line;
            if (!lines_create(1, e_line, e_line))
                return;
            //with currentpoint do
            str_object str(' ');
            int i = ps.currentpoint.col;
            str[i] = '!';
            if (i < MAX_STRLEN) {
                i += 1;
                str[i] = ' ';
            }
            while ((i < MAX_STRLEN) && *err_text) {
                i += 1;
                str[i] = *err_text++;
            }
            if (!line_change_length(e_line, i))
                return;
            // "i" can't be zero here, so e_line->str != nullptr
            e_line->str->copy(str, 1, i);
            e_line->used = str.length(' ', i);
            if (!lines_inject(e_line, e_line, ps.currentpoint.line))
                return;
            if (!mark_create(e_line, ps.currentpoint.col, current_frame->dot))
                return;
        }
    }
}

bool nextkey(parse_state &ps) {
    ps.eoln = false;
    if (!ps.from_span) {
        ps.key = vdu_get_key();
        if (tt_controlc)
            return false;
    } else {
        //with currentpoint do
        if ((ps.currentpoint.line == ps.endpoint.line) && (ps.currentpoint.col == ps.endpoint.col)) {
            ps.key = 0; // finished span
        } else {
            //with line^ do
            if (ps.currentpoint.col <= ps.currentpoint.line->used) {
                ps.key = ps.currentpoint.line->str->operator[](ps.currentpoint.col);
                ps.currentpoint.col += 1;
            } else if (ps.currentpoint.line != ps.endpoint.line) {
                ps.key  = ' ';
                ps.eoln = true;
                ps.currentpoint.line = ps.currentpoint.line->flink;
                ps.currentpoint.col  = 1;
            } else {
                ps.key = 0;   // finished the span
            }
        }
    }
    return true;
}

bool nextnonbl(parse_state &ps) {
 l1:;
    do {
        if (!nextkey(ps))
            return false;
        if (ps.from_span) {
            //with currentpoint,line^ do
            if ((ps.key == '<') && (ps.currentpoint.col <= ps.currentpoint.line->used)) {
                if (ps.currentpoint.line->str->operator[](ps.currentpoint.col) == '>')
                    ps.key = 0;
            }
        }
    } while (ps.key == ' ');
    if (ps.key == '!') { // Comment - throw away rest of line.
        if (ps.from_span) {
            //with currentpoint do
            ps.currentpoint.col = ps.currentpoint.line->used + 1;
            goto l1;
        } else {
            ps.status = MSG_COMMENTS_ILLEGAL;
            return false;
        }
    }
    return true;
}

bool generate(parse_state &ps, leadparam irep, int icnt, commands iop, tpar_ptr itpar, int ilbl, code_ptr icode) {
    ps.pc += 1;
    if (ps.code_base + ps.pc > MAX_CODE) {
        ps.status = MSG_COMPILER_CODE_OVERFLOW;
        return false;
    }
    //with compiler_code[code_base + pc]
    code_object &cc(compiler_code[ps.code_base + ps.pc]);
    cc.rep  = irep;
    cc.cnt  = icnt;
    cc.op   = iop;
    cc.tpar = itpar;
    cc.lbl  = ilbl;
    cc.code = icode;
    return true;
}

void poke(code_idx code_base, code_idx location, code_idx newlabel) {
    compiler_code[code_base + location].lbl = newlabel;
}

bool getcount(parse_state &ps, int &repcount) {
    const int MAX_REPCOUNT = 65535;

    if (ps.key >= '0' && ps.key <= '9') {
        repcount = 0;
        do {
            int digit = ps.key - '0';
            if (repcount <= (MAX_REPCOUNT - digit) / 10) {
                repcount = repcount * 10 + digit;
            } else {
                error(ps, "Count too large");
                return false;
            }
            if (!nextkey(ps))
                return false;
        } while (ps.key >= '0' && ps.key <= '9');
    } else {
        repcount = 1;
    }
    return true;
}

bool scan_leading_param(parse_state &ps, leadparam &repsym, int &repcount) {
    switch (ps.key) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        repsym = leadparam::pint;
        if (!getcount(ps, repcount))
            return false;
        break;

    case '+':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::plus;
        repcount = 1;
        if (ps.key >= '0' && ps.key <= '9') {
            repsym = leadparam::pint;
            if (!getcount(ps, repcount))
                return false;
        }
        break;

    case '-':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::minus;
        repcount = -1;
        if (ps.key >= '0' && ps.key <= '9') {
            repsym = leadparam::nint;
            if (!getcount(ps, repcount))
                return false;
            repcount = -repcount;
        }
        break;

    case '>':
    case '.':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::pindef;
        repcount = 0;
        break;

    case '<':
    case ',':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::nindef;
        repcount = 0;
        break;

    case '@':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::marker;
        if (!getcount(ps, repcount))
            return false;
        if ((repcount <= 0) || (repcount > MAX_MARK_NUMBER)) {
            error(ps, "Illegal mark number");
            return false;
        }
        break;

    case '=':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::marker;
        repcount = MARK_EQUALS;
        break;

    case '%':
        if (!nextkey(ps))
            return false;
        repsym = leadparam::marker;
        repcount = MARK_MODIFIED;
        break;

    default:
        repsym = leadparam::none;
        repcount = 1;
        break;
    }
    return true;
}

bool scan_trailing_param(parse_state &ps, commands command, leadparam repsym, tpar_ptr &tparam) {
    tpcount_type tc = cmd_attrib[command].tpcount;
    tparam = nullptr;
    // Some commands only take trailing parameters when repcount is +ve
    if (tc < 0) {
        if (repsym == leadparam::minus)
            tc = 0;
        else
            tc = -tc;
    }
    if (tc > 0) {
        if (!nextkey(ps))
            return false;
        key_code_range pardelim = ps.key;
        if (ps.key < accept_set_type::element_type::min() ||
            ps.key > accept_set_type::element_type::max() ||
            !punct().contains(pardelim.value())) {
            error(ps, "Illegal parameter delimiter");
            return false;
        }
        tpar_ptr tpl = nullptr;
        for (tpcount_type tci = 1; tci <= tc; ++tci) {
            do {
                int parlength = 0;
                str_object parstring;
                do {
                    if (!nextkey(ps))
                        return false;
                    if (ps.key == 0) {
                        error(ps, "Missing trailing delimiter");
                        return false;
                    }
                    parlength += 1;
                    parstring[parlength] = ps.key;
                } while (!ps.eoln && ps.key != pardelim);
                parlength -= 1;
                if (ps.eoln && !cmd_attrib[command].tpar_info[tci].ml_allowed) {
                    error(ps, "Missing trailing delimiter");
                    return false;
                }
                tpar_ptr tp = new tpar_object;
                //with tp^ do
                tp->len = parlength;
                tp->dlm = pardelim;
                tp->str = parstring;
                tp->nxt = nullptr;
                tp->con = nullptr;
                if (tparam == nullptr) {
                    // 1st time through
                    tparam = tp;
                    tpl = tp;
                } else {
                    if (tpl != nullptr) {
                        tpl->con = tp;
                        tpl = tp;
                    } else {
                        tparam->nxt = tp;
                        tpl = tp;
                    }
                }
            } while (ps.key != pardelim);
            tpl = nullptr;
        }
    }
    return true;
}

bool scan_command(parse_state &ps, bool full_scan);

bool scan_exit_handler(parse_state &ps, int pc1, int &pc4, bool full_scan) {
    if (!nextnonbl(ps))
        return false;
    if (ps.key == '[') {
        if (!nextnonbl(ps))
            return false;
        while ((ps.key != ':') && (ps.key != ']')) {
            // Construct exit part.
            if (!scan_command(ps, full_scan))
                return false;
        }
        if (ps.key == ':') {
            // Jump over fail handler.
            if (!generate(ps, leadparam::none, 0, commands::cmd_pcjump, nullptr, 0, nullptr))
                return false;
            pc4 = ps.pc;
            poke(ps.code_base, pc1, ps.pc + 1); // Set fail label for command
            if (!nextnonbl(ps))
                return false;
            while (ps.key != ']') {
                // Construct fail part.
                if (!scan_command(ps, full_scan))
                    return false;
            }
            poke(ps.code_base, pc4, ps.pc + 1);     // End of fail handler.
        } else {
            poke(ps.code_base, pc1, ps.pc + 1);     // Set fail label
        }
        if (!nextnonbl(ps))
            return false;
    }
    return true;
}

bool scan_simple_command(parse_state &ps,
                         const commands command, const leadparam repsym, int &repcount, tpar_ptr &tparam,
                         code_ptr &lookup_code, int &pc1, bool full_scan) {

    if (!cmd_attrib[command].lp_allowed.contains(repsym)) {
        error(ps, "Illegal leading parameter");
        return false;
    }
    if (command == commands::cmd_verify) {
        ps.verify_count += 1;
        if (ps.verify_count > MAX_VERIFY) {
            error(ps, "Too many verify commands in span");
            return false;
        }
        repcount = ps.verify_count;
    }

    lookup_code = lookup[ps.key].code;
    if (lookup[ps.key].tpar == nullptr) {
        if (cmd_attrib[command].tpcount != 0) {
            if (full_scan) {
                if (!scan_trailing_param(ps, command, repsym, tparam))
                    return false;
            } else {
                tparam = new tpar_object;
                //with tparam^ do
                tparam->len = 0;
                tparam->dlm = TPD_PROMPT;
                tparam->nxt = nullptr;
                tparam->con = nullptr;
                tpar_ptr tmp_tp = tparam;
                for (int i = 2; i <= cmd_attrib[command].tpcount; ++i) {
                    tmp_tp->nxt = new tpar_object;
                    tmp_tp = tmp_tp->nxt;
                    //with tmp_tp^ do
                    tmp_tp->len = 0;
                    tmp_tp->dlm = TPD_PROMPT;
                    tmp_tp->nxt = nullptr;
                    tmp_tp->con = nullptr;
                }
            }
        } else {
            tparam = nullptr;
        }
    } else {
        tpar_duplicate(lookup[ps.key].tpar, tparam);
    }
    if (lookup_code != nullptr)
        lookup_code->ref += 1;
    if (!generate(ps, repsym, repcount, command, tparam, 0, lookup_code))
        return false;
    pc1 = ps.pc;
    return true;
}

bool scan_compound_command(parse_state &ps, leadparam repsym, int repcount, int &pc1, int &pc2, int &pc3) {
    if (repsym != leadparam::none && repsym != leadparam::plus && repsym != leadparam::pint && repsym != leadparam::pindef) {
        error(ps, "Illegal leading parameter");
        return false;
    }
    if (!generate(ps, leadparam::none, 0, commands::cmd_exitto, nullptr, 0, nullptr))
        return false;
    pc2 = ps.pc;
    if (!generate(ps, leadparam::none, 0, commands::cmd_failto, nullptr, 0, nullptr))
        return false;
    pc1 = ps.pc;
    pc3 = ps.pc + 1;
    if (repsym != leadparam::pindef) {
        if (!generate(ps, leadparam::none, repcount, commands::cmd_iterate, nullptr, 0, nullptr))
            return false;
    }
    if (!nextnonbl(ps))
        return false;
    do {
        if (!scan_command(ps, true))
            return false;
    } while (ps.key != ')');
    if (!generate(ps, leadparam::none, 0, commands::cmd_pcjump, nullptr, pc3, nullptr))
        return false;
    poke(ps.code_base, pc2, ps.pc + 1);    // Fill in exit label.
    return true;
}

bool scan_command(parse_state &ps, bool full_scan) {
    int repcount;
    leadparam repsym;
    if (!scan_leading_param(ps, repsym, repcount))
        return false;
    if (ps.key >= accept_set_type::element_type::min() && ps.key <= accept_set_type::element_type::max() &&
        LOWER_SET.contains(ps.key.value()))
        ps.key = std::toupper(ps.key);
    commands command = lookup[ps.key].command;
    while (prefixes.contains(command)) {
        if (!nextkey(ps))
            return false;
        if (ps.key < 0) {
            error(ps, "Command not valid");
            return false;
        }
        const expand_lim_range *p = lookupexp_ptr.data(command);
        int i = p[0];
        int j = p[1];
//        int i  = lookupexp_ptr[command];
//        int j  = lookupexp_ptr[command + 1];
        while ((i < j) && (std::toupper(ps.key) != lookupexp[i].extn))
            i += 1;
        if (i < j) {
            command = lookupexp[i].command;
        } else {
            error(ps, "Command not valid");
            return false;
        }
    }
    int pc1;
    if (ps.key == '(') {
        int pc2, pc3;
        if (!scan_compound_command(ps, repsym, repcount, pc1, pc2, pc3))
            return false;
    } else if (command != commands::cmd_noop) {
        // FIXME: These can probably be declared in scan_simple_command
        tpar_ptr tparam;
        code_ptr lookup_code;
        if (!scan_simple_command(ps, command, repsym, repcount, tparam, lookup_code, pc1, full_scan))
            return false;
    } else {
        error(ps, "Command not valid");
        return false;
    }
    if (full_scan) {
        int pc4;
        if (!scan_exit_handler(ps, pc1, pc4, full_scan))
            return false;
    }
    return true;
}

bool code_compile(span_object &span, bool from_span) {
    bool result = false;
    parse_state ps;
    ps.status.clear();
    ps.eoln = false;
    ps.from_span = from_span;

    //with span do
    if (from_span) {
        ps.startpoint   = *span.mark_one;  // Make Local Copies of the
        ps.endpoint     = *span.mark_two;  // PHYSICAL marks.
        ps.currentpoint = ps.startpoint;
    }
    if (span.code != nullptr)
        code_discard(span.code);

    ps.code_base = code_top;
    ps.pc = 0;      // This will be incremented before code is written.
    ps.verify_count = 0;
    if (!nextnonbl(ps))
        goto l99;
    if (ps.key == 0) {
        error(ps, "Span contains no commands");
        goto l99;
    }
    if (from_span) {
        do {
            if (!scan_command(ps, true))
                goto l99;
        } while (ps.key != 0);
    } else if (!scan_command(ps, false))
        goto l99;

    if (!generate(ps, leadparam::pint, 1, commands::cmd_exit_success, nullptr, 0, nullptr))
        goto l99;

    // Fill in code header.
    span.code = new code_header;
    //with span do
    //with code^ do
    span.code->ref   = 1;
    span.code->code  = ps.code_base + 1;
    span.code->len   = ps.pc;
    span.code->flink = code_list->flink; // Link it into chain.
    span.code->blink = code_list;
    code_list->flink->blink = span.code;
    code_list->flink        = span.code;
    code_top = ps.code_base + ps.pc;
    result = true;
l99:
    if (!ps.status.empty()) {
        exit_abort = true;
        screen_message(ps.status);
    }
    return result;
}


bool code_interpret(leadparam rept, int count, code_ptr code_head, bool from_span) {
    struct labels_type {
        code_idx exitlabel;
        code_idx faillabel;
        int      count;
    };

    parray<labels_type, prange<1, 100>> labels;

    bool result = false;
    tpar_object request;
    request.nxt = nullptr;
    request.con = nullptr;

    code_head->ref += 1;

    if (rept == leadparam::pindef)
        count = -1;
    enum { success, failure, failforever} interp_status = success;
    verify_array verify_always = INITIAL_VERIFY;

    while ((count != 0) && (interp_status == success)) {
        count -= 1;
        int level = 1;
        //with labels[1] do begin
        labels[1].exitlabel = 0;
        labels[1].faillabel = 0;
        labels[1].count = 0;
        code_idx pc = 1;
        do {
#ifdef DEBUG
            if (pc > code_head->len) {
                screen_message(DBG_PC_OUT_OF_RANGE);
                goto l99;
            }
#endif
            interp_status = success;
            // Note! code_head->code may be changed by a span compilation/creation.
            //with compiler_code[code_head->code-1 + pc] do
            const auto &cc(compiler_code[code_head->code - 1 + pc]);
            code_idx  curr_lbl  = cc.lbl;                // label field
            commands  curr_op   = cc.op;                 // op-code
            leadparam curr_rep  = cc.rep;                // repeat count type
            int       curr_cnt  = cc.cnt;                // repeat count value
            tpar_ptr  curr_tpar = cc.tpar;               // trailing parameter record ptr
            code_ptr  curr_code = cc.code;
            pc += 1;

            if (INTERP_CMDS.contains(curr_op)) {
                switch (curr_op) {
                case commands::cmd_pcjump:
                    pc = curr_lbl;
                    break;

                case commands::cmd_exitto:
                    from_span = true; // This is done to fix \n(...) from being Not From_Span to From_Span
                    level += 1;
                    //with labels[level] do
                    labels[level].exitlabel = curr_lbl;
                    labels[level].faillabel = 0;
                    labels[level].count     = 0;
                    break;

                case commands::cmd_failto:
                    labels[level].faillabel = curr_lbl;
                    break;

                case commands::cmd_iterate:
                    if (labels[level].count == curr_cnt) {
                        pc = labels[level].exitlabel;
                        level -= 1;
                    } else {
                        labels[level].count += 1;
                    }
                    break;

                case commands::cmd_exit_success:
                    if (curr_rep == leadparam::pindef)
                        curr_cnt = level;
                    if (curr_cnt > 0) {
                        if (curr_cnt >= level)
                            level = 0;
                        else
                            level -= curr_cnt;
                    }
                    pc = labels[level + 1].exitlabel;
                    break;

                case commands::cmd_exit_fail:
                    interp_status = failure;
                    if (curr_rep == leadparam::pindef)
                        curr_cnt = level;
                    if (curr_cnt > 0) {
                        if (curr_cnt >= level)
                            level = 0;
                        else
                            level -= curr_cnt;
                    }
                    pc = labels[level + 1].faillabel;
                    break;

                case commands::cmd_exit_abort:
                    exit_abort = true;
                    interp_status = failforever;
                    pc = 0;
                    break;

                case commands::cmd_extended:
#ifdef DEBUG
                    if (curr_code == nullptr) {
                        screen_message(DBG_CODE_PTR_IS_NIL);
                        goto l99;
                    }
#endif
                    code_interpret(curr_rep, curr_cnt, curr_code, true);
                    break;

                case commands::cmd_verify:
                    if (!verify_always[curr_cnt]) {
                        if (ludwig_mode == ludwig_mode_type::ludwig_batch) {
                            exit_abort  = true;
                            interp_status = failforever;
                            pc = 0;
                        } else if (tpar_get_1(curr_tpar, commands::cmd_verify, request)) {
                            if (request.len == 0) { // If didnt specify, use default.
                                request = current_frame->verify_tpar;
                                if (request.len == 0) {
                                    screen_message(MSG_NO_DEFAULT_STR);
                                    goto l99;
                                }
                            } else {
                                // If did specify, save for next time.
                                current_frame->verify_tpar = request;
                            }
                            if (request.str[1] == 'Y') {
                                // do nothing
                            } else if (request.str[1] == 'A') {
                                verify_always[curr_cnt] = true;
                            } else if (request.str[1] == 'Q') {
                                exit_abort  = true;
                                interp_status = failforever;
                                pc = 0;
                            } else {
                                interp_status = failure;
                                pc = curr_lbl;
                            }
                        }
                    }
                    break;

#ifdef DEBUG
                case commands::cmd_noop:
                    screen_message(DBG_ILLEGAL_INSTRUCTION);
                    goto l99;
                    break;
#endif

                default:
                    // Ignored
                    break;
                }
            } else {
                // call execute command
                if (!execute(curr_op, curr_rep, curr_cnt, curr_tpar, from_span)) {
                    interp_status = failure;
                    pc = curr_lbl;
                }
                if (exit_abort) {
                    interp_status = failforever;
                    pc = 0;
                }
            }

            if (tt_controlc) {
                interp_status = failforever;
                pc = 0;
            }

            if (interp_status == failure) {
                while (pc == 0 && level >= 1) {
                    pc = labels[level].faillabel;
                    level -= 1;
                }
            }
        } while (pc != 0);
    } // count loop

    result = (interp_status == success);
l99:;
    tpar_clean_object(request);
    code_discard(code_head);
    return result;
}
