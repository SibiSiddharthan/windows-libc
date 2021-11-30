/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <strings.h>
#include <intrin.h>

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)

int wlibc_ffs32(int bytes_32)
{
	unsigned long index;
	if (_BitScanForward(&index, bytes_32) == 1)
	{
		// LSB is 1
		return index + 1;
	}
	return 0;
}

int wlibc_ffs64(long long int bytes_64)
{
	unsigned long index;
	if (_BitScanForward64(&index, bytes_64) == 1)
	{
		// LSB is 1
		return index + 1;
	}
	return 0;
}
