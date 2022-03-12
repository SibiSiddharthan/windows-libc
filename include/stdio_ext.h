/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_EXT_H
#define WLIBC_STDIO_EXT_H

#include <wlibc.h>
#include <stdio.h>

_WLIBC_BEGIN_DECLS

typedef struct _WLIBC_FILE FILE;

#define FSETLOCKING_QUERY    0
#define FSETLOCKING_INTERNAL 1
#define FSETLOCKING_BYCALLER 2

typedef enum _WLIBC_FILE_STREAM_OPERATIONS
{
	bufsize = 0,
	bufmode,
	reading,
	writing,
	readable,
	writeable,
	pending_read,
	pending_write,
	purge,
	increment,
	locking,
	buffer,
	seterr,
	maxop
} WLIBC_FILE_STREAM_OPERATIONS;

WLIBC_API size_t wlibc_fileops(FILE *stream, WLIBC_FILE_STREAM_OPERATIONS operation, void *arg);

WLIBC_INLINE size_t __fbufsize(FILE *stream)
{
	return wlibc_fileops(stream, bufsize, NULL);
}

WLIBC_INLINE size_t __fbufmode(FILE *stream)
{
	return wlibc_fileops(stream, bufmode, NULL);
}

WLIBC_INLINE int __freading(FILE *stream)
{
	return (int)wlibc_fileops(stream, reading, NULL);
}

WLIBC_INLINE int __fwriting(FILE *stream)
{
	return (int)wlibc_fileops(stream, writing, NULL);
}

WLIBC_INLINE int __freadable(FILE *stream)
{
	return (int)wlibc_fileops(stream, readable, NULL);
}

WLIBC_INLINE int __fwritable(FILE *stream)
{
	return (int)wlibc_fileops(stream, writeable, NULL);
}

WLIBC_INLINE int __ffbf(FILE *stream)
{
	if (wlibc_fileops(stream, bufmode, NULL) == _IOFBF)
	{
		return 1;
	}
	return 0;
}

WLIBC_INLINE int __flbf(FILE *stream)
{
	if (wlibc_fileops(stream, bufmode, NULL) == _IOLBF)
	{
		return 1;
	}
	return 0;
}

WLIBC_INLINE int __fnbf(FILE *stream)
{
	if (wlibc_fileops(stream, bufmode, NULL) == _IONBF)
	{
		return 1;
	}
	return 0;
}

WLIBC_INLINE size_t __freadahead(FILE *stream)
{
	return wlibc_fileops(stream, pending_read, NULL);
}

WLIBC_INLINE size_t __fpending(FILE *stream)
{
	return wlibc_fileops(stream, pending_write, NULL);
}

WLIBC_INLINE void __fpurge(FILE *stream)
{
	wlibc_fileops(stream, purge, NULL);
}

WLIBC_INLINE void __freadptrinc(FILE *stream, size_t size)
{
	wlibc_fileops(stream, increment, &size);
}

WLIBC_INLINE int __fsetlocking(FILE *stream, int type /*unused*/)
{
	return (int)wlibc_fileops(stream, locking, NULL);
}

WLIBC_INLINE const char *__freadptr(FILE *stream, size_t *bufsize)
{
	// Cast the size_t into a pointer.
	// CHECK 32bit
	return (const char *)wlibc_fileops(stream, buffer, bufsize);
}

WLIBC_INLINE void __fseterr(FILE *stream)
{
	wlibc_fileops(stream, seterr, NULL);
}

_WLIBC_END_DECLS

#endif
