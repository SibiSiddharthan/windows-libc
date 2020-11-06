/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#ifndef WLIBC_STDIO_HOOKS_H
#define WLIBC_STDIO_HOOKS_H

#include <stdio.h>

#define fopen wlibc_fopen
#define fdopen wlibc_fdopen
#define freopen wlibc_freopen
#define fileno wlibc_fileno
#define fclose wlibc_fclose

#include <wlibc-macros.h>

_WLIBC_BEGIN_DECLS

WLIBC_API FILE *wlibc_fopen(const char *filename, const char *mode);
WLIBC_API FILE *wlibc_fdopen(int fd, const char *mode);
WLIBC_API FILE *wlibc_freopen(const char *filename, const char *mode, FILE *stream);
WLIBC_API int wlibc_fileno(FILE* stream);
WLIBC_API int wlibc_fclose(FILE* stream);

_WLIBC_END_DECLS

#endif