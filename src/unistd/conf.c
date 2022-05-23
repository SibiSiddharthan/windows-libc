/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/param.h>
#include <unistd.h>

long wlibc_common_pathconf(int name)
{
	switch (name)
	{
	case _PC_LINK_MAX:
		return 1024;
	case _PC_MAX_CANON:
		return 255;
	case _PC_MAX_INPUT:
		return 255;
	case _PC_NAME_MAX:
		return 255;
	case _PC_PATH_MAX:
		return MAXPATHLEN;
	case _PC_PIPE_BUF:
		return 16384;
	case _PC_CHOWN_RESTRICTED:
		return 1;
	case _PC_NO_TRUNC:
		return 1;
	case _PC_VDISABLE:
		return 1;
	default:
		errno = EINVAL;
		return -1;
	}
}

long wlibc_sysconf(int name)
{
	switch (name)
	{
	case _SC_ARG_MAX:
		return 32768;
	case _SC_CHILD_MAX:
		return 1024;
	case _SC_CLK_TCK:
		return 1000; // See time.h
	case _SC_NGROUPS_MAX:
		return NOGROUP;
	case _SC_OPEN_MAX:
		return NOFILE;
	case _SC_STREAM_MAX:
		return 8192;
	case _SC_PAGESIZE:
		return wlibc_getpagesize();
	case _SC_SYMLOOP_MAX:
		return MAXSYMLINKS;
	case _SC_HOST_NAME_MAX:
		return MAXHOSTNAMELEN;
	default:
		errno = EINVAL;
		return -1;
	}
}
