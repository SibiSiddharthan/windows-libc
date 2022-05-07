/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE path at the root directory for details.
*/

#ifndef WLIBC_PROCESS_H
#define WLIBC_PROCESS_H

#include <wlibc.h>
#include <stdarg.h>

_WLIBC_BEGIN_DECLS

#define _P_OVERLAY 0 // do exec
#define _P_WAIT    1 // wait for child process to finish
#define _P_DETACH  2 // asynchronous child process with no access to the parent's console
#define _P_NOWAIT  3 // asynchronous child process
#define _P_NOWAITO 3

/* Old defines for compatibility */
#define P_OVERLAY _P_OVERLAY
#define P_WAIT    _P_WAIT
#define P_DETACH  _P_DETACH
#define P_NOWAIT  _P_NOWAIT
#define P_NOWAITO _P_NOWAITO

WLIBC_API int wlibc_execl(int use_path, int env_given, const char *path, const char *arg0, va_list args);
WLIBC_API int wlibc_execve(int use_path, const char *restrict path, char *restrict const argv[], char *restrict const env[]);

WLIBC_INLINE int execl(const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_execl(0, 0, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execle(const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_execl(0, 1, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execlp(const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_execl(1, 0, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execlpe(const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_execl(1, 1, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execv(const char *restrict path, char *restrict const argv[])
{
	return wlibc_execve(0, path, argv, NULL);
}

WLIBC_INLINE int execve(const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_execve(0, path, argv, env);
}

WLIBC_INLINE int execvp(const char *restrict path, char *restrict const argv[])
{
	return wlibc_execve(1, path, argv, NULL);
}

WLIBC_INLINE int execvpe(const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_execve(1, path, argv, env);
}

// For msvc compatibility
#define _execl   execl
#define _execle  execle
#define _execlp  execlp
#define _execlpe execlpe
#define _execv   execv
#define _execve  execve
#define _execvp  execvp
#define _execvpe execvpe

WLIBC_API int wlibc_spawnl(int use_path, int env_given, int mode, const char *path, const char *arg0, va_list args);
WLIBC_API int wlibc_spawnve(int use_path, int mode, const char *restrict path, char *restrict const argv[], char *restrict const env[]);

WLIBC_INLINE int spawnl(int mode, const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_spawnl(0, 0, mode, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnle(int mode, const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_spawnl(0, 1, mode, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnlp(int mode, const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_spawnl(1, 0, mode, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnlpe(int mode, const char *path, const char *arg0, ...)
{
	int status;
	va_list args;

	va_start(args, arg0);
	status = wlibc_spawnl(1, 1, mode, path, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnv(int mode, const char *restrict path, char *restrict const argv[])
{
	return wlibc_spawnve(0, mode, path, argv, NULL);
}

WLIBC_INLINE int spawnve(int mode, const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_spawnve(0, mode, path, argv, env);
}

WLIBC_INLINE int spawnvp(int mode, const char *restrict path, char *restrict const argv[])
{
	return wlibc_spawnve(1, mode, path, argv, NULL);
}

WLIBC_INLINE int spawnvpe(int mode, const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_spawnve(1, mode, path, argv, env);
}

// For msvc compatibility
#define _spawnl   spawnl
#define _spawnle  spawnle
#define _spawnlp  spawnlp
#define _spawnlpe spawnlpe
#define _spawnv   spawnv
#define _spawnve  spawnve
#define _spawnvp  spawnvp
#define _spawnvpe spawnvpe

_WLIBC_END_DECLS

#endif
