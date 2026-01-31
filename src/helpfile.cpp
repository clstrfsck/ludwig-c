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
! Name:         HELPFILE
!
! Description: Load and support indexed help file under unix.
!--*/

#include "type.h"

#include <fstream>
#include <unordered_map>

// Realistically, we could just bake the helpfile into the binary.
// It's <100k, which may have been a lot of memory once, but now is not a
// significant amount.  For now we are keeping it as a separate file.

namespace {
    const char *const NEW_HELPFILE_ENV = "LUD_NEWHELPFILE";
    const char *const OLD_HELPFILE_ENV = "LUD_HELPFILE";
    inline constexpr std::string_view NEW_DEFAULT_HLPFILE{"/usr/local/help/ludwignewhlp.idx"};
    inline constexpr std::string_view OLD_DEFAULT_HLPFILE{"/usr/local/help/ludwighlp.idx"};

    const size_t SKIP_MAX = 81; // No good reason for this value, but it's a lot of blank lines.

    struct key_type {
        long start_pos;
        long end_pos;
        std::string key;
    };

    std::ifstream helpfile;
    std::unordered_map<std::string, key_type> table;
    key_type current_key;

    bool help_openfile(const std::string_view &filename) {
        helpfile.open(filename, std::ifstream::in);
        return helpfile.is_open();
    }

    bool help_try_openfile(const char *env_name, const std::string_view &default_filename) {
        const char *help;
        if ((help = std::getenv(env_name)) == NULL || !help_openfile(help))
            return help_openfile(default_filename);
        return true;
    }

    bool read_index() {
        // Read in the size of the index, and the size in lines of the contents.
        long index_size;
        helpfile >> index_size;
        long contents_lines;
        helpfile >> contents_lines;

        // Read in index_size keys.
        for (long i = 0; i < index_size; ++i) {
            key_type k;
            helpfile >> k.key;
            helpfile >> k.start_pos;
            helpfile >> k.end_pos;
            table[k.key] = k;
        }
        // Skip final newline in index.
        helpfile.ignore(SKIP_MAX, '\n');

        // The key "0" is special, it's the contents page, it does NOT appear
        // in the index as it must be created on the fly while creating the
        // index file.  Hence it's entry appears after the index but before the
        // bulk of the entries.
        key_type contents;
        contents.key = "0";
        contents.start_pos = helpfile.tellg();
        for (long i = 0; i < contents_lines; ++i) {
            helpfile.ignore(SKIP_MAX, '\n');
        }
        contents.end_pos = helpfile.tellg();

        // Correct the positions in the index to point at the real offset in
        // the file.
        for (auto &p : table) {
            p.second.start_pos += contents.end_pos;
            p.second.end_pos += contents.end_pos;
        }

        // Add the contents to the table.
        table[contents.key] = contents;

        return true;
    }
}; // namespace

void helpfile_close() {
    if (helpfile.is_open()) {
        helpfile.close();
    }
    current_key.key.clear();
    table.clear();
}

bool helpfile_open(bool old_version) {
    if (helpfile.is_open()) { // we have done this before, dont do it again!
        return true;
    }
    if (old_version) {
        if (!help_try_openfile(OLD_HELPFILE_ENV, OLD_DEFAULT_HLPFILE)) {
            return false;
        }
    } else {
        if (!help_try_openfile(NEW_HELPFILE_ENV, NEW_DEFAULT_HLPFILE)) {
            return false;
        }
    }
    return read_index();
}

bool helpfile_open(const std::string_view &filename) {
    if (helpfile.is_open()) { // we have done this before, dont do it again!
        return true;
    }
    if (!help_openfile(filename)) {
        return false;
    }
    return read_index();
}

bool helpfile_read(const std::string &keystr, help_record &buffer) {
    current_key.key = keystr;

    auto result = table.find(current_key.key);
    if (result == table.end()) {
        return false;
    }

    // Find the offset in the helpfile for the entry and go there now.
    current_key = result->second;
    helpfile.seekg(current_key.start_pos);

    // Get the first line in the entry and package it up in a help_record.
    std::getline(helpfile, buffer.txt);
    buffer.key = current_key.key;
    return true;
}

bool helpfile_next(help_record &buffer) {
    // if the current position >= the end of the entry return false,
    // else give back the next line nicely packaged.
    if (helpfile.tellg() >= current_key.end_pos) {
        return false;
    } else {
        std::getline(helpfile, buffer.txt);
        buffer.key = current_key.key;
    }
    return true;
}
