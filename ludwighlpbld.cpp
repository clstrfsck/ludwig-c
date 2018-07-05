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

/*
 * Name:        LUDWIGHLP
 *
 * Description: This program converts a sequential Ludwig help file into
 *              an indexed file for fast access.
 *
 * Revision History:
 * 4-001 Ludwig V4.0 release.                                 7-Apr-1987
 * 4-002 Kelvin B. Nicolle                                    5-May-1987
 *       The input text has been reformatted so that column one contains
 *       only flag characters.
 * 4-003 Jeff Blows                                          23-Jun-1989
 *       Merge changes needed to compile on MS-DOS
 * 4-004 Ludwig                                              28-Feb-1990
 *       Stop complaining about line too long when it's a comment.
 */

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace {
    const size_t ENTRYSIZE = 78; // 77 + 1 for NUL
    const size_t KEYSIZE   = 4;

    // I don't think this is really required, as no-one is really
    // limited to DOS 8.3 filenames any more, but here we are...
#ifdef turboc
    const std::string INPUT_HELP_FILE("ludwighl.txt");
    const std::string OUTPUT_HELP_FILE("ludwighl.idx");
#else
    const std::string INPUT_HELP_FILE("ludwighlp.t");
    const std::string OUTPUT_HELP_FILE("ludwighlp.idx");
#endif
};

void process_files(std::ifstream &in, std::ofstream &out)
{
    std::string section("0");

    // Instead of using temporary files, we are going to use
    // stringstreams to deal with all the intermediate processing.
    std::stringstream index;
    std::stringstream contents;
    std::stringstream body;

    long index_lines = 0;
    long contents_lines = 0;
    char flag;
    std::string line;
    do {
        if (!in.get(flag)) {
            break;
        } else if (flag == '\n') {
            flag = ' ';
            line.clear();
        } else {
            if (!std::getline(in, line))
                break;
            if (line.size() > ENTRYSIZE) {
                line = line.substr(0, ENTRYSIZE);
                if (flag != '!' && flag != '{') {
                    std::cerr << "Line too long--truncated" << std::endl;
                    std::cerr << line << ">>" << std::endl;
                }
            }
        }
        switch (flag) {
        case '\\':
            if (!line.empty()) {
                switch (line[0]) {
                case '%':
                    body  << "\\%\n";
                    break;
                case '#':
                    if (section != "0")
                        index << std::setw(8) << body.tellp() << '\n';
                    break;
                default:
                    if (section != "0")
                        index << std::setw(8) << body.tellp() << '\n';
                    section = line.substr(0, KEYSIZE);
                    if (section != "0") {
                        index_lines += 1;
                        index << std::setw(KEYSIZE) << section << ' ';
                        index << std::setw(8) << body.tellp();
                    }
                    break;
                }
            }
            break;
        case '+':
            contents_lines += 1;
            contents << line << '\n';
            body << line << '\n';
            break;
        case ' ':
            if (section == "0") {
                contents_lines += 1;
                contents << line << '\n';
            } else {
                body << line << '\n';
            }
            break;
        case '{':
        case '!':
            break;
        default :
            std::cerr << "Illegal flag character." << std::endl;
            std::cerr << flag << line << ">>" << std::endl;
            break;
        }
    } while (flag != '\\' || line.empty() || line[0] != '#');

    // Rewind everything
    index.seekg(0);
    contents.seekg(0);
    body.seekg(0);

    out << index_lines << ' ' << contents_lines << '\n';
    while (std::getline(index, line))
        out << line << '\n';
    while (std::getline(contents, line))
        out << line << '\n';
    while (std::getline(body, line))
        out << line << '\n';
}

int main(int argc, char **argv)
{
    std::string infile = (--argc > 0) ? *++argv : INPUT_HELP_FILE;
    std::ifstream in(infile);
    if (!in.is_open()) {
        std::perror(infile.c_str());
        return 1;
    }
    std::string outfile = (--argc > 0) ? *++argv : OUTPUT_HELP_FILE;
    std::ofstream out(outfile);
    if (!out.is_open()) {
        std::perror(outfile.c_str());
        return 1;
    }
    process_files(in, out);
    std::cout << "Conversion complete." << std::endl;
    return 0;
}
