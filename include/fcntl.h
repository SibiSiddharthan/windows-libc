/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_FCNTL_H
#define WLIBC_FCNTL_H

#include <wlibc-macros.h>
#include <sys/types.h>
#include <wchar.h>
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
#define O_CREAT      0x0100    // create and open file
#define O_EXCL       0x0400    // open only if file doesn't already exist
#define O_DIRECT     0x400000  // Disable caching
#define O_NOFOLLOW   0x2000000 // Don't open symlinks
#define O_DIRECTORY  0x2000    // Fail if not a directory
#define O_NOTDIR     0x20000   // Fail if a directory (not posix, but simplifies fopen)
#define O_OBTAIN_DIR O_DIRECTORY
#define O_SEARCH     O_DIRECTORY // Open directory for searching

// Attributes
#define O_ASYNC       0x100000
#define O_DSYNC       0x0800    // Currently Unsupported
#define O_SYNC        0x800000  // Flush the file buffer immediately
#define O_NOATIME     0x200000  // Don't update last access time for read operations
#define O_NONBLOCK    0x4000000 // Nonblocking IO
#define O_NDELAY      O_NONBLOCK
#define O_NOINHERIT   0x0080 // child processes doesn't inherit file
#define O_CLOEXEC     O_NOINHERIT
#define O_PATH        0x80000 // Dont open the file for read or write
#define O_SEQUENTIAL  0x0020  // file access is primarily sequential
#define O_RANDOM      0x0010  // file access is primarily random
#define O_TMPFILE     0x0040  // temporary file
#define O_SHORT_LIVED O_TEMPORARY
#define O_TEMPORARY   O_TMPFILE

// Unsupported
#define O_LARGEFILE 0x0 // We always use 64 bit file offsets
#define O_NOCTTY    0x0 // Unsupported

// Compatibility with MSVC
#define O_TEXT    0x4000  // file mode is text (translated)
#define O_BINARY  0x8000  // file mode is binary (untranslated)
#define O_WTEXT   0x10000 // file mode is UTF16 (translated)
#define O_U16TEXT 0x20000 // file mode is UTF16 no BOM (translated)
#define O_U8TEXT  0x40000 // file mode is UTF8  no BOM (translated)
#define O_RAW     O_BINARY

// For *at functions
#define AT_FDCWD            0x1000000
#define AT_EMPTY_PATH       0x0 // Unsupported
#define AT_NO_AUTOMOUNT     0x0 // Unsupported
#define AT_EACCESS          0x0 // Unsupported
#define AT_SYMLINK_FOLLOW   0x1 // Dereference symlinks
#define AT_SYMLINK_NOFOLLOW 0x2 // Don't dereference symlinks
#define AT_REMOVEDIR        0x4 // Remove a directory

// fcntl operations
#define FD_CLOEXEC      O_CLOEXEC
#define F_DUPFD         1 // duplicate the file descriptor
#define F_DUPFD_CLOEXEC 2 // same as above, but add O_CLOEXEC flag also.
#define F_GETFD         3 // return O_CLOEXEC if set
#define F_SETFD         4 // set O_CLOEXEC
#define F_GETFL         5 // return the flags
#define F_SETFL         6 // set the flags, only O_APPEND, O_DIRECT, O_NONBLOCK are supported

#if 0
WLIBC_API int wlibc_creat(const char *name, const mode_t perm);
WLIBC_API int wlibc_wcreat(const wchar_t *wname, const mode_t perm);

WLIBC_INLINE int creat(const char *name, const mode_t perm)
{
	return wlibc_creat(name, perm);
}

WLIBC_INLINE int wcreat(const wchar_t *wname, const mode_t perm)
{
	return wlibc_wcreat(wname, perm);
}

WLIBC_API int wlibc_open(const char *name, const int oflags, va_list perm_args);
WLIBC_API int wlibc_wopen(const wchar_t *wname, const int oflags, va_list perm_args);

WLIBC_INLINE int open(const char *name, const int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_open(name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

WLIBC_INLINE int wopen(const wchar_t *wname, const int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_wopen(wname, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

WLIBC_API int wlibc_openat(int dirfd, const char *name, int oflags, va_list perm_args);
WLIBC_API int wlibc_wopenat(int dirfd, const wchar_t *wname, int oflags, va_list perm_args);

WLIBC_INLINE int openat(int dirfd, const char *name, int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_openat(dirfd, name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

WLIBC_INLINE int wopenat(int dirfd, const wchar_t *wname, int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_wopenat(dirfd, wname, oflags, perm_args);
	va_end(perm_args);
	return fd;
}
#endif

WLIBC_API int wlibc_open2(const char *name, const int oflags, ...);
WLIBC_API int wlibc_openat2(int dirfd, const char *name, int oflags, ...);

#define creat(name, perm) wlibc_open2(name, O_WRONLY | O_CREAT | O_TRUNC, perm)
/* int open(const char *name, const int oflags, ...); */
#define open              wlibc_open2
#define openat            wlibc_openat2

WLIBC_API int wlibc_open(const char *name, const int oflags, va_list perm_args);
WLIBC_API int wlibc_wopen(const wchar_t *wname, const int oflags, va_list perm_args);
WLIBC_INLINE int wopen(const wchar_t *wname, const int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_wopen(wname, oflags, perm_args);
	va_end(perm_args);
	return fd;
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
