/* -*- mode: C; tab-width: 8; c-basic-offset: 4; -*- */
/**********************************************************************/
/*                                                                    */
/*           L      U   U   DDDD   W      W  IIIII   GGGG             */
/*           L      U   U   D   D   W    W     I    G                 */
/*           L      U   U   D   D   W ww W     I    G   GG            */
/*           L      U   U   D   D    W  W      I    G    G            */
/*           LLLLL   UUU    DDDD     W  W    IIIII   GGGG             */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/*  Copyright (C) 1981, 1987                                          */
/*  Department of Computer Science, University of Adelaide, Australia */
/*  All rights reserved.                                              */
/*  Reproduction of the work or any substantial part thereof in any   */
/*  material form whatsoever is prohibited.                           */
/*                                                                    */
/**********************************************************************/

/*++
! Name:        FILESYS
!
! Description: The File interface to Unix.
!
! $Log: filesys.c.unix,v $
! Revision 4.21  90/10/31  13:02:35  ludwig
! Change type of the second parameter of filesys_close, to conform
! with the changes made in the IBM PC version.   KBN
!
! Revision 4.20  90/08/31  11:46:14  ludwig
! Modified ISO1 symbol usage.  KBN
!
! Revision 4.19  90/02/28  11:39:42  ludwig
! Implement the File Save command:
! . Add a new routine  filesys_save
! . Modify filesys_create_open to open output files for read and write access
! . Modify filesys_close to allow close processing on a file without
!   actually closing it
! Fix bug in filesys_close. The directory stream opened during file backup
! processing was not being closed.
!
! Revision 4.18  90/02/08  16:47:42  ludwig
! make call to vdu_process_window_args conditional on the parse being of a
! command line.
!
! Revision 4.17  90/01/26  09:43:04  ludwig
! Steven Nairn.
! Call vdu_process_window_args before using getopt to process command line
! arguments.
!
! Revision 4.16  90/01/18  18:54:59  ludwig
! Entered into RCS at revision level 4.16
!
! Revision History:
! 4-001 Ludwig V4.0 release.                                  7-Apr-1987
! 4-002 Jeff Blows                                           15-May-1987
!       Remove #include <sys/types.h>. It is included by <sys/param.h>
!       and the is68k C compiler objects to having the definitions
!       twice.
! 4-003 Kelvin B. Nicolle                                     5-Jun-1987
!       See const.h and type.h.
! 4-004 Mark R. Prior                                         7-Nov-1987
!       Rearrange code to do all test before setting the mode and
!         previous_id fields of the file data structure.
!       Add code to check for write access to a file.
!       Rename the file data structure field from buffer to buf.
!       Add a nul to the end of the read buffer.
!       Add "check_input = 1" to the parse of command line and FE using
!         the memory file.
! 4-005 Mark R. Prior                                        10-Nov-1987
!       Correct the use of umask when creating new files.
!       Change the mode of temporary files from 644 to 600.
! 4-006 Mark R. Prior                                        11-Nov-1987
!       Make the declaration of errno global.
! 4-007 Mark R. Prior                                        24-Nov-1987
!       Set the memory file name to NUL when the -M option is used.
! 4-008 Mark R. Prior                                        12-Dec-1987
!       If the last line of a file has no terminator, the line was being
!       lost.
! 4-009 Mark R. Prior                                        20-Feb-1988
!       Use conformant arrays to pass string parameters to ch routines.
!               string[offset],length -> string,offset,length
!       In all calls of ch_length, ch_upcase_str, ch_locase_str, and
!         ch_reverse_str, the offset was 1 and is now omitted.
! 4-010 Kelvin B. Nicolle                                     2-Sep-1988
!       Add flags to the C routine headers so that the include files
!       generated for the Multimax Pascal compiler will contain the
!       "nonpascal" directive. Also ensure that all routine headers are
!       terminated with a blank line.
! 4-011 Kelvin B. Nicolle                                     2-Sep-1988
!       Only the Ultrix Pascal compiler does not support underscores in
!       identifiers:  Put the underscores back in the Pascal sources and
!       make the macro definitions of the external names conditional in
!       the C sources.
! 4-012 Kelvin B. Nicolle                                    28-Oct-1988
!       Different operating systems have different implementations of
!       getopt.  Rationalize the construction of argv to be compatible.
! 4-013 Kelvin B. Nicolle                                     4-Nov-1988
!       Change the "multimax" flags in the routine headers to "umax" and
!       "mach".
! 4-014 Kelvin B. Nicolle                                    16-Dec-1988
!       In fileys_parse copy the usage strings before passing them to
!       screen_unix_message which blank pads the message.
! 4-015 Kelvin B. Nicolle                                    22-Mar-1989
!       Add dummy parameters to the ch_routines that expect conformant
!       arrays.
! 4-016 Kelvin N. Nicolle                                    13-Sep-1989
!       Modify Pascal headers for Tower version.
!--*/

