/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_UNISTD_H
#define WLIBC_UNISTD_H

#include <wlibc-macros.h>
#include <sys/types.h>
#include <wchar.h>

// Avoid C2375: 'unlink': redefinition different linkage
#include <stdio.h>
#define unlink wlibc_unlink

_WLIBC_BEGIN_DECLS

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0x0 // File Exists
#define R_OK 0x4 // Read permission
#define W_OK 0x2 // Write permission
#define X_OK 0x1 // Execute/Search permission

WLIBC_API int wlibc_access(const char *name, int mode);
WLIBC_API int wlibc_waccess(const wchar_t *wname, int mode);

WLIBC_INLINE int access(const char *name, int mode)
{
	return wlibc_access(name, mode);
}

WLIBC_INLINE int waccess(const wchar_t *wname, int mode)
{
	return wlibc_waccess(wname, mode);
}

WLIBC_API int wlibc_faccessat(int dirfd, const char *name, int mode, int flags);
WLIBC_API int wlibc_wfaccessat(int dirfd, const wchar_t *wname, int mode, int flags);

WLIBC_INLINE int faccessat(int dirfd, const char *name, int mode, int flags)
{
	return wlibc_faccessat(dirfd, name, mode, flags);
}

WLIBC_INLINE int wfaccessat(int dirfd, const wchar_t *wname, int mode, int flags)
{
	return wlibc_wfaccessat(dirfd, wname, mode, flags);
}

WLIBC_API int wlibc_close(int fd);
WLIBC_INLINE int close(int fd)
{
	return wlibc_close(fd);
}

WLIBC_API int wlibc_chdir(const char *name);
WLIBC_API int wlibc_wchdir(const wchar_t *wname);

WLIBC_INLINE int chdir(const char *name)
{
	return wlibc_chdir(name);
}

WLIBC_INLINE int wchdir(const wchar_t *wname)
{
	return wlibc_wchdir(wname);
}

WLIBC_API int wlibc_fchdir(int fd);

WLIBC_INLINE int fchdir(int fd)
{
	return wlibc_fchdir(fd);
}

WLIBC_API int wlibc_chown(const char *name, uid_t owner, gid_t group);
WLIBC_API int wlibc_wchown(const wchar_t *wname, uid_t owner, gid_t group);

WLIBC_INLINE int chown(const char *name, uid_t owner, gid_t group)
{
	return wlibc_chown(name, owner, group);
}

WLIBC_INLINE int wchown(const wchar_t *wname, uid_t owner, gid_t group)
{
	return wlibc_wchown(wname, owner, group);
}

WLIBC_API int wlibc_lchown(const char *name, uid_t owner, gid_t group);
WLIBC_API int wlibc_wlchown(const wchar_t *wname, uid_t owner, gid_t group);

WLIBC_INLINE int lchown(const char *name, uid_t owner, gid_t group)
{
	return wlibc_lchown(name, owner, group);
}

WLIBC_INLINE int wlchown(const wchar_t *wname, uid_t owner, gid_t group)
{
	return wlibc_wlchown(wname, owner, group);
}

WLIBC_API int wlibc_fchown(int fd, uid_t owner, gid_t group);

WLIBC_INLINE int fchown(int fd, uid_t owner, gid_t group)
{
	return wlibc_fchown(fd, owner, group);
}

int wlibc_fchownat(int dirfd, const char *name, uid_t owner, gid_t group, int flags);
int wlibc_wfchownat(int dirfd, const wchar_t *wname, uid_t owner, gid_t group, int flags);

WLIBC_INLINE int fchownat(int dirfd, const char *name, uid_t owner, gid_t group, int flags)
{
	return wlibc_fchownat(dirfd, name, owner, group, flags);
}

WLIBC_INLINE int wfchownat(int dirfd, const wchar_t *wname, uid_t owner, gid_t group, int flags)
{
	return wlibc_wfchownat(dirfd, wname, owner, group, flags);
}

WLIBC_API int wlibc_dup(int fd);
WLIBC_API int wlibc_dup2(int oldfd, int newfd);
WLIBC_API int wlibc_dup3(int oldfd, int newfd, int flags);

WLIBC_INLINE int dup(int fd)
{
	return wlibc_dup(fd);
}

WLIBC_INLINE int dup2(int oldfd, int newfd)
{
	return wlibc_dup2(oldfd, newfd);
}

WLIBC_INLINE int dup3(int oldfd, int newfd, int flags)
{
	return wlibc_dup3(oldfd, newfd, flags);
}

