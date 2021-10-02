/**********************************************************************}
{                                                                      }
{            L      U   U   DDDD   W      W  IIIII   GGGG              }
{            L      U   U   D   D   W    W     I    G                  }
{            L      U   U   D   D   W ww W     I    G   GG             }
{            L      U   U   D   D    W  W      I    G    G             }
{            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              }
{                                                                      }
{**********************************************************************}
{                                                                      }
{   This file written by Martin Sandiford                              }
{                                                                      }
{**********************************************************************/

/**
! Name:        SYS_LINUX
!
! Implementation O/S support routines for Linux, likely to work OK on
! other Unix-like systems also.
*/

#include "sys.h"

#include <algorithm>

#include <pwd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>


namespace {
    const std::string NL("\n");

    void do_exit(int status) {
        // Here would be the spot to tear-down what sys_initsig did.
        ::exit(status);
    }
};

bool sys_suspend() {
    return ::kill(0, SIGTSTP) == 0;
}

bool sys_shell() {
    // FIXME: Should really make this work
    return false;
}

bool sys_istty() {
    return ::isatty(0) && ::isatty(1);
}

bool sys_getenv(const std::string &environ, std::string &result) {
    char *env = ::getenv(environ.c_str());
    if (env == NULL)
        return false;
    result = env;
    return true;
}

void sys_initsig() {
    // Nothing yet
}

void sys_exit_success() {
    do_exit(EXIT_SUCCESS);
}

void sys_exit_failure() {
    do_exit(EXIT_FAILURE);
}

bool sys_expand_filename(std::string &filename) {
    if (filename.empty())
        return true;
    if (filename[0] == '~') {
        std::string::size_type slash = filename.find('/');
        // If there are no slashes, we just assume the users directory
        if (slash == std::string::npos) {
            slash = filename.size();
        }
        std::string username = filename.substr(1, slash - 1);
        std::string filepart = filename.substr(slash);
        std::string dir;
        if (username.empty()) {
            const char *s = getenv("HOME");
            dir = (s == NULL) ? "" : s;
        } else {
            struct passwd *passwd = ::getpwnam(username.c_str());
            if (passwd == NULL) {
                return false;
            }
            dir = passwd->pw_dir;
        }
        // Append slash if none
        if (!dir.empty() && dir.back() != '/') {
            dir.push_back('/');
        }
        filename = dir + filepart;
    }
    char *rp = ::realpath(filename.c_str(), NULL);
    if (rp == NULL) {
        return !filename.empty();
    }
    filename = rp;
    ::free(rp);
    return true;
}

bool sys_copy_filename(const std::string &src_path, std::string &dst_path) {
    // get the actual file name part of src_path.
    std::string::size_type slash = dst_path.rfind('/');
    if (!dst_path.empty() && dst_path != "/")
        dst_path += "/";
    if (slash == std::string::npos) {
        dst_path += src_path;
    } else {
        dst_path += src_path.substr(slash + 1);
    }
    return true;
}

int sys_open_command(const std::string &cmd) {
    int fd[2];
    if (::pipe(fd) == -1)
        return -1;
    pid_t pid = ::fork();
    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        ::close(fd[0]);
        if (::dup2(fd[1], 1) == -1 || dup2(fd[1], 2) == -1)
            ::exit(EXIT_FAILURE);
        ::close(0);
        ::open("/dev/null", O_RDONLY); // Just assuming this will be fd == 0
        int exitval = ::system(cmd.c_str());
        ::close(fd[1]);
        ::exit(WEXITSTATUS(exitval));
    } else {
        ::close(fd[1]);
        return fd[0];
    }
}

int sys_open_file(const std::string &filename) {
    return ::open(filename.c_str(), O_RDONLY, 0);
}

int sys_create_file(const std::string &filename) {
    return ::open(filename.c_str(), O_RDWR | O_CREAT, 0600);
}

int sys_file_mask() {
    mode_t m = umask(0);
    umask(m);
    return ~m;
}

