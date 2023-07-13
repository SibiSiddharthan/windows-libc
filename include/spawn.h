/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SPAWN_H
#define WLIBC_SPAWN_H

#include <wlibc.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

/* Data structure to contain attributes for process creation.  */
typedef struct _spawnattr_t
{
	short int flags;
	pid_t pgroup;
	sigset_t sigdefault;
	sigset_t sigmask;
	int schedpolicy;
	int schedpriority;
} spawnattr_t;

typedef spawnattr_t posix_spawnattr_t;

/* Data structure to contain information about the actions to be
   performed in the new process with respect to file descriptors.  */

typedef enum _spawn_action_type
{
	open_action,
	close_action,
	dup2_action,
	chdir_action,
	fchdir_action
} spawn_action_type;

struct spawn_action
{
	spawn_action_type type;

	union {
		struct
		{
			int fd;
			int oflag;
			mode_t mode;
			int length;
			char *path;
		} open_action;

		struct
		{
			int fd;
		} close_action;

		struct
		{
			int oldfd;
			int newfd;
		} dup2_action;

		struct
		{
			int length;
			char *path;
		} chdir_action;

		struct
		{
			int fd;
		} fchdir_action;
	};
};

typedef struct _spawn_actions_t
{
	unsigned short size;
	unsigned short used;
	struct spawn_action *actions;
} spawn_actions_t;

typedef spawn_actions_t posix_spawn_file_actions_t;

// Flags to be set in the posix_spawnattr_t.
#define POSIX_SPAWN_RESETIDS      0x0 // Unsupported
#define POSIX_SPAWN_USEVFORK      0x0 // Unsupported
#define POSIX_SPAWN_SETSID        0x0 // Unsupported
#define POSIX_SPAWN_SETPGROUP     0x01
#define POSIX_SPAWN_SETSIGDEF     0x02
#define POSIX_SPAWN_SETSIGMASK    0x04
#define POSIX_SPAWN_SETSCHEDPARAM 0x08
#define POSIX_SPAWN_SETSCHEDULER  0x10
#define POSIX_SPAWN_DETACH        0x20

// Spawn API.
WLIBC_API int wlibc_common_spawn(pid_t *restrict pid, const char *restrict path, const spawn_actions_t *restrict actions,
								 const spawnattr_t *restrict attributes, int use_path, char *restrict const argv[],
								 char *restrict const env[]);

WLIBC_INLINE int posix_spawn(pid_t *restrict pid, const char *restrict path, const posix_spawn_file_actions_t *restrict actions,
							 const posix_spawnattr_t *restrict attributes, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_common_spawn(pid, path, actions, attributes, 0, argv, env);
}
WLIBC_INLINE int posix_spawnp(pid_t *restrict pid, const char *restrict path, const posix_spawn_file_actions_t *restrict actions,
							  const posix_spawnattr_t *restrict attributes, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_common_spawn(pid, path, actions, attributes, 1, argv, env);
}

// Spawn attributes.
WLIBC_API int wlibc_spawnattr_init(spawnattr_t *attributes);
WLIBC_API int wlibc_spawnattr_getflags(const spawnattr_t *restrict attributes, short int *restrict flags);
WLIBC_API int wlibc_spawnattr_setflags(spawnattr_t *attributes, short int flags);
WLIBC_API int wlibc_spawnattr_getsigdefault(const spawnattr_t *restrict attributes, sigset_t *restrict sigdefault);
WLIBC_API int wlibc_spawnattr_setsigdefault(spawnattr_t *restrict attributes, const sigset_t *restrict sigdefault);
WLIBC_API int wlibc_spawnattr_getsigmask(const spawnattr_t *restrict attributes, sigset_t *restrict sigmask);
WLIBC_API int wlibc_spawnattr_setsigmask(spawnattr_t *restrict attributes, const sigset_t *restrict sigmask);
WLIBC_API int wlibc_spawnattr_getpgroup(const spawnattr_t *restrict attributes, pid_t *restrict pgroup);
WLIBC_API int wlibc_spawnattr_setpgroup(spawnattr_t *attributes, pid_t pgroup);
WLIBC_API int wlibc_spawnattr_getschedpolicy(const spawnattr_t *restrict attributes, int *restrict schedpolicy);
WLIBC_API int wlibc_spawnattr_setschedpolicy(spawnattr_t *attributes, int schedpolicy);
WLIBC_API int wlibc_spawnattr_getschedparam(const spawnattr_t *restrict attributes, struct sched_param *restrict schedparam);
WLIBC_API int wlibc_spawnattr_setschedparam(spawnattr_t *restrict attributes, const struct sched_param *restrict schedparam);

WLIBC_INLINE int posix_spawnattr_init(posix_spawnattr_t *attributes)
{
	return wlibc_spawnattr_init(attributes);
}

WLIBC_INLINE int posix_spawnattr_destroy(posix_spawnattr_t *attributes)
{
	if (attributes == NULL)
	{
		return -1;
	}
	return 0;
}

