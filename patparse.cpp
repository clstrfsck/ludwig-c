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
! Name:         PATPARSE
!
! Description:  This is the parser for the pattern matcher for EQS, Get
!               and Replace.
!
! $Log: patparse.pas,v $
! Revision 4.8  2002/07/15 10:19:21  martin
! Allow patparse to compile when using fpc
!
! Revision 4.7  1990/01/18 17:41:29  ludwig
! Entered into RCS at revision level 4.7
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Mark R. Prior                                        20-Feb-1988
!       Use conformant arrays to pass string parameters to ch routines.
!               string[offset],length -> string,offset,length
!       In all calls of ch_length, ch_upcase_str, ch_locase_str,
!         ch_reverse_str, ch_compare_str, and ch_search_str the offset
!         was 1 and is now omitted.
! 4-003 Kelvin B. Nicolle                                    25-Nov-1988
!       Change set expressions of the form [...] + [...] to [...,...] to
!       avoid a bug in Ultrix pc.
! 4-004 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-005 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-006 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-007 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!**/

#include "patparse.h"

#include "var.h"
#include "tpar.h"
#include "screen.h"

// Warning: Non-local goto's implemented via exceptions

namespace {
    struct local_exception {};
    struct other_exception {};

    const accept_set_type QUOTED({ TPD_LIT, TPD_EXACT });
    const accept_set_type DELIMITED({ PATTERN_KSTAR,
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                PATTERN_LRANGE_DELIM,
                PATTERN_PLUS });
    const accept_set_type CHARSETS({'s','S','a','A','c','C','l','L','u','U','n','N','p','P'});
    const accept_set_type POSITIONALS({'<','>','{','}','^'});
    const accept_set_type CH_AND_POS = CHARSETS.set_union(POSITIONALS);
    const accept_set_type SYNTAX({
            TPD_SPAN, TPD_PROMPT, TPD_EXACT, TPD_LIT, // strings
                PATTERN_LPAREN, PATTERN_LRANGE_DELIM,
                PATTERN_KSTAR,
                PATTERN_PLUS, PATTERN_NEGATE,
                PATTERN_MARK,
                PATTERN_EQUALS, PATTERN_MODIFIED,
                '0','1','2','3','4','5','6','7','8','9',
                PATTERN_DEFINE_SET_U,
                PATTERN_DEFINE_SET_L,
                'a', 'b', 'c', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
                'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u',
                'v', 'w', 'x', 'y', 'z',
                'A', 'B', 'C', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
                'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
                'V', 'W', 'X', 'Y', 'Z',
                '{', '}', '<', '>', '^'});
};

