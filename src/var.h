/** @file var.h
 * Declarations of variables used throughout the Ludwig code base.
 */

/**
! $Log: var.i,v $
! Revision 4.8  1990/01/22 18:55:12  ludwig
! Steven Nairn.
! removed the declarations of tt_height and tt_width.
!
! Revision 4.7  90/01/19  10:46:42  ludwig
! Declaration of global variable "tt_winchanged" to indicate that the window
! size has been modified. Gets set to true by signal handler for SIGWINCH,
! or resize event from X.
!
! Revision 4.6  90/01/18  17:09:25  ludwig
! Entered into RCS at revision level 4.6
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Kelvin B. Nicolle                                     5-May-1987
!       Add ludwig_version.
! 4-003 Mark R. Prior                                        22-Jun-1987
!   Version 4.1 developments incorporated into main source code:
!   . Replace the global variables tt_width and tt_height by
!     terminal_info.
! 4-004 Kelvin B. Nicolle                                    11-Nov-1988
!       Add hangup.
! 4-005 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-006 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
!**/

#ifndef VAR_H
#define VAR_H

#include "type.h"

#include <string>

extern const std::string     ludwig_version;
extern std::string           program_directory; // Used to determine program startup directory.
extern bool                  tt_controlc;       // User has typed CNTRL/C.
extern bool                  tt_winchanged;     // Window size has changed

// Keyboard interface.
extern key_names_range       nr_key_names;
extern std::vector<key_name_record> key_name_list;
extern accept_set_type       key_introducers;

// SPECIAL FRAMES.
extern frame_ptr             current_frame;     // Compiler/Interpreter use as focal frame.
extern frame_ptr             frame_oops;        // Pointer to frame OOPS.
extern frame_ptr             frame_cmd;         // Pointer to frame COMMAND.
extern frame_ptr             frame_heap;        // pointer to frame HEAP.

// GLOBAL VARIABLES.
extern bool                  ludwig_aborted;    // Something terrible has happened....
extern bool                  exit_abort;        // Set for doing XA commands.
extern bool                  vdu_free_flag;     // Set if vdu_free has been called already
extern bool                  hangup;            // Ludwig has received a hangup signal.

enum class mode_type {
    mode_overtype,
    mode_insert,
    mode_command
};

extern mode_type             edit_mode;         // User selectable editing mode.
extern mode_type             previous_mode;

extern parray<file_ptr, file_range>  files;     // I/O file pointers.
extern parray<frame_ptr, file_range> files_frames;

extern slot_range fgi_file;
extern slot_range fgo_file;

extern span_ptr   first_span;                   // Pointer to first in the list of spans.

enum class ludwig_mode_type {
    ludwig_batch,
    ludwig_hardcopy,
    ludwig_screen
};

extern ludwig_mode_type ludwig_mode;
extern key_code_range   command_introducer;     // Key used to prefix immediate commands.

extern parray<prompt_region_attrib, prange<1, MAX_TPCOUNT>> prompt_region;

extern frame_ptr scr_frame;                     // Frame that screen currently mapped into.
extern line_ptr scr_top_line;                   // Pointer to first line mapped on screen.
extern line_ptr scr_bot_line;                   // Pointer to last line mapped on screen.
extern scr_row_range scr_msg_row;               // First (highest) msg on scr, 0 if none.
extern bool    scr_needs_fix;                   // Set when user is viewing a corrupt screen.

// COMPILER VARIABLES.
extern parray<code_object, prange<1, MAX_CODE>> compiler_code;
extern code_ptr                                 code_list;
extern prange<0, MAX_CODE>                      code_top;

// VARIABLES USED IN INTERPRETING A COMMAND
extern penumset<commands> prefixes;
extern parray<command_object, key_code_range> lookup;

struct lookupexp_type {
    char     extn;
    commands command;
};
using expand_lim_range = prange<1, EXPAND_LIM>;

extern parray<lookupexp_type, expand_lim_range> lookupexp;
extern parray<expand_lim_range, prefix_plus>    lookupexp_ptr;
extern parray<cmd_attrib_rec, user_commands>    cmd_attrib;
extern parray<prompt_str, perange<prompt_type>> dflt_prompts;
extern prange<0, MAX_EXEC_RECURSION>            exec_level;

// INITIAL FRAME SETTINGS.
extern mark_array         initial_marks;
extern scr_row_range      initial_scr_height;
extern scr_col_range      initial_scr_width;
extern col_offset_range   initial_scr_offset;
extern col_range          initial_margin_left;
extern col_range          initial_margin_right;
extern scr_row_range      initial_margin_top;
extern scr_row_range      initial_margin_bottom;
extern tab_array          initial_tab_stops;
extern frame_options      initial_options;

// USEFUL STUFF.
extern const str_object   BLANK_STRING;
extern const verify_array INITIAL_VERIFY;
extern const tab_array    DEFAULT_TAB_STOPS;

// STRUCTURE POOLS
extern group_ptr          free_group_pool;
extern line_ptr           free_line_pool;
extern mark_ptr           free_mark_pool;

// Sets of characters
// Pattern matcher parser stuff
extern const accept_set_type PRINTABLE_SET;
extern const accept_set_type SPACE_SET;
extern const accept_set_type ALPHA_SET;
extern const accept_set_type LOWER_SET;
extern const accept_set_type UPPER_SET;
extern const accept_set_type NUMERIC_SET;
extern const accept_set_type PUNCTUATION_SET;

// Output file actions
extern file_data_type     file_data;

// Info about the terminal
extern terminal_info_type terminal_info;

// Word definition sets
extern parray<accept_set_type, word_set_range> word_elements;

#endif // !defined(VAR_H)
