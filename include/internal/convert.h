/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_CONVERT_INTERNAL_H
#define WLIBC_CONVERT_INTERNAL_H

#include <internal/nt.h>
#include <internal/buffer.h>
#include <sys/types.h>

#include <stdint.h>
#include <time.h>

struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT);
LARGE_INTEGER timespec_to_LARGE_INTEGER(const struct timespec *time);

// Flags
#define CONVERT_FORCE_SIGN   0x10
#define CONVERT_GROUP_DIGITS 0x40

#define TO_LOWER(x) ((x) | 0x20)

uint32_t uint_to_hex_common(uint8_t buffer[32], uint8_t upper, uintmax_t x);
uint32_t uint_from_hex_common(buffer_t *buffer, uintmax_t *value);

static inline uint32_t u8_to_hex(uint8_t buffer[32], uint8_t upper, uint8_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t u16_to_hex(uint8_t buffer[32], uint8_t upper, uint16_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t u32_to_hex(uint8_t buffer[32], uint8_t upper, uint32_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t u64_to_hex(uint8_t buffer[32], uint8_t upper, uint64_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t umax_to_hex(uint8_t buffer[32], uint8_t upper, uintmax_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t usize_to_hex(uint8_t buffer[32], uint8_t upper, size_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t uptr_to_hex(uint8_t buffer[32], uint8_t upper, uintptr_t x)
{
	return uint_to_hex_common(buffer, upper, x);
}

static inline uint32_t u8_from_hex(buffer_t *buffer, uint8_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uint8_t)result;

	return count;
}

static inline uint32_t u16_from_hex(buffer_t *buffer, uint16_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uint16_t)result;

	return count;
}

static inline uint32_t u32_from_hex(buffer_t *buffer, uint32_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uint32_t)result;

	return count;
}

static inline uint32_t u64_from_hex(buffer_t *buffer, uint64_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uint64_t)result;

	return count;
}

static inline uint32_t umax_from_hex(buffer_t *buffer, uintmax_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uintmax_t)result;

	return count;
}

static inline uint32_t usize_from_hex(buffer_t *buffer, size_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (size_t)result;

	return count;
}

static inline uint32_t uptr_from_hex(buffer_t *buffer, uintptr_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_hex_common(buffer, &result);
	*value = (uintptr_t)result;

	return count;
}

uint32_t uint_to_oct_common(uint8_t buffer[32], uintmax_t x);
uint32_t uint_from_oct_common(buffer_t *buffer, uintmax_t *value);

static inline uint32_t u8_to_oct(uint8_t buffer[32], uint8_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t u16_to_oct(uint8_t buffer[32], uint16_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t u32_to_oct(uint8_t buffer[32], uint32_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t u64_to_oct(uint8_t buffer[32], uint64_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t umax_to_oct(uint8_t buffer[32], uintmax_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t usize_to_oct(uint8_t buffer[32], size_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t uptr_to_oct(uint8_t buffer[32], uintptr_t x)
{
	return uint_to_oct_common(buffer, x);
}

static inline uint32_t u8_from_oct(buffer_t *buffer, uint8_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uint8_t)result;

	return count;
}

static inline uint32_t u16_from_oct(buffer_t *buffer, uint16_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uint16_t)result;

	return count;
}

static inline uint32_t u32_from_oct(buffer_t *buffer, uint32_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uint32_t)result;

	return count;
}

static inline uint32_t u64_from_oct(buffer_t *buffer, uint64_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uint64_t)result;

	return count;
}

static inline uint32_t umax_from_oct(buffer_t *buffer, uintmax_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uintmax_t)result;

	return count;
}

static inline uint32_t usize_from_oct(buffer_t *buffer, size_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (size_t)result;

	return count;
}

static inline uint32_t uptr_from_oct(buffer_t *buffer, uintptr_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_oct_common(buffer, &result);
	*value = (uintptr_t)result;

	return count;
}

uint32_t uint_to_bin_common(uint8_t buffer[64], uintmax_t x);
uint32_t uint_from_bin_common(buffer_t *buffer, uintmax_t *value);

static inline uint32_t u8_to_bin(uint8_t buffer[64], uint8_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t u16_to_bin(uint8_t buffer[64], uint16_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t u32_to_bin(uint8_t buffer[64], uint32_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t u64_to_bin(uint8_t buffer[64], uint64_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t umax_to_bin(uint8_t buffer[32], uintmax_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t usize_to_bin(uint8_t buffer[32], size_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t uptr_to_bin(uint8_t buffer[32], uintptr_t x)
{
	return uint_to_bin_common(buffer, x);
}

static inline uint32_t u8_from_bin(buffer_t *buffer, uint8_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uint8_t)result;

	return count;
}

static inline uint32_t u16_from_bin(buffer_t *buffer, uint16_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uint16_t)result;

	return count;
}

static inline uint32_t u32_from_bin(buffer_t *buffer, uint32_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uint32_t)result;

	return count;
}

static inline uint32_t u64_from_bin(buffer_t *buffer, uint64_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uint64_t)result;

	return count;
}

static inline uint32_t umax_from_bin(buffer_t *buffer, uintmax_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uintmax_t)result;

	return count;
}

static inline uint32_t usize_from_bin(buffer_t *buffer, size_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (size_t)result;

	return count;
}

static inline uint32_t uptr_from_bin(buffer_t *buffer, uintptr_t *value)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_bin_common(buffer, &result);
	*value = (uintptr_t)result;

	return count;
}

uint32_t uint_to_dec_common(uint8_t buffer[32], uintmax_t x, uint32_t flags);
uint32_t uint_from_dec_common(buffer_t *buffer, uintmax_t *value, uint32_t flags);

static inline uint32_t u8_to_dec(uint8_t buffer[32], uint8_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t u16_to_dec(uint8_t buffer[32], uint16_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t u32_to_dec(uint8_t buffer[32], uint32_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t u64_to_dec(uint8_t buffer[32], uint64_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t umax_to_dec(uint8_t buffer[32], uintmax_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t usize_to_dec(uint8_t buffer[32], size_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t uptr_to_dec(uint8_t buffer[32], uintptr_t x, uint32_t flags)
{
	return uint_to_dec_common(buffer, x, flags);
}

static inline uint32_t u8_from_dec(buffer_t *buffer, uint8_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uint8_t)result;

	return count;
}

static inline uint32_t u16_from_dec(buffer_t *buffer, uint16_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uint16_t)result;

	return count;
}

static inline uint32_t u32_from_dec(buffer_t *buffer, uint32_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uint32_t)result;

	return count;
}

static inline uint32_t u64_from_dec(buffer_t *buffer, uint64_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uint64_t)result;

	return count;
}

static inline uint32_t umax_from_dec(buffer_t *buffer, uintmax_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uintmax_t)result;

	return count;
}

static inline uint32_t usize_from_dec(buffer_t *buffer, size_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (size_t)result;

	return count;
}

static inline uint32_t uptr_from_dec(buffer_t *buffer, uintptr_t *value, uint32_t flags)
{
	uintmax_t result = 0;
	uint32_t count = 0;

	count = uint_from_dec_common(buffer, &result, flags);
	*value = (uintptr_t)result;

	return count;
}

uint32_t int_to_dec_common(uint8_t buffer[32], intmax_t x, uint32_t flags);
uint32_t int_from_dec_common(buffer_t *buffer, intmax_t *value, uint32_t flags);

static inline uint32_t i8_to_dec(uint8_t buffer[32], int8_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t i16_to_dec(uint8_t buffer[32], int16_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t i32_to_dec(uint8_t buffer[32], int32_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t i64_to_dec(uint8_t buffer[32], int64_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t imax_to_dec(uint8_t buffer[32], intmax_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t isize_to_dec(uint8_t buffer[32], ssize_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t iptr_to_dec(uint8_t buffer[32], intptr_t x, uint32_t flags)
{
	return int_to_dec_common(buffer, x, flags);
}

static inline uint32_t i8_from_dec(buffer_t *buffer, int8_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (int8_t)result;

	return count;
}

static inline uint32_t i16_from_dec(buffer_t *buffer, int16_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (int16_t)result;

	return count;
}

static inline uint32_t i32_from_dec(buffer_t *buffer, int32_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (int32_t)result;

	return count;
}

static inline uint32_t i64_from_dec(buffer_t *buffer, int64_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (int64_t)result;

	return count;
}

static inline uint32_t imax_from_dec(buffer_t *buffer, intmax_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (intmax_t)result;

	return count;
}

static inline uint32_t isize_from_dec(buffer_t *buffer, ssize_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (ssize_t)result;

	return count;
}

static inline uint32_t iptr_from_dec(buffer_t *buffer, intptr_t *value, uint32_t flags)
{
	intmax_t result = 0;
	uint32_t count = 0;

	count = int_from_dec_common(buffer, &result, flags);
	*value = (intptr_t)result;

	return count;
}

uint32_t float_from_hex_common(buffer_t *buffer, double *value);

static inline uint32_t float32_from_hex(buffer_t *buffer, float *value)
{
	uint32_t count = 0;
	double result = 0;

	count = float_from_hex_common(buffer, &result);
	*value = (float)result;

	return count;
}

uint32_t float32_to_hex(uint8_t buffer[64], uint8_t upper, float x);
uint32_t float64_to_hex(uint8_t buffer[64], uint8_t upper, double x);

static inline uint32_t float64_from_hex(buffer_t *buffer, double *value)
{
	return float_from_hex_common(buffer, value);
}

uint32_t float_from_normal_common(buffer_t *buffer, double *value, uint32_t flags);

static inline uint32_t float32_from_normal(buffer_t *buffer, float *value, uint32_t flags)
{
	uint32_t count = 0;
	double result = 0;

	count = float_from_normal_common(buffer, &result, flags);
	*value = (float)result;

	return count;
}

static inline uint32_t float64_from_normal(buffer_t *buffer, double *value, uint32_t flags)
{
	return float_from_normal_common(buffer, value, flags);
}

uint32_t pointer_encode(uint8_t buffer[32], void *ptr);
uint32_t pointer_decode(buffer_t *buffer, void **value);

uint32_t utf8_encode(uint8_t buffer[32], uint32_t codepoint);
uint32_t utf8_decode(void *buffer, uint8_t size, uint32_t *codepoint);

uint32_t utf16_encode(uint8_t buffer[32], uint32_t codepoint);
uint32_t utf16_decode(void *buffer, uint8_t size, uint32_t *codepoint);


#endif
