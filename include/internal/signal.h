/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SIGNAL_INTERNAL_H
#define WLIBC_SIGNAL_INTERNAL_H

#include <internal/nt.h>
#include <internal/validate.h>
#include <signal.h>

typedef struct _siginfo
{
	signal_t action;
	int flags;
	int mask;
} siginfo;

extern RTL_SRWLOCK _wlibc_signal_srw;
extern siginfo _wlibc_signal_table[NSIG];

void signal_init(void);
void signal_cleanup(void);

void get_siginfo(int sig, siginfo *sinfo);
void set_siginfo(int sig, const siginfo *sinfo);

#define SHARED_LOCK_SIGNAL_TABLE()      RtlAcquireSRWLockShared(&_wlibc_signal_srw)
#define SHARED_UNLOCK_SIGNAL_TABLE()    RtlReleaseSRWLockShared(&_wlibc_signal_srw)
#define EXCLUSIVE_LOCK_SIGNAL_TABLE()   RtlAcquireSRWLockExclusive(&_wlibc_signal_srw)
#define EXCLUSIVE_UNLOCK_SIGNAL_TABLE() RtlReleaseSRWLockExclusive(&_wlibc_signal_srw)

#define VALIDATE_SIGSET(sigset) VALIDATE_PTR(sigset, EINVAL, -1)

#define VALIDATE_SIGNAL(signal)        \
	if (signal <= 0 || signal >= NSIG) \
	{                                  \
		errno = EINVAL;                \
		return -1;                     \
	}

#endif
