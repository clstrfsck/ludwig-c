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
! Description: The File interface to Unix
!
!--*/

#include "filesys.h"

#include "screen.h"
#include "lwgetopt.h"

#include <cstring>
#include <sstream>

#include <pwd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>

/*----------------------------------------------------------------------------*/

static bool filesys_expand_file_name(file_name_str &fnm, int *fns) {
    if (*fns == 0 || fnm[1] == '\0' || fnm[1] == '/')
        return true;
    else if (fnm[1] == '~') {
        std::string start(fnm.data() + 1, *fns - 1);
        std::string end;
        std::string::size_type slash = start.find('/');
        if (slash != std::string::npos) {
            end = start.substr(slash);
            start.erase(slash);
        }
        if (start.empty()) {
            const char *s = getenv("HOME");
            start = (s == NULL) ? "" : s;
        } else {
            struct passwd *passwd = getpwnam(start.c_str());
            if (passwd == NULL) {
                return false;
            }
            start = passwd->pw_dir;
        }
        start += end;
        fnm.copy(start.c_str(), start.size() + 1);
        *fns = start.size();
        return true;
    } else if (fnm[1] != '.') {
        std::vector<char> vcwd(MAXPATHLEN, '\0');
        if (getcwd(vcwd.data(), MAXPATHLEN) == NULL) {
            perror("getcwd");
            return false;
        }
        std::string cwd(vcwd.data());
        std::string tmp;
        if (cwd == "/") {
            tmp = std::string("/") + fnm.data();
        } else if (fnm[1] != '\0') {
            std::string s(fnm.data());
            tmp = cwd + "/" + s;
        } else {
            tmp = cwd;
        }
        fnm.copy(tmp.data(), tmp.size() + 1);
        *fns = tmp.size();
        return true;
    } else {
        std::vector<char> vcwd(MAXPATHLEN, '\0');
        if (getcwd(vcwd.data(), MAXPATHLEN) == NULL) {
            perror("getcwd");
            return false;
        }
        std::string cwd(vcwd.data());
        std::string tmp(fnm.data(), *fns);
        const char *q = tmp.c_str();
        const char *s = tmp.c_str();
        while (*s == '.' && *++q == '.' && *++q == '/') {
            /*
             * filename references parent so modify cwd
             */
            std::string::size_type rs = cwd.rfind('/');
            if (rs == std::string::npos) {
                return false;
            }
            if (rs != 0) /* if cwd == "/" dont touch the '/'!! */
                cwd.erase(rs);
            else
                cwd.erase(rs + 1); /* make cwd "/" NOT ""! */
            q += 1;
            s = q;
        }
        std::string new_fnm = cwd;
        if (!new_fnm.empty() && new_fnm[new_fnm.size() - 1] != '/')
            new_fnm += "/";
        fnm.copy(new_fnm.c_str(), new_fnm.size() + 1);
        *fns = new_fnm.size();
        return true;
    }
}

/*----------------------------------------------------------------------------*/