bool sys_file_exists(const std::string &filename) {
    return ::access(filename.c_str(), F_OK) == 0;
}

bool sys_file_writeable(const std::string &filename) {
    return ::access(filename.c_str(), W_OK) == 0;
}

bool sys_write_filename(const std::string &memory, const std::string &filename) {
    if (!memory.empty()) {
        // File is specific to user, so use 0600 mask
        int envfd = ::open(memory.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);
        if (envfd >= 0) {
            ssize_t filename_size = filename.size();
            ssize_t nl_size = NL.size();
            bool ok = ::write(envfd, filename.data(), filename.size()) == filename_size;
            ok = ::write(envfd, NL.data(), NL.size()) == nl_size && ok;
            ::close(envfd);
            return ok;
        }
    }
    return false;
}

bool sys_read_filename(const std::string &memory, std::string &filename) {
    int envfd = ::open(memory.c_str(), O_RDONLY);
    if (envfd >= 0) {
        std::string tmpname(FILENAME_MAX, '\0');
        ssize_t read_len = ::read(envfd, &(tmpname[0]), FILENAME_MAX - 1);
        ::close(envfd);
        if (read_len > 0) {
            ssize_t nl_idx = 0;
            while (nl_idx < read_len) {
                if (tmpname[nl_idx] == '\n' || tmpname[nl_idx] == '\r')
                    break;
                nl_idx += 1;
            }
            filename = tmpname.substr(0, nl_idx);
            return !filename.empty();
        }
    }
    return false;
}

void sys_reap_children() {
    int pstat;
    while (::waitpid(0, &pstat, WNOHANG) == 0) {
        /* Do nothing */
    }
}

long sys_read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}

long sys_write(int fd, const void *buf, size_t count) {
    return ::write(fd, buf, count);
}

int sys_close(int fd) {
    return ::close(fd);
}

bool sys_unlink(const std::string &filename) {
    return ::unlink(filename.c_str()) == 0;
}

bool sys_rename(const std::string &oldname, const std::string &newname) {
    return ::rename(oldname.c_str(), newname.c_str()) == 0;
}

bool sys_chmod(const std::string &filename, int mask) {
    return ::chmod(filename.c_str(), mask) == 0;
}

bool sys_seek(int fd, long where) {
    return ::lseek(fd, where, L_SET) == 0;
}

long sys_tell(int fd) {
    return ::lseek(fd, 0, L_INCR);
}

file_status sys_file_status(const std::string &filename) {
    file_status fs;
    fs.valid = false;
    fs.mode  = 0600;
    fs.mtime = -1;
    fs.isdir = false;

    struct stat st;
    if (stat(filename.c_str(), &st) != 0)
        return fs;
    fs.valid = true;
    fs.mode  = st.st_mode & 07777;
    fs.mtime = st.st_mtime;
    fs.isdir = S_ISDIR(st.st_mode);
    return fs;
}

std::vector<long> sys_list_backups(const std::string &backup_name) {
    std::string dir(backup_name);
    // Find out what directory we are putting this file in
    std::string::size_type slash = dir.rfind('/');
    // Prefix for backup name of file
    std::string bname = slash == std::string::npos ? dir : dir.substr(slash + 1);
    size_t bname_len = bname.size();
    if (slash == std::string::npos) {
        // Current directory
        dir = ".";
    } else if (slash == 0) {
        // If directory is "/", then use that
        dir = "/";
    } else {
        // Just remove the file part
        dir.erase(slash);
    }
    std::vector<long> versions;
    DIR *dirp = opendir(dir.c_str());
    if (dirp != NULL) {
        for (struct dirent *dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
            std::string name(dp->d_name);
            if (name.size() >= bname_len && name.substr(0, bname_len) == bname) {
                size_t chars_used;
                long vnum = std::stol(name.substr(bname_len), &chars_used);
                if (chars_used == name.size() - bname_len)
                    versions.push_back(vnum);
            }
        }
        closedir(dirp);
    }
    // Oldest to newest
    std::sort(std::begin(versions), std::end(versions));
    return versions;
}
