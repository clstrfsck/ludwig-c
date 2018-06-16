/** @file const.h
 * Declarations of constants used throughout the Ludwig code base.
 */

/**
! $Log: const.i,v $
! Revision 4.15  2002/07/20 16:46:00  martin
! Remove special turbop constants.  They are now in the units
! that they are used by.  Changed some constants to be conditional
! on msdos rather than turbop, and added some fpc conditionals
! for use by the fpc port.
!
! Revision 4.15  2002/07/20 15:50:36  martin
! Remove random constants in #if turbop section.
!
! Revision 4.14  1990/10/19 14:32:16  ludwig
! Make dbg messages conditional on debug flag.  KBN
!
! Revision 4.13  90/02/28  10:53:56  ludwig
! Add two messages for File Save command.
!
! Revision 4.12  90/02/17  16:09:16  ludwig
! increaed the maximum number of special keys and key names up to 1000 for
! the XWINDOWS version
!
! Revision 4.11  90/01/18  18:20:29  ludwig
! Entered into RCS at revision level 4.11
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Kelvin B. Nicolle                                     5-May-1987
!       Add definition of ascii_vt.
! 4-003 Kelvin B. Nicolle                                     5-May-1987
!       Remove lw_vers--it is now ludwig_version in var.inc.
! 4-004 Mark R. Prior                                        12-Dec-1987
!       Add max_word_sets.
! 4-005 Mark R. Prior                                        19-Feb-1988
!       Add max_word_sets_m1.
! 4-006 Kelvin B. Nicolle                                     9-Dec-1988
!       Move the ascii constants to filesys.pas.
! 4-007 Kelvin B. Nicolle                                     9-Dec-1988
!       Work around a bug in Ultrix pc which does not allow double
!       quotes in string when compiled with the debug option.
! 4-008 Kelvin B. Nicolle                                    16-Dec-1988
!       Expand all message string constants to msg_str_len characters.
! 4-009 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-010 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-011 Kelvin B. Nicolle                                    12-Jul-1989
!       Set max_word_sets to 2 to revert to old definition of a word.
!**/

#ifndef CONST_H
#define CONST_H

// For definition of FILENAME_MAX
#include <cstdio>
// For definition of EXIT_SUCCESS / EXIT_FAILURE
#include <cstdlib>

#include <limits>

// Easy helper definition
const int MAXINT = std::numeric_limits<int>::max();

// Max value of character
const int ORD_MAXCHAR         = 255;

// Max files
const int MAX_FILES           = 100;

// Max lines per group
const int MAX_GROUPLINES      = 64;
const int MAX_GROUPLINEOFFSET = MAX_GROUPLINES - 1;

// Max lines per frame
const int MAX_LINES           = MAXINT;

const int MIN_MARK_NUMBER     = -1;

// Highest User Mark number
const int MAX_MARK_NUMBER     = 9;
const int MARK_EQUALS         = 0;
const int MARK_MODIFIED       = -1;

//  Max chars allowed per frame
const int MAX_SPACE           = 1000000;

// Max length rec. in inp file
const int MAX_REC_SIZE        = 512;

// Max length of a string
const int MAX_STRLEN          = 400;
const int MAX_STRLENP         = MAX_STRLEN + 1;

// Max nr of rows on screen
const int MAX_SCR_ROWS        = 100;

// Max nr of cols on screen
const int MAX_SCR_COLS        = 255;

// Length of code array
const int MAX_CODE            = 4000;

// Max nr of V commands in span
const int MAX_VERIFY          = 256;

const int MAX_TPAR_RECURSION  = 100;
const int MAX_TPCOUNT         = 2;
const int MAX_EXEC_RECURSION  = 100;

// Max nr of word element sets
const int MAX_WORD_SETS       = 2;
const int MAX_WORD_SETS_M1    = MAX_WORD_SETS - 1;

// Frame names
extern const char BLANK_FRAME_NAME[];   // Empty
extern const char DEFAULT_FRAME_NAME[]; // Ludwig

// TPar interpretations
const char TPD_LIT         = '\''; // don't do fancy processing on this one
const char TPD_SMART       = '`';  // search target is a pattern
const char TPD_EXACT       = '"';  // use exact case during search
const char TPD_SPAN        = '$';  // span substitution
const char TPD_PROMPT      = '&';  // get parameter from user terminal
const char TPD_ENVIRONMENT = '?';  // environment enquiry
const int  EXPAND_LIM      = 130;  // at least # of multi-letter commands + 1

