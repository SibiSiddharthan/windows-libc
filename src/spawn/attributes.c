/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/sched.h>
#include <internal/validate.h>
#include <errno.h>
#include <spawn.h>

#define VALIDATE_SPAWNATTR(attributes) VALIDATE_PTR(attributes, EINVAL, -1)

int wlibc_spawnattr_init(spawnattr_t *attributes)
{
	VALIDATE_SPAWNATTR(attributes);

	attributes->flags = 0;
	// Unused currently. TODO.
	attributes->sigmask = 0;
	attributes->sigdefault = 0;
	attributes->pgroup = 0;
	attributes->schedpolicy = SCHED_RR;
	attributes->schedpriority = 0;

	return 0;
}

int wlibc_spawnattr_getsigdefault(const spawnattr_t *restrict attributes, sigset_t *restrict sigdefault)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(sigdefault, EINVAL, -1);

	*sigdefault = attributes->sigdefault;
	return 0;
}

int wlibc_spawnattr_setsigdefault(spawnattr_t *restrict attributes, const sigset_t *restrict sigdefault)
{
	VALIDATE_SPAWNATTR(attributes);

	attributes->sigdefault = *sigdefault;
	return 0;
}

int wlibc_spawnattr_getsigmask(const spawnattr_t *restrict attributes, sigset_t *restrict sigmask)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(sigmask, EINVAL, -1);

	*sigmask = attributes->sigmask;
	return 0;
}

int wlibc_spawnattr_setsigmask(spawnattr_t *restrict attributes, const sigset_t *restrict sigmask)
{
	VALIDATE_SPAWNATTR(attributes);

	attributes->sigmask = *sigmask;
	return 0;
}

int wlibc_spawnattr_getflags(const spawnattr_t *restrict attributes, short int *restrict flags)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(flags, EINVAL, -1);

	*flags = attributes->flags;
	return 0;
}

int wlibc_spawnattr_setflags(spawnattr_t *attributes, short int flags)
{
	VALIDATE_SPAWNATTR(attributes);
	if (flags > ((POSIX_SPAWN_RESETIDS | POSIX_SPAWN_SETSID | POSIX_SPAWN_USEVFORK | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGDEF |
				  POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER | POSIX_SPAWN_DETACH)))
	{
		errno = EINVAL;
		return -1;
	}

	attributes->flags = flags;
	return 0;
}

int wlibc_spawnattr_getpgroup(const spawnattr_t *restrict attributes, pid_t *restrict pgroup)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(pgroup, EINVAL, -1);

	*pgroup = attributes->pgroup;
	return 0;
}

int wlibc_spawnattr_setpgroup(spawnattr_t *attributes, pid_t pgroup)
{
	VALIDATE_SPAWNATTR(attributes);

	attributes->pgroup = pgroup;
	return 0;
}

int wlibc_spawnattr_getschedpolicy(const spawnattr_t *restrict attributes, int *restrict schedpolicy)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(schedpolicy, EINVAL, -1);

	*schedpolicy = attributes->schedpolicy;
	return 0;
}

int wlibc_spawnattr_setschedpolicy(spawnattr_t *attributes, int schedpolicy)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_SCHED_POLICY(schedpolicy, EINVAL, -1);

	attributes->schedpolicy = schedpolicy;
	return 0;
}

int wlibc_spawnattr_getschedparam(const spawnattr_t *restrict attributes, struct sched_param *restrict schedparam)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(schedparam, EINVAL, -1);

	schedparam->sched_priority = attributes->schedpriority;
	return 0;
}

int wlibc_spawnattr_setschedparam(spawnattr_t *restrict attributes, const struct sched_param *restrict schedparam)
{
	VALIDATE_SPAWNATTR(attributes);
	VALIDATE_PTR(schedparam, EINVAL, -1);
	VALIDATE_SCHED_PRIORITY(schedparam->sched_priority, EINVAL, -1);

	attributes->schedpriority = schedparam->sched_priority;
	return 0;
}
