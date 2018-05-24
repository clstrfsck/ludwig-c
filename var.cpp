/** @file var.cpp
 * Definitions of variables used throughout the Ludwig code base.
 */

#include "var.h"

name_str              ludwig_version;
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
char_set                                 repeatsyms;
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
str_object         blank_string;
verify_array       initial_verify;
tab_array          default_tab_stops;

// STRUCTURE POOLS
group_ptr          free_group_pool;
line_ptr           free_line_pool;
mark_ptr           free_mark_pool;

// Pattern matcher parser stuff
accept_set_type    printable_set;
accept_set_type    space_set;
accept_set_type    alpha_set;
accept_set_type    lower_set;
accept_set_type    upper_set;
accept_set_type    numeric_set;
accept_set_type    punctuation_set;

// Output file actions
file_data_type     file_data;

// Info about the terminal
terminal_info_type terminal_info;

// Word definition sets
parray<accept_set_type, word_set_range> word_elements;
