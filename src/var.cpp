/** @file var.cpp
 * Definitions of variables used throughout the Ludwig code base.
 */

#include "var.h"

const std::string     ludwig_version("X5.0-006");
std::string           program_directory; // Used to determine program startup directory.
//FIXME: terminal_capabilities tt_capabilities;   // H/W abilities of terminal.
bool                  tt_controlc;       // User has typed CNTRL/C.
bool                  tt_winchanged;     // Window size has changed

// Keyboard interface.
key_names_range       nr_key_names;
key_name_record_ptr   key_name_list_ptr;
accept_set_type       key_introducers;

// SPECIAL FRAMES.
frame_ptr             current_frame;     // Compiler/Interpreter use as focal frame.
frame_ptr             frame_oops;        // Pointer to frame OOPS.
frame_ptr             frame_cmd;         // Pointer to frame COMMAND.
frame_ptr             frame_heap;        // pointer to frame HEAP.

// GLOBAL VARIABLES.
bool                  ludwig_aborted;    // Something terrible has happened....
bool                  exit_abort;        // Set for doing XA commands.
bool                  vdu_free_flag;     // Set if vdu_free has been called already
bool                  hangup;            // Ludwig has received a hangup signal.

mode_type             edit_mode;         // User selectable editing mode.
mode_type             previous_mode;

parray<file_ptr, file_range>  files;     // I/O file pointers.
parray<frame_ptr, file_range> files_frames;

slot_range fgi_file;
slot_range fgo_file;

span_ptr   first_span;                   // Pointer to first in the list of spans.

ludwig_mode_type ludwig_mode;
key_code_range   command_introducer;     // Key used to prefix immediate commands.

parray<prompt_region_attrib, prange<1, MAX_TPCOUNT>> prompt_region;

frame_ptr scr_frame;                     // Frame that screen currently mapped into.
line_ptr scr_top_line;                   // Pointer to first line mapped on screen.
line_ptr scr_bot_line;                   // Pointer to last line mapped on screen.
scr_row_range scr_msg_row;               // First (highest) msg on scr, 0 if none.
bool    scr_needs_fix;                   // Set when user is viewing a corrupt screen.

// COMPILER VARIABLES.
parray<code_object, prange<1, MAX_CODE>> compiler_code;
code_ptr                                 code_list;
prange<0, MAX_CODE>                      code_top;

// VARIABLES USED IN INTERPRETING A COMMAND
penumset<commands>                       prefixes;
parray<command_object, key_code_range>   lookup;
parray<lookupexp_type, expand_lim_range> lookupexp;
parray<expand_lim_range, prefix_plus>    lookupexp_ptr;
parray<cmd_attrib_rec, user_commands>    cmd_attrib;
parray<prompt_str, perange<prompt_type>> dflt_prompts;
prange<0, MAX_EXEC_RECURSION>            exec_level;

// INITIAL FRAME SETTINGS.
mark_array         initial_marks;
scr_row_range      initial_scr_height;
scr_col_range      initial_scr_width;
col_offset_range   initial_scr_offset;
col_range          initial_margin_left;
col_range          initial_margin_right;
scr_row_range      initial_margin_top;
scr_row_range      initial_margin_bottom;
tab_array          initial_tab_stops;
frame_options      initial_options;

// USEFUL STUFF.
const str_object   BLANK_STRING(' ');       // Blank chars only
const verify_array INITIAL_VERIFY(false);   //
const tab_array    DEFAULT_TAB_STOPS({false, true, false, false, false, false, false, false});

// STRUCTURE POOLS
group_ptr          free_group_pool;
line_ptr           free_line_pool;
mark_ptr           free_mark_pool;

// Output file actions
file_data_type     file_data;

// Info about the terminal
terminal_info_type terminal_info;

// Word definition sets
parray<accept_set_type, word_set_range> word_elements;

// Pattern matcher parser stuff

// ' '
// the S (space) pattern specifier
const accept_set_type SPACE_SET(' ');
// 'a'..'z'
// the L (lowercase) pattern specifier
const accept_set_type LOWER_SET('a', 'z');
// 'A'..'Z'
// the U (uppercase) pattern specifier
const accept_set_type UPPER_SET('A', 'Z');
// the A (alphabetic) pattern specifier
const accept_set_type ALPHA_SET(LOWER_SET.set_union(UPPER_SET));
// '0'..'9'
// the N (numeric) pattern specifier
const accept_set_type NUMERIC_SET('0', '9');
// ' '..'~'
// the C (printable char) pattern specifier
const accept_set_type PRINTABLE_SET(32, 126);
// '!','"','''','(',')',',','.',':',';','?','`'
// the P (punctuation) pattern specifier
const accept_set_type PUNCTUATION_SET({33, 34,  39, 40, 41, 44, 46, 58, 59, 63, 96});