#include "filesys.h"

#include "sys.h"
#include "screen.h"
#include "lwgetopt.h"

#include <sstream>

/*----------------------------------------------------------------------------*/

namespace {
    const std::string NL("\n");
    const int NL_SIZE = NL.size();

    template <typename II>
    void remove_backup_files(const std::string &backup_file, II begin, II end) {
        for (; begin != end; ++begin) {
            std::string file_name = backup_file + std::to_string(*begin);
            sys_unlink(file_name);
        }
    }

    bool starts_with(const std::string &haystack, const std::string &needle) {
        return haystack.size() >= needle.size() &&
            haystack.substr(0, needle.size()) == needle;
    }

    std::vector<std::string> to_argv(const std::string &cmdline) {
        std::istringstream iss(cmdline);
        std::vector<std::string> v;
        std::string token;
        while (iss >> token) {
            v.push_back(token);
        }
        return v;
    }
};

/*----------------------------------------------------------------------------*/

bool filesys_create_open(file_ptr fyle, const_file_ptr rfyle, bool ordinary_open) {
#ifdef DEBUG
    // Check the file has a 'Z' in the right place
    if (fyle->zed != 'Z') {
        screen_message("FILE and FILESYS definition of file_object disagree.");
        return false;
    }
#endif
    fyle->l_counter = 0;
    if (!fyle->output_flag) { // open file for reading
        if (fyle->filename.empty())
            return false;
        if (!ordinary_open) { // really executing a command
            int fd = sys_open_command(fyle->filename);
            if (fd == -1) {
                screen_message("Cannot create pipe");
                return false;
            }
            fyle->fd = fd;
        } else {
            if (!sys_expand_filename(fyle->filename)) {
                std::stringstream s;
                s << "Error in filename (" << fyle->filename << ")";
                screen_message(s.str());
                return false;
            }
            file_status fs = sys_file_status(fyle->filename);
            if (!fs.valid || fs.isdir) {
                return false;
            }
            fyle->fd = sys_open_file(fyle->filename);
            if (fyle->fd < 0) {
                return false; // fail, return false
            }
            fyle->mode = fs.mode;
            fyle->previous_file_id = fs.mtime;
        }
        fyle->idx = 0;
        fyle->len = 0;
        fyle->eof = 0;
    } else { // otherwise open new file for output
        std::string related;
        if (rfyle != nullptr)
            related = rfyle->filename;
        if (fyle->filename.empty()) { // default to related filename
            fyle->filename = related;
        }
        if (fyle->filename.empty()) {
            return false;
        } else if (!sys_expand_filename(fyle->filename)) {
            std::stringstream s;
            s << "Error in filename (" << fyle->filename << ")";
            screen_message(s.str());
            return false;
        }
        // if the file given is a directory create a filename using the input
        // filename in the given directory.
        file_status fs = sys_file_status(fyle->filename);
        if (fs.valid && fs.isdir && !related.empty()) {
            // Get the filename from the related name.
            sys_copy_filename(related, fyle->filename);
            fs = sys_file_status(fyle->filename);
        }
        if (fs.valid) {
            // if we wanted to create a new file then complain
            if (fyle->create) {
                std::stringstream s;
                s << "File (" << fyle->filename << ") already exists";
                screen_message(s.str());
                return false;
            }
            // check that the file we may overwrite is not a directory
            if (fs.isdir) {
                std::stringstream s;
                s << "File (" << fyle->filename << ") is a directory";
                screen_message(s.str());
                return false;
            }
            // check that we can write over the current version
            if (!sys_file_writeable(fyle->filename)) {
                std::stringstream s;
                s << "Write access to file (" << fyle->filename << ") is denied";
                screen_message(s.str());
                return false;
            }
            fyle->mode = fs.mode;
            fyle->previous_file_id = fs.mtime;
        } else {
            fyle->mode = 0666 & sys_file_mask();
            fyle->previous_file_id = 0;
        }
        // now create the temporary name
        int uniq = 0;
        fyle->tnm = fyle->filename + "-lw";
        while (sys_file_exists(fyle->tnm)) {
            fyle->tnm = fyle->filename + std::string("-lw") + std::to_string(++uniq);
        }
        fyle->fd = sys_create_file(fyle->tnm);
        if (fyle->fd < 0) {
            std::stringstream s;
            s << "Error opening (" << fyle->tnm << ") as output";
            screen_message(s.str());
            return false;     // fail, return false
        }
    }
    return true;              // succeed, return true
}

