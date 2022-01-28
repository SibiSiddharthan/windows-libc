/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_PATH_INTERNAL_H
#define WLIBC_PATH_INTERNAL_H

#include <internal/nt.h>
#include <sys/types.h>
//#include <wchar.h>

// \Device\HarddiskVolume1\Windows\System32
UNICODE_STRING *xget_absolute_ntpath(int dirfd, const char *path);
// C:\Windows\System32
//wchar_t *get_absolute_dospath(int dirfd, const char *path);

void free_ntpath(UNICODE_STRING *ntpath);

#endif
