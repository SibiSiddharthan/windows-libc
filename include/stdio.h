/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_H
#define WLIBC_STDIO_H

#include <wlibc-macros.h>
#include <stdarg.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

#define _FILE_DEFINED
/* Buffered I/O macros */

#define BUFSIZ    512
#define L_tmpnam  260
#define L_ctermid 260
#define L_cuserid
#define P_tmpdir "/tmp" // point this to tempdir

typedef struct WLIBC_FILE FILE;

extern FILE *_wlibc_stdin;
extern FILE *_wlibc_stdout;
extern FILE *_wlibc_stderr;

#define stdin  _wlibc_stdin
#define stdout _wlibc_stdout
#define stderr _wlibc_stderr

#ifndef NULL
#	ifdef __cplusplus
#		define NULL 0
#	else
#		define NULL ((void *)0)
#	endif
#endif

#define _NSTREAM_ 512

#define EOF (-1)

#define _IOFBF 0x0010 // Full buffering
#define _IOLBF 0x0020 // line buffering
#define _IONBF 0x0040 // no buffering


/* Seek method constants */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FILENAME_MAX 260 // 32768
#define FOPEN_MAX    20  // 8192
#define TMP_MAX      _CRT_INT_MAX
#define _SYS_OPEN    20

typedef long long fpos_t;

// file access
WLIBC_API FILE *wlibc_fopen(const char *name, const char *mode);

WLIBC_INLINE FILE *fopen(const char *name, const char *mode)
{
	return wlibc_fopen(name, mode);
}

WLIBC_API FILE *wlibc_fdopen(int fd, const char *mode);

WLIBC_INLINE FILE *fdopen(int fd, const char *mode)
{
	return wlibc_fdopen(fd, mode);
}

WLIBC_API FILE *wlibc_freopen(const char *name, const char *mode, FILE *stream);

WLIBC_INLINE FILE *freopen(const char *name, const char *mode, FILE *stream)
{
	return wlibc_freopen(name, mode, stream);
}

WLIBC_API int wlibc_fclose(FILE *stream);

WLIBC_INLINE int fclose(FILE *stream)
{
	return wlibc_fclose(stream);
}

WLIBC_API FILE *wlibc_popen(const char *command, const char *mode);

WLIBC_INLINE FILE *popen(const char *command, const char *mode)
{
	return wlibc_popen(command, mode);
}

WLIBC_API int wlibc_pclose(FILE *stream);

WLIBC_INLINE int pclose(FILE *stream)
{
	return wlibc_pclose(stream);
}

WLIBC_API int wlibc_fileno(FILE *stream);

WLIBC_INLINE int fileno(FILE *stream)
{
	return wlibc_fileno(stream);
}

// file input
WLIBC_API size_t wlibc_fread(void *buffer, size_t size, size_t count, FILE *stream);
WLIBC_API char *wlibc_fgets(void *buffer, size_t count, FILE *stream);
WLIBC_API int wlibc_fgetc(FILE *stream);

WLIBC_INLINE size_t fread(void *buffer, size_t size, size_t count, FILE *stream)
{
	return wlibc_fread(buffer, size, count, stream);
}

WLIBC_INLINE char *fgets(void *buffer, size_t count, FILE *stream)
{
	return wlibc_fgets(buffer, count, stream);
}

WLIBC_INLINE int fgetc(FILE *stream)
{
	return wlibc_fgetc(stream);
}

WLIBC_INLINE int getc(FILE *stream)
{
	return wlibc_fgetc(stream);
}

WLIBC_INLINE int getchar(void)
{
	return wlibc_fgetc(stdin);
}

WLIBC_API ssize_t wlibc_getdelim(char **buffer, size_t *size, int delimiter, FILE *stream);

WLIBC_INLINE ssize_t getdelim(char **buffer, size_t *size, int delimiter, FILE *stream)
{
	return wlibc_getdelim(buffer, size, delimiter, stream);
}