/*----------------------------------------------------------------------------*/

bool filesys_close(const_file_ptr fyle, int action, bool msgs) {
/* Closes a file, described by fileptr fyle.
 * Action is an integer interpreted as follows:
 *   0 : close
 *   1 : close and delete
 *   2 : process the file as if closing it, but do not close it
 * The msgs parameter indicates whether we want to be told about what
 * is going on.
 * Returns true (1) on success, false (0) on failure.
 */
    if (fyle->output_flag == 0) {
        // reap any children
        sys_reap_children();
        // an ordinary input file, just close
        if (sys_close(fyle->fd) < 0)
            return false;      // fail, return false
        if (msgs) {
            std::stringstream s;
            s << "File " << fyle->filename << " closed (" <<
                fyle->l_counter << " line" <<
                (fyle->l_counter == 1 ? "" : "s") << " read).";
            screen_message(s.str());
        }
        return true;           // succeed, return true
    }
    // an output file to close
    if ((action != 2) && (sys_close(fyle->fd) < 0))
        return false;          // fail, return false
    if (action == 1) {
        // remove the file
        if (sys_unlink(fyle->tnm)) {
            if (msgs) {
                std::stringstream s;
                s << "Output file " << fyle->tnm << " deleted.";
                screen_message(s.str());
            }
            return true;       // succeed, return true
        } else {
            return false;
        }
    }
    // check that another file hasn't been created while we were editting
    // with the name we are going to use as the output name.
    file_status fs = sys_file_status(fyle->filename);
    if (fs.valid && fs.mtime != fyle->previous_file_id) {
        std::stringstream s;
        s << fyle->filename << " was modified by another process";
        screen_message(s.str());
    }
    std::string tname(fyle->filename);
    tname += "~";
    long max_vnum = 0;
    if (fyle->purge) {
        const std::vector<long> versions(sys_list_backups(tname));
        if (fyle->versions <= 0) {
            // Just remove all of them in this case
            remove_backup_files(tname, std::begin(versions), std::end(versions));
        } else {
            // Work out how many to retain, and how many to remove.
            size_t to_retain = fyle->versions - 1;
            if (versions.size() > to_retain) {
                size_t to_remove = versions.size() - to_retain;
                remove_backup_files(tname, std::begin(versions), std::next(std::begin(versions), to_remove));
            }
        }
        max_vnum = versions.empty() ? 0 : versions.back();
    } else {
        const std::vector<long> versions(sys_list_backups(tname));
        if (!versions.empty() && versions.size() >= fyle->versions) {
            // Remove just one file
            remove_backup_files(tname, std::begin(versions), std::next(std::begin(versions)));
        }
        max_vnum = versions.empty() ? 0 : versions.back();
    }
    // Perform Backup on original file if -
    //    a. fyle->versions != 0
    // or b. doing single backup and backup already done at least once
    if (fyle->versions != 0 || (!fyle->purge && max_vnum != 0)) {
        // does the real file already exist?
        if (sys_file_exists(fyle->filename)) {
            // try to rename current file to backup
            std::string temp = tname + std::to_string(max_vnum + 1);
            sys_rename(fyle->filename, temp);
        }
    }
    // now rename the temp file to the real thing
    sys_chmod(fyle->tnm, fyle->mode & 07777);
    if (!sys_rename(fyle->tnm, fyle->filename)) {
        std::stringstream s;
        s << "Cannot rename " << fyle->tnm << " to " << fyle->filename;
        screen_message(s.str());
        return false;      // fail, return false
    } else {
        if (msgs) {
            std::stringstream s;
            s << "File " << fyle->filename << " created (" << fyle->l_counter <<
                " line" << (fyle->l_counter == 1 ? "" : "s") << " written).";
            screen_message(s.str());
        }
        // Time to set the memory, if it's required and we aren't writing in
        // one of the global tmp directories
        if (!starts_with(fyle->filename, "/tmp/") &&
            !starts_with(fyle->filename, "/usr/tmp/") &&
            !starts_with(fyle->filename, "/var/tmp/")) {
            sys_write_filename(fyle->memory, fyle->filename);
        }
        return true;       // succeed, return true
    }
}