WLIBC_INLINE int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attributes, short int *restrict flags)
{
	return wlibc_spawnattr_getflags(attributes, flags);
}

WLIBC_INLINE int posix_spawnattr_setflags(posix_spawnattr_t *attributes, short int flags)
{
	return wlibc_spawnattr_setflags(attributes, flags);
}

WLIBC_INLINE int posix_spawnattr_getsigdefault(const posix_spawnattr_t *restrict attributes, sigset_t *restrict sigdefault)
{
	return wlibc_spawnattr_getsigdefault(attributes, sigdefault);
}

WLIBC_INLINE int posix_spawnattr_setsigdefault(posix_spawnattr_t *restrict attributes, const sigset_t *restrict sigdefault)
{
	return wlibc_spawnattr_setsigdefault(attributes, sigdefault);
}

WLIBC_INLINE int posix_spawnattr_getsigmask(const posix_spawnattr_t *restrict attributes, sigset_t *restrict sigmask)
{
	return wlibc_spawnattr_getsigmask(attributes, sigmask);
}

WLIBC_INLINE int posix_spawnattr_setsigmask(posix_spawnattr_t *restrict attributes, const sigset_t *restrict sigmask)
{
	return wlibc_spawnattr_setsigmask(attributes, sigmask);
}

WLIBC_INLINE int posix_spawnattr_getpgroup(const posix_spawnattr_t *restrict attributes, pid_t *restrict pgroup)
{
	return wlibc_spawnattr_getpgroup(attributes, pgroup);
}

WLIBC_INLINE int posix_spawnattr_setpgroup(posix_spawnattr_t *attributes, pid_t pgroup)
{
	return wlibc_spawnattr_setpgroup(attributes, pgroup);
}

WLIBC_INLINE int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *restrict attributes, int *restrict schedpolicy)
{
	return wlibc_spawnattr_getschedpolicy(attributes, schedpolicy);
}

WLIBC_INLINE int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attributes, int schedpolicy)
{
	return wlibc_spawnattr_setschedpolicy(attributes, schedpolicy);
}

WLIBC_INLINE int posix_spawnattr_getschedparam(const posix_spawnattr_t *restrict attributes, struct sched_param *restrict schedparam)
{
	return wlibc_spawnattr_getschedparam(attributes, schedparam);
}

WLIBC_INLINE int posix_spawnattr_setschedparam(posix_spawnattr_t *restrict attributes, const struct sched_param *restrict schedparam)
{
	return wlibc_spawnattr_setschedparam(attributes, schedparam);
}

// Spawn actions.
WLIBC_API int wlibc_spawn_file_actions_init(spawn_actions_t *actions);
WLIBC_API int wlibc_spawn_file_actions_destroy(spawn_actions_t *actions);
WLIBC_API int wlibc_spawn_file_actions_addopen(spawn_actions_t *actions, int fd, const char *restrict path, int oflag, mode_t mode);
WLIBC_API int wlibc_spawn_file_actions_addclose(spawn_actions_t *actions, int fd);
WLIBC_API int wlibc_spawn_file_actions_adddup2(spawn_actions_t *actions, int oldfd, int newfd);
WLIBC_API int wlibc_spawn_file_actions_addchdir(spawn_actions_t *restrict actions, const char *restrict path);
WLIBC_API int wlibc_spawn_file_actions_addfchdir(spawn_actions_t *actions, int fd);

WLIBC_INLINE int posix_spawn_file_actions_init(posix_spawn_file_actions_t *actions)
{
	return wlibc_spawn_file_actions_init(actions);
}

WLIBC_INLINE int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *actions)
{
	return wlibc_spawn_file_actions_destroy(actions);
}

WLIBC_INLINE int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *restrict actions, int fd, const char *restrict path,
												  int oflag, mode_t mode)
{
	return wlibc_spawn_file_actions_addopen(actions, fd, path, oflag, mode);
}

WLIBC_INLINE int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *actions, int fd)
{
	return wlibc_spawn_file_actions_addclose(actions, fd);
}

WLIBC_INLINE int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *actions, int oldfd, int newfd)
{
	return wlibc_spawn_file_actions_adddup2(actions, oldfd, newfd);
}

WLIBC_INLINE int posix_spawn_file_actions_addchdir(posix_spawn_file_actions_t *restrict actions, const char *restrict path)
{
	return wlibc_spawn_file_actions_addchdir(actions, path);
}

WLIBC_INLINE int posix_spawn_file_actions_addfchdir(posix_spawn_file_actions_t *actions, int fd)
{
	return wlibc_spawn_file_actions_addfchdir(actions, fd);
}

#define posix_spawn_file_actions_addchdir_np  posix_spawn_file_actions_addchdir
#define posix_spawn_file_actions_addfchdir_np posix_spawn_file_actions_addfchdir

_WLIBC_END_DECLS

#endif
