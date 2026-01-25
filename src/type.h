/** @file type.h
 * Declarations of types used throughout the Ludwig code base.
 */

#ifndef TYPE_H
#define TYPE_H

#include "const.h"
#include "parray.h"
#include "prange.h"
#include "perange.h"
#include "penumset.h"
#include "prangeset.h"

#include <set>
#include <list>
#include <string>
#include <vector>


// POINTERS TO ALL DYNAMIC OBJECTS.

using code_ptr = struct code_header *;
using file_ptr = struct file_object *;
using const_file_ptr = const struct file_object *;
using frame_ptr = struct frame_object *;
using group_ptr = struct group_object *;
using const_group_ptr = const struct group_object *;
using line_ptr = struct line_hdr_object *;
using const_line_ptr = const struct line_hdr_object *;
using mark_ptr = struct mark_object *;
using const_mark_ptr = const struct mark_object *;
using span_ptr = struct span_object *;
using const_span_ptr = const struct span_object *;
using tpar_ptr = struct tpar_object *;
using const_tpar_ptr = const struct tpar_object *;
using dfa_table_ptr = struct dfa_table_object *;
using transition_ptr = struct transition_object *;
using const_transition_ptr = const struct transition_object *;
using state_elt_ptr_type = struct state_elt_object *;
using const_state_elt_ptr_type = const struct state_elt_object *;

// MISCELLANEOUS ENTITIES.

enum class verify_response {
    verify_reply_yes
    ,verify_reply_no
    ,verify_reply_always
    ,verify_reply_quit
};

enum class parse_type {
    parse_command
    ,parse_input
    ,parse_output
    ,parse_edit
    ,parse_stdin
    ,parse_execute
};


// SUBRANGES.

using code_idx = prange<0, MAX_CODE>;
using col_offset_range = prange<0, MAX_STRLEN>;
using code_idx = prange<0, MAX_CODE>;
using col_offset_range = prange<0, MAX_STRLEN>;
using col_range = prange<1, MAX_STRLENP>;
using col_width_range = prange<0, MAX_STRLENP>;
using file_range = prange<1, MAX_FILES>;
using slot_range = prange<0, MAX_FILES>;
using group_line_range = prange<0, MAX_GROUPLINES>;
using line_offset_range = prange<0, MAX_GROUPLINEOFFSET>;
using line_range = prange<0, MAX_LINES>;
using mark_range = prange<MIN_MARK_NUMBER, MAX_MARK_NUMBER>;
using space_range = prange<0, MAXINT>;
using scr_col_range = prange<0, MAX_SCR_COLS>;
using scr_row_range = prange<0, MAX_SCR_ROWS>;
using strlen_range = prange<0, MAX_STRLEN>;
using nfa_state_range = prange<0, MAX_NFA_STATE_RANGE>;
using dfa_state_range = prange<0, MAX_DFA_STATE_RANGE>;
using accept_set_range = prange<0, MAX_SET_RANGE>;
using word_set_range = prange<0, MAX_WORD_SETS_M1>;

// SETS.

enum class frame_options_elts {
    opt_auto_indent,
    opt_auto_wrap,
    opt_new_line,
    opt_special_frame, // OOPS,COMMAND,HEAP
    last_entry
};
using frame_options = penumset<frame_options_elts>;

using nfa_set_type = prangeset<nfa_state_range>;
using dfa_set_type = prangeset<dfa_state_range>;
using accept_set_type = prangeset<accept_set_range>;

// Arrays
using tab_array = parray<bool, col_width_range>;
using verify_array = parray<bool, prange<1, MAX_VERIFY>>;

// Strings
using file_name_str = std::string;
using write_str = std::string;
using key_str = std::string;


// Keyboard interface.
using key_code_range = prange<-MAX_SPECIAL_KEYS, ORD_MAXCHAR>;
using key_names_range = prange<0, MAX_NR_KEY_NAMES>;
struct key_name_record {
    std::string    key_name;
    key_code_range key_code;
};

// Objects
using str_object = parray<char, prange<1, MAX_STRLEN>>;
using str_ptr = str_object *;
using const_str_ptr = const str_object *;

// Trailing parameter for command
struct tpar_object {
    strlen_range len;
    char         dlm;
    str_object   str;
    tpar_ptr     nxt;
    tpar_ptr     con;
};

struct code_header {
    code_ptr            flink;
    code_ptr            blink; // Links into code_list
    size_t              ref;   // Reference count
    prange<1, MAX_CODE> code;  // Pointer into code array
    prange<0, MAX_CODE> len;   // Length of segment
};

