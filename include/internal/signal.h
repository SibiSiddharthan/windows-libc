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

extern CRITICAL_SECTION _wlibc_signal_critical;

extern _crt_signal_t _wlibc_signal_table[NSIG];
extern int _wlibc_signal_flags[NSIG];
extern int _wlibc_signal_mask[NSIG];

void signal_init(void);
void signal_cleanup(void);

_crt_signal_t get_action(int signum);
_crt_signal_t set_action(int signum, _crt_signal_t action); // return the old action

#define VALIDATE_SIGSET(sigset) VALIDATE_PTR(sigset, EINVAL, -1)

#define VALIDATE_SIGNAL(signal)        \
	if (signal <= 0 || signal >= NSIG) \
	{                                  \
		errno = EINVAL;                \
		return -1;                     \
	}

#endif