bool filesys_create_open(file_ptr fyle, file_ptr rfyle, bool ordinary_open) {
    char   related[FILE_NAME_LEN+1];
    char   *p;
    struct stat stat_buffer;
    int    exists;

#ifdef DEBUG
    /* Check the file has a 'Z' in the right place */
    if (fyle->zed != 'Z') {
        screen_message("FILE and FILESYS definition of file_object disagree.");
        return false;
    }
#endif
    fyle->l_counter = 0;
    if (!fyle->output_flag) { /* open file for reading */
        if (fyle->fns == 0)
            return false;
        if (!ordinary_open) { /* really executing a command */
            int fd[2];
            if (pipe(fd) == -1) {
                screen_message("Cannot create pipe");
                return false;
            }
            pid_t pid = fork();
            if (pid == -1) {
                screen_message("Failed to fork");
                return false;
            } else if (pid == 0) {
                close(fd[0]);
                if (dup2(fd[1], 1) == -1 || dup2(fd[1], 2) == -1)
                    exit(1);
                close(0);
                open("/dev/null", O_RDONLY); // Just assuming this will be fd == 0
                fyle->fnm[fyle->fns] = '\0';
                system(fyle->fnm.data());
                close(fd[1]);
                exit(0);
            } else {
                close(fd[1]);
                fyle->fd = fd[0];
            }
        } else {
            if (!filesys_expand_file_name(fyle->fnm, &fyle->fns)) {
                std::stringstream s;
                s << "Error in filename (" << fyle->fnm.data() << ")";
                screen_message(s.str().c_str());
                return false;
            }
            if (stat(fyle->fnm.data(), &stat_buffer) != 0) {
                return false;
            }
            if ((stat_buffer.st_mode & S_IFMT) == S_IFDIR) {
                errno = EISDIR;
                return false;
            }
            if ((fyle->fd = open(fyle->fnm.data(), O_RDONLY, 0)) < 0) {
                return false; /* fail,    return false */
            }
            fyle->mode = stat_buffer.st_mode & 07777;
            fyle->previous_file_id = stat_buffer.st_mtime;
        }
        fyle->idx = 0;
        fyle->len = 0;
        fyle->eof = 0;
    } else { /* otherwise open new file for output */
        if (rfyle != NULL)
            strcpy(related, rfyle->fnm.data());
        else
            *related = '\0';
        if (fyle->fns == 0) { /* default to related filename */
            size_t len = std::strlen(related);
            fyle->fnm.copy(related, len);
            fyle->fns = len;
        }
        if (fyle->fns == 0)
            return false;
        else if (!filesys_expand_file_name(fyle->fnm, &fyle->fns)) {
            std::stringstream s;
            s << "Error in filename (" << fyle->fnm.data() << ")";
            screen_message(s.str().c_str());
            return false;
        }
        /*
         * if the file given is a directory create a filename using the input
         * filename in the given directory.
         */
        if (   (exists = (stat(fyle->fnm.data(), &stat_buffer) == 0))
            && (stat_buffer.st_mode & S_IFMT) == S_IFDIR
            && *related != '\0') {
            /*
             * get the actual file name part of the related file spec
             * a '/' MUST exist in the name since the path has been fully
             * expanded, only problem is if the original filename is '/'
             */
            p = rindex(related,'/');
            if (strcmp(fyle->fnm.data(), "/") == 0)
                fyle->fnm.copy(p, std::strlen(p) + 1);
            else
                fyle->fnm.copy(p, std::strlen(p) + 1, std::strlen(fyle->fnm.data()) + 1);
            exists = (stat(fyle->fnm.data(), &stat_buffer) == 0);
        }
        if (exists) {
            /* if we wanted to create a new file then complain */
            if (fyle->create) {
                std::stringstream s;
                s << "File (" << fyle->fnm.data() << ") already exists";
                screen_message(s.str().c_str());
                return false;
            }
            /* check that the file we may overwrite is not a directory */
            if ((stat_buffer.st_mode & S_IFMT) == S_IFDIR) {
                std::stringstream s;
                s << "File (" << fyle->fnm.data() << ") is a directory";
                screen_message(s.str().c_str());
                return false;
            }
            /* check that we can write over the current version */
            if (access(fyle->fnm.data(), W_OK) == -1) {
                std::stringstream s;
                s << "Write access to file (" << fyle->fnm.data() << ") is denied";
                screen_message(s.str().c_str());
                return false;
            }
            fyle->mode = stat_buffer.st_mode & 07777;
            fyle->previous_file_id = stat_buffer.st_mtime;
        } else {
            fyle->mode = 0666 & ~umask(0);
            fyle->previous_file_id = 0;
        }
        /* now create the temporary name */
        int uniq = 0;

        fyle->tnm = std::string(fyle->fnm.data()) + "-lw";
        while (access(fyle->tnm.c_str(), F_OK) == 0) {
            fyle->tnm = std::string(fyle->fnm.data()) + std::string("-lw") + std::to_string(++uniq);
        }
        if ((fyle->fd = open(fyle->tnm.c_str(), O_RDWR | O_CREAT, 0600)) < 0) {
            std::stringstream s;
            s << "Error opening (" << fyle->tnm << ") as output";
            screen_message(s.str().c_str());
            return false;     /* fail,    return false */
        }
    }
    return true;              /* succeed, return true  */
}

/*----------------------------------------------------------------------------*/