/*----------------------------------------------------------------------------*/

bool filesys_read(file_ptr fyle, str_object &output_buffer, strlen_range &outlen) {
/* Attempts to read MAX_STRLEN characters into buffer.
 * Number of characters read is returned in outlen.
 * Returns true (1) on success, false (0) on failure.
 */
    outlen = 0;
    do {
        if (fyle->idx >= fyle->len) {
            fyle->buf.resize(MAX_STRLEN);
            fyle->len = sys_read(fyle->fd, fyle->buf.data(), MAX_STRLEN);
            fyle->idx = 0;
        }
        if (fyle->len <= 0) {
            fyle->eof = 1;
            // If the last line is not terminated properly,
            // the buffer is not empty and we must return the buffer.
            if (outlen)
                break;
            return false;  // fail, return false
        }
        int ch = toascii(fyle->buf[fyle->idx++]);
        if (std::isprint(ch)) {
            output_buffer[++outlen] = ch;
        } else if (ch == '\t') { // expand the tab
            int exp = 8 - (outlen % 8);

            if (outlen + exp > MAX_STRLEN)
                exp = MAX_STRLEN - outlen;
            for (; exp > 0; exp--)
                output_buffer[++outlen] = ' ';
        } else if (ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f') {
            break;              // finished if newline or carriage return
        } // forget other control characters
    } while (outlen < MAX_STRLEN);
    fyle->l_counter += 1;
    return true;           // succeed, return true
}

/*----------------------------------------------------------------------------*/

bool filesys_rewind(file_ptr fyle) {
/*
 * Rewinds file described by the FILE_PTR `fyle'.
 */
    if (!sys_seek(fyle->fd, 0L))
        return false;
    fyle->idx = 0;
    fyle->len = 0;
    fyle->eof = 0;
    fyle->l_counter = 0;
    return true;
}

/*----------------------------------------------------------------------------*/

bool filesys_write(file_ptr fyle, str_ptr buffer, strlen_range bufsiz) {
/* Attempts to write bufsiz characters from buffer to the file described by
 * fyle. Returns true (1) on success, false (0) on failure.
 */
    if (bufsiz > 0) {
        int offset = 0;
        int tabs = 0;
        if (fyle->entab) {
            int i;
            for (i = 0; i < bufsiz; ++i)
                if ((*buffer)[i + 1] != ' ') break;
            tabs = i / 8;
            offset = tabs * 7;
            for (i = 1; i <= tabs; i++)
                (*buffer)[offset + i] = '\t';
        }
        int count = sys_write(fyle->fd, buffer->data() + offset, bufsiz - offset);
        if (tabs) {
            for (int i = 1; i <= tabs; i++)
                (*buffer)[offset + i] = ' ';
        }
        if (count != bufsiz - offset)
            return false;
    }
    int ok = sys_write(fyle->fd, NL.data(), NL_SIZE) == NL_SIZE;
    fyle->l_counter += 1;
    return ok; // succeed, return true if all written
}

/*----------------------------------------------------------------------------*/