struct file_object {
    // FIELDS FOR "FILE.PAS" ONLY.
    bool          valid;
    line_ptr      first_line;  // List of lines read in so far,
    line_ptr      last_line;   // but not yet handed on to any
    line_range    line_count;  // other place. # lines in list.

    // FIELDS SET BY "FILESYS", READ BY "FILE".
    bool          output_flag; // Is this an output file?
    bool          eof;         // Set when inp file reaches eof.
    std::string   filename;    // Length of file name.
    int           l_counter;

    // FIELDS FOR "FILESYS" ONLY.
    std::string   memory;
    std::string   tnm;
    bool          entab;
    bool          create;
    int           fd;
    int           mode;
    int           idx;
    int           len;
    std::vector<char> buf;
    long          previous_file_id;

    // Fields for controlling version backup
    bool          purge;
    size_t        versions;

    // THE FOLLOWING FIELD SHOULD BE SET TO 'Z' BY "FILE",
    // IT IS CHECKED BY "FILESYS" AS A CONSISTENCY CHECK.
    char          zed; // MUST BE 'Z'
};

struct mark_object {
    mark_ptr          next;
    line_ptr          line;
    int               col;
};
using mark_array = parray<mark_ptr, mark_range>;

struct frame_object {
    group_ptr         first_group;
    group_ptr         last_group;
    mark_ptr          dot;
    mark_array        marks;
    scr_row_range     scr_height;
    scr_col_range     scr_width;
    col_offset_range  scr_offset;
    scr_row_range     scr_dot_line;
    span_ptr          span;
    frame_ptr         return_frame;
    uint32_t          input_count;
    space_range       space_limit;
    space_range       space_left;
    bool              text_modified;
    col_range         margin_left;
    col_range         margin_right;
    scr_row_range     margin_top;
    scr_row_range     margin_bottom;
    tab_array         tab_stops;
    frame_options     options;
    slot_range        input_file;
    slot_range        output_file;
    tpar_object       get_tpar;        // Default search targ.
    dfa_table_ptr     get_pattern_ptr; // and pattern
    tpar_object       eqs_tpar;        // Default equals targ.
    dfa_table_ptr     eqs_pattern_ptr;
    tpar_object       rep1_tpar;       // Default replace targ.
    dfa_table_ptr     rep_pattern_ptr;
    tpar_object       rep2_tpar;       // Default replace new.
    tpar_object       verify_tpar;     // Default verify answer
};

struct group_object {
    group_ptr         flink;
    group_ptr         blink;
    frame_ptr         frame;
    line_ptr          first_line;
    line_ptr          last_line;
    line_range        first_line_nr;
    group_line_range  nr_lines;
};

struct line_hdr_object {
    line_ptr          flink;
    line_ptr          blink;
    group_ptr         group;
    line_offset_range offset_nr;
    std::list<mark_ptr> marks;
    str_ptr           str;
    strlen_range      len;
    strlen_range      used;
    scr_row_range     scr_row_nr;
};

struct span_object {
    span_ptr          flink;
    span_ptr          blink;
    frame_ptr         frame;
    mark_ptr          mark_one;
    mark_ptr          mark_two;
    std::string       name;
    code_ptr          code;
};

struct prompt_region_attrib {
    scr_row_range     line_nr;
    line_ptr          redraw;
};

using transition_ptr = struct transition_object *;
struct transition_object {
    accept_set_type   transition_accept_set; // on this input set
    dfa_state_range   accept_next_state;     // goto this dfa state
    transition_ptr    next_transition;       // link to next object
    bool              start_flag;            // special case flag for starting patterns
};

struct nfa_attribute_type {
    nfa_set_type       generator_set;
    state_elt_ptr_type equiv_list;
    nfa_set_type       equiv_set;
};

using state_elt_ptr_type = struct state_elt_object *;
struct state_elt_object {
    nfa_state_range    state_elt;
    state_elt_ptr_type next_elt;
};

struct dfa_state_type {
    transition_ptr     transitions;
    bool               marked;
    nfa_attribute_type nfa_attributes;
    bool               pattern_start;
    bool               final_accept;
    bool               left_transition;
    bool               right_transition;
    bool               left_context_check;
};

struct pattern_def_type {
    str_object        strng;
    int               length;
};

struct dfa_table_object {
    parray<dfa_state_type, dfa_state_range> dfa_table;
    dfa_state_range  dfa_states_used;
    pattern_def_type definition;
};