static bool filesys_write_file_name(const char *mem, const char *fnm, int fns) {
    if (mem != NULL && mem[0] != '\0') {
        int envfd = open(mem, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (envfd >= 0) {
            write(envfd, fnm, fns);
            write(envfd, "\n", strlen("\n"));
            close(envfd);
            return true;
        }
    }
    return false;
}

/*----------------------------------------------------------------------------*/

static bool filesys_read_file_name(const char *mem, file_name_str &fnm, int &fns)
{
    int envfd;
    if (mem != nullptr && (envfd = open(mem, O_RDONLY)) >= 0) {
        int read_len = read(envfd, fnm.data(), file_name_str::index_type::size() - 1);
        close(envfd);
        if (read_len > 0) {
            int nl_idx;
            for (nl_idx = 1; nl_idx <= read_len; ++nl_idx) {
                if (fnm[nl_idx] == '\n' || fnm[nl_idx] == '\r')
                    break;
            }
            fnm.fill(0, nl_idx);
            fns = nl_idx - 1;
            return fns > 0;
        }
    }
    return false;
}

/*----------------------------------------------------------------------------*/

bool filesys_close(file_ptr fyle, int action, bool msgs) {
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
        /* reap any children */
        int pstat;
        while (waitpid(0, &pstat, WNOHANG) == 0) {
            /* Do nothing */
        }
        /* an ordinary input file, just close */
        if (close(fyle->fd) < 0)
            return false;      /* fail,    return false */
        if (msgs) {
            std::stringstream s;
            s << "File " << fyle->fnm.data() << " closed (" <<
                fyle->l_counter << " line" <<
                (fyle->l_counter == 1 ? "" : "s") << " read).";
            screen_message(s.str().c_str());
        }
        return true;           /* succeed, return true */
    }
    /* an output file to close */
    if ((action != 2) && (close(fyle->fd) < 0))
        return false;          /* fail,    return false */
    if (action == 1) {
        /* remove the file */
        if (!unlink(fyle->tnm.c_str())) {
            if (msgs) {
                std::stringstream s;
                s << "Output file " << fyle->tnm << " deleted.";
                screen_message(s.str().c_str());
            }
            return true;       /* succeed, return true  */
        } else {
            return false;
        }
    }
    /*
     * check that another file hasn't been created while we were editting
     * with the name we are going to use as the output name.
     */
    struct stat stat_buffer;
    if (stat(fyle->fnm.data(), &stat_buffer) == 0) {
        if (stat_buffer.st_mtime != fyle->previous_file_id) {
            std::stringstream s;
            s << fyle->fnm.data() << " was modified by another process";
            screen_message(s.str().c_str());
        }
    }
    std::string dir(fyle->fnm.data());
    /* find out what directory we are putting this file in */
    std::string::size_type ls = dir.rfind(dir, '/');
    // Backup name
    std::string bname = dir.substr(ls + 1) + "~";
    std::string tname = bname + "%ld";
    /* if directory is "/" it is OK to make it "" since we add a '/' later */
    dir.erase(ls);
    long max_vnum = 0;
    if (fyle->purge) {
        /*
         * make sure we allocate some space, even if fyle->versions = 0
         */
        std::vector<long> versions;
        long entries = 0;
        DIR *dirp = opendir(dir.empty() ? "/" : dir.c_str());
        for (struct dirent *dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
            long vnum;
            if (sscanf(dp->d_name, tname.c_str(), &vnum) == 1) {
                int i;
                for (i = 0; i < entries && versions[i] > vnum; ++i)
                    /* Nothing to do here */;
                if (i < fyle->versions - 1) {
                    if (entries == fyle->versions - 1) {
                        std::string temp = dir + std::string("/") + bname + std::to_string(versions[entries - 1]);
                        unlink(temp.c_str());
                    } else {
                        entries += 1;
                        versions.push_back(0);
                    }
                    for (int j = entries - 1; j > i; --j) {
                        versions[j] = versions[j - 1];
                    }
                    versions[i] = vnum;
                } else {
                    std::string temp = dir + std::string("/") + dp->d_name;
                    unlink(temp.c_str());
                }
            }
        }
        closedir(dirp);
        max_vnum = versions.empty() ? 0 : versions[0];
    } else {
        long min_vnum = -1;
        long entries = 0;
        DIR *dirp = opendir(dir.empty() ? "/" : dir.c_str());
        for (struct dirent *dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
            long vnum;
            if (sscanf(dp->d_name, tname.c_str(), &vnum) == 1) {
                max_vnum = std::max(max_vnum, vnum);
                min_vnum = std::min(min_vnum, vnum);
                entries += 1;
                if (vnum > max_vnum)
                    max_vnum = vnum;
                if (min_vnum == -1 || vnum < min_vnum)
                    min_vnum = vnum;
                entries += 1;
            }
        }
        closedir(dirp);
        if (entries >= fyle->versions) {
            std::string temp = dir + "/" + bname + std::to_string(min_vnum);
            unlink(temp.c_str());
        }
    }
    /*
     * Perform Backup on original file if -
     *    a. fyle->versions != 0
     * or b. doing single backup and backup already done at least once
     */
    if (fyle->versions != 0 || (!fyle->purge && max_vnum != 0)) {
        /* does the real file already exist? */
        if (stat(fyle->fnm.data(), &stat_buffer) == 0) {
            /* try to rename current file to backup */
            std::string temp = dir + "/" + bname + std::to_string(max_vnum + 1);
            rename(fyle->fnm.data(), temp.c_str());
        }
    }
    /* now rename the temp file to the real thing */
    chmod(fyle->tnm.c_str(), fyle->mode & 07777);
    if (rename(fyle->tnm.c_str(), fyle->fnm.data())) {
        std::stringstream s;
        s << "Cannot rename " << fyle->tnm << " to " << fyle->fnm.data();
        screen_message(s.str().c_str());
        return false;      /* fail,    return false */
    } else {
        if (msgs) {
            std::stringstream s;
            s << "File " << fyle->fnm.data() << " created (" << fyle->l_counter <<
                " line" << (fyle->l_counter == 1 ? "" : "s") << " written).";
            screen_message(s.str().c_str());
        }
        /*
         * Time to set the memory, if it's required and we aren't writing in
         * one of the global tmp directories
         */
        if (dir != "/tmp" && dir != "/usr/tmp") {
            filesys_write_file_name(fyle->memory.c_str(), fyle->fnm.data(), fyle->fns);
        }
        return true;       /* succeed, return true  */
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
        if (fyle->idx == fyle->len) {
            fyle->buf.resize(MAX_STRLEN);
            fyle->len = read(fyle->fd, fyle->buf.data(), MAX_STRLEN);
            fyle->idx = 0;
        }
        if (fyle->len <= 0) {
            fyle->eof = 1;
            /*
             * If the last line is not terminated properly,
             * the buffer is not empty and we must return the buffer.
             */
            if (outlen)
                break;
            return false;  /* fail,    return false */
        }
        int ch = toascii(fyle->buf[fyle->idx++]);
        if (std::isprint(ch)) {
            output_buffer[++outlen] = ch;
        } else if (ch == '\t') { /* expand the tab */
            int exp = 8 - (outlen % 8);

            if (outlen + exp > MAX_STRLEN)
                exp = MAX_STRLEN - outlen;
            for (; exp > 0; exp--)
                output_buffer[++outlen] = ' ';
        } else if (ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f') {
            break;              /* finished if newline or carriage return */
        } /* forget other control characters */
    } while (outlen < MAX_STRLEN - 1);
    fyle->l_counter += 1;
    output_buffer[outlen + 1] = '\0';
    return true;           /* succeed, return true  */
}

