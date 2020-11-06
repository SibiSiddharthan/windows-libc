/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <misc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl_internal.h>

int common_chown(const wchar_t *wname, uid_t owner, gid_t group, int do_lchown)
{
	return 0; // TODO
}

int wlibc_chown(const char *name, uid_t owner, gid_t group)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_chown(wname, owner, group, 0);
	free(wname);

	return status;
}

int wlibc_wchown(const wchar_t *wname, uid_t owner, gid_t group)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_chown(wname, owner, group, 0);
}

int wlibc_lchown(const char *name, uid_t owner, gid_t group)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_chown(wname, owner, group, 1);
	free(wname);

	return status;
}

int wlibc_wlchown(const wchar_t *wname, uid_t owner, gid_t group)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_chown(wname, owner, group, 1);
}

int wlibc_fchown(int fd, uid_t owner, gid_t group)
{
	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	const wchar_t *wname = get_fd_path(fd);
	return common_chown(wname, owner, group, 0);
}