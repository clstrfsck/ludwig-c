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

#include <string>
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
inline constexpr std::string_view BLANK_FRAME_NAME{ "" };
inline constexpr std::string_view DEFAULT_FRAME_NAME{ "LUDWIG" };

// TPar interpretations
const char TPD_LIT         = '\''; // don't do fancy processing on this one
const char TPD_SMART       = '`';  // search target is a pattern
const char TPD_EXACT       = '"';  // use exact case during search
const char TPD_SPAN        = '$';  // span substitution
const char TPD_PROMPT      = '&';  // get parameter from user terminal
const char TPD_ENVIRONMENT = '?';  // environment enquiry
const int  EXPAND_LIM      = 130;  // at least # of multi-letter commands + 1

// String lengths
const int NAME_LEN      = 31;            // Max length of a spn/frm name
const int FILE_NAME_LEN = FILENAME_MAX;  // Default length of file name
const int TPAR_PROM_LEN = 8;
const int KEY_LEN       = 4;             // Key length for HELP FILE

// Keyboard interface.
const int MAX_SPECIAL_KEYS = 1000; // Taken from original XWin def
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
inline constexpr std::string_view MSG_BLANK                      { "" };
inline constexpr std::string_view MSG_ABORT                      { "Aborted. Output Files may be CORRUPTED" };
inline constexpr std::string_view MSG_BAD_FORMAT_IN_TAB_TABLE    { "Bad Format for list of Tab stops." };
inline constexpr std::string_view MSG_CANT_KILL_FRAME            { "Can't Kill Frame." };
inline constexpr std::string_view MSG_CANT_SPLIT_NULL_LINE       { "Can't split the Null line." };
inline constexpr std::string_view MSG_COMMAND_NOT_VALID          { "No Command starts with this character." };
inline constexpr std::string_view MSG_COMMAND_RECURSION_LIMIT    { "Command recursion limit exceeded." };
inline constexpr std::string_view MSG_COMMENTS_ILLEGAL           { "Immediate mode comments are not allowed." };
inline constexpr std::string_view MSG_COMPILER_CODE_OVERFLOW     { "Compiler code overflow, too many compiled spans." };
inline constexpr std::string_view MSG_COPYRIGHT_AND_LOADING_FILE { "Copyright (C) 1981, 1987,  University of Adelaide." };
inline constexpr std::string_view MSG_COUNT_TOO_LARGE            { "Count too large." };
inline constexpr std::string_view MSG_DECOMMITTED                { "Warning - Decommitted feature." };
inline constexpr std::string_view MSG_EMPTY_SPAN                 { "Span is empty." };
inline constexpr std::string_view MSG_EQUALS_NOT_SET             { "The Equals mark is not defined." };
inline constexpr std::string_view MSG_ERROR_OPENING_KEYS_FILE    { "Error opening keys definitions file." };
inline constexpr std::string_view MSG_EXECUTING_INIT_FILE        { "Executing initialization file." };
inline constexpr std::string_view MSG_FILE_ALREADY_IN_USE        { "File already in use." };
inline constexpr std::string_view MSG_FILE_ALREADY_OPEN          { "File already open." };
inline constexpr std::string_view MSG_FRAME_HAS_FILES_ATTACHED   { "Frame Has Files Attached." };
inline constexpr std::string_view MSG_FRAME_OF_THAT_NAME_EXISTS  { "Frame of that Name Already Exists." };
inline constexpr std::string_view MSG_ILLEGAL_LEADING_PARAM      { "Illegal leading parameter." };
inline constexpr std::string_view MSG_ILLEGAL_MARK_NUMBER        { "Illegal mark number." };
inline constexpr std::string_view MSG_ILLEGAL_PARAM_DELIMITER    { "Illegal parameter delimiter." };
inline constexpr std::string_view MSG_INCOMPAT                   { "Incompatible switches specified." };
inline constexpr std::string_view MSG_INTERACTIVE_MODE_ONLY      { "Allowed in interactive mode only." };
inline constexpr std::string_view MSG_INVALID_CMD_INTRODUCER     { "Invalid command introducer." };
inline constexpr std::string_view MSG_INVALID_INTEGER            { "Trailing parameter integer is invalid." };
inline constexpr std::string_view MSG_INVALID_KEYS_FILE          { "Invalid keys definition file." };
inline constexpr std::string_view MSG_INVALID_SCREEN_HEIGHT      { "Invalid height for screen." };
inline constexpr std::string_view MSG_INVALID_SLOT_NUMBER        { "Invalid file slot number." };
inline constexpr std::string_view MSG_INVALID_T_OPTION           { "Invalid Tab Option." };
inline constexpr std::string_view MSG_INVALID_RULER              { "Invalid Ruler." };
inline constexpr std::string_view MSG_INVALID_PARAMETER_CODE     { "Invalid Parameter Code." };
inline constexpr std::string_view MSG_LEFT_MARGIN_GE_RIGHT       { "Specified Left Margin is not less than Right Margin." };
inline constexpr std::string_view MSG_LONG_INPUT_LINE            { "Long input line has been split." };
inline constexpr std::string_view MSG_MARGIN_OUT_OF_RANGE        { "Margin out of Range." };
inline constexpr std::string_view MSG_MARGIN_SYNTAX_ERROR        { "Margin Syntax Error." };
inline constexpr std::string_view MSG_MARK_NOT_DEFINED           { "Mark Not Defined." };
inline constexpr std::string_view MSG_MISSING_TRAILING_DELIM     { "Missing trailing delimiter." };
inline constexpr std::string_view MSG_NO_DEFAULT_STR             { "No default for trailing parameter string." };
inline constexpr std::string_view MSG_NO_FILE_OPEN               { "No file open." };
inline constexpr std::string_view MSG_NO_MORE_FILES_ALLOWED      { "No more files are allowed." };
inline constexpr std::string_view MSG_NO_ROOM_ON_LINE            { "Operation would cause a line to become too long." };
inline constexpr std::string_view MSG_NO_SUCH_FRAME              { "No such frame." };
inline constexpr std::string_view MSG_NO_SUCH_SPAN               { "No such span." };
inline constexpr std::string_view MSG_NONPRINTABLE_INTRODUCER    { "Command Introducer is not printable" };
inline constexpr std::string_view MSG_NOT_ENOUGH_INPUT_LEFT      { "Not enough input left to satisfy request." };
inline constexpr std::string_view MSG_NOT_IMPLEMENTED            { "Not implemented." };
inline constexpr std::string_view MSG_NOT_INPUT_FILE             { "File is not an input file." };
inline constexpr std::string_view MSG_NOT_OUTPUT_FILE            { "File is not an output file." };
inline constexpr std::string_view MSG_NOT_WHILE_EDITING_CMD      { "Operation not allowed while editing frame COMMAND." };
inline constexpr std::string_view MSG_NOT_ALLOWED_IN_INSERT_MODE { "Command not allowed in insert mode." };
inline constexpr std::string_view MSG_OPTIONS_SYNTAX_ERROR       { "Syntax error in options." };
inline constexpr std::string_view MSG_OUT_OF_RANGE_TAB_VALUE     { "Invalid value for tab stop." };
inline constexpr std::string_view MSG_PARAMETER_TOO_LONG         { "Parameter is too long." };
inline constexpr std::string_view MSG_PROMPTS_ARE_ONE_LINE       { "A prompt string must be on one line." };
inline constexpr std::string_view MSG_SCREEN_MODE_ONLY           { "Command allowed in screen mode only." };
inline constexpr std::string_view MSG_SCREEN_WIDTH_INVALID       { "Invalid screen width specified." };
inline constexpr std::string_view MSG_SPAN_MUST_BE_ONE_LINE      { "A span used as a trailing parameter for this command must be one line." };
inline constexpr std::string_view MSG_SPAN_NAMES_ARE_ONE_LINE    { "A span name must be on one line." };
inline constexpr std::string_view MSG_SPAN_OF_THAT_NAME_EXISTS   { "Span of that name already exists." };
inline constexpr std::string_view MSG_ENQUIRY_MUST_BE_ONE_LINE   { "An enquiry item must be on one line." };
inline constexpr std::string_view MSG_UNKNOWN_ITEM               { "Unknown enquiry item." };
inline constexpr std::string_view MSG_SYNTAX_ERROR               { "Syntax error." };
inline constexpr std::string_view MSG_SYNTAX_ERROR_IN_OPTIONS    { "Syntax error in options." };
inline constexpr std::string_view MSG_SYNTAX_ERROR_IN_PARAM_CMD  { "Syntax error in parameter command." };
inline constexpr std::string_view MSG_TOP_MARGIN_LSS_BOTTOM      { "Top margin must be less than or equal to bottom margin." };
inline constexpr std::string_view MSG_TPAR_TOO_DEEP              { "Trailing parameter translation has gone too deep." };
inline constexpr std::string_view MSG_UNKNOWN_OPTION             { "Not a valid option." };
inline constexpr std::string_view MSG_PAT_NO_MATCHING_DELIM      { "Pattern - No matching delimiter in pattern." };
inline constexpr std::string_view MSG_PAT_ILLEGAL_PARAMETER      { "Pattern - Illegal parameter in pattern." };
inline constexpr std::string_view MSG_PAT_ILLEGAL_MARK_NUMBER    { "Pattern - Illegal mark number in pattern." };
inline constexpr std::string_view MSG_PAT_PREMATURE_PATTERN_END  { "Pattern - Premature pattern end." };
inline constexpr std::string_view MSG_PAT_SET_NOT_DEFINED        { "Pattern - Set not defined." };
inline constexpr std::string_view MSG_PAT_ILLEGAL_SYMBOL         { "Pattern - Illegal symbol in pattern." };
inline constexpr std::string_view MSG_PAT_SYNTAX_ERROR           { "Pattern - Syntax error in pattern." };
inline constexpr std::string_view MSG_PAT_NULL_PATTERN           { "Pattern - Null pattern." };
inline constexpr std::string_view MSG_PAT_PATTERN_TOO_COMPLEX    { "Pattern - Pattern too complex." };
inline constexpr std::string_view MSG_PAT_ERROR_IN_SPAN          { "Pattern - Error in dereferenced span." };
inline constexpr std::string_view MSG_PAT_ERROR_IN_RANGE         { "Pattern - Error in range specification." };
inline constexpr std::string_view MSG_RESERVED_TPD               { "Delimiter reserved for future use." };
inline constexpr std::string_view MSG_INTEGER_NOT_IN_RANGE       { "Integer not in range" };
inline constexpr std::string_view MSG_MODE_ERROR                 { "Illegal Mode specification -- must be O,C or I" };
inline constexpr std::string_view MSG_WRITING_FILE               { "Writing File." };
inline constexpr std::string_view MSG_LOADING_FILE               { "Loading File." };
inline constexpr std::string_view MSG_SAVING_FILE                { "Saving File." };
inline constexpr std::string_view MSG_PAGING                     { "Paging." };
inline constexpr std::string_view MSG_SEARCHING                  { "Searching." };
inline constexpr std::string_view MSG_QUITTING                   { "Quitting." };
inline constexpr std::string_view MSG_NO_OUTPUT                  { "This Frame has no Output File attached." };
inline constexpr std::string_view MSG_NOT_MODIFIED               { "This Frame has not been modified." };
inline constexpr std::string_view MSG_NOT_RENAMED                { "Output Files have '-lw*' appended to filename" };
inline constexpr std::string_view MSG_CANT_INVOKE                { "Character cannot be invoked by a key" };
inline constexpr std::string_view MSG_EXCEEDED_DYNAMIC_MEMORY    { "Exceeded dynamic memory limit." };
inline constexpr std::string_view MSG_INCONSISTENT_QUALIFIER     { "Use of this qualifier is inconsistent with file operation" };
inline constexpr std::string_view MSG_UNRECOGNIZED_KEY_NAME      { "Unrecognized key name" };
inline constexpr std::string_view MSG_KEY_NAME_TRUNCATED         { "Key name too long, name truncated" };
inline constexpr std::string_view DBG_INTERNAL_LOGIC_ERROR       { "Internal logic error." };
inline constexpr std::string_view DBG_BADFILE                    { "FILE and FILESYS definition of file_object disagree." };
inline constexpr std::string_view DBG_CANT_MARK_SCR_BOT_LINE     { "Can't mark scr bot line." };
inline constexpr std::string_view DBG_CODE_PTR_IS_NIL            { "Code ptr is nil." };
inline constexpr std::string_view DBG_FAILED_TO_UNLOAD_ALL_SCR   { "Failed to unload all scr." };
inline constexpr std::string_view DBG_FATAL_ERROR_SET            { "Fatal error set." };
inline constexpr std::string_view DBG_FIRST_FOLLOWS_LAST         { "First follows last." };
inline constexpr std::string_view DBG_FIRST_NOT_AT_TOP           { "First Not at Top." };
inline constexpr std::string_view DBG_FLINK_OR_BLINK_NOT_NIL     { "Flink or blink not nil." };
inline constexpr std::string_view DBG_FRAME_CREATION_FAILED      { "Frame creation failed." };
inline constexpr std::string_view DBG_FRAME_PTR_IS_NIL           { "Frame ptr is nil." };
inline constexpr std::string_view DBG_GROUP_HAS_LINES            { "Group has lines." };
inline constexpr std::string_view DBG_GROUP_PTR_IS_NIL           { "Group ptr is nil." };
inline constexpr std::string_view DBG_ILLEGAL_INSTRUCTION        { "Illegal Instruction." };
inline constexpr std::string_view DBG_INVALID_BLINK              { "Incorrect blink." };
inline constexpr std::string_view DBG_INVALID_COLUMN_NUMBER      { "Invalid column number." };
inline constexpr std::string_view DBG_INVALID_FLAGS              { "Invalid flags." };
inline constexpr std::string_view DBG_INVALID_FRAME_PTR          { "Invalid frame ptr." };
inline constexpr std::string_view DBG_INVALID_GROUP_PTR          { "Invalid group ptr." };
inline constexpr std::string_view DBG_INVALID_LINE_LENGTH        { "Invalid line length." };
inline constexpr std::string_view DBG_INVALID_LINE_NR            { "Invalid line nr." };
inline constexpr std::string_view DBG_INVALID_LINE_PTR           { "Invalid line ptr." };
inline constexpr std::string_view DBG_INVALID_LINE_USED_LENGTH   { "Invalid line used length." };
inline constexpr std::string_view DBG_INVALID_NR_LINES           { "Invalid nr lines." };
inline constexpr std::string_view DBG_INVALID_OFFSET_NR          { "Invalid offset nr." };
inline constexpr std::string_view DBG_INVALID_SCR_PARAM          { "Invalid SCR Parameter." };
inline constexpr std::string_view DBG_INVALID_SCR_ROW_NR         { "Invalid scr row nr." };
inline constexpr std::string_view DBG_INVALID_SPAN_PTR           { "Invalid span ptr." };
inline constexpr std::string_view DBG_LAST_NOT_AT_END            { "Last not at end." };
inline constexpr std::string_view DBG_LIBRARY_ROUTINE_FAILURE    { "Library routine call failed." };
inline constexpr std::string_view DBG_LINE_FROM_NUMBER_FAILED    { "Line from number failed." };
inline constexpr std::string_view DBG_LINE_HAS_MARKS             { "Line has marks." };
inline constexpr std::string_view DBG_LINE_IS_EOP                { "Line is eop." };
inline constexpr std::string_view DBG_LINE_NOT_IN_SCR_FRAME      { "Line not in scr frame." };
inline constexpr std::string_view DBG_LINE_ON_SCREEN             { "Line on screen." };
inline constexpr std::string_view DBG_LINE_PTR_IS_NIL            { "Line ptr is nil." };
inline constexpr std::string_view DBG_LINE_TO_NUMBER_FAILED      { "Line to number failed." };
inline constexpr std::string_view DBG_LINES_FROM_DIFF_FRAMES     { "Lines from diff frames." };
inline constexpr std::string_view DBG_MARK_IN_WRONG_FRAME        { "Mark in wrong frame." };
inline constexpr std::string_view DBG_MARK_MOVE_FAILED           { "Mark move failed." };
inline constexpr std::string_view DBG_MARK_PTR_IS_NIL            { "Mark ptr is nil." };
inline constexpr std::string_view DBG_MARKS_FROM_DIFF_FRAMES     { "Marks from diff frames." };
inline constexpr std::string_view DBG_NEEDED_FRAME_NOT_FOUND     { "Frame C or OOPS is not in the span list" };
inline constexpr std::string_view DBG_NOT_IMMED_CMD              { "Not immed cmd." };
inline constexpr std::string_view DBG_NXT_NOT_NIL                { "Nxt should be nil here." };
inline constexpr std::string_view DBG_PC_OUT_OF_RANGE            { "PC is out of Range." };
inline constexpr std::string_view DBG_REF_COUNT_IS_ZERO          { "Reference count is zero." };
inline constexpr std::string_view DBG_REPEAT_NEGATIVE            { "Repeat Negative." };
inline constexpr std::string_view DBG_SPAN_NOT_DESTROYED         { "Span not destroyed." };
inline constexpr std::string_view DBG_TOP_LINE_NOT_DRAWN         { "Top line not drawn." };
inline constexpr std::string_view DBG_TPAR_NIL                   { "Tpar should not be nil." };
inline constexpr std::string_view DBG_WRONG_ROW_NR               { "Wrong row nr." };

#endif // !defined(CONST_H)