/*----------------------------------------------------------------------------*/

bool filesys_rewind(file_ptr fyle) {
/*
 * Rewinds file described by the FILE_PTR `fyle'.
 */
    fflush(stdout);
    if (lseek(fyle->fd, 0, L_SET) < 0)
        return 0;
    fyle->idx = 0;
    fyle->len = 0;
    fyle->eof = 0;
    fyle->l_counter = 0;
    return 1;
}

/*----------------------------------------------------------------------------*/

bool filesys_write(file_ptr fyle, str_object &buffer, strlen_range bufsiz) {
/* Attempts to write bufsiz characters from buffer to the file described by
 * fyle. Returns true (1) on success, false (0) on failure.
 */
    const static char nl = '\n';

    int offset = 0;
    int tabs = 0;
    if (fyle->entab) {
        int i;
        for (i = 0; i < bufsiz; ++i)
            if (buffer[i + 1] != ' ') break;
        tabs = i / 8;
        offset = tabs * 7;
        for (i = 1; i <= tabs; i++)
            buffer[offset + i] = '\t';
    }
    int count = write(fyle->fd, buffer.data() + offset, bufsiz - offset);
    if (tabs) {
        for (int i = 1; i <= tabs; i++)
            buffer[offset + i] = ' ';
    }
    if (count != bufsiz - offset)
        return false;
    write(fyle->fd, &nl, 1);
    fyle->l_counter += 1;
    return true; /* succeed, return true  */
}

