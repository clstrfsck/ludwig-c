#ifndef SYS_H
#define SYS_H

// The intention is that anything that requires something that
// is not part of the C++ standard library will have an interface
// routine here that can be implemented appropriately on various
// operating systems.

#include <string>
#include <vector>

// Initialisation and tear-down
void sys_initsig();
void sys_exit_success();
void sys_exit_failure();

// O/S query and support
bool sys_suspend();
bool sys_shell();
bool sys_istty();
bool sys_getenv(const std::string &environ, std::string &result);

struct file_status {
    bool valid;
    int mode;
    long mtime;
    bool isdir;
};

// Filesystem access
bool sys_expand_filename(std::string &filename);
bool sys_copy_filename(const std::string &src_path, std::string &dst_path);
int sys_open_command(const std::string &cmd);
int sys_open_file(const std::string &filename);
int sys_create_file(const std::string &filename);
long sys_read(int fd, void *buf, size_t count);
long sys_write(int fd, const void *buf, size_t count);
int sys_close(int fd);
bool sys_seek(int fd, long where);
long sys_tell(int fd);

bool sys_file_exists(const std::string &filename);
bool sys_file_writeable(const std::string &filename);

file_status sys_file_status(const std::string &filename);

int sys_file_mask();
bool sys_write_filename(const std::string &memory, const std::string &filename);
bool sys_read_filename(const std::string &memory, std::string &filename);
bool sys_unlink(const std::string &filename);
bool sys_rename(const std::string &oldname, const std::string &newname);
bool sys_chmod(const std::string &filename, int mask);
void sys_reap_children();

std::vector<long> sys_list_backups(const std::string &filename);

#endif // !defined(SYS_H)
