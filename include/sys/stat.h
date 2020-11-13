/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_STAT_H
#define WLIBC_SYS_STAT_H

#include <wlibc-macros.h>
#include <wchar.h>
#include <sys/types.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

#define S_IFMT  0xF000 // File type mask
#define S_IFBLK 0x0100 // Block special
#define S_IFLNK 0x0200 // Symbolic Link
#define S_IFIFO 0x1000 // Pipe
#define S_IFCHR 0x2000 // Character special
#define S_IFDIR 0x4000 // Directory
#define S_IFREG 0x8000 // Regular

#define S_IREAD  0x0100 // Read permission, owner
#define S_IWRITE 0x0080 // Write permission, owner
#define S_IEXEC  0x0040 // Execute/search permission, owner

#define S_IRUSR   S_IREAD
#define S_IRGRP   S_IREAD
#define S_IROTH   S_IREAD
#define S_IWUSR   S_IWRITE
#define S_IWGRP   S_IWRITE
#define S_IWOTH   S_IWRITE
#define S_IXUSR   S_IEXEC
#define S_IXGRP   S_IEXEC
#define S_IXOTH   S_IEXEC
#define S_IRWXU   (S_IREAD | S_IWRITE | S_IEXEC)
#define S_IRWXG   S_IRWXU
#define S_IRWXO   S_IRWXU
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)

struct stat
{
	dev_t st_dev;            // ID of device containing file
	ino_t st_ino;            // file serial number
	mode_t st_mode;          // mode of file
	nlink_t st_nlink;        // number of links to the file
	uid_t st_uid;            // user ID of file
	gid_t st_gid;            // group ID of file
	dev_t st_rdev;           // device ID (if file is character or block special)
	off_t st_size;           // file size in bytes (if file is a regular file)
	struct timespec st_atim; // time of last access
	struct timespec st_mtim; // time of last data modification
	struct timespec st_ctim; // time of last status change
	blksize_t st_blksize;    // block size of filesystem
	blkcnt_t st_blocks;      // number of 512B blocks(sectors) allocated

#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};

#define S_ISBLK(m)  (m & S_IFBLK)
#define S_ISCHR(m)  (m & S_IFCHR)
#define S_ISDIR(m)  (m & S_IFDIR)
#define S_ISREG(m)  (m & S_IFREG)
#define S_ISLNK(m)  (m & S_IFLNK)
#define S_ISFIFO(m) (m & S_IFIFO)

#define UTIME_NOW  -1 // Change timestamp to current timestamp
#define UTIME_OMIT -2 // Ignore timestamp

WLIBC_API int wlibc_chmod(const char *name, mode_t mode);
WLIBC_API int wlibc_wchmod(const wchar_t *wname, mode_t mode);

WLIBC_INLINE int chmod(const char *name, mode_t mode)
{
	return wlibc_chmod(name, mode);
}

WLIBC_INLINE int wchmod(const wchar_t *wname, mode_t mode)
{
	return wlibc_wchmod(wname, mode);
}

WLIBC_API int wlibc_fchmod(int fd, mode_t mode);

WLIBC_INLINE int fchmod(int fd, mode_t mode)
{
	return wlibc_fchmod(fd, mode);
}

WLIBC_API int wlibc_fchmodat(int dirfd, const char *name, mode_t mode, int flags /*unused*/);
WLIBC_API int wlibc_wfchmodat(int dirfd, const wchar_t *name, mode_t mode, int flags /*unused*/);

WLIBC_INLINE int fchmodat(int dirfd, const char *name, mode_t mode, int flags)
{
	return wlibc_fchmodat(dirfd, name, mode, flags);
}

WLIBC_INLINE int wfchmodat(int dirfd, const wchar_t *wname, mode_t mode, int flags)
{
	return wlibc_wfchmodat(dirfd, wname, mode, flags);
}

WLIBC_API int wlibc_stat(const char *name, struct stat *statbuf);
WLIBC_API int wlibc_wstat(const wchar_t *wname, struct stat *statbuf);

WLIBC_INLINE int stat(const char *name, struct stat *statbuf)
{
	return wlibc_stat(name, statbuf);
}

WLIBC_INLINE int wstat(const wchar_t *wname, struct stat *statbuf)
{
	return wlibc_wstat(wname, statbuf);
}

WLIBC_API int wlibc_lstat(const char *name, struct stat *statbuf);
WLIBC_API int wlibc_wlstat(const wchar_t *wname, struct stat *statbuf);

WLIBC_INLINE int lstat(const char *name, struct stat *statbuf)
{
	return wlibc_lstat(name, statbuf);
}

WLIBC_INLINE int wlstat(const wchar_t *wname, struct stat *statbuf)
{
	return wlibc_wlstat(wname, statbuf);
}

WLIBC_API int wlibc_fstat(int fd, struct stat *statbuf);

WLIBC_INLINE int fstat(int fd, struct stat *statbuf)
{
	return wlibc_fstat(fd, statbuf);
}

WLIBC_API int wlibc_fstatat(int dirfd, const char *name, struct stat *statbuf, int flags);
WLIBC_API int wlibc_wfstatat(int dirfd, const wchar_t *wname, struct stat *statbuf, int flags);

WLIBC_INLINE int fstatat(int dirfd, const char *name, struct stat *statbuf, int flags)
{
	return wlibc_fstatat(dirfd, name, statbuf, flags);
}

WLIBC_INLINE int wfstatat(int dirfd, const wchar_t *wname, struct stat *statbuf, int flags)
{
	return wlibc_wfstatat(dirfd, wname, statbuf, flags);
}

WLIBC_API int wlibc_mkdir(const char *path, mode_t mode);
WLIBC_API int wlibc_wmkdir(const wchar_t *wpath, mode_t mode);

WLIBC_INLINE int mkdir(const char *path, mode_t mode)
{
	return wlibc_mkdir(path, mode);
}

WLIBC_INLINE int wmkdir(const wchar_t *wpath, mode_t mode)
{
	return wlibc_wmkdir(wpath, mode);
}

WLIBC_API int wlibc_mkdirat(int dirfd, const char *path, mode_t mode);
WLIBC_API int wlibc_wmkdirat(int dirfd, const wchar_t *wpath, mode_t mode);

WLIBC_INLINE int mkdirat(int dirfd, const char *path, mode_t mode)
{
	return wlibc_mkdirat(dirfd, path, mode);
}

WLIBC_INLINE int wmkdirat(int dirfd, const wchar_t *wpath, mode_t mode)
{
	return wlibc_wmkdirat(dirfd, wpath, mode);
}

WLIBC_API int wlibc_utimensat(int dirfd, const char *name, const struct timespec times[2], int flags);
WLIBC_API int wlibc_wutimensat(int dirfd, const wchar_t *wname, const struct timespec times[2], int flags);

WLIBC_INLINE int utimensat(int dirfd, const char *name, const struct timespec times[2], int flags)
{
	return wlibc_utimensat(dirfd, name, times, flags);
}

WLIBC_INLINE int wutimensat(int dirfd, const wchar_t *wname, const struct timespec times[2], int flags)
{
	return wlibc_wutimensat(dirfd, wname, times, flags);
}

WLIBC_API int wlibc_futimens(int fd, const struct timespec times[2]);

WLIBC_INLINE int futimens(int fd, const struct timespec times[2])
{
	return wlibc_futimens(fd, times);
}

_WLIBC_END_DECLS

#endif