bool filesys_save(file_ptr i_fyle, file_ptr o_fyle, int copy_lines) {
/*  Implements part of the File Save command. */
    file_object fyle;

    bool input_eof;
    off_t input_position;
    str_object line;
    strlen_range line_len;
    if (i_fyle != nullptr) {
        //  remember things to be restored
        input_eof = i_fyle->eof;
        input_position = sys_tell(o_fyle->fd);

        // copy unread portion of input file to output file
        do {
            if (filesys_read(i_fyle, line, line_len))
                filesys_write(o_fyle, &line, line_len);
        } while (!i_fyle->eof);

        // close input file
        filesys_close(i_fyle, 0, true);
    }

    // rename temporary file to output file
    // process backup options, but do not close the file
    filesys_close(o_fyle, 2, true);

    // make the old output file the new input file
    if (i_fyle == NULL) {
          i_fyle = &fyle;
          i_fyle->output_flag = false;
          i_fyle->first_line = nullptr;
          i_fyle->last_line = nullptr;
          i_fyle->line_count = 0;
    }
    i_fyle->filename = o_fyle->filename;
    i_fyle->fd = o_fyle->fd;

    // rewind the input file
    filesys_rewind(i_fyle);

    // open a new output file
    o_fyle->create = false;
    if (!filesys_create_open(o_fyle, nullptr, true))
        return false;

    // copy lines from the input file to the output file
    for (int i = 0; i < copy_lines; i++) {
        if (!filesys_read(i_fyle, line, line_len))
            return false;
        if (!filesys_write(o_fyle, &line, line_len))
            return false;
    }

    // reposition or close the input file
    if (i_fyle == &fyle) {
        sys_close(i_fyle->fd);
    } else {
        i_fyle->eof = input_eof;
        sys_seek(i_fyle->fd, input_position);
        i_fyle->idx = 0;
        i_fyle->len = 0;
    }
    return true;
}

/*----------------------------------------------------------------------------*/

