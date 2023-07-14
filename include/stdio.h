/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDIO_H
#define WLIBC_STDIO_H

#include <wlibc.h>
#include <stdarg.h>
#include <sys/types.h>
#include <fcntl.h>

_WLIBC_BEGIN_DECLS

#define _FILE_DEFINED
/* Buffered I/O macros */

#define BUFSIZ    512
#define L_tmpnam  260
#define L_ctermid 260
#define L_cuserid

typedef struct _WLIBC_FILE FILE;

extern FILE *_wlibc_stdin;
extern FILE *_wlibc_stdout;
extern FILE *_wlibc_stderr;

#define stdin  _wlibc_stdin
#define stdout _wlibc_stdout
#define stderr _wlibc_stderr

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
WLIBC_API FILE *wlibc_fopen(const char *restrict name, const char *restrict mode);

WLIBC_INLINE FILE *fopen(const char *restrict name, const char *restrict mode)
{
	return wlibc_fopen(name, mode);
}

WLIBC_API FILE *wlibc_fdopen(int fd, const char *mode);

WLIBC_INLINE FILE *fdopen(int fd, const char *mode)
{
	return wlibc_fdopen(fd, mode);
}

WLIBC_API FILE *wlibc_freopen(const char *restrict name, const char *restrict mode, FILE *restrict stream);

WLIBC_INLINE FILE *freopen(const char *restrict name, const char *restrict mode, FILE *restrict stream)
{
	return wlibc_freopen(name, mode, stream);
}

WLIBC_API int wlibc_fclose(FILE *stream);

WLIBC_INLINE int fclose(FILE *stream)
{
	return wlibc_fclose(stream);
}

WLIBC_API FILE *wlibc_popen(const char *restrict command, const char *restrict mode);

WLIBC_INLINE FILE *popen(const char *restrict command, const char *restrict mode)
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

/*
  Compilers like clang and gcc do library procedure optimizations. Eg puts -> fwrite
  To prevent this these functions need to be declared as macros
*/

WLIBC_API size_t wlibc_fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
WLIBC_API char *wlibc_fgets(char *restrict buffer, size_t count, FILE *restrict stream);
WLIBC_API int wlibc_fgetc(FILE *stream);

WLIBC_INLINE size_t fread(void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	return wlibc_fread(buffer, size, count, stream);
}

#define fgets(buffer, count, stream) wlibc_fgets(buffer, count, stream)
#define fgetc(stream)                wlibc_fgetc(stream)
#define getc(stream)                 wlibc_fgetc(stream)
#define getchar()                    wlibc_fgetc(stdin)

WLIBC_API ssize_t wlibc_getdelim(char **restrict buffer, size_t *restrict size, int delimiter, FILE *restrict stream);

WLIBC_INLINE ssize_t getdelim(char **restrict buffer, size_t *restrict size, int delimiter, FILE *restrict stream)
{
	return wlibc_getdelim(buffer, size, delimiter, stream);
}

WLIBC_INLINE ssize_t getline(char **restrict buffer, size_t *restrict size, FILE *restrict stream)
{
	return wlibc_getdelim(buffer, size, '\n', stream);
}

// file output
WLIBC_API size_t wlibc_fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
WLIBC_API int wlibc_fputs(const char *restrict buffer, FILE *restrict stream);
WLIBC_API int wlibc_fputc(int ch, FILE *stream);

WLIBC_INLINE size_t fwrite(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	return wlibc_fwrite(buffer, size, count, stream);
}

#define fputs(buffer, stream) wlibc_fputs(buffer, stream)
#define puts(buffer)          wlibc_fputs(buffer, stdout)
#define fputc(ch, stream)     wlibc_fputc(ch, stream)
#define putc(ch, stream)      wlibc_fputc(ch, stream)
#define putchar(ch)           wlibc_fputc(ch, stdout)

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

WLIBC_INLINE int fsetpos(FILE *restrict stream, fpos_t *restrict pos)
{
	return wlibc_fseek(stream, *pos, SEEK_SET);
}

WLIBC_INLINE int fgetpos(FILE *restrict stream, fpos_t *restrict pos)
{
	*pos = wlibc_ftell(stream);
	return *pos != -1ll ? 0 : -1;
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

WLIBC_API int wlibc_setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size);