WLIBC_INLINE ssize_t getline(char **buffer, size_t *size, FILE *stream)
{
	return wlibc_getdelim(buffer, size, '\n', stream);
}

// file output
WLIBC_API size_t wlibc_fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
WLIBC_API int wlibc_fputs(const char *buffer, FILE *stream);
WLIBC_API int wlibc_fputc(int ch, FILE *stream);

WLIBC_INLINE size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream)
{
	return wlibc_fwrite(buffer, size, count, stream);
}

WLIBC_INLINE int fputs(const char *buffer, FILE *stream)
{
	return wlibc_fputs(buffer, stream);
}

WLIBC_INLINE int fputc(int ch, FILE *stream)
{
	return wlibc_fputc(ch, stream);
}

WLIBC_INLINE int puts(const char *buffer)
{
	return wlibc_fputs(buffer, stdout);
}

WLIBC_INLINE int putc(int ch, FILE *stream)
{
	return wlibc_fputc(ch, stream);
}

WLIBC_INLINE int putchar(int ch)
{
	return wlibc_fputc(ch, stdout);
}

// file positioning
WLIBC_API int wlibc_fseek(FILE *stream, ssize_t offset, int whence);
WLIBC_API ssize_t wlibc_ftell(FILE *stream);

WLIBC_INLINE int fseek(FILE *stream, ssize_t offset, int whence)
{
	return wlibc_fseek(stream, offset, whence);
}

WLIBC_INLINE int fseeko(FILE *stream, off_t offset, int whence)
{
	return wlibc_fseek(stream, offset, whence);
}

WLIBC_INLINE ssize_t ftell(FILE *stream)
{
	return wlibc_ftell(stream);
}

WLIBC_INLINE off_t ftello(FILE *stream)
{
	return wlibc_ftell(stream);
}

WLIBC_INLINE int fsetpos(FILE *stream, fpos_t *pos)
{
	return wlibc_fseek(stream, *pos, SEEK_SET);
}

WLIBC_INLINE int fgetpos(FILE *stream, fpos_t *pos)
{
	*pos = wlibc_ftell(stream);
	return *pos != -1ull ? 0 : -1;
}

WLIBC_INLINE int rewind(FILE *stream)
{
	return wlibc_fseek(stream, 0, SEEK_SET);
}

WLIBC_API int wlibc_ungetc(int ch, FILE *stream);

WLIBC_INLINE int ungetc(int ch, FILE *stream)
{
	return wlibc_ungetc(ch, stream);
}

// file misc
WLIBC_API int wlibc_fflush(FILE *stream);

WLIBC_INLINE int fflush(FILE *stream)
{
	return wlibc_fflush(stream);
}

WLIBC_API int wlibc_setvbuf(FILE *stream, char *buffer, int mode, size_t size);

WLIBC_INLINE int setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
	return wlibc_setvbuf(stream, buffer, mode, size);
}

WLIBC_INLINE int setbuf(FILE *stream, char *buffer)
{
	return buffer == NULL ? wlibc_setvbuf(stream, NULL, _IONBF, 0) : wlibc_setvbuf(stream, buffer, _IOFBF, BUFSIZ);
}

// file error handling
WLIBC_API int wlibc_feof(FILE *stream);

WLIBC_INLINE int feof(FILE *stream)
{
	return wlibc_feof(stream);
}

WLIBC_API int wlibc_ferror(FILE *stream);

WLIBC_INLINE int ferror(FILE *stream)
{
	return wlibc_ferror(stream);
}

WLIBC_API void wlibc_clearerr(FILE *stream);

WLIBC_INLINE void clearerr(FILE *stream)
{
	wlibc_clearerr(stream);
}

WLIBC_API void wlibc_perror(char const *message);

WLIBC_INLINE void perror(char const *message)
{
	return wlibc_perror(message);
}

// file locking
//#define LOCK    0
//#define UNLOCK  1
//#define TRYLOCK 2
WLIBC_API int wlibc_lockfile_op(FILE *stream, int op);

