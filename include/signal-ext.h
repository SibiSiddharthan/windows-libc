/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SIGNAL_H
#define WLIBC_SIGNAL_H

#include <wlibc-macros.h>
#include <signal.h>

_WLIBC_BEGIN_DECLS

#define SIGHUP  1
#define SIGQUIT 3
#define SIGTRAP 5
#define SIGKILL 9 // Call abort always
#define SIGPIPE 13
#define SIGALRM 14
#define SIGCHLD 17
#define SIGCLD  17 // Same as SIGCHLD
#define SIGSTOP 18
#define SIGTSTP 18 // Same as SIGSTOP
#define SIGCONT 19
#define SIGTTIN 21 // Same as SIGBREAK

#define raise wlibc_raise
WLIBC_API int wlibc_raise(int sig);

#define signal wlibc_signal
WLIBC_API _crt_signal_t wlibc_signal(int sig, _crt_signal_t handler);

_WLIBC_END_DECLS

#endif