/*----------------------------------------------------------------------------*/

bool filesys_save(file_ptr i_fyle, file_ptr o_fyle, int copy_lines) {
/*  Implements part of the File Save command. */
//        BOOLEAN input_eof;
//        int input_position;
//        STR_OBJECT line;
//        STRLEN_RANGE line_len;
//        int i;
    file_object fyle;

    bool input_eof;
    off_t input_position;
    str_object line;
    strlen_range line_len;
    if (i_fyle != nullptr) {
        /*  remember things to be restored */
        input_eof = i_fyle->eof;
        input_position = lseek(o_fyle->fd, 0, L_INCR);

        /*  copy unread portion of input file to output file */
        do {
            if (filesys_read(i_fyle, line, line_len))
                filesys_write(o_fyle, line, line_len);
        } while (!i_fyle->eof);

        /* close input file */
        filesys_close(i_fyle, 0, true);
    }

    /* rename temporary file to output file */
    /* process backup options, but do not close the file */
    filesys_close(o_fyle, 2, true);

    /* make the old output file the new input file */
    if (i_fyle == NULL) {
          i_fyle = &fyle;
          i_fyle->output_flag = false;
          i_fyle->first_line = nullptr;
          i_fyle->last_line = nullptr;
          i_fyle->line_count = 0;
    }
    i_fyle->fns = o_fyle->fns;
    i_fyle->fnm = o_fyle->fnm;
    i_fyle->fd  = o_fyle->fd;

    /* rewind the input file */
    filesys_rewind(i_fyle);

    /* open a new output file */
    o_fyle->create = false;
    if (!filesys_create_open(o_fyle, nullptr, true))
        return false;

    /* copy lines from the input file to the output file */
    for (int i = 0; i < copy_lines; i++) {
        if (!filesys_read(i_fyle, line, line_len))
            return false;
        if (!filesys_write(o_fyle, line, line_len))
            return false;
    }

    /* reposition or close the input file */
    if (i_fyle == &fyle) {
        close(i_fyle->fd);
    } else {
        i_fyle->eof = input_eof;
        lseek(i_fyle->fd, input_position, L_SET);
        i_fyle->idx = 0;
        i_fyle->len = 0;
    }
    return true;
}

/*----------------------------------------------------------------------------*/