WLIBC_INLINE void wlibc_flockfile(FILE *stream)
{
	wlibc_lockfile_op(stream, 0);
}

WLIBC_INLINE void wlibc_funlockfile(FILE *stream)
{
	wlibc_lockfile_op(stream, 1);
}

WLIBC_INLINE int wlibc_ftrylockfile(FILE *stream)
{
	return wlibc_lockfile_op(stream, 2);
}

// printf
WLIBC_API int wlibc_vsnprintf(char *buffer, size_t size, const char *format, va_list args);
WLIBC_API int wlibc_vdprintf(int fd, const char *format, va_list args);
WLIBC_API int wlibc_vfprintf(FILE *stream, const char *format, va_list args);
WLIBC_API int wlibc_vasprintf(char **buffer, const char *format, va_list args);

WLIBC_INLINE int vasprintf(char **buffer, const char *format, va_list args)
{
	return wlibc_vasprintf(buffer, format, args);
}

WLIBC_INLINE int vsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
	return wlibc_vsnprintf(buffer, size, format, args);
}

WLIBC_INLINE int vsprintf(char *buffer, const char *format, va_list args)
{
	return wlibc_vsnprintf(buffer, -1, format, args);
}

WLIBC_INLINE int vdprintf(int fd, const char *format, va_list args)
{
	return wlibc_vdprintf(fd, format, args);
}

WLIBC_INLINE int vfprintf(FILE *stream, const char *format, va_list args)
{
	return wlibc_vfprintf(stream, format, args);
}

WLIBC_INLINE int vprintf(const char *format, va_list args)
{
	return wlibc_vfprintf(stdout, format, args);
}