WLIBC_API int wlibc_fdatasync(int fd);
WLIBC_API int wlibc_fsync(int fd);

WLIBC_INLINE int fdatasync(int fd)
{
	return wlibc_fdatasync(fd);
}

WLIBC_INLINE int fsync(int fd)
{
	return wlibc_fsync(fd);
}

WLIBC_API char *wlibc_getcwd(char *buf, size_t size);
WLIBC_API wchar_t *wlibc_wgetcwd(wchar_t *wbuf, size_t size);

WLIBC_INLINE char *getcwd(char *buf, size_t size)
{
	return wlibc_getcwd(buf, size);
}

WLIBC_INLINE wchar_t *wgetcwd(wchar_t *wbuf, size_t size)
{
	return wlibc_wgetcwd(wbuf, size);
}

WLIBC_API gid_t wlibc_getgid();
WLIBC_API gid_t wlibc_getegid();

WLIBC_INLINE gid_t getgid()
{
	return wlibc_getgid();
}

WLIBC_INLINE gid_t getegid()
{
	return wlibc_getegid();
}

WLIBC_API pid_t wlibc_getpid();
WLIBC_API pid_t wlibc_getppid();

WLIBC_INLINE pid_t getpid()
{
	return wlibc_getpid();
}

WLIBC_INLINE pid_t getppid()
{
	return wlibc_getppid();
}

WLIBC_API uid_t wlibc_getuid();
WLIBC_API uid_t wlibc_geteuid();

WLIBC_INLINE uid_t getuid()
{
	return wlibc_getuid();
}

WLIBC_INLINE uid_t geteuid()
{
	return wlibc_geteuid();
}

WLIBC_API int wlibc_getpagesize();
WLIBC_INLINE int getpagesize()
{
	return wlibc_getpagesize();
}

WLIBC_API int wlibc_pipe(int pipefd[2]);
WLIBC_INLINE int pipe(int pipefd[2])
{
	return wlibc_pipe(pipefd);
}

WLIBC_API int wlibc_pipe2(int pipefd[2], int flags);
WLIBC_INLINE int pipe2(int pipefd[2], int flags)
{
	return wlibc_pipe2(pipefd, flags);
}

WLIBC_API int wlibc_isatty(int fd);
WLIBC_INLINE int isatty(int fd)
{
	return wlibc_isatty(fd);
}

WLIBC_API int wlibc_kill(pid_t pid, int sig);
WLIBC_INLINE int kill(pid_t pid, int sig)
{
	return wlibc_kill(pid, sig);
}

WLIBC_API int wlibc_link(const char *source, const char *target);
WLIBC_API int wlibc_wlink(const wchar_t *wsource, const wchar_t *wtarget);

WLIBC_INLINE int link(const char *source, const char *target)
{
	return wlibc_link(source, target);
}

WLIBC_INLINE int wlink(const wchar_t *wsource, const wchar_t *wtarget)
{
	return wlibc_wlink(wsource, wtarget);
}

WLIBC_API int wlibc_linkat(int olddirfd, const char *source, int newdirfd, const char *target, int flags);
WLIBC_API int wlibc_wlinkat(int olddirfd, const wchar_t *wsource, int newdirfd, const wchar_t *wtarget, int flags);

WLIBC_INLINE int linkat(int olddirfd, const char *source, int newdirfd, const char *target, int flags)
{
	return wlibc_linkat(olddirfd, source, newdirfd, target, flags);
}

WLIBC_INLINE int wlinkat(int olddirfd, const wchar_t *wsource, int newdirfd, const wchar_t *wtarget, int flags)
{
	return wlibc_wlinkat(olddirfd, wsource, newdirfd, wtarget, flags);
}

WLIBC_API off_t wlibc_lseek(int fd, off_t offset, int whence);
WLIBC_INLINE off_t lseek(int fd, off_t offset, int whence)
{
	return wlibc_lseek(fd, offset, whence);
}

WLIBC_API ssize_t wlibc_read(int fd, void *buf, size_t count);
WLIBC_INLINE ssize_t read(int fd, void *buf, size_t count)
{
	return wlibc_read(fd, buf, count);
}

WLIBC_API int wlibc_rmdir(const char *path);
WLIBC_API int wlibc_wrmdir(const wchar_t *wpath);

WLIBC_INLINE int rmdir(const char *path)
{
	return wlibc_rmdir(path);
}

WLIBC_INLINE int wrmdir(const wchar_t *wpath)
{
	return wlibc_wrmdir(wpath);
}

WLIBC_API int wlibc_symlink(const char *source, const char *target);
WLIBC_API int wlibc_wsymlink(const wchar_t *wsource, const wchar_t *wtarget);

