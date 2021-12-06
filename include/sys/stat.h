/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_STAT_H
#define WLIBC_SYS_STAT_H

#include <wlibc.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

_WLIBC_BEGIN_DECLS

#define S_IFMT   0xF000 // File type mask
#define S_IFIFO  0x1000 // Pipe or FIFO (FIFO is unsupported)
#define S_IFCHR  0x2000 // Character special
#define S_IFDIR  0x4000 // Directory
#define S_IFBLK  0x6000 // Block special
#define S_IFREG  0x8000 // Regular
#define S_IFLNK  0xA000 // Symbolic Link
#define S_IFSOCK 0xC000 // Socket

// User permissions
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100

// Group permissions
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010

// Other permissions
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001

// Combination of permissions
#define S_IRUGO   (S_IRUSR | S_IRGRP | S_IROTH) // Read everyone
#define S_IWUGO   (S_IWUSR | S_IWGRP | S_IWOTH) // Write everyone
#define S_IXUGO   (S_IXUSR | S_IXGRP | S_IXOTH) // Execute everyone
#define S_IRWXU   (S_IRUSR | S_IWUSR | S_IXUSR) // All permissions user
#define S_IRWXG   (S_IRGRP | S_IWGRP | S_IXGRP) // All permissions group
#define S_IRWXO   (S_IROTH | S_IWOTH | S_IXOTH) // All permissions others
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO) // All permissions everyone

// Owner permissions, same as user
#define S_IREAD  S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC  S_IXUSR

#define S_ISUID 0x800 // Set user ID on execution
#define S_ISGID 0x400 // Set group ID on execution
#define S_ISVTX 0x200 // Sticky bit (Obsolete)

struct stat
{
	dev_t st_dev;                // ID of device containing file
	ino_t st_ino;                // file serial number
	mode_t st_mode;              // mode of file
	nlink_t st_nlink;            // number of links to the file
	uid_t st_uid;                // user ID of file
	gid_t st_gid;                // group ID of file
	dev_t st_rdev;               // device ID (if file is character or block special)
	off_t st_size;               // file size in bytes (if file is a regular file)
	struct timespec st_atim;     // time of last access
	struct timespec st_mtim;     // time of last data modification
	struct timespec st_ctim;     // time of last status change
	struct timespec st_birthtim; // time of birth
	blksize_t st_blksize;        // block size of filesystem
	blkcnt_t st_blocks;          // number of 512B blocks(sectors) allocated

#define st_atimespec     st_atim
#define st_atime         st_atim.tv_sec
#define st_atimensec     st_atim.tv_nsec
#define st_mtimespec     st_mtim
#define st_mtime         st_mtim.tv_sec
#define st_mtimensec     st_mtim.tv_nsec
#define st_ctimespec     st_ctim
#define st_ctime         st_ctim.tv_sec
#define st_ctimensec     st_ctim.tv_nsec
#define st_birthtimespec st_birthtim
#define st_birthtime     st_birthtim.tv_sec
#define st_birthtimensec st_birthtim.tv_nsec
};

#define S_ISTYPE(mode, type) (((mode)&S_IFMT) == (type))

#define S_ISFIFO(mode) S_ISTYPE((mode), S_IFIFO)
#define S_ISCHR(mode)  S_ISTYPE((mode), S_IFCHR)
#define S_ISDIR(mode)  S_ISTYPE((mode), S_IFDIR)
#define S_ISBLK(mode)  S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)  S_ISTYPE((mode), S_IFREG)
#define S_ISLNK(mode)  S_ISTYPE((mode), S_IFLNK)
#define S_ISSOCK(mode) S_ISTYPE((mode), S_IFSOCK)

#define UTIME_NOW  -1 // Change timestamp to current timestamp
#define UTIME_OMIT -2 // Ignore timestamp

WLIBC_API int wlibc_common_chmod(int dirfd, const char *path, mode_t mode, int flags);

WLIBC_INLINE int chmod(const char *path, mode_t mode)
{
	return wlibc_common_chmod(AT_FDCWD, path, mode, 0);
}

WLIBC_INLINE int lchmod(const char *path, mode_t mode)
{
	return wlibc_common_chmod(AT_FDCWD, path, mode, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int fchmod(int fd, mode_t mode)
{
	return wlibc_common_chmod(fd, NULL, mode, AT_EMPTY_PATH);
}

WLIBC_INLINE int fchmodat(int dirfd, const char *path, mode_t mode, int flags)
{
	return wlibc_common_chmod(dirfd, path, mode, flags);
}

WLIBC_API int wlibc_common_stat(int dirfd, const char *restrict path, struct stat *restrict statbuf, int flags);

WLIBC_INLINE int stat(const char *restrict path, struct stat *restrict statbuf)
{
	return wlibc_common_stat(AT_FDCWD, path, statbuf, 0);
}

WLIBC_INLINE int lstat(const char *restrict path, struct stat *restrict statbuf)
{
	return wlibc_common_stat(AT_FDCWD, path, statbuf, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int fstat(int fd, struct stat *statbuf)
{
	return wlibc_common_stat(fd, NULL, statbuf, AT_EMPTY_PATH);
}

WLIBC_INLINE int fstatat(int dirfd, const char *restrict path, struct stat *restrict statbuf, int flags)
{
	return wlibc_common_stat(dirfd, path, statbuf, flags);
}

WLIBC_API int wlibc_common_mkdir(int dirfd, const char *path, mode_t mode);

WLIBC_INLINE int mkdir(const char *path, mode_t mode)
{
	return wlibc_common_mkdir(AT_FDCWD, path, mode);
}

WLIBC_INLINE int mkdirat(int dirfd, const char *path, mode_t mode)
{
	return wlibc_common_mkdir(dirfd, path, mode);
}

WLIBC_API int wlibc_common_utimens(int dirfd, const char *path, const struct timespec times[2], int flags);

WLIBC_INLINE int utimens(const char *path, const struct timespec times[2])
{
	return wlibc_common_utimens(AT_FDCWD, path, times, 0);
}

WLIBC_INLINE int lutimens(const char *path, const struct timespec times[2])
{
	return wlibc_common_utimens(AT_FDCWD, path, times, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int utimensat(int dirfd, const char *path, const struct timespec times[2], int flags)
{
	return wlibc_common_utimens(dirfd, path, times, flags);
}

WLIBC_INLINE int futimens(int fd, const struct timespec times[2])
{
	return wlibc_common_utimens(fd, NULL, times, AT_EMPTY_PATH);
}

WLIBC_INLINE int fdutimens(int fd, const char *path, const struct timespec times[2])
{
	if (fd >= 0 && fd != AT_FDCWD)
	{
		// ignore path
		return wlibc_common_utimens(fd, NULL, times, AT_EMPTY_PATH);
	}
	else
	{
		// ignore fd
		return wlibc_common_utimens(AT_FDCWD, path, times, 0);
	}
}

WLIBC_INLINE mode_t umask(mode_t mask /*unused*/)
{
	/*
	  According to the POSIX documentation, the umask value is ignored if the parent directory has an ACL.
	  In Windows all directories have ACLs and this function serves no meaningful purpose.
	  Just return 0077 i.e ~0700 (owner has full control, rest have nothing)
	*/
	return 0077;
}

_WLIBC_END_DECLS

#endif
