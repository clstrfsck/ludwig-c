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

#include <cstdlib>
#include <fstream>
#include <unordered_map>

namespace {
    const char NEW_HELPFILE_ENV[]    = "LUD_NEWHELPFILE";
    const char NEW_DEFAULT_HLPFILE[] = "/usr/local/help/ludwignewhlp.idx";
    const char OLD_HELPFILE_ENV[]    = "LUD_HELPFILE";
    const char OLD_DEFAULT_HLPFILE[] = "/usr/local/help/ludwighlp.idx";

    // Special magic, the origin of which seems to be lost.
    // I guess the mnemonic is "STatuS: Record Not Found" or similar.
    const int STSRNF = 98994;

    struct key_type {
        long        start_pos;
        long        end_pos;
        std::string key;
    };

    std::ifstream                             helpfile;
    std::unordered_map<std::string, key_type> table;
    key_type                                  current_key;
};

bool openfile(const char *filename) {
    helpfile.open(filename, std::ifstream::in);
    return helpfile.is_open();
}

bool try_openfile(const char *env_name, const char *default_filename) {
    const char *help;
    if ((help = std::getenv(env_name)) == NULL || !openfile(help))
        return openfile(default_filename);
    return true;
}

bool helpfile_open(bool old_version) {
    if (helpfile.is_open()) /* we have done this before, dont do it again! */
        return true;
    if (old_version) {
        if (!try_openfile(OLD_HELPFILE_ENV, OLD_DEFAULT_HLPFILE))
            return false;
    } else {
        if (!try_openfile(NEW_HELPFILE_ENV, NEW_DEFAULT_HLPFILE))
            return false;
    }

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
    helpfile.ignore(WRITE_STR_LEN + 1, '\n');

    /*
     * The key "0" is special, it's the contents page, it does NOT appear in the
     * index as it must be created on the fly while creating the index file.
     * Hence it's entry appears after the index but before the bulk of the
     * entries.
     */
    key_type contents;
    contents.key = "0";
    contents.start_pos = helpfile.tellg();
    for (long i = 0; i < contents_lines; ++i) {
        helpfile.ignore(WRITE_STR_LEN + 1, '\n');
    }
    contents.end_pos = helpfile.tellg();

    /*
     * Correct the positions in the index to point at the real offset in the
     * file.
     */
    for (auto &p : table) {
        p.second.start_pos += contents.end_pos;
        p.second.end_pos   += contents.end_pos;
    }

    // Add the contents to the table.
    table[contents.key] = contents;

    return true;
}

int helpfile_read(const key_str &keystr, int keylen, help_record &buffer, int buflen, int &reclen) {
    current_key.key.clear();
    current_key.key.append(keystr.data(), keylen);

    /*
     * lookup the key in the table, if it fails return the mysterious
     * status STSRNF
     */
    auto result = table.find(current_key.key);
    if (result == table.end())
	return STSRNF;

    /* Find the offset in the helpfile for the entry and go there */
    current_key = result->second;
    helpfile.seekg(current_key.start_pos);

    /* Get the first line in the entry and package it up in a help_record */
    std::string buf;
    std::getline(helpfile, buf);
    reclen = KEY_LEN + buf.size();
    buffer.key.fill(' ');
    buffer.txt.fill(' ');
    buffer.key.copy(current_key.key.data(),
                         std::min(current_key.key.size(), key_str::size()));
    buffer.txt.copy(buf.data(),
                         std::min(buf.size(), write_str::size()));
    return 1;
}

int helpfile_next(help_record &buffer, int buflen, int &reclen) {
    /*
     * if the current position = the end of the entry return 0, else give back
     * the next line nicely packaged.
     */
    if (helpfile.tellg() == current_key.end_pos) {
	return 0;
    } else {
        std::string buf;
        std::getline(helpfile, buf);
	reclen = KEY_LEN + buf.size();
        buffer.key.fill(' ');
        buffer.txt.fill(' ');
        buffer.key.copy(current_key.key.data(),
                        std::min(current_key.key.size(), key_str::size()));
        buffer.txt.copy(buf.data(),
                        std::min(buf.size(), write_str::size()));
    }
    return 1;
}

#ifdef TEST

#include <iostream>

int main() {
    if (helpfile_open(true) == 0) {
        std::cerr << "Cannot open helpfile!" << std::endl;
        return 1;
    }
    while (true) {
        std::cout << "Topic: ";
        std::string string;
        if (!(std::cin >> string) || (string == "exit")) {
            return 0;
        }
        key_str key(' ');
        size_t sz = std::min(string.size(), key_str::size());
        key.copy(string.data(), sz);
        help_record record;
        int buflen;
	if (helpfile_read(key, sz, record, KEY_LEN + WRITE_STR_LEN, buflen)) {
            size_t key_len = record.key.length(' ');
            size_t txt_len = record.txt.length(' ');
            std::cout << std::string(record.key.data(), key_len) << std::endl;
            std::cout << std::string(record.txt.data(), txt_len) << std::endl;
	    while (helpfile_next(record, KEY_LEN + WRITE_STR_LEN, buflen)) {
                txt_len = record.txt.length(' ');
                std::cout << std::string(record.txt.data(), txt_len) << std::endl;
            }
	} else {
            std::cerr << "Nothing found on '" << string << "'." << std::endl;
        }
    }
}
#endif