// String lengths
const int NAME_LEN      = 31;           // Max length of a spn/frm name
const int FILE_NAME_LEN = FILENAME_MAX; // Default length of file name
const int TPAR_PROM_LEN = 8;
const int WRITE_STR_LEN = 80;
const int MSG_STR_LEN   = 70;
const int KEY_LEN       = 4;            // Key length for HELP FILE

// Keyboard interface.
const int MAX_SPECIAL_KEYS = 1000; // Taken from original XWin def
const int KEY_NAME_LEN     = 40; // WARNING - this value is assumed in USER.PAS
const int MAX_NR_KEY_NAMES = 1000;
const int MAX_PARSE_TABLE  = 300;

// Application exit codes
const int NORMAL_EXIT   = EXIT_SUCCESS;
const int ABNORMAL_EXIT = EXIT_FAILURE;

// Regular expression state machine
const int MAX_NFA_STATE_RANGE   = 200;   // no of states in NFA
const int MAX_DFA_STATE_RANGE   = 255;   // no of states in DFA
                                         // as big as VAX v2 pascal allows
const int MAX_SET_RANGE   = ORD_MAXCHAR; // no of elts in accept sets
const int PATTERN_NULL          = 0;     // acts as nil in array ptrs
const int PATTERN_NFA_START     = 1;     // the first NFA state allocated
const int PATTERN_DFA_KILL      = 0;     // the dfa killer state
const int PATTERN_DFA_FAIL      = 0;     // To keep old versions happy
const int PATTERN_DFA_START     = 2;     // The DFA starting state
const int PATTERN_MAX_DEPTH     = 20;    // maximum recursion depth in parser

// Symbols used in pattern specification
const char PATTERN_KSTAR         = '*';  // Kleene star
const char PATTERN_COMMA         = ',';  // The context Delimiter
const char PATTERN_RPAREN        = ')';
const char PATTERN_LPAREN        = '(';
const char PATTERN_DEFINE_SET_U  = 'D';  // These two used as a pair
const char PATTERN_DEFINE_SET_L  = 'd';  // The define a char set symbol
const char PATTERN_MARK          = '@';  // match the numbered mark
const char PATTERN_EQUALS        = '=';  // match the Equals mark
const char PATTERN_MODIFIED      = '%';  // match the Modified mark
const char PATTERN_PLUS          = '+';  // Kleene Plus
const char PATTERN_NEGATE        = '-';  // To negate a char set
const char PATTERN_BAR           = '|';  // Alternation ( OR )
const char PATTERN_LRANGE_DELIM  = '[';  // For specification of a range of
const char PATTERN_RRANGE_DELIM  = ']';  // repetitions
const char PATTERN_SPACE         = ' ';

// set locations for line end specifiers
const int PATTERN_BEG_LINE         = 0;  // the <  (beginning of line) specifier
const int PATTERN_END_LINE         = 1;  // the >  (end of line) specifier
const int PATTERN_LEFT_MARGIN      = 3;  // l.brace( left margin ) specifier
const int PATTERN_RIGHT_MARGIN     = 4;  // r.brace( right margin ) specifier
const int PATTERN_DOT_COLUMN       = 5;  // the ^  ( dots column)   specifier

const int PATTERN_MARKS_START      = 20; // mark 1 = 21, 2 = 22, etc
const int PATTERN_MARKS_MODIFIED   = 19; // marks_start + mark_modified
const int PATTERN_MARKS_EQUALS     = 20; // marks_start + mark_equals

const int PATTERN_ALPHA_START      = 32; // IE. ASCII