WLIBC_INLINE int symlink(const char *source, const char *target)
{
	return wlibc_symlink(source, target);
}

WLIBC_INLINE int wsymlink(const wchar_t *wsource, const wchar_t *wtarget)
{
	return wlibc_wsymlink(wsource, wtarget);
}

WLIBC_API int wlibc_symlinkat(const char *source, int newdirfd, const char *target);
WLIBC_API int wlibc_wsymlinkat(const wchar_t *wsource, int newdirfd, const wchar_t *wtarget);

WLIBC_INLINE int symlinkat(const char *source, int newdirfd, const char *target)
{
	return wlibc_symlinkat(source, newdirfd, target);
}

WLIBC_INLINE int wsymlinkat(const wchar_t *wsource, int newdirfd, const wchar_t *wtarget)
{
	return wlibc_wsymlinkat(wsource, newdirfd, wtarget);
}

WLIBC_API ssize_t wlibc_readlink(const char *path, char *buf, size_t bufsiz);
WLIBC_API ssize_t wlibc_wreadlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz);

WLIBC_INLINE ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	return wlibc_readlink(path, buf, bufsiz);
}

WLIBC_INLINE ssize_t wreadlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz)
{
	return wlibc_wreadlink(wpath, wbuf, bufsiz);
}

WLIBC_API ssize_t wlibc_readlinkat(int dirfd, const char *path, char *buf, size_t bufsiz);
WLIBC_API ssize_t wlibc_wreadlinkat(int dirfd, const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz);

WLIBC_INLINE ssize_t readlinkat(int dirfd, const char *path, char *buf, size_t bufsiz)
{
	return wlibc_readlinkat(dirfd, path, buf, bufsiz);
}

WLIBC_INLINE ssize_t wreadlinkat(int dirfd, const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz)
{
	return wlibc_wreadlinkat(dirfd, wpath, wbuf, bufsiz);
}

WLIBC_API int wlibc_truncate(const char *path, off_t length);
WLIBC_API int wlibc_wtruncate(const wchar_t *wpath, off_t length);

WLIBC_INLINE int truncate(const char *path, off_t length)
{
	return wlibc_truncate(path, length);
}

WLIBC_INLINE int wtruncate(const wchar_t *wpath, off_t length)
{
	return wlibc_wtruncate(wpath, length);
}

WLIBC_API int wlibc_ftruncate(int fd, off_t length);
WLIBC_INLINE int ftruncate(int fd, off_t length)
{
	return wlibc_ftruncate(fd, length);
}

WLIBC_API char *wlibc_ttyname(int fd);
WLIBC_API wchar_t *wlibc_wttyname(int fd);

WLIBC_INLINE char *ttyname(int fd)
{
	return wlibc_ttyname(fd);
}

WLIBC_INLINE wchar_t *wttyname(int fd)
{
	return wlibc_wttyname(fd);
}

WLIBC_API int wlibc_ttyname_r(int fd, char *buf, size_t bufsiz);
WLIBC_API int wlibc_wttyname_r(int fd, wchar_t *wbuf, size_t bufsiz);

WLIBC_INLINE int ttyname_r(int fd, char *buf, size_t bufsiz)
{
	return wlibc_ttyname_r(fd, buf, bufsiz);
}

WLIBC_INLINE int wttyname_r(int fd, wchar_t *wbuf, size_t bufsiz)
{
	return wlibc_wttyname_r(fd, wbuf, bufsiz);
}

WLIBC_API int wlibc_unlink(const char *path);
WLIBC_API int wlibc_wunlink(const wchar_t *wpath);

WLIBC_INLINE int wunlink(const wchar_t *wpath)
{
	return wlibc_wunlink(wpath);
}

WLIBC_API int wlibc_unlinkat(int dirfd, const char *path, int flags);
WLIBC_API int wlibc_wunlinkat(int dirfd, const wchar_t *wpath, int flags);

WLIBC_INLINE int unlinkat(int dirfd, const char *path, int flags)
{
	return wlibc_unlinkat(dirfd, path, flags);
}

WLIBC_INLINE int wunlinkat(int dirfd, const wchar_t *wpath, int flags)
{
	return wlibc_wunlinkat(dirfd, wpath, flags);
}

WLIBC_API ssize_t wlibc_write(int fd, const void *buf, size_t count);
WLIBC_INLINE ssize_t write(int fd, const void *buf, size_t count)
{
	return wlibc_write(fd, buf, count);
}

_WLIBC_END_DECLS

#endif
