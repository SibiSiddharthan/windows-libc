/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_STATVFS_H
#define WLIBC_SYS_STATVFS_H

#include <wlibc.h>
#include <sys/types.h>

struct statvfs
{
	unsigned long f_bsize;   // Filesystem block size
	unsigned long f_frsize;  // Fragment size
	fsblkcnt_t f_blocks;     // Total number of blocks
	fsblkcnt_t f_bfree;      // Number of free blocks
	fsblkcnt_t f_bavail;     // Number of free blocks for unprivileged users
	fsfilcnt_t f_files;      // Number of inodes
	fsfilcnt_t f_ffree;      // Number of free inodes
	fsfilcnt_t f_favail;     // Number of free inodes for unprivileged users
	unsigned long f_fsid;    // Filesystem ID
	unsigned long f_flag;    // Mount flags
	unsigned long f_namemax; // Maximum path length
};

#define ST_RDONLY      0x001 // Mount read-only.
#define ST_NOSUID      0x002 // Ignore suid and sgid bits.
#define ST_NODEV       0x004 // Disallow access to device special files.
#define ST_NOEXEC      0x008 // Disallow program execution.
#define ST_SYNCHRONOUS 0x010 // Writes are synced at once.
#define ST_MANDLOCK    0x020 // Allow mandatory locks on an FS.
#define ST_NOATIME     0x040 // Do not update access times.
#define ST_NODIRATIME  0x080 // Do not update directory access times.
#define ST_RELATIME    0x100 // Update atime relative to mtime/ctime.

WLIBC_API int wlibc_common_statvfs(int fd, const char *restrict path, struct statvfs *restrict statvfsbuf);

WLIBC_INLINE int statvfs(const char *restrict path, struct statvfs *restrict statvfsbuf)
{
	return wlibc_common_statvfs(-1, path, statvfsbuf);
}

WLIBC_INLINE int fstatvfs(int fd, struct statvfs *statvfsbuf)
{
	return wlibc_common_statvfs(fd, NULL, statvfsbuf);
}

#endif
