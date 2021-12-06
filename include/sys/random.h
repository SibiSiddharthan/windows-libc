/*
   Copyright (c) 2020-2021 Sibi Siddharthan

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

WLIBC_INLINE ssize_t getrandom(void *buffer, size_t length, unsigned int flags /*unused*/)
{
	return wlibc_generate_random_bytes(buffer, length, 1);
}

WLIBC_INLINE int getentropy(void *buffer, size_t length)
{
	return (int)wlibc_generate_random_bytes(buffer, length, 2);
}

_WLIBC_END_DECLS

#endif