bool pattern_parser(tpar_object &pattern, nfa_table_type &nfa_table,
                    nfa_state_range &first_pattern_start, nfa_state_range &pattern_final_state,
                    nfa_state_range &left_context_end, nfa_state_range &middle_context_end,
                    pattern_def_type &pattern_definition, nfa_state_range &states_used) {

    nfa_state_range first_pattern_end;
    strlen_range    parse_count;
    char            pat_ch;
    accept_set_type aux_set;

    auto pattern_new_nfa = [&]() -> nfa_state_range {
        if (states_used < MAX_NFA_STATE_RANGE) {
            nfa_state_range new_nfa = states_used;
            //with nfa_table[states_used] do
            nfa_table[states_used].fail = false;
            nfa_table[states_used].indefinite = false;
            states_used += 1;
            return new_nfa;
        } else {
            screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
            throw local_exception();
        }
    };

    auto pattern_duplicate_nfa = [&](nfa_state_range copy_this_start, nfa_state_range copy_this_finish,
                                     nfa_state_range current_state, nfa_state_range &duplicate_finish) -> bool {
        // duplicates the NFA between states copy_this_start and finish inclusive
        // and splices the duplicated path onto the end of the current_state
        // current_state must be an epslion transition
        // The duplicates are patched using first_out, second_out = pattern_null
        // Returns in duplicate_finish the last of the duplicated states
        // which will be an uninitialized state
        // Note : this function is hacky and tailor made for the job of making
        // paramterized PATTERNs
        // Hack. As states are allocated in
        // sequence, just duplicate block with offsets.
        // the syntax ensures that there are no transitions out
        // of the block being duplicated when used on a single PATTERN
        nfa_state_range offset;
        nfa_state_range aux_state;
        nfa_state_range aux;
        nfa_state_range duplicate_start;

        offset = (current_state - copy_this_start) + 1;
        if ((states_used + offset) > MAX_NFA_STATE_RANGE) {
            screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
            throw local_exception();
        }
        duplicate_start = states_used;   // states_used private to NFA utilities
                                         // states_used = next state to be allocated
        for (aux = copy_this_start; aux <= copy_this_finish; ++aux) {
            aux_state = pattern_new_nfa();   // get new state
            //with nfa_table[aux_state] do
            nfa_table[aux_state].fail = nfa_table[aux].fail;
            if (!nfa_table[aux_state].fail) {
                nfa_table[aux_state].epsilon_out = nfa_table[aux].epsilon_out;
                if (nfa_table[aux_state].epsilon_out) {
                    if (nfa_table[aux].ept.first_out == PATTERN_NULL)
                        nfa_table[aux_state].ept.first_out = nfa_table[aux].ept.first_out;
                    else
                        nfa_table[aux_state].ept.first_out = nfa_table[aux].ept.first_out + offset;
                    if (nfa_table[aux].ept.second_out == PATTERN_NULL)
                        nfa_table[aux_state].ept.second_out = nfa_table[aux].ept.second_out;
                    else
                        nfa_table[aux_state].ept.second_out = nfa_table[aux].ept.second_out + offset;
                } else {
                    if (nfa_table[aux].epf.next_state == PATTERN_NULL)
                        nfa_table[aux_state].epf.next_state = nfa_table[aux].epf.next_state;
                    else
                        nfa_table[aux_state].epf.next_state = nfa_table[aux].epf.next_state + offset;
                    nfa_table[aux_state].epf.accept_set = nfa_table[aux].epf.accept_set;
                }
            }
            //with nfa_table[current_state] do
            nfa_table[current_state].epsilon_out = true;
            nfa_table[current_state].ept.first_out = duplicate_start;
            // second_out  is left untouched
        }
        duplicate_finish = aux_state;
        return true;
    };

    auto pattern_getch = [&](strlen_range &parse_count, char &ch, tpar_object in_string) -> bool {
        bool result = true;
        if (parse_count < in_string.len){
            parse_count += 1;
            ch = in_string.str[parse_count];
            //with pattern_definition do
            pattern_definition.length += 1;
            if (pattern_definition.length > MAX_STRLEN) {
                screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
                throw local_exception();
            }
            pattern_definition.strng[pattern_definition.length] = ch;
        } else {
            ch = '\0'; // a null
            result = false;
        }
        return result;
    };

    auto pattern_getnumb = [&](strlen_range &parse_count, int &number, char &ch, tpar_object in_string) -> bool {
        bool aux_bool = pattern_getch(parse_count, ch, in_string);
        bool result   = aux_bool && (ch >= '0' && ch <= '9');

        number =  0;
        while (aux_bool && (ch >= '0' && ch <= '9')) {
            number = (number * 10) + (ch - '0');
            aux_bool = pattern_getch(parse_count, ch, in_string);
        }
        if (aux_bool) // we are not at the end of the string
            parse_count -= 1;
        return result;
    };

    std::function<void(nfa_state_range, nfa_state_range &, strlen_range &, tpar_object, char &, int)>
        pattern_compound = [&](nfa_state_range first, nfa_state_range &finish, strlen_range &parse_count,
                          tpar_object in_string, char &pat_ch, int depth) {
        nfa_state_range compound_finish;
        nfa_state_range current_e_start;
        bool dummy;

        std::function<void(nfa_state_range, nfa_state_range &, strlen_range &, tpar_object, char &, int)>
        pattern_pattern = [&](nfa_state_range first, nfa_state_range &finish, strlen_range &parse_count,
                                   tpar_object in_string, char &pat_ch, int depth) {
            parameter_type leading_param;
            strlen_range aux;
            strlen_range aux_count;
            strlen_range temporary;
            char delimiter;
            char aux_ch_1;
            char aux_ch_2;
            char aux_pat_ch;
            tpar_object deref_span;
            commands tpar_sort;
            nfa_state_range current_state;
            nfa_state_range aux_state;
            nfa_state_range begin_state;
            bool end_of_input;
            bool negate;
            bool no_dereference;
            nfa_state_range range_patch;
            int range_start;
            int range_end;
            int auxi;
            bool range_indefinite;

            auto pattern_range_delimgen = [&](nfa_state_range &range_patch, int &range_start, int &range_end,
                                              bool &range_indefinite, parameter_type &leading_param) {
                range_indefinite = false;
                leading_param = parameter_type::pattern_range;
                switch (pat_ch) {
                case PATTERN_KSTAR:
                    range_start = 0;
                    range_end = 0;
                    range_indefinite = true;
                    break;
                case PATTERN_PLUS:
                    range_start = 1;
                    range_end = 0;
                    range_indefinite = true;
                    break;
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
                    parse_count -= 1;    // back up YECH!!
                    pattern_getnumb(parse_count, range_start, pat_ch, in_string);
                    range_end = range_start;
                    break;
                case PATTERN_LRANGE_DELIM:
                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                        throw local_exception();
                    }
                    if (pat_ch >= '0' && pat_ch <= '9') {
                        parse_count -= 1;
                        dummy = pattern_getnumb(parse_count, range_start, pat_ch, in_string);
                        dummy = pattern_getch(parse_count, pat_ch, in_string);
                    } else {
                        // therefore no start no. default to 0
                        range_start = 0;
                    }
                    if (pat_ch == PATTERN_COMMA) {
                        // eat comma
                        if (!pattern_getch(parse_count, pat_ch, in_string)) {
                            screen_message(MSG_PAT_NO_MATCHING_DELIM);
                            throw local_exception();
                        }
                    } else {
                        screen_message(MSG_PAT_ERROR_IN_RANGE);
                        throw local_exception();
                    }
                    if (pat_ch >= '0' && pat_ch <= '9') {
                        parse_count -= 1;
                        dummy = pattern_getnumb(parse_count, range_end, pat_ch, in_string);
                        dummy = pattern_getch(parse_count, pat_ch, in_string);
                    } else {
                        range_indefinite = true;
                        range_end = 0;
                    }
                    if (pat_ch != PATTERN_RRANGE_DELIM) {
                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                        throw local_exception();
                    }
                }
                if (range_start == 0) {
                    // if 0 we need to have a diversion state
                    range_patch = current_state;
                    //with nfa_table[range_patch] do
                    nfa_table[range_patch].epsilon_out = true;
                    nfa_table[range_patch].ept.first_out = pattern_new_nfa();
                    nfa_table[range_patch].ept.second_out = PATTERN_NULL;
                    current_state = nfa_table[range_patch].ept.first_out;
                } else {
                    range_patch = PATTERN_NULL;
                }
            };

            auto pattern_range_build = [&](int range_start, int range_end, nfa_state_range range_path, bool indefinite) {
                int aux;
                nfa_state_range end_state;
                nfa_state_range divert_ptr;
                nfa_state_range aux_ptr;
                nfa_state_range indefinite_patch;

                end_state  = current_state;
                indefinite_patch = begin_state;
                divert_ptr = range_patch; // = null if no divert state created
                //with nfa_table[current_state] do
                nfa_table[current_state].epsilon_out = true;
                nfa_table[current_state].ept.first_out = PATTERN_NULL;
                nfa_table[current_state].ept.second_out = PATTERN_NULL;
                for (aux = 2; aux <= range_start; ++aux) {
                    // duplicates without diversions
                    indefinite_patch = current_state;
                    dummy = pattern_duplicate_nfa(begin_state, end_state, current_state, current_state);
                }
                if (range_start > 0) // HACK !! (sort of)
                    range_start -= 1;
                for (aux = range_start +2; aux <= range_end; ++aux) {
                    // duplicates with diversions
                    nfa_table[current_state].ept.second_out = divert_ptr;
                    divert_ptr = current_state;
                    dummy = pattern_duplicate_nfa(begin_state, end_state, current_state, current_state);
                }
                nfa_table[current_state].ept.second_out = PATTERN_NULL;
                // now fixup the diversions
                while (divert_ptr != PATTERN_NULL) {
                    aux_ptr = nfa_table[divert_ptr].ept.second_out;
                    nfa_table[divert_ptr].ept.second_out = current_state;
                    divert_ptr = aux_ptr;
                }
                if (indefinite) {
                    // send a pointer back to the begining of the last duplicate
                    //with nfa_table[current_state] do
                    nfa_table[current_state].epsilon_out = true;
                    nfa_table[current_state].ept.first_out = indefinite_patch;
                    nfa_table[current_state].ept.second_out = pattern_new_nfa();
                    current_state = nfa_table[current_state].ept.second_out;
                    nfa_table[indefinite_patch].indefinite = true;
                }
            };

            //with deref_span do
            deref_span.nxt = nullptr;
            deref_span.con = nullptr;
            if (depth > PATTERN_MAX_DEPTH) {
                screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
                throw local_exception();
            }
            end_of_input = false;
            current_state = first;
            end_of_input = false;
            while (!end_of_input && (pat_ch == PATTERN_SPACE))
                end_of_input = !pattern_getch(parse_count, pat_ch, in_string);
            // eat any preceeding spaces
            while ((pat_ch != PATTERN_COMMA && pat_ch != PATTERN_RPAREN && pat_ch != PATTERN_BAR) && !end_of_input) {
                if (!SYNTAX.contains(pat_ch)) {
                    // pat_ch not in syntax
                    screen_message(MSG_PAT_ILLEGAL_SYMBOL);
                    throw local_exception();
                }
                switch (pat_ch) {
                case TPD_SPAN:
                case TPD_PROMPT: {
                    delimiter = pat_ch;
                    tpar_object deref_tpar;
                    aux = 0;
                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                        throw local_exception();
                    }
                    while (pat_ch != delimiter) {
                        aux += 1;
                        deref_tpar.str[aux] = pat_ch;
                        if (!pattern_getch(parse_count, pat_ch, in_string)) {
                            screen_message(MSG_PAT_NO_MATCHING_DELIM);
                            throw local_exception();
                        }
                    }
                    pattern_definition.length = pattern_definition.length - (aux + 2);
                    tpar_sort = commands::cmd_pattern_dummy_pattern;    // back up over delims and string
                    deref_tpar.len = aux;
                    deref_tpar.dlm = delimiter;
                    if (!tpar_get_1(&deref_tpar, tpar_sort, deref_span)) {
                        throw other_exception();
                    }

                    // get the dereferenced span or whatever else it is
                    if (QUOTED.contains(deref_span.dlm)) {
                        //with deref_span do
                        // H A C K !!!!!!!!!
                        deref_span.str.insert(1, 1); // Tpar_get strips off the quotes
                        deref_span.len += 2;         // so we've got to put them back
                        deref_span.str[deref_span.len] = deref_span.dlm;
                        deref_span.str[1] = deref_span.dlm;
                    }
                    aux_count = 0;
                    if (pattern_getch(aux_count, aux_pat_ch, deref_span)) {
                        // if span not empty
                        pattern_compound(current_state, current_state, aux_count, deref_span, aux_pat_ch, depth + 1);
                        // parse the span as a pattern   { ###### }
                        if ((aux_count != deref_span.len) || (deref_span.str[aux_count] == PATTERN_COMMA)) {
                            screen_message(MSG_PAT_ERROR_IN_SPAN);
                            throw local_exception();
                        }
                    }
                }
                    break;

                // ALL PARAMETERIZED CLAUSES
                case TPD_EXACT: case TPD_LIT: // strings
                case PATTERN_LPAREN:          // compounds
                case PATTERN_LRANGE_DELIM:    // ranges
                case PATTERN_KSTAR:           // leading parameters
                case PATTERN_PLUS: case PATTERN_NEGATE:
                case PATTERN_MARK:            // marks
                case PATTERN_EQUALS:          // Equals mark
                case PATTERN_MODIFIED:        // Modified mark
                case '0': case '1': case '2': // numeric repeat counts
                case '3': case '4': case '5':
                case '6': case '7': case '8': case '9':
                case PATTERN_DEFINE_SET_U:    // set definitions
                case PATTERN_DEFINE_SET_L:
                case 'a': case 'b': case 'c': // Sets
                case 'e': case 'f': case 'g': // D,d, out
                case 'h': case 'i': case 'j': case 'k': case 'l':
                case 'm': case 'n': case 'o': case 'p': case 'q':
                case 'r': case 's': case 't': case 'u': case 'v':
                case 'w': case 'x': case 'y': case 'z':
                case 'A': case 'B': case 'C': case 'E': case 'F':
                case 'G': case 'H': case 'I': case 'J': case 'K':
                case 'L': case 'M': case 'N': case 'O': case 'P':
                case 'Q': case 'R': case 'S': case 'T': case 'U':
                case 'V': case 'W': case 'X': case 'Y': case 'Z':
                case '{': case '}': case '<': // positionals
                case '>': case '^':
                    // get the parameter if any
                    leading_param = parameter_type::null_param;
                    if (DELIMITED.contains(pat_ch)) {
                        pattern_range_delimgen(range_patch, range_start, range_end, range_indefinite, leading_param);
                        if (!pattern_getch(parse_count, pat_ch, in_string)) {
                            screen_message(MSG_PAT_PREMATURE_PATTERN_END);
                            throw local_exception();
                        }
                        begin_state = current_state;
                    }
                    switch (pat_ch) {
                    case TPD_EXACT:
                    case TPD_LIT:
                        aux_ch_1 = pat_ch;
                        aux_count = parse_count;
                        if (!pattern_getch(parse_count, pat_ch, in_string)) {
                            screen_message(MSG_PAT_NO_MATCHING_DELIM);
                            throw local_exception();
                        }
                        if (pat_ch == TPD_SPAN || pat_ch == TPD_PROMPT) {
                            no_dereference = false;
                            delimiter = pat_ch;
                            tpar_object deref_tpar;
                            aux = 0;
                            do { // build potential deref tpar
                                if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                    screen_message(MSG_PAT_NO_MATCHING_DELIM);
                                    throw local_exception();
                                }
                                aux += 1;
                                deref_tpar.str[aux] = pat_ch;
                            } while (pat_ch != aux_ch_1);
                            if (aux >= 2) { // one $ plus one "
                                if (deref_tpar.str[aux - 1] == delimiter) {
                                    // criterion for a correct  deref
                                    pattern_definition.length -= (aux + 2); // wipe out the deref stuff and quotes
                                    tpar_sort = commands::cmd_pattern_dummy_text;  // so prompts "text  :" ##
                                    deref_tpar.len = aux - 2; // wipe out delim and quote
                                    deref_tpar.dlm = delimiter;
                                    if (!tpar_get_1(&deref_tpar, tpar_sort, deref_span)) {
                                        throw other_exception(); // get the dereferenced span
                                    }
                                    //with deref_span do
                                    // wrap it in quotes
                                    deref_span.str.insert(1, 1);
                                    deref_span.len += 2;
                                    deref_span.str[deref_span.len] = aux_ch_1;
                                    deref_span.str[1] = aux_ch_1;
                                    aux_count = 0;        // recur
                                    if (pattern_getch(aux_count, aux_pat_ch, deref_span))
                                        pattern_pattern(current_state, current_state,
                                                        aux_count, deref_span, aux_pat_ch, depth + 1);
                                } else {
                                    // its just looking for a string that happened to
                                    //  start with a Dereference delimiter, put it all back
                                    pat_ch = delimiter;
                                    pattern_definition.length += (aux + 2);
                                    parse_count = aux_count + 1;
                                    no_dereference = true;   // ok we drop out and let the normal processing take it
                                }
                            } else {
                                // was just a single $ or &
                                pat_ch = delimiter;
                                pattern_definition.length += (aux + 2);
                                parse_count = aux_count + 1;
                                no_dereference = true;
                            }
                        } else {
                            no_dereference = true;
                        }
                        
                        if (no_dereference) {
                            if (aux_ch_1 == TPD_EXACT) {
                                while (pat_ch != TPD_EXACT) {
                                    //with nfa_table[current_state] do
                                    nfa_table[current_state].epsilon_out = false;
                                    nfa_table[current_state].epf.accept_set = accept_set_type(pat_ch);
                                    nfa_table[current_state].epf.next_state = pattern_new_nfa();
                                    current_state = nfa_table[current_state].epf.next_state;
                                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                                        throw local_exception();
                                    }
                                }
                            } else {
                                // must be tpd_lit
                                while (pat_ch != TPD_LIT) {
                                    //with nfa_table[current_state] do
                                    nfa_table[current_state].epsilon_out = false;
                                    if (pat_ch >= 'a' && pat_ch <= 'z')
                                        nfa_table[current_state].epf.accept_set = accept_set_type({pat_ch, std::toupper(pat_ch)});
                                    else if (pat_ch >= 'A' && pat_ch <= 'Z')
                                        nfa_table[current_state].epf.accept_set = accept_set_type({pat_ch, std::tolower(pat_ch)});
                                    else
                                        nfa_table[current_state].epf.accept_set = accept_set_type(pat_ch);
                                    nfa_table[current_state].epf.next_state = pattern_new_nfa();
                                    current_state = nfa_table[current_state].epf.next_state;
                                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                                        throw local_exception();
                                    }
                                }
                            }
                        }
                        break; // TPD_LIT, TPD_EXACT
                    case PATTERN_LPAREN:
                        dummy = pattern_getch(parse_count, pat_ch, in_string);
                        pattern_compound(current_state, aux_state, parse_count, in_string, pat_ch, depth + 1);
                        current_state = aux_state;
                        if (pat_ch != PATTERN_RPAREN) {
                            screen_message(MSG_PAT_NO_MATCHING_DELIM);
                            throw local_exception();
                        }
                        break;
                    case PATTERN_MARK:
                        if (pattern_getnumb(parse_count, auxi, pat_ch, in_string)) {
                            if ((auxi == 0) || (auxi > MAX_MARK_NUMBER)) {
                                screen_message(MSG_PAT_ILLEGAL_MARK_NUMBER);
                                throw local_exception();
                            }
                            //with nfa_table[current_state] do
                            nfa_table[current_state].epsilon_out = false;
                            nfa_table[current_state].epf.accept_set = accept_set_type(auxi + PATTERN_MARKS_START);
                            nfa_table[current_state].epf.next_state = pattern_new_nfa();
                            current_state = nfa_table[current_state].epf.next_state;
                        } else {
                            screen_message(MSG_PAT_ILLEGAL_MARK_NUMBER);
                            throw local_exception();
                        }
                        break;
                    case PATTERN_EQUALS:
                    case PATTERN_MODIFIED:
                        //with nfa_table[current_state] do
                        nfa_table[current_state].epsilon_out = false;
                        if (pat_ch == PATTERN_EQUALS)
                            nfa_table[current_state].epf.accept_set = accept_set_type(PATTERN_MARKS_EQUALS);
                        else
                            nfa_table[current_state].epf.accept_set = accept_set_type(PATTERN_MARKS_MODIFIED);
                        nfa_table[current_state].epf.next_state = pattern_new_nfa();
                        current_state = nfa_table[current_state].epf.next_state;
                        break;
                    default:
                        // any alpha and '-'
                        // SET
                        negate = false;
                        if (pat_ch == PATTERN_NEGATE) {
                            negate = true;
                            if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                screen_message(MSG_PAT_PREMATURE_PATTERN_END);
                                throw local_exception();
                            }
                        }
                        // have eaten any '-' therefore only have alpha
                        if ((pat_ch == PATTERN_DEFINE_SET_U) || (pat_ch == PATTERN_DEFINE_SET_L)) {
                            // make a set
                            aux_set.clear();
                            if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                screen_message(MSG_PAT_PREMATURE_PATTERN_END);
                                throw local_exception();
                            }
                            // eat the define symbol
                            delimiter = pat_ch;
                            temporary = pattern_definition.length;
                            pattern_definition.strng[temporary] = '\0';
                            // a null, so that all delimiters become the same
                            if (delimiter == TPD_SPAN || delimiter == TPD_PROMPT) {
                                // is a $ or &, therefore build tpar
                                tpar_object deref_tpar;
                                deref_tpar.dlm = delimiter;
                                if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                    screen_message(MSG_PAT_PREMATURE_PATTERN_END);
                                    throw local_exception();
                                }
                                // eat the delimiter
                                aux = 0;
                                while (pat_ch != delimiter) {
                                    aux += 1;
                                    deref_tpar.str[aux] = pat_ch;
                                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                                        throw local_exception();
                                    }
                                }
                                pattern_definition.length -= (aux + 1); // wipe the deref but leave the NULL
                                deref_tpar.len = aux;
                                if (!tpar_get_1(&deref_tpar, tpar_sort, deref_span)) {
                                    throw other_exception();
                                }
                                // get the dereferenced span or whatever else it is
                                // set is returned in deref_span
                            } else {
                                // a user defined set string
                                // build set specifier in deref_span
                                aux = 0;
                                if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                    screen_message(MSG_PAT_PREMATURE_PATTERN_END);
                                    throw local_exception();
                                }
                                pattern_definition.strng[pattern_definition.length] = '\0';
                                while (pat_ch != delimiter) {
                                    aux += 1;
                                    deref_span.str[aux] = pat_ch;
                                    if (!pattern_getch(parse_count, pat_ch, in_string)) {
                                        screen_message(MSG_PAT_NO_MATCHING_DELIM);
                                        throw local_exception();
                                    }
                                }
                                deref_span.len = aux;
                            } // user defined set string
                            aux_set.clear();
                            //with deref_span do
                            // Form the character set.
                            aux = 1;
                            while (aux <= deref_span.len) {
                                aux_ch_1 = deref_span.str[aux];
                                aux_ch_2 = aux_ch_1;
                                aux += 1;
                                if (aux + 2 <= deref_span.len) {
                                    if ((deref_span.str[aux] == '.') && (deref_span.str[aux + 1] == '.')) {
                                        aux_ch_2 = deref_span.str[aux + 2];
                                        aux = aux + 3;
                                    }
                                }
                                aux_set.add_range(aux_ch_1, aux_ch_2);
                            }
                            //with pattern_definition do
                            // put set definition into Pattern_definition
                            for (aux_count = 1; aux_count <= deref_span.len; ++aux_count)
                                pattern_definition.strng[temporary + aux_count] = deref_span.str[aux_count];
                            pattern_definition.length = temporary + deref_span.len + 1; // plus room for null
                            pattern_definition.strng[pattern_definition.length] = '\0'; // put in other delimiter
                            if (negate)
                                aux_set = accept_set_type(PATTERN_ALPHA_START, MAX_SET_RANGE).remove(aux_set);
                            //with nfa_table[current_state] do
                            nfa_table[current_state].epsilon_out = false;
                            nfa_table[current_state].epf.accept_set = aux_set;
                            nfa_table[current_state].epf.next_state = pattern_new_nfa();
                            current_state = nfa_table[current_state].epf.next_state;
                        } else if (CH_AND_POS.contains(pat_ch)) {
                            //with nfa_table[current_state] do
                            nfa_table[current_state].epsilon_out = false;
                            if (POSITIONALS.contains(pat_ch)) {
                                // predefined positional sets
                                if (negate) {
                                    screen_message(MSG_PAT_ILLEGAL_PARAMETER);
                                    throw local_exception();
                                }
                                nfa_table[current_state].epf.accept_set.clear();
                                switch (pat_ch) {
                                case '<': nfa_table[current_state].epf.accept_set.add(PATTERN_BEG_LINE);     break;
                                case '>': nfa_table[current_state].epf.accept_set.add(PATTERN_END_LINE);     break;
                                case '{': nfa_table[current_state].epf.accept_set.add(PATTERN_LEFT_MARGIN);  break;
                                case '}': nfa_table[current_state].epf.accept_set.add(PATTERN_RIGHT_MARGIN); break;
                                case '^': nfa_table[current_state].epf.accept_set.add(PATTERN_DOT_COLUMN);   break;
                                }
                            } else {
                                // predefined char sets
                                switch (std::toupper(pat_ch)) {
                                case 'S': nfa_table[current_state].epf.accept_set = SPACE_SET;       break;
                                case 'C': nfa_table[current_state].epf.accept_set = PRINTABLE_SET;   break;
                                case 'A': nfa_table[current_state].epf.accept_set = ALPHA_SET;       break;
                                case 'L': nfa_table[current_state].epf.accept_set = LOWER_SET;       break;
                                case 'U': nfa_table[current_state].epf.accept_set = UPPER_SET;       break;
                                case 'N': nfa_table[current_state].epf.accept_set = NUMERIC_SET;     break;
                                case 'P': nfa_table[current_state].epf.accept_set = PUNCTUATION_SET; break;
                                }
                                if (negate) {
                                    nfa_table[current_state].epf.accept_set =
                                        accept_set_type(PATTERN_ALPHA_START,
                                                        MAX_SET_RANGE).remove(nfa_table[current_state].epf.accept_set);
                                }
                            }
                            nfa_table[current_state].epf.next_state = pattern_new_nfa();
                            current_state = nfa_table[current_state].epf.next_state;
                        } else {
                            screen_message(MSG_PAT_SET_NOT_DEFINED);
                            throw local_exception();
                        }
                        break;
                    }
                    if (leading_param == parameter_type::pattern_range) {
                        nfa_table[current_state].epsilon_out = true;
                        nfa_table[current_state].ept.first_out = PATTERN_NULL;
                        pattern_range_build(range_start, range_end, range_patch, range_indefinite);
                    }
                }
                // get next char and eat trailing blanks
                do {
                    end_of_input = !pattern_getch(parse_count, pat_ch, in_string);
                } while (!end_of_input && (pat_ch == PATTERN_SPACE));
            }
            finish = current_state;
            tpar_clean_object(deref_span);
        };

        if (depth > PATTERN_MAX_DEPTH) {
            screen_message(MSG_PAT_PATTERN_TOO_COMPLEX);
            throw local_exception();
        }
        current_e_start = first;
        //with nfa_table[current_e_start] do
        nfa_table[current_e_start].epsilon_out = true;
        nfa_table[current_e_start].ept.second_out = PATTERN_NULL;
        nfa_table[current_e_start].ept.first_out = pattern_new_nfa();
        pattern_pattern(nfa_table[current_e_start].ept.first_out, finish, parse_count, in_string, pat_ch, depth + 1);
        compound_finish = finish;
        if (pat_ch == PATTERN_BAR) {
            if (pattern_getch(parse_count, pat_ch, in_string)) {
                nfa_table[current_e_start].ept.second_out = pattern_new_nfa();
                pattern_compound(nfa_table[current_e_start].ept.second_out, finish, parse_count, in_string, pat_ch, depth + 1);
                //with nfa_table[compound_finish] do
                nfa_table[compound_finish].epsilon_out = true;
                nfa_table[compound_finish].ept.first_out = PATTERN_NULL;
                nfa_table[compound_finish].ept.second_out = finish;
            } else {
                nfa_table[current_e_start].ept.second_out = finish;
            }
        }
    };

    bool result = true;
    try
    {
        exit_abort = true;  // set true in case of syntax error
        pattern_definition.length = 0;
        //with nfa_table[pattern_null] do
        nfa_table[PATTERN_NULL].epsilon_out = true;
        nfa_table[PATTERN_NULL].ept.first_out   = PATTERN_NULL;
        nfa_table[PATTERN_NULL].ept.second_out  = PATTERN_NULL;
        
        states_used         = PATTERN_NFA_START;
        first_pattern_start = pattern_new_nfa();
        parse_count = 0;
        if (pattern_getch(parse_count, pat_ch, pattern)) {
            pattern_compound(first_pattern_start, first_pattern_end, parse_count, pattern, pat_ch, 1); // 1st
            if (pat_ch == PATTERN_COMMA) {
                // we have a second pattern
                if (pattern_getch(parse_count, pat_ch, pattern)) {
                    // otherwise pattern null
                    left_context_end   = first_pattern_end;
                    pattern_compound(left_context_end, middle_context_end, parse_count, pattern, pat_ch, 1); // 2nd
                } else {
                    // null middle
                    middle_context_end   = first_pattern_end;
                    pattern_final_state  = first_pattern_end;
                }
            } else {
                // no further input so pattern is middle context
                left_context_end     = first_pattern_start;
                pattern_final_state  = first_pattern_end;
                middle_context_end   = first_pattern_end;
            }
            if (pat_ch == PATTERN_COMMA) {
                // we have a third pattern
                if (pattern_getch(parse_count, pat_ch, pattern)) {
                    pattern_compound(middle_context_end, pattern_final_state, parse_count, pattern, pat_ch, 1); // 3rd
                } else {
                    // null right context
                    pattern_final_state = middle_context_end;
                }
            } else {
                // no right context
                pattern_final_state = middle_context_end;
            }
        } else {
            // not Pattern_getch
            screen_message(MSG_PAT_NULL_PATTERN);
            throw local_exception();
        }
        //with nfa_table[pattern_final_state] do
        nfa_table[pattern_final_state].epsilon_out = true;
        nfa_table[pattern_final_state].ept.first_out   = PATTERN_NULL;
        nfa_table[pattern_final_state].ept.second_out  = PATTERN_NULL;

        result = true;
        // hey wow ! we got through the parser, guess we'd
        // better not crunge out then
        exit_abort = false;
    }
    catch (const local_exception &ex)
    {
        // exit with a pattern error
        // Nothing
    }
    catch (const other_exception &ex)
    {
        // exit with someone else's error
    }
    return result;
}
