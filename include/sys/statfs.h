/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_STATFS_H
#define WLIBC_SYS_STATFS_H

#include <wlibc.h>
#include <sys/fstypes.h>
#include <sys/types.h>

#define MFSTYPENAMELEN 16
#define MNAMELEN       256

typedef struct _fsid_t
{
	unsigned int major;
	unsigned int minor;
} fsid_t;

struct statfs
{
	unsigned long f_type;              // Type of filesystem
	unsigned long f_fssubtype;         // Filesystem flavor
	unsigned long f_bsize;             // Filesystem block size
	unsigned long f_iosize;            // Optimal transfer size
	fsblkcnt_t f_blocks;               // Total number of blocks
	fsblkcnt_t f_bfree;                // Number of free blocks
	fsblkcnt_t f_bavail;               // Number of free blocks for unprivileged users
	fsfilcnt_t f_files;                // Number of inodes
	fsfilcnt_t f_ffree;                // Number of free inodes
	fsfilcnt_t f_favail;               // Number of free inodes for unprivileged users
	uid_t f_owner;                     // User that mounted the filesystem
	fsid_t f_fsid;                     // Filesystem ID
	unsigned long f_flag;              // Mount flags
	unsigned long f_namemax;           // Maximum path length
	char f_fstypename[MFSTYPENAMELEN]; // Filesystem name
	char f_mntfromname[MNAMELEN];      // Mounted filesystem
	char f_mntonname[MNAMELEN];        // Directory on which mounted
};

// NOTE: MNT_* values and ST_* values should match.
#define MNT_RDONLY      0x00000001 // Mount read-only.
#define MNT_NOSUID      0x00000002 // Ignore suid and sgid bits.
#define MNT_NODEV       0x00000004 // Disallow access to device special files.
#define MNT_NOEXEC      0x00000008 // Disallow program execution.
#define MNT_SYNCHRONOUS 0x00000010 // Writes are synced at once.
#define MNT_DEFWRITE    0x00000020 // Writes are deffered.
#define MNT_NOATIME     0x00000040 // Do not update access times.
#define MNT_STRICTTIME  0x00000200 // Update access times.
#define MNT_UNION       0x00000400 // Union with underlying filesystem.
#define MNT_ASYNC       0x00000800 // Writes are asynchronous.
#define MNT_QUOTA       0x00001000 // Filesystem supports quotas.
#define MNT_JOURNALED   0x00002000 // Filesystem supports quotas.
#define MNT_REMOVABLE   0x00010000 // Filesystem is removable.

WLIBC_API int wlibc_common_statfs(int fd, const char *restrict path, struct statfs *restrict statfsbuf);

WLIBC_INLINE int statfs(const char *restrict path, struct statfs *restrict statfsbuf)
{
	return wlibc_common_statfs(-1, path, statfsbuf);
}

WLIBC_INLINE int fstatfs(int fd, struct statfs *statfsbuf)
{
	return wlibc_common_statfs(fd, NULL, statfsbuf);
}

#endif
