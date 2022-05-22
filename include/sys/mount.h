/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_MOUNT_H
#define WLIBC_SYS_MOUNT_H

#include <wlibc.h>
#include <sys/statfs.h>

#define MNT_WAIT   0
#define MNT_NOWAIT 0

WLIBC_API int wlibc_getmntinfo(struct statfs **mounts, int mode /* unused */);

WLIBC_INLINE int getmntinfo(struct statfs **mounts, int mode)
{
	return wlibc_getmntinfo(mounts, mode);
}

#endif
