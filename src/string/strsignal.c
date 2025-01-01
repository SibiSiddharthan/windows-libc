/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <string.h>
#include <signal.h>

const char *const sys_siglist[NSIG] = {"Hangup",
									   "Interrupt",
									   "Quit",
									   "Illegal instruction",
									   "Trap",
									   "Abnormal termination",
									   "Bus error",
									   "Floating point exception",
									   "Kill",
									   "User defined signal 1",
									   "Segment violation",
									   "User defined signal 2",
									   "Broken pipe",
									   "Alarm clock",
									   "Terminate",
									   "Stack fault ",
									   "Child terminated or stopped",
									   "Continue",
									   "Background read from control terminal",
									   "Background write to control terminal",
									   "Keyboard stop",
									   "Stop",
									   "Urgent data is available at a socket",
									   "CPU time limit exceeded",
									   "File size limit exceeded",
									   "Virtual timer expired",
									   "Profiling timer expired",
									   "Window size change",
									   "Pollable event",
									   "Power failure imminent",
									   "Bad syscall"};

char *wlibc_strsignal(int sig)
{
	if (sig >= NSIG)
	{
		return "Unkown signal";
	}

	return (char *)sys_siglist[sig];
}
