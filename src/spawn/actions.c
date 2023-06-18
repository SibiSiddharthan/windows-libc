/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/validate.h>
#include <errno.h>
#include <spawn.h>
#include <string.h>

#define VALIDATE_ACTIONS(actions) VALIDATE_PTR(actions, EINVAL, -1)

#define DOUBLE_ACTIONS_IF_NEEDED(actions)      \
	if (actions->used == actions->size)        \
	{                                          \
		if (double_the_actions(actions) == -1) \
		{                                      \
			return -1;                         \
		}                                      \
	}

#define SIMPLE_VALIDATE_FD(fd) \
	if (fd < 0)                \
	{                          \
		errno = EINVAL;        \
		return -1;             \
	}

static int double_the_actions(spawn_actions_t *actions)
{
	struct spawn_action *temp = (struct spawn_action *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, actions->actions,
																		 sizeof(struct spawn_action) * actions->size * 2);

	if (temp == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	actions->actions = temp;
	actions->size *= 2;

	return 0;
}

int wlibc_spawn_file_actions_init(spawn_actions_t *actions)
{
	VALIDATE_ACTIONS(actions);

	actions->size = 16;
	actions->used = 0;
	actions->actions = (struct spawn_action *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(struct spawn_action) * actions->size);

	if (actions->actions == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	return 0;
}

int wlibc_spawn_file_actions_destroy(spawn_actions_t *actions)
{
	VALIDATE_ACTIONS(actions);

	// Free the copied path buffers.
	for (int i = 0; i < actions->used; ++i)
	{
		if (actions->actions[i].type == open_action)
		{
			RtlFreeHeap(NtCurrentProcessHeap(), 0, actions->actions[i].open_action.path);
		}
		if (actions->actions[i].type == chdir_action)
		{
			RtlFreeHeap(NtCurrentProcessHeap(), 0, actions->actions[i].chdir_action.path);
		}
	}

	// Free the entire structure.
	RtlFreeHeap(NtCurrentProcessHeap(), 0, actions->actions);
	memset(actions, 0, sizeof(spawn_actions_t));

	return 0;
}

int wlibc_spawn_file_actions_addopen(spawn_actions_t *actions, int fd, const char *restrict path, int oflag, mode_t mode)
{
	VALIDATE_ACTIONS(actions);
	SIMPLE_VALIDATE_FD(fd);
	VALIDATE_PATH(path, EINVAL, -1);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	int length = (int)strlen(path);
	actions->actions[actions->used] = (struct spawn_action){.type = open_action, .open_action = {fd, oflag, mode, length, NULL}};
	actions->actions[actions->used].open_action.path = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, length + 1);

	if (actions->actions[actions->used].open_action.path == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	memcpy(actions->actions[actions->used].open_action.path, path, length + 1);

	++actions->used;

	return 0;
}

int wlibc_spawn_file_actions_addclose(spawn_actions_t *actions, int fd)
{
	VALIDATE_ACTIONS(actions);
	SIMPLE_VALIDATE_FD(fd);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	actions->actions[actions->used++] = (struct spawn_action){.type = close_action, .close_action = {fd}};
	return 0;
}

int wlibc_spawn_file_actions_adddup2(spawn_actions_t *actions, int oldfd, int newfd)
{
	VALIDATE_ACTIONS(actions);
	SIMPLE_VALIDATE_FD(oldfd);
	SIMPLE_VALIDATE_FD(newfd);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	actions->actions[actions->used++] = (struct spawn_action){.type = dup2_action, .dup2_action = {oldfd, newfd}};
	return 0;
}

int wlibc_spawn_file_actions_addchdir(spawn_actions_t *restrict actions, const char *restrict path)
{
	VALIDATE_ACTIONS(actions);
	VALIDATE_PATH(path, EINVAL, -1);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	int length = (int)strlen(path);
	actions->actions[actions->used] = (struct spawn_action){.type = chdir_action, .chdir_action = {length, NULL}};
	actions->actions[actions->used].chdir_action.path = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, length + 1);

	if (actions->actions[actions->used].chdir_action.path == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	memcpy(actions->actions[actions->used].chdir_action.path, path, length + 1);

	++actions->used;

	return 0;
}

int wlibc_spawn_file_actions_addfchdir(spawn_actions_t *actions, int fd)
{
	VALIDATE_ACTIONS(actions);
	SIMPLE_VALIDATE_FD(fd);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	actions->actions[actions->used++] = (struct spawn_action){.type = fchdir_action, .fchdir_action = {fd}};
	return 0;
}
