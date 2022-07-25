/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_WAIT_H
#define WLIBC_SYS_WAIT_H

#include <wlibc.h>
#include <sys/types.h>
#include <signal.h>

_WLIBC_BEGIN_DECLS

// Processes with exit codes 3(SIGABRT compat) or greater than 128 are considered to be terminated a signal.
#define W_EXITCODE(wstatus, sig) (wstatus)
#define WIFEXITED(wstatus)       ((wstatus) != 3 && (wstatus) < 128)
#define WEXITSTATUS(wstatus)     (wstatus) // exit status

// Only SIGSTOP can suspend a process.
#define W_STOPCODE(sig)     (128 + SIGSTOP)
#define WIFSTOPPED(wstatus) ((wstatus) == 128 + SIGSTOP)
#define WSTOPSIG(status)    (SIGSTOP)

// Maintain MSVCRT compatibility for abort behaviour. `abort` sets the exit code as 3.
#define WIFSIGNALED(wstatus) ((wstatus) == 3 || (wstatus) > 128)
#define WTERMSIG(wstatus)    ((wstatus) == 3 ? SIGABRT : ((wstatus)-128))
#define WCOREDUMP(wstatus)   (0) // Core dumps, not supported
#define WCOREFLAG            0

#define W_CONTINUED           -1
#define WIFCONTINUED(wstatus) ((wstatus) == W_CONTINUED)

// Options for waitpid.
#define WUNTRACED  0x0 // Unsupported
#define WNOHANG    0x1 // wait returns immediately
#define WCONTINUED 0x2 // Unsupported

WLIBC_API pid_t wlibc_waitpid(pid_t pid, int *wstatus, int options);
WLIBC_INLINE pid_t waitpid(pid_t pid, int *wstatus, int options)
{
	return wlibc_waitpid(pid, wstatus, options);
}

WLIBC_INLINE pid_t wait(int *wstatus)
{
	return wlibc_waitpid(-1, wstatus, 0);
}

_WLIBC_END_DECLS

#endif
