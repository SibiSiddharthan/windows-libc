/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SCHED_INTERNAL_H
#define WLIBC_SCHED_INTERNAL_H

#include <sched.h>

#define VALIDATE_SCHED_POLICY(policy, error, ret)                                                                                \
	if (policy != SCHED_IDLE && policy != SCHED_RR && policy != SCHED_FIFO && policy != SCHED_BATCH && policy != SCHED_SPORADIC) \
	{                                                                                                                            \
		errno = error;                                                                                                           \
		return ret;                                                                                                              \
	}

#define VALIDATE_SCHED_PRIORITY(priority, error, ret)                       \
	if ((priority) > SCHED_MAX_PRIORITY || (priority) < SCHED_MIN_PRIORITY) \
	{                                                                       \
		errno = error;                                                      \
		return ret;                                                         \
	}

#endif