WLIBC_INLINE int setvbuf(FILE *restrict stream, char *restrict buffer, int mode, size_t size)
{
	return wlibc_setvbuf(stream, buffer, mode, size);
}

WLIBC_INLINE int setbuf(FILE *restrict stream, char *restrict buffer)
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
	wlibc_perror(message);
}

/*
#define LOCK    0
#define UNLOCK  1
#define TRYLOCK 2
*/
WLIBC_API int wlibc_lockfile_op(FILE *stream, int op);

WLIBC_INLINE void flockfile(FILE *stream)
{
	wlibc_lockfile_op(stream, 0);
}

WLIBC_INLINE void funlockfile(FILE *stream)
{
	wlibc_lockfile_op(stream, 1);
}

WLIBC_INLINE int ftrylockfile(FILE *stream)
{
	return wlibc_lockfile_op(stream, 2);
}

// printf
WLIBC_API int wlibc_vsnprintf(char *restrict buffer, size_t size, const char *restrict format, va_list args);
WLIBC_API int wlibc_vdprintf(int fd, const char *restrict format, va_list args);
WLIBC_API int wlibc_vfprintf(FILE *restrict stream, const char *restrict format, va_list args);
WLIBC_API int wlibc_vasprintf(char **restrict buffer, const char *restrict format, va_list args);
WLIBC_API char *wlibc_vasnprintf(char *restrict buffer, size_t *size, const char *restrict format, va_list args);

WLIBC_INLINE int vasprintf(char **restrict buffer, const char *restrict format, va_list args)
{
	return wlibc_vasprintf(buffer, format, args);
}

WLIBC_INLINE int vsnprintf(char *restrict buffer, size_t size, const char *restrict format, va_list args)
{
	return wlibc_vsnprintf(buffer, size, format, args);
}

WLIBC_INLINE char *vasnprintf(char *restrict buffer, size_t *size, const char *restrict format, va_list args)
{
	return wlibc_vasnprintf(buffer, size, format, args);
}

WLIBC_INLINE int vsprintf(char *restrict buffer, const char *restrict format, va_list args)
{
	return wlibc_vsnprintf(buffer, (size_t)-1, format, args);
}

WLIBC_INLINE int vdprintf(int fd, const char *restrict format, va_list args)
{
	return wlibc_vdprintf(fd, format, args);
}

WLIBC_INLINE int vfprintf(FILE *restrict stream, const char *restrict format, va_list args)
{
	return wlibc_vfprintf(stream, format, args);
}

WLIBC_INLINE int vprintf(const char *restrict format, va_list args)
{
	return wlibc_vfprintf(stdout, format, args);
}

