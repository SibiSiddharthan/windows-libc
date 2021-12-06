/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_PROCESS_H
#define WLIBC_PROCESS_H

#include <wlibc.h>
#include <stdarg.h>
#include <errno.h>
#include <wchar.h>

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

#define __WLIBC_VALIDATE_SPAWN_MODE(mode) \
	if (mode < 0 || mode > 3)             \
	{                                     \
		errno = EINVAL;                   \
		return -1;                        \
	}

#define __WLIBC_VALIDATE_SPAWN_ARGS2(arg1, arg2) \
	if (arg1 == NULL || arg2 == NULL)            \
	{                                            \
		errno = EINVAL;                          \
		return -1;                               \
	}

WLIBC_API int wlibc_common_spawnl(int mode, int search_path, int argc, int envc, const char *file, const char *arg0, va_list args);
WLIBC_API int wlibc_common_spawnv(int mode, int search_path, const char *file, char const *const *argv, char const *const *env);

WLIBC_INLINE int execl(const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(_P_OVERLAY, 0, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execle(const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, char *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(_P_OVERLAY, 0, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execlp(const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(_P_OVERLAY, 1, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execlpe(const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, char *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(_P_OVERLAY, 1, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int execv(char const *file, char const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_spawnv(_P_OVERLAY, 0, file, args, NULL);
}

WLIBC_INLINE int execve(char const *file, char const *const *args, char const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_spawnv(_P_OVERLAY, 0, file, args, env);
}

WLIBC_INLINE int execvp(char const *file, char const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_spawnv(_P_OVERLAY, 1, file, args, NULL);
}

WLIBC_INLINE int execvpe(char const *file, char const *const *args, char const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_spawnv(_P_OVERLAY, 1, file, args, env);
}

#define _execl   execl
#define _execle  execle
#define _execlp  execlp
#define _execlpe execlpe
#define _execv   execv
#define _execve  execve
#define _execvp  execvp
#define _execvpe execvpe

WLIBC_INLINE int spawnl(int mode, const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(mode, 0, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnle(int mode, const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, char *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(mode, 0, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnlp(int mode, const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(mode, 1, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnlpe(int mode, const char *file, const char *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, char *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, char *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_spawnl(mode, 1, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int spawnv(int mode, char const *file, char const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_spawnv(mode, 0, file, args, NULL);
}

WLIBC_INLINE int spawnve(int mode, char const *file, char const *const *args, char const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_spawnv(mode, 0, file, args, env);
}

WLIBC_INLINE int spawnvp(int mode, char const *file, char const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_spawnv(mode, 1, file, args, NULL);
}

WLIBC_INLINE int spawnvpe(int mode, char const *file, char const *const *args, char const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_spawnv(mode, 0, file, args, env);
}

#define _spawnl   spawnl
#define _spawnle  spawnle
#define _spawnlp  spawnlp
#define _spawnlpe spawnlpe
#define _spawnv   spawnv
#define _spawnve  spawnve
#define _spawnvp  spawnvp
#define _spawnvpe spawnvpe

/*
 * wide character equivalents
 */

WLIBC_API int wlibc_common_wspawnl(int mode, int search_path, int argc, int envc, const wchar_t *file, const wchar_t *arg0, va_list args);
WLIBC_API int wlibc_common_wspawnv(int mode, int search_path, const wchar_t *file, wchar_t const *const *argv, wchar_t const *const *env);

WLIBC_INLINE int wexecl(const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(_P_OVERLAY, 0, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wexecle(const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, wchar_t *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(_P_OVERLAY, 0, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wexeclp(const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(_P_OVERLAY, 1, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wexeclpe(const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, wchar_t *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(_P_OVERLAY, 1, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wexecv(wchar_t const *file, wchar_t const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_wspawnv(_P_OVERLAY, 0, file, args, NULL);
}

WLIBC_INLINE int wexecve(wchar_t const *file, wchar_t const *const *args, wchar_t const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_wspawnv(_P_OVERLAY, 0, file, args, env);
}

WLIBC_INLINE int wexecvp(wchar_t const *file, wchar_t const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_wspawnv(_P_OVERLAY, 1, file, args, NULL);
}

WLIBC_INLINE int wexecvpe(wchar_t const *file, wchar_t const *const *args, wchar_t const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);

	return wlibc_common_wspawnv(_P_OVERLAY, 1, file, args, env);
}

#define _wexecl   wexecl
#define _wexecle  wexecle
#define _wexeclp  wexeclp
#define _wexeclpe wexeclpe
#define _wexecv   wexecv
#define _wexecve  wexecve
#define _wexecvp  wexecvp
#define _wexecvpe wexecvpe

WLIBC_INLINE int wspawnl(int mode, const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(mode, 0, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wspawnle(int mode, const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, wchar_t *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(mode, 0, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wspawnlp(int mode, const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0;
	va_list args;
	va_start(args, arg0);
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(mode, 1, argc, 0, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wspawnlpe(int mode, const wchar_t *file, const wchar_t *arg0, ...)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, arg0);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	int argc = 0, envc = 0;
	va_list args;
	va_start(args, arg0);
	// argv
	while (va_arg(args, wchar_t *) != NULL)
	{
		++argc;
	}
	// env
	while (va_arg(args, wchar_t *) != NULL)
	{
		++envc;
	}
	va_end(args);

	va_start(args, arg0);
	int status = wlibc_common_wspawnl(mode, 1, argc, envc, file, arg0, args);
	va_end(args);

	return status;
}

WLIBC_INLINE int wspawnv(int mode, wchar_t const *file, wchar_t const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_wspawnv(mode, 0, file, args, NULL);
}

WLIBC_INLINE int wspawnve(int mode, wchar_t const *file, wchar_t const *const *args, wchar_t const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_wspawnv(mode, 0, file, args, env);
}

WLIBC_INLINE int wspawnvp(int mode, wchar_t const *file, wchar_t const *const *args)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_wspawnv(mode, 1, file, args, NULL);
}

WLIBC_INLINE int wspawnvpe(int mode, wchar_t const *file, wchar_t const *const *args, wchar_t const *const *env)
{
	__WLIBC_VALIDATE_SPAWN_ARGS2(file, args);
	__WLIBC_VALIDATE_SPAWN_MODE(mode);

	return wlibc_common_wspawnv(mode, 0, file, args, env);
}

#define _wspawnl   wspawnl
#define _wspawnle  wspawnle
#define _wspawnlp  wspawnlp
#define _wspawnlpe wspawnlpe
#define _wspawnv   wspawnv
#define _wspawnve  wspawnve
#define _wspawnvp  wspawnvp
#define _wspawnvpe wspawnvpe

_WLIBC_END_DECLS

#endif
