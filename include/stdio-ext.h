/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_EXT_H
#define WLIBC_STDIO_EXT_H

#include <wlibc-macros.h>
#include <wchar.h>

// Avoid C2375: 'rename': redefinition different linkage
#include <stdio.h>
#define rename wlibc_rename


_WLIBC_BEGIN_DECLS

// For renameat2
// Using same values from linux/fs.h
#define RENAME_NOREPLACE 0x1 // Don't overwrite
#define RENAME_EXCHANGE  0x2 // Exchange the files
#define RENAME_WHITEOUT  0x4 // Unsupported

WLIBC_API int wlibc_rename(const char *oldname, const char *newname);
WLIBC_API int wlibc_wrename(const wchar_t *woldname, const wchar_t *wnewname);

WLIBC_INLINE int wrename(const wchar_t *woldname, const wchar_t *wnewname)
{
	return wlibc_wrename(woldname, wnewname);
}

WLIBC_API int wlibc_renameat(int olddirfd, const char *oldname, int newdirfd, const char *newname);
WLIBC_API int wlibc_wrenameat(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname);

WLIBC_INLINE int renameat(int olddirfd, const char *oldname, int newdirfd, const char *newname)
{
	return wlibc_renameat(olddirfd, oldname, newdirfd, newname);
}

WLIBC_INLINE int wrenameat(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname)
{
	return wlibc_wrenameat(olddirfd, woldname, newdirfd, wnewname);
}

WLIBC_API int wlibc_renameat2(int olddirfd, const char *oldname, int newdirfd, const char *newname, unsigned int flags);
WLIBC_API int wlibc_wrenameat2(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname, unsigned int flags);

WLIBC_INLINE int renameat2(int olddirfd, const char *oldname, int newdirfd, const char *newname, unsigned int flags)
{
	return wlibc_renameat2(olddirfd, oldname, newdirfd, newname, flags);
}

WLIBC_INLINE int wrenameat2(int olddirfd, const wchar_t *woldname, int newdirfd, const wchar_t *wnewname, unsigned int flags)
{
	return wlibc_wrenameat2(olddirfd, woldname, newdirfd, wnewname, flags);
}

_WLIBC_END_DECLS

#endif
