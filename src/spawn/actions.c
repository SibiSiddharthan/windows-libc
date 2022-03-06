/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <spawn.h>
#include <string.h>
#include <stdlib.h>

#define VALIDATE_ACTIONS(actions) \
	if (actions == NULL)          \
	{                             \
		errno = EINVAL;           \
		return -1;                \
	}

#define DOUBLE_ACTIONS_IF_NEEDED(actions) \
	if (actions->used == actions->size)   \
	{                                     \
		double_the_actions(actions);      \
	}

#define SIMPLE_VALIDATE_FD(fd) \
	if (fd < 0)                \
	{                          \
		errno = EINVAL;        \
		return -1;             \
	}

#define SIMPLE_VALIDATE_PATH(path)       \
	if (path == NULL || path[0] == '\0') \
	{                                    \
		errno = EINVAL;                  \
		return -1;                       \
	}

static void double_the_actions(spawn_actions_t *actions)
{
	struct spawn_action *temp = (struct spawn_action *)malloc(sizeof(struct spawn_action) * actions->size * 2);
	memcpy(temp, actions->actions, sizeof(struct spawn_action) * actions->size);
	free(actions->actions);
	actions->actions = temp;
	actions->size *= 2;
}

int wlibc_spawn_file_actions_init(spawn_actions_t *actions)
{
	VALIDATE_ACTIONS(actions);

	actions->size = 16;
	actions->used = 0;
	actions->actions = (struct spawn_action *)malloc(sizeof(struct spawn_action) * actions->size);

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
			free(actions->actions[i].open_action.path);
		}
		if (actions->actions[i].type == chdir_action)
		{
			free(actions->actions[i].chdir_action.path);
		}
	}

	// Free the entire structure.
	free(actions->actions);
	memset(actions, 0, sizeof(spawn_actions_t));

	return 0;
}

int wlibc_spawn_file_actions_addopen(spawn_actions_t *actions, int fd, const char *restrict path, int oflag, mode_t mode)
{
	VALIDATE_ACTIONS(actions);
	SIMPLE_VALIDATE_FD(fd);
	SIMPLE_VALIDATE_PATH(path);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	int length = strlen(path);
	actions->actions[actions->used] = (struct spawn_action){.type = open_action, .open_action = {fd, oflag, mode, length, NULL}};
	actions->actions[actions->used].open_action.path = (char *)malloc(length + 1);
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
	SIMPLE_VALIDATE_PATH(path);
	DOUBLE_ACTIONS_IF_NEEDED(actions);

	int length = strlen(path);
	actions->actions[actions->used] = (struct spawn_action){.type = chdir_action, .chdir_action = {length, NULL}};
	actions->actions[actions->used].chdir_action.path = (char *)malloc(length + 1);
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