// Messages
extern const char MSG_BLANK[];
extern const char MSG_ABORT[];
extern const char MSG_BAD_FORMAT_IN_TAB_TABLE[];
extern const char MSG_CANT_KILL_FRAME[];
extern const char MSG_CANT_SPLIT_NULL_LINE[];
extern const char MSG_COMMAND_NOT_VALID[];
extern const char MSG_COMMAND_RECURSION_LIMIT[];
extern const char MSG_COMMENTS_ILLEGAL[];
extern const char MSG_COMPILER_CODE_OVERFLOW[];
extern const char MSG_COPYRIGHT_AND_LOADING_FILE[];
extern const char MSG_COUNT_TOO_LARGE[];
extern const char MSG_DECOMMITTED[];
extern const char MSG_EMPTY_SPAN[];
extern const char MSG_EQUALS_NOT_SET[];
extern const char MSG_ERROR_OPENING_KEYS_FILE[];
extern const char MSG_EXECUTING_INIT_FILE[];
extern const char MSG_FILE_ALREADY_IN_USE[];
extern const char MSG_FILE_ALREADY_OPEN[];
extern const char MSG_FRAME_HAS_FILES_ATTACHED[];
extern const char MSG_FRAME_OF_THAT_NAME_EXISTS[];
extern const char MSG_ILLEGAL_LEADING_PARAM[];
extern const char MSG_ILLEGAL_MARK_NUMBER[];
extern const char MSG_ILLEGAL_PARAM_DELIMITER[];
extern const char MSG_INCOMPAT[];
extern const char MSG_INTERACTIVE_MODE_ONLY[];
extern const char MSG_INVALID_CMD_INTRODUCER[];
extern const char MSG_INVALID_INTEGER[];
extern const char MSG_INVALID_KEYS_FILE[];
extern const char MSG_INVALID_SCREEN_HEIGHT[];
extern const char MSG_INVALID_SLOT_NUMBER[];
extern const char MSG_INVALID_T_OPTION[];
extern const char MSG_INVALID_RULER[];
extern const char MSG_INVALID_PARAMETER_CODE[];
extern const char MSG_LEFT_MARGIN_GE_RIGHT[];
extern const char MSG_LONG_INPUT_LINE[];
extern const char MSG_MARGIN_OUT_OF_RANGE[];
extern const char MSG_MARGIN_SYNTAX_ERROR[];
extern const char MSG_MARK_NOT_DEFINED[];
extern const char MSG_MISSING_TRAILING_DELIM[];
extern const char MSG_NO_DEFAULT_STR[];
extern const char MSG_NO_FILE_OPEN[];
extern const char MSG_NO_MORE_FILES_ALLOWED[];
extern const char MSG_NO_ROOM_ON_LINE[];
extern const char MSG_NO_SUCH_FRAME[];
extern const char MSG_NO_SUCH_SPAN[];
extern const char MSG_NONPRINTABLE_INTRODUCER[];
extern const char MSG_NOT_ENOUGH_INPUT_LEFT[];
extern const char MSG_NOT_IMPLEMENTED[];
extern const char MSG_NOT_INPUT_FILE[];
extern const char MSG_NOT_OUTPUT_FILE[];
extern const char MSG_NOT_WHILE_EDITING_CMD[];
extern const char MSG_NOT_ALLOWED_IN_INSERT_MODE[];
extern const char MSG_OPTIONS_SYNTAX_ERROR[];
extern const char MSG_OUT_OF_RANGE_TAB_VALUE[];
extern const char MSG_PARAMETER_TOO_LONG[];
extern const char MSG_PROMPTS_ARE_ONE_LINE[];
extern const char MSG_SCREEN_MODE_ONLY[];
extern const char MSG_SCREEN_WIDTH_INVALID[];
extern const char MSG_SPAN_MUST_BE_ONE_LINE[];
extern const char MSG_SPAN_NAMES_ARE_ONE_LINE[];
extern const char MSG_SPAN_OF_THAT_NAME_EXISTS[];
extern const char MSG_ENQUIRY_MUST_BE_ONE_LINE[];
extern const char MSG_UNKNOWN_ITEM[];
extern const char MSG_SYNTAX_ERROR[];
extern const char MSG_SYNTAX_ERROR_IN_OPTIONS[];
extern const char MSG_SYNTAX_ERROR_IN_PARAM_CMD[];
extern const char MSG_TOP_MARGIN_LSS_BOTTOM[];
extern const char MSG_TPAR_TOO_DEEP[];
extern const char MSG_UNKNOWN_OPTION[];
extern const char MSG_PAT_NO_MATCHING_DELIM[];
extern const char MSG_PAT_ILLEGAL_PARAMETER[];
extern const char MSG_PAT_ILLEGAL_MARK_NUMBER[];
extern const char MSG_PAT_PREMATURE_PATTERN_END[];
extern const char MSG_PAT_SET_NOT_DEFINED[];
extern const char MSG_PAT_ILLEGAL_SYMBOL[];
extern const char MSG_PAT_SYNTAX_ERROR[];
extern const char MSG_PAT_NULL_PATTERN[];
extern const char MSG_PAT_PATTERN_TOO_COMPLEX[];
extern const char MSG_PAT_ERROR_IN_SPAN[];
extern const char MSG_PAT_ERROR_IN_RANGE[];
extern const char MSG_RESERVED_TPD[];
extern const char MSG_INTEGER_NOT_IN_RANGE[];
extern const char MSG_MODE_ERROR[];
extern const char MSG_WRITING_FILE[];
extern const char MSG_LOADING_FILE[];
extern const char MSG_SAVING_FILE[];
extern const char MSG_PAGING[];
extern const char MSG_SEARCHING[];
extern const char MSG_QUITTING[];
extern const char MSG_NO_OUTPUT[];
extern const char MSG_NOT_MODIFIED[];
extern const char MSG_NOT_RENAMED[];
extern const char MSG_CANT_INVOKE[];
extern const char MSG_EXCEEDED_DYNAMIC_MEMORY[];
extern const char MSG_INCONSISTENT_QUALIFIER[];
extern const char MSG_UNRECOGNIZED_KEY_NAME[];
extern const char MSG_KEY_NAME_TRUNCATED[];
extern const char DBG_INTERNAL_LOGIC_ERROR[];
extern const char DBG_BADFILE[];
extern const char DBG_CANT_MARK_SCR_BOT_LINE[];
extern const char DBG_CODE_PTR_IS_NIL[];
extern const char DBG_FAILED_TO_UNLOAD_ALL_SCR[];
extern const char DBG_FATAL_ERROR_SET[];
extern const char DBG_FIRST_FOLLOWS_LAST[];
extern const char DBG_FIRST_NOT_AT_TOP[];
extern const char DBG_FLINK_OR_BLINK_NOT_NIL[];
extern const char DBG_FRAME_CREATION_FAILED[];
extern const char DBG_FRAME_PTR_IS_NIL[];
extern const char DBG_GROUP_HAS_LINES[];
extern const char DBG_GROUP_PTR_IS_NIL[];
extern const char DBG_ILLEGAL_INSTRUCTION[];
extern const char DBG_INVALID_BLINK[];
extern const char DBG_INVALID_COLUMN_NUMBER[];
extern const char DBG_INVALID_FLAGS[];
extern const char DBG_INVALID_FRAME_PTR[];
extern const char DBG_INVALID_GROUP_PTR[];
extern const char DBG_INVALID_LINE_LENGTH[];
extern const char DBG_INVALID_LINE_NR[];
extern const char DBG_INVALID_LINE_PTR[];
extern const char DBG_INVALID_LINE_USED_LENGTH[];
extern const char DBG_INVALID_NR_LINES[];
extern const char DBG_INVALID_OFFSET_NR[];
extern const char DBG_INVALID_SCR_PARAM[];
extern const char DBG_INVALID_SCR_ROW_NR[];
extern const char DBG_INVALID_SPAN_PTR[];
extern const char DBG_LAST_NOT_AT_END[];
extern const char DBG_LIBRARY_ROUTINE_FAILURE[];
extern const char DBG_LINE_FROM_NUMBER_FAILED[];
extern const char DBG_LINE_HAS_MARKS[];
extern const char DBG_LINE_IS_EOP[];
extern const char DBG_LINE_NOT_IN_SCR_FRAME[];
extern const char DBG_LINE_ON_SCREEN[];
extern const char DBG_LINE_PTR_IS_NIL[];
extern const char DBG_LINE_TO_NUMBER_FAILED[];
extern const char DBG_LINES_FROM_DIFF_FRAMES[];
extern const char DBG_MARK_IN_WRONG_FRAME[];
extern const char DBG_MARK_MOVE_FAILED[];
extern const char DBG_MARK_PTR_IS_NIL[];
extern const char DBG_MARKS_FROM_DIFF_FRAMES[];
extern const char DBG_NEEDED_FRAME_NOT_FOUND[];
extern const char DBG_NOT_IMMED_CMD[];
extern const char DBG_NXT_NOT_NIL[];
extern const char DBG_PC_OUT_OF_RANGE[];
extern const char DBG_REF_COUNT_IS_ZERO[];
extern const char DBG_REPEAT_NEGATIVE[];
extern const char DBG_SPAN_NOT_DESTROYED[];
extern const char DBG_TOP_LINE_NOT_DRAWN[];
extern const char DBG_TPAR_NIL[];
extern const char DBG_WRONG_ROW_NR[];

#endif // !defined(CONST_H)
