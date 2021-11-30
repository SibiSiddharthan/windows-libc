/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/random.h>
#include <sys/types.h>
#include <errno.h>
#include <immintrin.h>

#define RD_RAND 1 // random
#define RD_SEED 2 // entropy

#define MAX_LENGTH 1048576 // 1MB

/*
   Intrinsics don't play well with function pointers.
   Using macros works but clang throws incorrect errors
   Hence, do it the tedious way
*/

static ssize_t fill_buffer_using_rdrand(void *buffer, size_t length)
{
	size_t bytes_filled = 0;

	while (bytes_filled != length)
	{
		if (length - bytes_filled >= 8)
		{
			_rdrand64_step((unsigned long long *)((char *)buffer + bytes_filled));
			bytes_filled += 8;
			continue;
		}

		// We do not fill the last byte if lenght % 2 != 0
		// NOTE: filling the entire length of the buffer is not a requirement
		switch (length - bytes_filled)
		{
		case 7:
			_rdrand32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			_rdrand16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 6:
			_rdrand32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			_rdrand16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 5:
			_rdrand32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			break;
		case 4:
			_rdrand32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			break;
		case 3:
			_rdrand16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 2:
			_rdrand16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 1:
			break;
		}

		// break the loop after the switch is executed
		break;
	}

	return bytes_filled;
}

static ssize_t fill_buffer_using_rdseed(void *buffer, size_t length)
{
	size_t bytes_filled = 0;

	while (bytes_filled != length)
	{
		if (length - bytes_filled >= 8)
		{
			_rdseed64_step((unsigned long long *)((char *)buffer + bytes_filled));
			bytes_filled += 8;
			continue;
		}

		// We do not fill the last byte if lenght % 2 != 0
		// NOTE: filling the entire length of the buffer is not a requirement
		switch (length - bytes_filled)
		{
		case 7:
			_rdseed32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			_rdseed16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 6:
			_rdseed32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			_rdseed16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 5:
			_rdseed32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			break;
		case 4:
			_rdseed32_step((unsigned int *)((char *)buffer + bytes_filled));
			bytes_filled += 4;
			break;
		case 3:
			_rdseed16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 2:
			_rdseed16_step((unsigned short *)((char *)buffer + bytes_filled));
			bytes_filled += 2;
			break;
		case 1:
			break;
		}

		// break the loop after the switch is executed
		break;
	}

	return bytes_filled;
}

ssize_t wlibc_generate_random_bytes(void *buffer, size_t length, int source)
{
	size_t bytes_filled = 0;

	if (buffer == NULL || length > MAX_LENGTH)
	{
		errno = EINVAL;
		return -1;
	}

	if (source != RD_RAND && source != RD_SEED)
	{
		errno = EINVAL;
		return -1;
	}

	if (source == RD_RAND)
	{
		bytes_filled = fill_buffer_using_rdrand(buffer, length);
	}
	else // source == RD_SEED
	{
		bytes_filled = fill_buffer_using_rdseed(buffer, length);
	}

	return bytes_filled;
}
