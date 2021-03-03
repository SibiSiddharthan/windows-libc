/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SIGNAL_INTERNAL_H
#define WLIBC_SIGNAL_INTERNAL_H

#include <signal-ext.h>
#include <Windows.h>

extern CRITICAL_SECTION _wlibc_signal_critical;

extern _crt_signal_t _wlibc_signal_table[NSIG];

void signal_init();
void signal_cleanup();

_crt_signal_t get_action(int signum);
_crt_signal_t set_action(int signum, _crt_signal_t action); // return the old action

#endif
