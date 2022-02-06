/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_PATH_INTERNAL_H
#define WLIBC_PATH_INTERNAL_H

#include <internal/nt.h>
#include <sys/types.h>

// \Device\HarddiskVolume1\Windows\System32
UNICODE_STRING *xget_absolute_ntpath(int dirfd, const char *path);
// C:\Windows\System32
//wchar_t *xget_absolute_dospath(int dirfd, const char *path);
UNICODE_STRING *xget_fd_dospath(int fd);

#endif
