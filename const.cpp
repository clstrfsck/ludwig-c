/** @file const.cpp
 * Definitions of constants used throughout the Ludwig code base.
 */

#include "const.h"

// Frame names
const char BLANK_FRAME_NAME[]   = "                               ";
const char DEFAULT_FRAME_NAME[] = "LUDWIG                         ";

// Messages
const char MSG_BLANK[]                      = "                                                                      ";
const char MSG_ABORT[]                      = "Aborted. Output Files may be CORRUPTED                                ";
const char MSG_BAD_FORMAT_IN_TAB_TABLE[]    = "Bad Format for list of Tab stops.                                     ";
const char MSG_CANT_KILL_FRAME[]            = "Can't Kill Frame.                                                     ";
const char MSG_CANT_SPLIT_NULL_LINE[]       = "Can't split the Null line.                                            ";
const char MSG_COMMAND_NOT_VALID[]          = "No Command starts with this character.                                ";
const char MSG_COMMAND_RECURSION_LIMIT[]    = "Command recursion limit exceeded.                                     ";
const char MSG_COMMENTS_ILLEGAL[]           = "Immediate mode comments are not allowed.                              ";
const char MSG_COMPILER_CODE_OVERFLOW[]     = "Compiler code overflow, too many compiled spans.                      ";
const char MSG_COPYRIGHT_AND_LOADING_FILE[] = "Copyright (C) 1981, 1987,  University of Adelaide.                    ";
const char MSG_COUNT_TOO_LARGE[]            = "Count too large.                                                      ";
const char MSG_DECOMMITTED[]                = "Warning - Decommitted feature.                                        ";
const char MSG_EMPTY_SPAN[]                 = "Span is empty.                                                        ";
const char MSG_EQUALS_NOT_SET[]             = "The Equals mark is not defined.                                       ";
const char MSG_ERROR_OPENING_KEYS_FILE[]    = "Error opening keys definitions file.                                  ";
const char MSG_EXECUTING_INIT_FILE[]        = "Executing initialization file.                                        ";
const char MSG_FILE_ALREADY_IN_USE[]        = "File already in use.                                                  ";
const char MSG_FILE_ALREADY_OPEN[]          = "File already open.                                                    ";
const char MSG_FRAME_HAS_FILES_ATTACHED[]   = "Frame Has Files Attached.                                             ";
const char MSG_FRAME_OF_THAT_NAME_EXISTS[]  = "Frame of that Name Already Exists.                                    ";
const char MSG_ILLEGAL_LEADING_PARAM[]      = "Illegal leading parameter.                                            ";
const char MSG_ILLEGAL_MARK_NUMBER[]        = "Illegal mark number.                                                  ";
const char MSG_ILLEGAL_PARAM_DELIMITER[]    = "Illegal parameter delimiter.                                          ";
const char MSG_INCOMPAT[]                   = "Incompatible switches specified.                                      ";
const char MSG_INTERACTIVE_MODE_ONLY[]      = "Allowed in interactive mode only.                                     ";
const char MSG_INVALID_CMD_INTRODUCER[]     = "Invalid command introducer.                                           ";
const char MSG_INVALID_INTEGER[]            = "Trailing parameter integer is invalid.                                ";
const char MSG_INVALID_KEYS_FILE[]          = "Invalid keys definition file.                                         ";
const char MSG_INVALID_SCREEN_HEIGHT[]      = "Invalid height for screen.                                            ";
const char MSG_INVALID_SLOT_NUMBER[]        = "Invalid file slot number.                                             ";
const char MSG_INVALID_T_OPTION[]           = "Invalid Tab Option.                                                   ";
const char MSG_INVALID_RULER[]              = "Invalid Ruler.                                                        ";
const char MSG_INVALID_PARAMETER_CODE[]     = "Invalid Parameter Code.                                               ";
const char MSG_LEFT_MARGIN_GE_RIGHT[]       = "Specified Left Margin is not less than Right Margin.                  ";
const char MSG_LONG_INPUT_LINE[]            = "Long input line has been split.                                       ";
const char MSG_MARGIN_OUT_OF_RANGE[]        = "Margin out of Range.                                                  ";
const char MSG_MARGIN_SYNTAX_ERROR[]        = "Margin Syntax Error.                                                  ";
const char MSG_MARK_NOT_DEFINED[]           = "Mark Not Defined.                                                     ";
const char MSG_MISSING_TRAILING_DELIM[]     = "Missing trailing delimiter.                                           ";
const char MSG_NO_DEFAULT_STR[]             = "No default for trailing parameter string.                             ";
const char MSG_NO_FILE_OPEN[]               = "No file open.                                                         ";
const char MSG_NO_MORE_FILES_ALLOWED[]      = "No more files are allowed.                                            ";
const char MSG_NO_ROOM_ON_LINE[]            = "Operation would cause a line to become too long.                      ";
const char MSG_NO_SUCH_FRAME[]              = "No such frame.                                                        ";
const char MSG_NO_SUCH_SPAN[]               = "No such span.                                                         ";
const char MSG_NONPRINTABLE_INTRODUCER[]    = "Command Introducer is not printable                                   ";
const char MSG_NOT_ENOUGH_INPUT_LEFT[]      = "Not enough input left to satisfy request.                             ";
const char MSG_NOT_IMPLEMENTED[]            = "Not implemented.                                                      ";
const char MSG_NOT_INPUT_FILE[]             = "File is not an input file.                                            ";
const char MSG_NOT_OUTPUT_FILE[]            = "File is not an output file.                                           ";
const char MSG_NOT_WHILE_EDITING_CMD[]      = "Operation not allowed while editing frame COMMAND.                    ";
const char MSG_NOT_ALLOWED_IN_INSERT_MODE[] = "Command not allowed in insert mode.                                   ";
const char MSG_OPTIONS_SYNTAX_ERROR[]       = "Syntax error in options.                                              ";
const char MSG_OUT_OF_RANGE_TAB_VALUE[]     = "Invalid value for tab stop.                                           ";
const char MSG_PARAMETER_TOO_LONG[]         = "Parameter is too long.                                                ";
const char MSG_PROMPTS_ARE_ONE_LINE[]       = "A prompt string must be on one line.                                  ";
const char MSG_SCREEN_MODE_ONLY[]           = "Command allowed in screen mode only.                                  ";
const char MSG_SCREEN_WIDTH_INVALID[]       = "Invalid screen width specified.                                       ";
const char MSG_SPAN_MUST_BE_ONE_LINE[]      = "A span used as a trailing parameter for this command must be one line.";
const char MSG_SPAN_NAMES_ARE_ONE_LINE[]    = "A span name must be on one line.                                      ";
const char MSG_SPAN_OF_THAT_NAME_EXISTS[]   = "Span of that name already exists.                                     ";
const char MSG_ENQUIRY_MUST_BE_ONE_LINE[]   = "An enquiry item must be on one line.                                  ";
const char MSG_UNKNOWN_ITEM[]               = "Unknown enquiry item.                                                 ";
const char MSG_SYNTAX_ERROR[]               = "Syntax error.                                                         ";
const char MSG_SYNTAX_ERROR_IN_OPTIONS[]    = "Syntax error in options.                                              ";
const char MSG_SYNTAX_ERROR_IN_PARAM_CMD[]  = "Syntax error in parameter command.                                    ";
const char MSG_TOP_MARGIN_LSS_BOTTOM[]      = "Top margin must be less than or equal to bottom margin.               ";
const char MSG_TPAR_TOO_DEEP[]              = "Trailing parameter translation has gone too deep.                     ";
const char MSG_UNKNOWN_OPTION[]             = "Not a valid option.                                                   ";
const char MSG_PAT_NO_MATCHING_DELIM[]      = "Pattern - No matching delimiter in pattern.                           ";
const char MSG_PAT_ILLEGAL_PARAMETER[]      = "Pattern - Illegal parameter in pattern.                               ";
const char MSG_PAT_ILLEGAL_MARK_NUMBER[]    = "Pattern - Illegal mark number in pattern.                             ";
const char MSG_PAT_PREMATURE_PATTERN_END[]  = "Pattern - Premature pattern end.                                      ";
const char MSG_PAT_SET_NOT_DEFINED[]        = "Pattern - Set not defined.                                            ";
const char MSG_PAT_ILLEGAL_SYMBOL[]         = "Pattern - Illegal symbol in pattern.                                  ";
const char MSG_PAT_SYNTAX_ERROR[]           = "Pattern - Syntax error in pattern.                                    ";
const char MSG_PAT_NULL_PATTERN[]           = "Pattern - Null pattern.                                               ";
const char MSG_PAT_PATTERN_TOO_COMPLEX[]    = "Pattern - Pattern too complex.                                        ";
const char MSG_PAT_ERROR_IN_SPAN[]          = "Pattern - Error in dereferenced span.                                 ";
const char MSG_PAT_ERROR_IN_RANGE[]         = "Pattern - Error in range specification.                               ";
const char MSG_RESERVED_TPD[]               = "Delimiter reserved for future use.                                    ";
const char MSG_INTEGER_NOT_IN_RANGE[]       = "Integer not in range                                                  ";
const char MSG_MODE_ERROR[]                 = "Illegal Mode specification -- must be O,C or I                        ";
const char MSG_WRITING_FILE[]               = "Writing File.                                                         ";
const char MSG_LOADING_FILE[]               = "Loading File.                                                         ";
const char MSG_SAVING_FILE[]                = "Saving File.                                                          ";
const char MSG_PAGING[]                     = "Paging.                                                               ";
const char MSG_SEARCHING[]                  = "Searching.                                                            ";
const char MSG_QUITTING[]                   = "Quitting.                                                             ";
const char MSG_NO_OUTPUT[]                  = "This Frame has no Output File attached.                               ";
const char MSG_NOT_MODIFIED[]               = "This Frame has not been modified.                                     ";
const char MSG_NOT_RENAMED[]                = "Output Files have '-lw*' appended to filename                         ";
const char MSG_CANT_INVOKE[]                = "Character cannot be invoked by a key                                  ";
const char MSG_EXCEEDED_DYNAMIC_MEMORY[]    = "Exceeded dynamic memory limit.                                        ";
const char MSG_INCONSISTENT_QUALIFIER[]     = "Use of this qualifier is inconsistent with file operation             ";
const char MSG_UNRECOGNIZED_KEY_NAME[]      = "Unrecognized key name                                                 ";
const char MSG_KEY_NAME_TRUNCATED[]         = "Key name too long, name truncated                                     ";
const char DBG_INTERNAL_LOGIC_ERROR[]       = "Internal logic error.                                                 ";
const char DBG_BADFILE[]                    = "FILE and FILESYS definition of file_object disagree.                  ";
const char DBG_CANT_MARK_SCR_BOT_LINE[]     = "Can't mark scr bot line.                                              ";
const char DBG_CODE_PTR_IS_NIL[]            = "Code ptr is nil.                                                      ";
const char DBG_FAILED_TO_UNLOAD_ALL_SCR[]   = "Failed to unload all scr.                                             ";
const char DBG_FATAL_ERROR_SET[]            = "Fatal error set.                                                      ";
const char DBG_FIRST_FOLLOWS_LAST[]         = "First follows last.                                                   ";
const char DBG_FIRST_NOT_AT_TOP[]           = "First Not at Top.                                                     ";
const char DBG_FLINK_OR_BLINK_NOT_NIL[]     = "Flink or blink not nil.                                               ";
const char DBG_FRAME_CREATION_FAILED[]      = "Frame creation failed.                                                ";
const char DBG_FRAME_PTR_IS_NIL[]           = "Frame ptr is nil.                                                     ";
const char DBG_GROUP_HAS_LINES[]            = "Group has lines.                                                      ";
const char DBG_GROUP_PTR_IS_NIL[]           = "Group ptr is nil.                                                     ";
const char DBG_ILLEGAL_INSTRUCTION[]        = "Illegal Instruction.                                                  ";
const char DBG_INVALID_BLINK[]              = "Incorrect blink.                                                      ";
const char DBG_INVALID_COLUMN_NUMBER[]      = "Invalid column number.                                                ";
const char DBG_INVALID_FLAGS[]              = "Invalid flags.                                                        ";
const char DBG_INVALID_FRAME_PTR[]          = "Invalid frame ptr.                                                    ";
const char DBG_INVALID_GROUP_PTR[]          = "Invalid group ptr.                                                    ";
const char DBG_INVALID_LINE_LENGTH[]        = "Invalid line length.                                                  ";
const char DBG_INVALID_LINE_NR[]            = "Invalid line nr.                                                      ";
const char DBG_INVALID_LINE_PTR[]           = "Invalid line ptr.                                                     ";
const char DBG_INVALID_LINE_USED_LENGTH[]   = "Invalid line used length.                                             ";
const char DBG_INVALID_NR_LINES[]           = "Invalid nr lines.                                                     ";
const char DBG_INVALID_OFFSET_NR[]          = "Invalid offset nr.                                                    ";
const char DBG_INVALID_SCR_PARAM[]          = "Invalid SCR Parameter.                                                ";
const char DBG_INVALID_SCR_ROW_NR[]         = "Invalid scr row nr.                                                   ";
const char DBG_INVALID_SPAN_PTR[]           = "Invalid span ptr.                                                     ";
const char DBG_LAST_NOT_AT_END[]            = "Last not at end.                                                      ";
const char DBG_LIBRARY_ROUTINE_FAILURE[]    = "Library routine call failed.                                          ";
const char DBG_LINE_FROM_NUMBER_FAILED[]    = "Line from number failed.                                              ";
const char DBG_LINE_HAS_MARKS[]             = "Line has marks.                                                       ";
const char DBG_LINE_IS_EOP[]                = "Line is eop.                                                          ";
const char DBG_LINE_NOT_IN_SCR_FRAME[]      = "Line not in scr frame.                                                ";
const char DBG_LINE_ON_SCREEN[]             = "Line on screen.                                                       ";
const char DBG_LINE_PTR_IS_NIL[]            = "Line ptr is nil.                                                      ";
const char DBG_LINE_TO_NUMBER_FAILED[]      = "Line to number failed.                                                ";
const char DBG_LINES_FROM_DIFF_FRAMES[]     = "Lines from diff frames.                                               ";
const char DBG_MARK_IN_WRONG_FRAME[]        = "Mark in wrong frame.                                                  ";
const char DBG_MARK_MOVE_FAILED[]           = "Mark move failed.                                                     ";
const char DBG_MARK_PTR_IS_NIL[]            = "Mark ptr is nil.                                                      ";
const char DBG_MARKS_FROM_DIFF_FRAMES[]     = "Marks from diff frames.                                               ";
const char DBG_NEEDED_FRAME_NOT_FOUND[]     = "Frame C or OOPS is not in the span list                               ";
const char DBG_NOT_IMMED_CMD[]              = "Not immed cmd.                                                        ";
const char DBG_NXT_NOT_NIL[]                = "Nxt should be nil here.                                               ";
const char DBG_PC_OUT_OF_RANGE[]            = "PC is out of Range.                                                   ";
const char DBG_REF_COUNT_IS_ZERO[]          = "Reference count is zero.                                              ";
const char DBG_REPEAT_NEGATIVE[]            = "Repeat Negative.                                                      ";
const char DBG_SPAN_NOT_DESTROYED[]         = "Span not destroyed.                                                   ";
const char DBG_TOP_LINE_NOT_DRAWN[]         = "Top line not drawn.                                                   ";
const char DBG_TPAR_NIL[]                   = "Tpar should not be nil.                                               ";
const char DBG_WRONG_ROW_NR[]               = "Wrong row nr.                                                         ";
