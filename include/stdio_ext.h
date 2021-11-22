/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_EXT_H
#define WLIBC_STDIO_EXT_H

#include <wlibc-macros.h>
#include <stdio.h>

_WLIBC_BEGIN_DECLS

typedef struct WLIBC_FILE FILE;

#define FSETLOCKING_QUERY    0
#define FSETLOCKING_INTERNAL 1
#define FSETLOCKING_BYCALLER 2

typedef enum _WLIBC_FILE_STREAM_OPERATIONS
{
	bufsize = 0,
	reading,
	writing,
	readable,
	writeable,
	buffered,
	pending,
	purge,
	locking,
	maxop
} WLIBC_FILE_STREAM_OPERATIONS;

WLIBC_API size_t wlibc_fileops(FILE *stream, WLIBC_FILE_STREAM_OPERATIONS operation);

WLIBC_INLINE size_t __fbufsize(FILE *stream)
{
	return wlibc_fileops(stream, bufsize);
}

WLIBC_INLINE int __freading(FILE *stream)
{
	return wlibc_fileops(stream, reading);
}

WLIBC_INLINE int __fwriting(FILE *stream)
{
	return wlibc_fileops(stream, writing);
}

WLIBC_INLINE int __freadable(FILE *stream)
{
	return wlibc_fileops(stream, readable);
}

WLIBC_INLINE int __fwritable(FILE *stream)
{
	return wlibc_fileops(stream, writeable);
}

WLIBC_INLINE int __flbf(FILE *stream)
{
	return wlibc_fileops(stream, buffered);
}

WLIBC_INLINE size_t __fpending(FILE *stream)
{
	return wlibc_fileops(stream, pending);
}

WLIBC_INLINE void __fpurge(FILE *stream)
{
	wlibc_fileops(stream, purge);
}

WLIBC_INLINE int __fsetlocking(FILE *stream, int type /*unused*/)
{
	return wlibc_fileops(stream, locking);
}

_WLIBC_END_DECLS

#endif