// The following are the Structures that the outer level Ludwig Routines
// use. These include the structures used by the compiler

enum class commands {
    cmd_noop,

    cmd_up,                 // cursor movement
    cmd_down,
    cmd_left,
    cmd_right,
    cmd_home,
    cmd_return,
    cmd_tab,
    cmd_backtab,

    cmd_rubout,
    cmd_jump,
    cmd_advance,
    cmd_position_column,
    cmd_position_line,
    cmd_op_sys_command,

    cmd_window_forward,     // window control
    cmd_window_backward,
    cmd_window_left,
    cmd_window_right,
    cmd_window_scroll,
    cmd_window_top,
    cmd_window_end,
    cmd_window_new,
    cmd_window_middle,
    cmd_window_setheight,
    cmd_window_update,

    cmd_get,                // search and comparison
    cmd_next,
    cmd_bridge,
    cmd_replace,
    cmd_equal_string,
    cmd_equal_column,
    cmd_equal_mark,
    cmd_equal_eol,
    cmd_equal_eop,
    cmd_equal_eof,

    cmd_overtype_mode,
    cmd_insert_mode,

    cmd_overtype_text,      // text insertion/deletion
    cmd_insert_text,
    cmd_type_text,
    cmd_insert_line,
    cmd_insert_char,
    cmd_insert_invisible,
    cmd_delete_line,
    cmd_delete_char,

    cmd_swap_line,          // text manipulation
    cmd_split_line,
    cmd_ditto_up,
    cmd_ditto_down,
    cmd_case_up,
    cmd_case_low,
    cmd_case_edit,
    cmd_set_margin_left,
    cmd_set_margin_right,

    cmd_line_fill,          // word processing
    cmd_line_justify,
    cmd_line_squash,
    cmd_line_centre,
    cmd_line_left,
    cmd_line_right,
    cmd_word_advance,
    cmd_word_delete,
    cmd_advance_paragraph,
    cmd_delete_paragraph,

    cmd_span_define,        // span commands
    cmd_span_transfer,
    cmd_span_copy,
    cmd_span_compile,
    cmd_span_jump,
    cmd_span_index,
    cmd_span_assign,

    cmd_block_define,       // block commands
    cmd_block_transfer,
    cmd_block_copy,

    cmd_frame_kill,         // frame commands
    cmd_frame_edit,
    cmd_frame_return,
    cmd_span_execute,              //###
    cmd_span_execute_no_recompile, //###
    cmd_frame_parameters,

    cmd_file_input,         // open and attach input file
    cmd_file_output,        // create and attach output file
    cmd_file_edit,          // open files for input and output
    cmd_file_read,          // read from global input
    cmd_file_write,         // write to global output
    cmd_file_close,         // close a file -- only used internally
    cmd_file_rewind,        // rewind input file
    cmd_file_kill,          // delete output file
    cmd_file_execute,       // execute a file of commands
    cmd_file_save,          // save a file without clearing the frame
    cmd_file_table,         // displays current filetable
    cmd_file_global_input,  // declare a global input file
    cmd_file_global_output, // declare a global output file
    cmd_file_global_rewind, // rewind global input file
    cmd_file_global_kill,   // delete global output file

    cmd_user_command_introducer,
    cmd_user_key,
    cmd_user_parent,
    cmd_user_subprocess,
    cmd_user_undo,
    cmd_user_learn,
    cmd_user_recall,

    cmd_resize_window,      // window size has changed, so handle it

    cmd_help,               // miscellaneous
    cmd_verify,
    cmd_command,
    cmd_mark,
    cmd_page,
    cmd_quit,
    cmd_dump,               // ~D in debug version.
    cmd_validate,           // ~V in debug version.
    cmd_execute_string,
    cmd_do_last_command,

    cmd_extended,

    cmd_exit_abort,         // exit back to immediate mode
    cmd_exit_fail,          // exit with failure
    cmd_exit_success,       // exit with success


    // -----End of user commands-----

    // Dummy commands for pattern matcher
    cmd_pattern_dummy_pattern,
    cmd_pattern_dummy_text,

    // compiler commands
    cmd_pcjump,             // Intermediate code jump
    cmd_exitto,             // Set exit point from loop
    cmd_failto,             // Set fail point from loop
    cmd_iterate,            // Repeat loop n times

