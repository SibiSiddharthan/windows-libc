/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SIGNAL_H
#define WLIBC_SIGNAL_H

#include <wlibc-macros.h>
#include <signal.h>

// SIGABRT is 6 on POSIX platforms, but it is defined as 22
#undef SIGABRT

_WLIBC_BEGIN_DECLS

typedef int sigset_t;
// 7,
// POSIX signals not supported by msvcrt
#define SIGHUP    1
#define SIGQUIT   3
#define SIGTRAP   5
#define SIGABRT   6
#define SIGBUS    7  // Same action as SIGSEGV
#define SIGKILL   9  // Call abort always
#define SIGUSR1   10 // User defined signal
#define SIGUSR2   12 // User defined signal
#define SIGPIPE   13
#define SIGALRM   14
#define SIGSTKFLT 16 // Unused
#define SIGCHLD   17
#define SIGCLD    17 // Same as SIGCHLD
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20 // Unsupported
#define SIGTTIN   21 // Same as SIGBREAK

// Values for the 'how' argument to sigprocmask.
#define SIG_BLOCK   0 // Block signals
#define SIG_UNBLOCK 1 // Unblock signals
#define SIG_SETMASK 2 // Set the set of blocked signals

#define raise wlibc_raise
WLIBC_API int wlibc_raise(int sig);

#define signal wlibc_signal
WLIBC_API _crt_signal_t wlibc_signal(int sig, _crt_signal_t handler);

// Signal blocking, unblocking machinery
WLIBC_API int wlibc_sigemptyset(sigset_t *set);          // Initialize signal set
WLIBC_API int wlibc_sigfillset(sigset_t *set);           // Initialize and fill signal set with all supported signals
WLIBC_API int wlibc_sigaddset(sigset_t *set, int sig);   // Add to signal set
WLIBC_API int wlibc_sigdelset(sigset_t *set, int sig);  // Delete from signal set
WLIBC_API int wlibc_sigismember(sigset_t *set, int sig); // Is member
WLIBC_API int wlibc_sigpending(sigset_t *set);           // Get the set of blocked signals that are raised
WLIBC_API int wlibc_sigprocmask(int how, const sigset_t *set, sigset_t *oldset); // Block, Unblock signals based on signal set

WLIBC_INLINE int sigemptyset(sigset_t *set)
{
	return wlibc_sigemptyset(set);
}

WLIBC_INLINE int sigfillset(sigset_t *set)
{
	return wlibc_sigfillset(set);
}

WLIBC_INLINE int sigaddset(sigset_t *set, int sig)
{
	return wlibc_sigaddset(set, sig);
}

WLIBC_INLINE int sigdelset(sigset_t *set, int sig)
{
	return wlibc_sigdelset(set, sig);
}

WLIBC_INLINE int sigismember(sigset_t *set, int sig)
{
	return wlibc_sigismember(set, sig);
}

WLIBC_INLINE int sigpending(sigset_t *set)
{
	return wlibc_sigpending(set);
}

WLIBC_INLINE int sigprocmask(int how, const sigset_t *newset, sigset_t *oldset)
{
	return wlibc_sigprocmask(how, newset, oldset);
}

_WLIBC_END_DECLS

#endif
