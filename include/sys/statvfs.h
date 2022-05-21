/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_STATVFS_H
#define WLIBC_SYS_STATVFS_H

#include <wlibc.h>
#include <sys/fstypes.h>
#include <sys/types.h>

#define VFSTYPENAMELEN 16

struct statvfs
{
	unsigned long f_type;              // Type of filesystem
	unsigned long f_bsize;             // Filesystem block size
	unsigned long f_frsize;            // Fragment size
	fsblkcnt_t f_blocks;               // Total number of blocks
	fsblkcnt_t f_bfree;                // Number of free blocks
	fsblkcnt_t f_bavail;               // Number of free blocks for unprivileged users
	fsfilcnt_t f_files;                // Number of inodes
	fsfilcnt_t f_ffree;                // Number of free inodes
	fsfilcnt_t f_favail;               // Number of free inodes for unprivileged users
	unsigned long f_fsid;              // Filesystem ID
	unsigned long f_flag;              // Mount flags
	unsigned long f_namemax;           // Maximum path length
	char f_fstypename[VFSTYPENAMELEN]; // Filesystem name
};

// NOTE: MNT_* values and ST_* values should match.
#define ST_RDONLY      0x0001 // Mount read-only.
#define ST_NOSUID      0x0002 // Ignore suid and sgid bits.
#define ST_NODEV       0x0004 // Disallow access to device special files.
#define ST_NOEXEC      0x0008 // Disallow program execution.
#define ST_SYNCHRONOUS 0x0010 // Writes are synced at once.
#define ST_NOATIME     0x0040 // Do not update access times.
#define ST_NODIRATIME  0x0080 // Do not update directory access times.
#define ST_RELATIME    0x0100 // Update atime relative to mtime/ctime.
#define ST_MANDLOCK    0x4000 // Allow mandatory locks on an FS.

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
