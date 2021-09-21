/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_WAIT_H
#define WLIBC_SYS_WAIT_H

#include <wlibc-macros.h>
#include <sys/types.h>
#include <signal-ext.h>

_WLIBC_BEGIN_DECLS

#define WCOREDUMP(wstatus)   (0)       // Core dumps, not supported
#define WEXITSTATUS(wstatus) (wstatus) // exit status

/* All the signals which cause the process to exit set the return code as 3.
   So, WTERMSIG shall return SIGABRT, other signal checks shall check for this number 3.
*/
#define WIFEXITED(wstatus)   ((wstatus) != 3)
#define WIFSIGNALED(wstatus) ((wstatus) == 3)
#define WTERMSIG(wstatus)    ((SIGABRT))
#define WIFSTOPPED           WIFSIGNALED

#define WUNTRACED  0x0 // Unsupported
#define WNOHANG    0x1 // wait returns immediately
#define WCONTINUED 0x2 // Unsupported

WLIBC_API pid_t wlibc_waitpid(pid_t pid, int *wstatus, int options);
WLIBC_INLINE pid_t waitpid(pid_t pid, int *wstatus, int options)
{
	return wlibc_waitpid(pid, wstatus, options);
}

WLIBC_API pid_t wlibc_wait(int *wstatus);
WLIBC_INLINE pid_t wait(int *wstatus)
{
	return wlibc_wait(wstatus);
}

_WLIBC_END_DECLS

#endif
