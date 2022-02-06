/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_INTERNAL_ERROR_H
#define WLIBC_INTERNAL_ERROR_H

#include <internal/nt.h>

void map_win32_error_to_wlibc(unsigned long error);
void map_ntstatus_to_errno(NTSTATUS status);

#endif
