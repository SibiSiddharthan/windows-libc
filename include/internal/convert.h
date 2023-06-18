/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_CONVERT_INTERNAL_H
#define WLIBC_CONVERT_INTERNAL_H

#include <internal/nt.h>
#include <time.h>

struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT);
// struct timespec is greater than 8 bytes, pass it as pointer.
LARGE_INTEGER timespec_to_LARGE_INTEGER(const struct timespec *time);

#endif