    cmd_prefix_ast,         // prefix commands
    cmd_prefix_a,
    cmd_prefix_b,
    cmd_prefix_c,
    cmd_prefix_d,
    cmd_prefix_e,
    cmd_prefix_eo,
    cmd_prefix_eq,
    cmd_prefix_f,
    cmd_prefix_fg,          // global files
    cmd_prefix_i,
    cmd_prefix_k,
    cmd_prefix_l,
    cmd_prefix_o,
    cmd_prefix_p,
    cmd_prefix_s,
    cmd_prefix_t,
    cmd_prefix_tc,
    cmd_prefix_tf,
    cmd_prefix_u,
    cmd_prefix_w,
    cmd_prefix_x,
    cmd_prefix_y,
    cmd_prefix_z,
    cmd_prefix_tilde,

    cmd_nosuch,             // sentinel

    last_entry
};

using user_commands   = perange<commands, commands::cmd_noop,       commands::cmd_pattern_dummy_text>;
using comp_commands   = perange<commands, commands::cmd_pcjump,     commands::cmd_iterate>;
using prefix_commands = perange<commands, commands::cmd_prefix_ast, commands::cmd_prefix_tilde>;
using prefix_plus     = perange<commands, commands::cmd_prefix_ast, commands::cmd_nosuch>;

enum class leadparam {
    none,           // no leading parameter
    plus,           // + without integer
    minus,          // - without integer
    pint,           // +ve integer
    nint,           // -ve integer
    pindef,         // >
    nindef,         // <
    marker,         // @ or = or %
    last_entry
};

// equalaction is a command attribute type used to control the
// behaviour of mark Equals.
enum class equalaction {
    eqnil,          // leave mark alone (N.B. action routine may
                    // shift mark e.g. get and replace
    eqdel,          // delete mark e.g. delete and kill
    eqold           // set mark to cursor posn. before cmd
                    // normal for cmds which shift cursor
};

// tpcount_type indicates the number of trailing parameters required
// for the command.
// if -ve, means the command requires no tp's iff leading parameter
// is -ve (used for -FI)
using tpcount_type = prange<-MAX_TPCOUNT, MAX_TPCOUNT>;

enum class prompt_type {
    no_prompt,
    char_prompt,
    get_prompt,
    equal_prompt,
    key_prompt,
    cmd_prompt,
    span_prompt,
    text_prompt,
    frame_prompt,
    file_prompt,
    column_prompt,
    mark_prompt,
    param_prompt,
    topic_prompt,
    replace_prompt,
    by_prompt,
    verify_prompt,
    pattern_prompt,
    pattern_set_prompt,

    last_entry
};

// used for default prompt strings
struct tpar_attribute {
    prompt_type prompt_name;
    bool        trim_reply;
    bool        ml_allowed;
};

struct cmd_attrib_rec {
    penumset<leadparam> lp_allowed;
    equalaction         eq_action;
    tpcount_type        tpcount;
    parray<tpar_attribute, prange<1, MAX_TPCOUNT>> tpar_info;
};

struct help_record {
    key_str   key;
    write_str txt;
};

// more Pattern Matcher stuff

struct nfa_transition_type {
    nfa_transition_type() {
        indefinite = false;
        fail = false;
        epsilon_out = false;
        epf.next_state = 0;
    }
    nfa_transition_type(const nfa_transition_type &other) {
        indefinite = other.indefinite;
        fail = other.fail;
        epsilon_out = other.epsilon_out;
        if (epsilon_out) {
            ept = other.ept;
        } else {
            epf = other.epf;
        }
    }
    bool indefinite;
    bool fail;
    bool epsilon_out;
    union {
        struct {
            nfa_state_range first_out;
            nfa_state_range second_out;
        } ept;
        struct {
            nfa_state_range next_state;
            accept_set_type accept_set;
        } epf;
    };
};

using nfa_table_type = parray<nfa_transition_type, nfa_state_range>;

enum class parameter_type {
    pattern_fail,
    pattern_range,
    null_param
};

// global file defaults and other things filesys_parse would like to know

struct file_data_type {
    bool          old_cmds;
    bool          entab;
    size_t        space;
    std::string   initial;
    bool          purge;
    size_t        versions;
};

struct code_object {
    leadparam     rep;      // Repeat type
    int           cnt;      // Repeat count
    commands      op;       // Opcode
    tpar_ptr      tpar;     // Trailing param
    code_ptr      code;     // Code for cmd_extended
    code_idx      lbl;      // Label field
};

struct command_object {
    commands      command;
    code_ptr      code;
    tpar_ptr      tpar;
};

struct terminal_info_type {
    std::string   name;
    scr_col_range width;
    scr_row_range height;
};

#endif // !defined(TYPE_H)