WLIBC_INLINE int snprintf(char *restrict buffer, size_t size, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsnprintf(buffer, size, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int sprintf(char *restrict buffer, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsnprintf(buffer, (size_t)-1, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int dprintf(int fd, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vdprintf(fd, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int fprintf(FILE *restrict stream, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfprintf(stream, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int printf(const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfprintf(stdout, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int asprintf(char **restrict buffer, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vasprintf(buffer, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE char *asnprintf(char *restrict buffer, size_t *size, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	char *result = wlibc_vasnprintf(buffer, size, format, args);
	va_end(args);
	return result;
}

// scanf
WLIBC_API int wlibc_vsscanf(const char *restrict str, const char *restrict format, va_list args);
WLIBC_API int wlibc_vfscanf(FILE *restrict stream, const char *restrict format, va_list args);

WLIBC_INLINE int vsscanf(const char *restrict str, const char *restrict format, va_list args)
{
	return wlibc_vsscanf(str, format, args);
}

WLIBC_INLINE int vfscanf(FILE *restrict stream, const char *restrict format, va_list args)
{
	return wlibc_vfscanf(stream, format, args);
}

WLIBC_INLINE int vscanf(const char *restrict format, va_list args)
{
	return wlibc_vfscanf(stdin, format, args);
}

WLIBC_INLINE int sscanf(const char *restrict str, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vsscanf(str, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int fscanf(FILE *restrict stream, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfscanf(stream, format, args);
	va_end(args);
	return result;
}

WLIBC_INLINE int scanf(const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int result = wlibc_vfscanf(stdin, format, args);
	va_end(args);
	return result;
}

// misc
// from unistd.h
WLIBC_API int wlibc_common_remove(int dirfd, const char *path, int flags);
WLIBC_INLINE int remove(const char *path)
{
	return wlibc_common_remove(AT_FDCWD, path, AT_REMOVEANY);
}

WLIBC_API char *wlibc_tmpdir(void);

WLIBC_API FILE *wlibc_tmpfile(void);

WLIBC_INLINE FILE *tmpfile(void)
{
	return wlibc_tmpfile();
}

WLIBC_API char *wlibc_tempnam(const char *restrict dir, const char *restrict prefix);

WLIBC_INLINE char *tempnam(const char *restrict dir, const char *restrict prefix)
{
	return wlibc_tempnam(dir, prefix);
}

WLIBC_API char *wlibc_tmpnam(const char *name);

WLIBC_INLINE char *tmpnam(const char *name)
{
	return wlibc_tmpnam(name);
}

WLIBC_INLINE char *tmpnam_r(const char *name)
{
	// Same as tmpnam
	return wlibc_tmpnam(name);
}

#define P_tmpdir wlibc_tmpdir()

WLIBC_API int wlibc_fcloseall();

WLIBC_INLINE int fcloseall()
{
	return wlibc_fcloseall();
}

// Available flags
#define RENAME_WHITEOUT  0x0 // Unsupported
#define RENAME_NOREPLACE 0x1 // Don't overwrite
#define RENAME_EXCHANGE  0x2 // Exchange the files

WLIBC_API int wlibc_common_rename(int olddirfd, const char *restrict oldpath, int newdirfd, const char *restrict newpath, int flags);

WLIBC_INLINE int rename(const char *restrict oldname, const char *restrict newname)
{
	return wlibc_common_rename(AT_FDCWD, oldname, AT_FDCWD, newname, 0);
}

WLIBC_INLINE int renameat(int olddirfd, const char *restrict oldname, int newdirfd, const char *restrict newname)
{
	return wlibc_common_rename(olddirfd, oldname, newdirfd, newname, 0);
}

WLIBC_INLINE int renameat2(int olddirfd, const char *restrict oldname, int newdirfd, const char *restrict newname, unsigned int flags)
{
	return wlibc_common_rename(olddirfd, oldname, newdirfd, newname, flags);
}

// memstream
WLIBC_API FILE *wlibc_fmemopen(void *restrict, size_t, const char *restrict);
WLIBC_API FILE *wlibc_open_memstream(char **, size_t *);

// Unlocked

// input
WLIBC_API size_t wlibc_fread_unlocked(void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
WLIBC_API char *wlibc_fgets_unlocked(char *restrict buffer, size_t count, FILE *restrict stream);
WLIBC_API int wlibc_fgetc_unlocked(FILE *stream);

WLIBC_INLINE size_t fread_unlocked(void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	return wlibc_fread_unlocked(buffer, size, count, stream);
}

#define fgets_unlocked(buffer, count, stream) wlibc_fgets_unlocked(buffer, count, stream)
#define fgetc_unlocked(stream)                wlibc_fgetc_unlocked(stream)
#define getc_unlocked(stream)                 wlibc_fgetc_unlocked(stream)
#define getchar_unlocked()                    wlibc_fgetc_unlocked(stdin)

// output
WLIBC_API size_t wlibc_fwrite_unlocked(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream);
WLIBC_API int wlibc_fputs_unlocked(const char *restrict buffer, FILE *restrict stream);
WLIBC_API int wlibc_fputc_unlocked(int ch, FILE *stream);

WLIBC_INLINE size_t fwrite_unlocked(const void *restrict buffer, size_t size, size_t count, FILE *restrict stream)
{
	return wlibc_fwrite_unlocked(buffer, size, count, stream);
}

#define fputs_unlocked(buffer, stream) wlibc_fputs_unlocked(buffer, stream)
#define fputc_unlocked(ch, stream)     wlibc_fputc_unlocked(ch, stream)
#define putc_unlocked(ch, stream)      wlibc_fputc_unlocked(ch, stream)
#define putchar_unlocked(ch)           wlibc_fputc_unlocked(ch, stdout)

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
