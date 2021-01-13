/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_HOOKS_H
#define WLIBC_STDIO_HOOKS_H

#include <stdio.h>

#define fopen   wlibc_fopen
#define fdopen  wlibc_fdopen
#define freopen wlibc_freopen
#define fileno  wlibc_fileno
#define fclose  wlibc_fclose
#define popen   wlibc_popen
#define pclose  wlibc_pclose

#include <wlibc-macros.h>

_WLIBC_BEGIN_DECLS

// File hooks
WLIBC_API FILE *wlibc_fopen(const char *filename, const char *mode);
WLIBC_API FILE *wlibc_fdopen(int fd, const char *mode);
WLIBC_API FILE *wlibc_freopen(const char *filename, const char *mode, FILE *stream);
WLIBC_API int wlibc_fileno(FILE *stream);
WLIBC_API int wlibc_fclose(FILE *stream);

// Pipe hooks
WLIBC_API FILE *wlibc_popen(const char *command, const char *mode);
WLIBC_API int wlibc_pclose(FILE *stream);

_WLIBC_END_DECLS

#endif