bool filesys_parse(const char *command_line, parse_type parse,
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

    /* create an argc and argv from the "command_line" */
    std::vector<char> cmdline(command_line, command_line + std::strlen(command_line) + 1);
    std::vector<const char *> argv;
    argv.push_back("Ludwig");
    for (char *temp = strtok(cmdline.data(), " "); temp != nullptr; temp = strtok(nullptr, " "))
        argv.push_back(temp);
    argv.push_back(nullptr);

    int  argc     = argv.size() - 1; // Exclude nullptr
    bool entab    = file_data.entab;
    int  space    = file_data.space;
    bool purge    = file_data.purge;
    int  versions = file_data.versions;

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
        initialize = std::string(getenv("HOME")) + "/.ludwigrc";
        memory = std::string(getenv("HOME")) + "/.lud_memory";
    } else {
        initialize.clear();
        memory.clear();
    }
    lwoptreset = 1;
    lwoptind = 1;
    int c;
    while ((c = lwgetopt(argc, argv.data(), "cri:Is:m:MtTb:B:oOu")) != -1) {
        switch (c) {
        case 'c' :
            if (read_only_flag)
                errors++;
            else
                create_flag = 1;
            break;
        case 'r' :
            if (create_flag)
                errors++;
            else
                read_only_flag = 1;
            break;
        case 'i' :
            initialize = lwoptarg;
            break;
        case 'I' :
            initialize = "";
            break;
        case 's' :
            space_flag = 1;
            sscanf(lwoptarg, "%d", &space);
            break;
        case 'm' :
            memory = lwoptarg;
            break;
        case 'M' :
            memory = "";
            break;
        case 't' :
            entab = 1;
            break;
        case 'T' :
            entab = 0;
            break;
        case 'b' :
            purge = 0;
            sscanf(lwoptarg, "%d", &versions);
            break;
        case 'B' :
            purge = 1;
            sscanf(lwoptarg, "%d", &versions);
            break;
        case 'o' :
            version_flag = 1;
            file_data.old_cmds = true;
            break;
        case 'O' :
            version_flag = 1;
            file_data.old_cmds = false;
            break;
        case 'u' :
            usage_flag = 1;
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
        if (!initialize.empty()) {
            int len = initialize.size();
            file_data.initial.fill(' ');
            file_data.initial.copy(initialize.data(), len);
        } else {
            file_data.initial.fill(' ');
        }
        file_data.space = space;
        file_data.entab = entab;
        file_data.purge = purge;
        file_data.versions = versions;
    } else if (create_flag || read_only_flag || !initialize.empty() || space_flag || version_flag) {
        return false;
    }
    std::vector<std::string> file;
    for (int files = 0; lwoptind < argc; lwoptind++) {
        if (files >= 2) {
            screen_message("More than two files specified");
            return false;
        }
        file.push_back(argv[lwoptind]);
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
        if (file.size() > 0) {
            input->fns = file[0].size();
            input->fnm.copy(file[0].c_str(), file[0].size() + 1);
        } else if (!memory.empty()) {
            if (filesys_read_file_name(memory.c_str(), input->fnm, input->fns)) {
                check_input = true;
            } else {
                if (parse == parse_type::parse_command) {
                    input->fns = 0;
                    return true;
                }
                std::stringstream s;
                s << "Error opening memory file (" << memory << ")";
                screen_message(s.str().c_str());
                return false;
            }
        } else if (parse == parse_type::parse_command) {
            input->fns = 0;
            return true;
        } else {
            input->fns = 0;
            return false;
        }
        if (file.size() > 1) {
            output->fns = file[1].size();
            output->fnm.copy(file[1].c_str(), file[1].size() + 1);
        } else {
            output->fns = input->fns;
            output->fnm = input->fnm;
        }
        if (!memory.empty())
            output->memory = memory;
        else
            output->memory.clear();
        output->entab = entab;
        output->purge = purge;
        output->versions = versions;
        if (read_only_flag) {
            input->create = 0;
            if (!filesys_create_open(input, nullptr, true)) {
                std::stringstream s;
                s << "Error opening (" << input->fnm.data() << ") as input";
                screen_message(s.str().c_str());
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
                s << "Error opening (" << input->fnm.data() << ") as input";
                screen_message(s.str().c_str());
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
            input->fns = file[0].size();
            input->fnm.copy(file[0].c_str(), file[0].size() + 1);
        } else if (memory.empty() || !filesys_read_file_name(memory.c_str(), input->fnm, input->fns)) {
            input->fns = 0;
            return false;
        }
        input->create = false;
        if (input->fns == 0 || !filesys_create_open(input, nullptr, true)) {
            std::stringstream s;
            s << "Error opening (" << input->fnm.data() << ") as input";
            screen_message(s.str().c_str());
            return false;
        }
        input->valid = true;
        break;
    case parse_type::parse_execute:
        if (file.size() == 1) {
            input->fns = file[0].size();
            input->fnm.copy(file[0].c_str(), file[0].size() + 1);
        } else {
            input->fns = 0;
        }
        input->create = false;
        if (input->fns == 0 || !filesys_create_open(input, nullptr, true))
            return false;
        input->valid = true;
        break;
    case parse_type::parse_output:
        if (file.size() == 1) {
            output->fns = file[0].size();
            output->fnm.copy(file[0].c_str(), file[0].size() + 1);
        } else if (input != nullptr) {
            output->fns = input->fns;
            output->fnm = input->fnm;
        } else {
            output->fns = 0;
        }
        output->memory   = memory;
        output->entab    = entab;
        output->purge    = purge;
        output->versions = versions;
        output->create   = false;
        if (output->fns == 0 || !filesys_create_open(output, input, true))
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

