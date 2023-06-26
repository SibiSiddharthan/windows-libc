/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/time.h>
#include <unistd.h>

unsigned int wlibc_alarm(unsigned int seconds)
{
	unsigned int result;
	struct itimerval due = {0}, old = {0};

	// Cancel the timer first.
	setitimer(ITIMER_REAL, &due, &old);

	// Set the new timer.
	due.it_value.tv_sec = seconds;
	setitimer(ITIMER_REAL, &due, NULL);

	result = (unsigned int)old.it_value.tv_sec;

	return result;
}