WLIBC_INLINE int snprintf(char *buffer, size_t size, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsnprintf(buffer, size, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int sprintf(char *buffer, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsnprintf(buffer, -1, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int dprintf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vdprintf(fd, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int fprintf(FILE *stream, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfprintf(stream, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfprintf(stdout, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int asprintf(char **buffer, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vasprintf(buffer, format, args);
	va_end(args);
	return result;
}

// scanf
WLIBC_API int wlibc_vsscanf(const char *str, const char *format, va_list args);
WLIBC_API int wlibc_vfscanf(FILE *stream, const char *format, va_list args);

WLIBC_INLINE int vsscanf(const char *str, const char *format, va_list args)
{
	return wlibc_vsscanf(str, format, args);
}

WLIBC_INLINE int vfscanf(FILE *stream, const char *format, va_list args)
{
	return wlibc_vfscanf(stream, format, args);
}

WLIBC_INLINE int vscanf(const char *format, va_list args)
{
	return wlibc_vfscanf(stdin, format, args);
}

WLIBC_INLINE int sscanf(const char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsscanf(str, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int fscanf(FILE *stream, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfscanf(stream, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int scanf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfscanf(stdin, format, args);
	va_end(args);
	return result;
}

// misc
WLIBC_API int wlibc_remove(const char *path);

WLIBC_INLINE int remove(const char *path)
{
	return wlibc_remove(path);
}

WLIBC_API FILE *wlibc_tmpfile();

WLIBC_INLINE FILE *tmpfile()
{
	return wlibc_tmpfile();
}

WLIBC_API char *wlibc_tmpnam(char *name);

WLIBC_INLINE char *tmpnam(char *name)
{
	return wlibc_tmpnam(name);
}

WLIBC_API int wlibc_fcloseall();

WLIBC_INLINE int fcloseall()
{
	return wlibc_fcloseall();
}


#if 0 
// For renameat2
// Using same values from linux/fs.h
#define RENAME_NOREPLACE 0x1 // Don't overwrite
#define RENAME_EXCHANGE  0x2 // Exchange the files
#define RENAME_WHITEOUT  0x4 // Unsupported

WLIBC_API int wlibc_renameat2(int olddirfd, const char *oldname, int newdirfd, const char *newname, unsigned int flags);

WLIBC_INLINE int rename(const char *oldname, const char *newname)
{
	// FIXME put AT_FDCWD here
	return wlibc_renameat2(0x1000000, oldname, 0x1000000, newname, 0);
}

WLIBC_INLINE int renameat(int olddirfd, const char *oldname, int newdirfd, const char *newname)
{
	return wlibc_renameat2(olddirfd, oldname, newdirfd, newname, 0);
}

WLIBC_INLINE int renameat2(int olddirfd, const char *oldname, int newdirfd, const char *newname, unsigned int flags)
{
	return wlibc_renameat2(olddirfd, oldname, newdirfd, newname, flags);
}
#endif

// memstream
WLIBC_API FILE *wlibc_fmemopen(void *restrict, size_t, const char *restrict);
WLIBC_API FILE *wlibc_open_memstream(char **, size_t *);

// Unlocked

// input
WLIBC_API size_t wlibc_fread_unlocked(void *buffer, size_t size, size_t count, FILE *stream);
WLIBC_API char *wlibc_fgets_unlocked(void *buffer, size_t count, FILE *stream);
WLIBC_API int wlibc_fgetc_unlocked(FILE *stream);

WLIBC_INLINE size_t fread_unlocked(void *buffer, size_t size, size_t count, FILE *stream)
{
	return wlibc_fread_unlocked(buffer, size, count, stream);
}

WLIBC_INLINE char *fgets_unlocked(void *buffer, size_t count, FILE *stream)
{
	return wlibc_fgets_unlocked(buffer, count, stream);
}

WLIBC_INLINE int fgetc_unlocked(FILE *stream)
{
	return wlibc_fgetc_unlocked(stream);
}

WLIBC_INLINE int getc_unlocked(FILE *stream)
{
	return wlibc_fgetc_unlocked(stream);
}

WLIBC_INLINE int getchar_unlocked(void)
{
	return wlibc_fgetc_unlocked(stdin);
}

// output
WLIBC_API size_t wlibc_fwrite_unlocked(const void *buffer, size_t size, size_t count, FILE *stream);
WLIBC_API int wlibc_fputs_unlocked(const char *buffer, FILE *stream);
WLIBC_API int wlibc_fputc_unlocked(int ch, FILE *stream);

WLIBC_INLINE size_t fwrite_unlocked(const void *buffer, size_t size, size_t count, FILE *stream)
{
	return wlibc_fwrite_unlocked(buffer, size, count, stream);
}

WLIBC_INLINE int fputs_unlocked(const char *buffer, FILE *stream)
{
	return wlibc_fputs_unlocked(buffer, stream);
}

WLIBC_INLINE int fputc_unlocked(int ch, FILE *stream)
{
	return wlibc_fputc_unlocked(ch, stream);
}

WLIBC_INLINE int putc_unlocked(int ch, FILE *stream)
{
	return wlibc_fputc_unlocked(ch, stream);
}

WLIBC_INLINE int putchar_unlocked(int ch)
{
	return wlibc_fputc_unlocked(ch, stdout);
}

// misc
WLIBC_API int wlibc_feof_unlocked(FILE *stream);

WLIBC_INLINE int feof_unlocked(FILE *stream)
{
	return wlibc_feof_unlocked(stream);
}

WLIBC_API int wlibc_ferror_unlocked(FILE *stream);

WLIBC_INLINE int ferror_unlocked(FILE *stream)
{
	return wlibc_ferror_unlocked(stream);
}

WLIBC_API void wlibc_clearerr_unlocked(FILE *stream);

WLIBC_INLINE void clearerr_unlocked(FILE *stream)
{
	wlibc_clearerr_unlocked(stream);
}

WLIBC_API int wlibc_fileno_unlocked(FILE *stream);

WLIBC_INLINE int fileno_unlocked(FILE *stream)
{
	return wlibc_fileno_unlocked(stream);
}

WLIBC_API int wlibc_fflush_unlocked(FILE *stream);

WLIBC_INLINE int fflush_unlocked(FILE *stream)
{
	return wlibc_fflush_unlocked(stream);
}

_WLIBC_END_DECLS

#endif