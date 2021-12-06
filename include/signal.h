/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SIGNAL_H
#define WLIBC_SIGNAL_H

#include <wlibc-macros.h>
#include <corecrt.h>

_WLIBC_BEGIN_DECLS

typedef void(__cdecl *_crt_signal_t)(int);
typedef int sig_atomic_t;
typedef int sigset_t;

struct sigaction
{
	_crt_signal_t sa_handler;
	// void (*sa_sigaction)(int, siginfo_t *, void *); Unsupported
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void); // Not intended for public use. See POSIX doc
};

#define SIGHUP    1
#define SIGINT    2 // Interrupt
#define SIGQUIT   3
#define SIGILL    4 // Illegal instruction
#define SIGTRAP   5
#define SIGABRT   6
#define SIGBUS    7  // Same action as SIGSEGV
#define SIGFPE    8  // floating point exception
#define SIGKILL   9  // Call abort always
#define SIGUSR1   10 // User defined signal
#define SIGSEGV   11 // Segment violation
#define SIGUSR2   12 // User defined signal
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15 // Termination
#define SIGSTKFLT 16 // Unused
#define SIGCHLD   17
#define SIGCLD    17 // Same as SIGCHLD
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20 // Unsupported
#define SIGTTIN   21 // Ctrl-Break sequence
#define SIGBREAK  SIGTTIN

#define NSIG 22 // Biggest signal number + 1

// Signal action codes
#define SIG_ERR ((_crt_signal_t)-1) // Signal error value
#define SIG_DFL ((_crt_signal_t)0)  // Default signal action
#define SIG_IGN ((_crt_signal_t)1)  // Ignore signal
#define SIG_GET ((_crt_signal_t)2)  // Return current value
#define SIG_SGE ((_crt_signal_t)3)  // Signal error
#define SIG_ACK ((_crt_signal_t)4)  // Acknowledge
#define SIG_DIE ((_crt_signal_t)5)  // Terminate process

// Values for the 'how' argument to sigprocmask.
#define SIG_BLOCK   0 // Block signals
#define SIG_UNBLOCK 1 // Unblock signals
#define SIG_SETMASK 2 // Set the set of blocked signals

// Values for sa_flags
#define SA_NOCLDSTOP 0x01 // Don't raise SIGCHLD when children stop
#define SA_NOCLDWAIT 0x02 // Unsupported. No zombies here
#define SA_NODEFER   0x04 // Don't automatically block the signal when its handler is being executed
#define SA_ONSTACK   0x08 // Unsupported
#define SA_RESETHAND 0x10 // Reset the signal handler to the default action
#define SA_RESTART   0x20 // Unsupported
#define SA_RESTORER  0x40 // Not intended for public use. See POSIX doc

/* DO NOT define this. If we define this, it implies that we support sa_sigaction.
   sa_sigaction is not possible to emulate in user space.
#define SA_SIGINFO 0x80 // Invoke the signal handler with 3 arguments
*/

#define SA_NOMASK  SA_NODEFER
#define SA_ONESHOT SA_RESETHAND
#define SA_STACK   SA_ONSTACK

#define raise wlibc_raise
WLIBC_API int wlibc_raise(int sig);

#define signal wlibc_signal
WLIBC_API _crt_signal_t wlibc_signal(int sig, _crt_signal_t handler);

// Signal blocking, unblocking machinery
WLIBC_API int wlibc_sigemptyset(sigset_t *set);          // Initialize signal set
WLIBC_API int wlibc_sigfillset(sigset_t *set);           // Initialize and fill signal set with all supported signals
WLIBC_API int wlibc_sigaddset(sigset_t *set, int sig);   // Add to signal set
WLIBC_API int wlibc_sigdelset(sigset_t *set, int sig);   // Delete from signal set
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

WLIBC_API int wlibc_sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action);
WLIBC_INLINE int sigaction(int sig, const struct sigaction *new_action, struct sigaction *old_action)
{
	return wlibc_sigaction(sig, new_action, old_action);
}

_WLIBC_END_DECLS

#endif