bool filesys_parse(const std::string &command_line, parse_type parse,
                   file_data_type &file_data, file_ptr &input, file_ptr &output) {
    static const char usage[] = "usage : ludwig [-c] [-r] [-i value] [-I] "
                                "[-s value] [-m file] [-M] [-t] [-T] "
                                "[-b value] [-B value] [-o] [-O] [-u] "
                                "[file [file]]";
    static const char file_usage[] = "usage : [-m file] [-t] [-T] [-b value] "
                                     "[-B value] [file [file]]";

    if (parse == parse_type::parse_stdin) {
        input->valid = true;
        input->fd = 0;
        input->eof = false;
        input->l_counter = 0;
        return true;
    }

    // create an argc and argv from the "command_line"
    std::vector<std::string> argv;
    argv.emplace_back("Ludwig");
    std::vector<std::string> cl(to_argv(command_line));
    argv.insert(std::end(argv), std::begin(cl), std::end(cl));

    size_t argc     = argv.size();
    bool   entab    = file_data.entab;
    int    space    = file_data.space;
    bool   purge    = file_data.purge;
    size_t versions = file_data.versions;

    bool create_flag    = false;
    bool read_only_flag = false;
    bool space_flag     = false;
    bool usage_flag     = false;
    bool version_flag   = false;

    int errors      = 0;
    int check_input = 0;

    std::string initialize;
    std::string memory;

    if (parse == parse_type::parse_command) {
        std::string home;
        if (!sys_getenv("HOME", home))
            home = ".";
        initialize = home + "/.ludwigrc";
        memory = home + "/.lud_memory";
    } else {
        initialize.clear();
        memory.clear();
    }
    lwoptreset = 1;
    lwoptind = 1;
    int c;
    while ((c = lwgetopt(argv, "cri:Is:m:MtTb:B:oOu")) != -1) {
        switch (c) {
        case 'c':
            if (read_only_flag)
                errors++;
            else
                create_flag = true;
            break;
        case 'r':
            if (create_flag)
                errors++;
            else
                read_only_flag = true;
            break;
        case 'i':
            initialize = lwoptarg;
            break;
        case 'I':
            initialize = "";
            break;
        case 's':
            try {
                space = static_cast<size_t>(std::stoul(lwoptarg));
                space_flag = true;
            } catch (const std::logic_error &ex) {
                errors++;
            }
            break;
        case 'm':
            memory = lwoptarg;
            break;
        case 'M':
            memory = "";
            break;
        case 't':
            entab = true;
            break;
        case 'T':
            entab = false;
            break;
        case 'b':
            try {
                versions = static_cast<size_t>(std::stoul(lwoptarg));
                purge = false;
            } catch (const std::logic_error &ex) {
                errors++;
            }
            break;
        case 'B':
            try {
                versions = static_cast<size_t>(std::stoul(lwoptarg));
                purge = true;
            } catch (const std::logic_error &ex) {
                errors++;
            }
            break;
        case 'o':
            version_flag = true;
            file_data.old_cmds = true;
            break;
        case 'O':
            version_flag = true;
            file_data.old_cmds = false;
            break;
        case 'u':
            usage_flag = true;
            break;
        }
    }
    if (usage_flag || errors) {
        if (parse == parse_type::parse_command)
            screen_message(usage);
        else
            screen_message(file_usage);
        return false;
    }
    if (parse == parse_type::parse_command) {
        file_data.initial  = initialize;
        file_data.space    = space;
        file_data.entab    = entab;
        file_data.purge    = purge;
        file_data.versions = versions;
    } else if (create_flag || read_only_flag || !initialize.empty() || space_flag || version_flag) {
        return false;
    }
    std::vector<std::string> file;
    for (int files = 0; lwoptind < argc; ++files) {
        if (files >= 2) {
            screen_message("More than two files specified");
            return false;
        }
        file.emplace_back(argv[lwoptind++]);
    }
    if (file.size() == 2) {
        check_input = true;
        if (   parse == parse_type::parse_input || parse == parse_type::parse_output
            || parse == parse_type::parse_execute || create_flag || read_only_flag) {
            screen_message("Only one file name can be specified");
            return false;
        }
    }
    switch (parse) {
    case parse_type::parse_command:
    case parse_type::parse_edit:
        if (!file.empty()) {
            input->filename = file[0];
        } else if (!memory.empty()) {
            if (sys_read_filename(memory, input->filename)) {
                check_input = true;
            } else {
                if (parse == parse_type::parse_command) {
                    input->filename.clear();
                    return true;
                }
                std::stringstream s;
                s << "Error opening memory file (" << memory << ")";
                screen_message(s.str());
                return false;
            }
        } else if (parse == parse_type::parse_command) {
            input->filename.clear();
            return true;
        } else {
            input->filename.clear();
            return false;
        }
        if (file.size() > 1) {
            output->filename = file[1];
        } else {
            output->filename = input->filename;
        }
        output->memory = memory;
        output->entab = entab;
        output->purge = purge;
        output->versions = versions;
        if (read_only_flag) {
            input->create = 0;
            if (!filesys_create_open(input, nullptr, true)) {
                std::stringstream s;
                s << "Error opening (" << input->filename << ") as input";
                screen_message(s.str());
                return false;
            }
            input->valid = true;
        } else if (create_flag) {
            output->create = true;
            if (!filesys_create_open(output, nullptr, true))
                return false;
            output->valid = true;
        } else {
            input->create = false;
            output->create = false;
            if (filesys_create_open(input, nullptr, true))
                input->valid = true;
            else if (check_input || parse == parse_type::parse_edit || errno == EWOULDBLOCK || errno == EISDIR) {
                std::stringstream s;
                s << "Error opening (" << input->filename << ") as input";
                screen_message(s.str());
                return false;
            }
            if (filesys_create_open(output, input, true) != 0)
                output->valid = true;
            else
                return false;
        }
        break;
    case parse_type::parse_input:
        if (file.size() == 1) {
            input->filename = file[0];
        } else if (memory.empty() || !sys_read_filename(memory, input->filename)) {
            input->filename.clear();
            return false;
        }
        input->create = false;
        if (input->filename.empty() || !filesys_create_open(input, nullptr, true)) {
            std::stringstream s;
            s << "Error opening (" << input->filename << ") as input";
            screen_message(s.str());
            return false;
        }
        input->valid = true;
        break;
    case parse_type::parse_execute:
        if (file.size() == 1) {
            input->filename = file[0];
        } else {
            input->filename.clear();
        }
        input->create = false;
        if (input->filename.empty() || !filesys_create_open(input, nullptr, true))
            return false;
        input->valid = true;
        break;
    case parse_type::parse_output:
        if (file.size() == 1) {
            output->filename = file[0];
        } else if (input != nullptr) {
            output->filename = input->filename;
        } else {
            output->filename.clear();
        }
        output->memory   = memory;
        output->entab    = entab;
        output->purge    = purge;
        output->versions = versions;
        output->create   = false;
        if (output->filename.empty() || !filesys_create_open(output, input, true))
            return false;
        output->valid = true;
        break;
    case parse_type::parse_stdin:
        // Handled at the top of this function
        return false;
    }
    return true;
}

/*----------------------------------------------------------------------------*/
