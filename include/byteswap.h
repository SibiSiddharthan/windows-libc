/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_BYTESWAP_H
#define WLIBC_BYTESWAP_H

// clang-format off

// Given an unsigned N-bit argument X, reverse the byte order and return.

// N = 16
#define bswap_16(x) ((((x) & 0x00ffui16) << 8) | \
                     (((x) & 0xff00ui16) >> 8))

// N = 32
#define bswap_32(x) ((((x) & 0x000000ffui32) << 24) | \
                     (((x) & 0x0000ff00ui32) << 8)  | \
                     (((x) & 0x00ff0000ui32) >> 8)  | \
                     (((x) & 0xff000000ui32) >> 24))

// N = 64
#define bswap_64(x) ((((x) & 0x00000000000000ffui64) << 56) | \
                     (((x) & 0x000000000000ff00ui64) << 40) | \
                     (((x) & 0x0000000000ff0000ui64) << 24) | \
                     (((x) & 0x00000000ff000000ui64) << 8)  | \
                     (((x) & 0x000000ff00000000ui64) >> 8)  | \
                     (((x) & 0x0000ff0000000000ui64) >> 24) | \
                     (((x) & 0x00ff000000000000ui64) >> 40) | \
                     (((x) & 0xff00000000000000ui64) >> 56))

// clang-format on

#endif
