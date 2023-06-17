/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_FCNTL_H
#define WLIBC_FCNTL_H

#include <wlibc.h>
#include <sys/types.h>
#include <stdarg.h>

_WLIBC_BEGIN_DECLS

// One of these should be specified
#define O_RDONLY  0x0000 // open for reading only
#define O_WRONLY  0x0001 // open for writing only
#define O_RDWR    0x0002 // open for reading and writing
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)

// Flags
#define O_APPEND 0x0008 // writes done at eof
#define O_TRUNC  0x0200 // open and truncate

// Creation flags
#define O_CREAT      0x0100      // create and open file
#define O_EXCL       0x0400      // open only if file doesn't already exist
#define O_DIRECT     0x400000    // Disable caching
#define O_NOFOLLOW   0x2000000   // Don't open symlinks
#define O_DIRECTORY  0x2000      // Fail if not a directory
#define O_NOTDIR     0x20000     // Fail if a directory (not posix, but simplifies fopen)
#define O_OBTAIN_DIR O_DIRECTORY
#define O_SEARCH     O_DIRECTORY // Open directory for searching

// Attributes
#define O_ASYNC       0x100000
#define O_DSYNC       0x0800     // Currently Unsupported
#define O_SYNC        0x800000   // Flush the file buffer immediately
#define O_NOATIME     0x200000   // Don't update last access time for read operations
#define O_NONBLOCK    0x4000000  // Nonblocking IO
#define O_NDELAY      O_NONBLOCK
#define O_NOINHERIT   0x0080     // child processes doesn't inherit file
#define O_CLOEXEC     O_NOINHERIT
#define O_PATH        0x80000    // Dont open the file for read or write
#define O_SEQUENTIAL  0x0020     // file access is primarily sequential
#define O_RANDOM      0x0010     // file access is primarily random
#define O_TMPFILE     0x0040     // temporary file
#define O_TEMPORARY   O_TMPFILE
#define O_SHORT_LIVED 0x1000     // like temporary file but name of file is given(to implement msvc T,D)
#define O_READONLY    0x10000    // file is read-only
#define O_IMMUTABLE   O_READONLY
#define O_ARCHIVE     0x0004     // file can be backed up
#define O_HIDDEN      0x10000000 // file is hidden
#define O_ENCRYPTED   0x20000000 // file is encrypted by the file system
#define O_SYSTEM      0x40000000 // file is a system file

// Unsupported
#define O_LARGEFILE 0x0 // We always use 64 bit file offsets
#define O_NOCTTY    0x0 // Unsupported

// Compatibility with MSVC
#define O_TEXT   0x4000 // file mode is text (translated)
#define O_BINARY 0x8000 // file mode is binary (untranslated)
#define O_RAW    O_BINARY

// For *at functions
#define AT_FDCWD            0x1000000
#define AT_NO_AUTOMOUNT     0x00 // Unsupported
#define AT_EACCESS          0x00 // Unsupported
#define AT_SYMLINK_FOLLOW   0x01 // Dereference symlinks
#define AT_SYMLINK_NOFOLLOW 0x02 // Don't dereference symlinks
#define AT_REMOVEDIR        0x04 // Remove a directory
#define AT_REMOVEANY        0x08 // Remove both directory as well as a file
#define AT_EMPTY_PATH       0x10 // Operate on the file descriptor given

// fcntl operations
#define FD_CLOEXEC      O_CLOEXEC
#define F_DUPFD         1 // duplicate the file descriptor
#define F_DUPFD_CLOEXEC 2 // same as above, but add O_CLOEXEC flag also.
#define F_GETFD         3 // return O_CLOEXEC if set
#define F_SETFD         4 // set O_CLOEXEC
#define F_GETFL         5 // return the flags
#define F_SETFL         6 // set the flags, only O_APPEND, O_DIRECT, O_NONBLOCK are supported

WLIBC_API int wlibc_common_open(int dirfd, const char *name, int oflags, va_list perm_args);

WLIBC_INLINE int open(const char *name, const int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_common_open(AT_FDCWD, name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

WLIBC_INLINE int openat(int dirfd, const char *name, const int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_common_open(dirfd, name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

WLIBC_INLINE int creat(const char *name, mode_t mode)
{
	return open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

WLIBC_API int wlibc_fcntl(int fd, int cmd, va_list args);

WLIBC_INLINE int fcntl(int fd, int cmd, ...)
{
	va_list args;
	va_start(args, cmd);
	int return_val = wlibc_fcntl(fd, cmd, args);
	va_end(args);
	return return_val;
}

_WLIBC_END_DECLS

#endif
