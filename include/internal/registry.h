/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_REGISTRY_INTERNAL_H
#define WLIBC_REGISTRY_INTERNAL_H

#include <sys/types.h>
#include <wchar.h>

void *get_registry_value(const wchar_t *restrict basekey, const wchar_t *restrict subkey, size_t *restrict outsize);

#endif
