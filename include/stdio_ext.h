/*
   Copyright (c) 2020-2023 Sibi Siddharthan

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

// Buffer queries
WLIBC_API int wlibc_fbufmode(FILE *stream);
WLIBC_API size_t wlibc_fbufsize(FILE *stream);
WLIBC_API const char *wlibc_freadptr(FILE *stream, size_t *bufsize);

WLIBC_INLINE size_t __fbufsize(FILE *stream)
{
	return wlibc_fbufsize(stream);
}

WLIBC_INLINE size_t __fbufmode(FILE *stream)
{
	return wlibc_fbufmode(stream);
}

WLIBC_INLINE int __ffbf(FILE *stream)
{
	return wlibc_fbufmode(stream) == _IOFBF;
}

WLIBC_INLINE int __flbf(FILE *stream)
{
	return wlibc_fbufmode(stream) == _IOLBF;
}

WLIBC_INLINE int __fnbf(FILE *stream)
{
	return wlibc_fbufmode(stream) == _IONBF;
}

WLIBC_INLINE const char *__freadptr(FILE *stream, size_t *bufsize)
{
	return wlibc_freadptr(stream, bufsize);
}

// Mode query
WLIBC_API int wlibc_freading(FILE *stream);
WLIBC_API int wlibc_fwriting(FILE *stream);
WLIBC_API int wlibc_freadable(FILE *stream);
WLIBC_API int wlibc_fwritable(FILE *stream);
WLIBC_API size_t wlibc_freadahead(FILE *stream);
WLIBC_API size_t wlibc_fpending(FILE *stream);

WLIBC_INLINE int __freading(FILE *stream)
{
	return wlibc_freading(stream);
}

WLIBC_INLINE int __fwriting(FILE *stream)
{
	return wlibc_fwriting(stream);
}

WLIBC_INLINE int __freadable(FILE *stream)
{
	return wlibc_freadable(stream);
}

WLIBC_INLINE int __fwritable(FILE *stream)
{
	return wlibc_fwritable(stream);
}

WLIBC_INLINE size_t __freadahead(FILE *stream)
{
	return wlibc_freadahead(stream);
}

WLIBC_INLINE size_t __fpending(FILE *stream)
{
	return wlibc_fpending(stream);
}

// Buffer operations
WLIBC_API void wlibc_fpurge(FILE *stream);
WLIBC_API void wlibc_freadptrinc(FILE *stream, size_t size);

WLIBC_INLINE void __fpurge(FILE *stream)
{
	wlibc_fpurge(stream);
}

WLIBC_INLINE void __freadptrinc(FILE *stream, size_t size)
{
	wlibc_freadptrinc(stream, size);
}

// Stream control
WLIBC_API void wlibc_fseterr(FILE *stream);
WLIBC_API int wlibc_fsetlocking(FILE *stream, int type /*unused*/);

WLIBC_INLINE void __fseterr(FILE *stream)
{
	wlibc_fseterr(stream);
}

WLIBC_INLINE int __fsetlocking(FILE *stream, int type /*unused*/)
{
	return wlibc_fsetlocking(stream, type);
}

_WLIBC_END_DECLS

#endif
