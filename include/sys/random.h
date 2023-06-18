/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_RANDOM_H
#define WLIBC_SYS_RANDOM_H

#include <wlibc.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

#define GRND_RANDOM   0 // Unsupported
#define GRND_NONBLOCK 0 // Unsupported

/*
#define RD_RAND 1 // random
#define RD_SEED 2 // entropy
*/

WLIBC_API ssize_t wlibc_generate_random_bytes(void *buffer, size_t length, int source);

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

WLIBC_INLINE ssize_t getrandom(void *buffer, size_t length, unsigned int flags WLIBC_UNUSED)
{
	return wlibc_generate_random_bytes(buffer, length, 1);
}

#pragma warning(pop)

WLIBC_INLINE int getentropy(void *buffer, size_t length)
{
	return (int)wlibc_generate_random_bytes(buffer, length, 2);
}

_WLIBC_END_DECLS

#endif
