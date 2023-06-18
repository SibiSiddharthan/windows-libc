/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/thread.h>
#include <thread.h>
#include <intrin.h>

#pragma intrinsic(_BitScanForward64)

#define VALIDATE_TLS_ENTRY(index, result)           \
	if ((_wlibc_tls_bitmap & (1ull << index)) == 0) \
	{                                               \
		return result;                              \
	}

int wlibc_tss_create(key_t *index, dtor_t destructor)
{
	// Find a free slot.
	if (_BitScanForward64(index, ~_wlibc_tls_bitmap) == 1)
	{
		_wlibc_tls_bitmap |= (1ull << *index);
		_wlibc_tls_destructors[*index] = destructor;
		return 0;
	}

	// No free slot found.
	return -1;
}

void *wlibc_tss_get(key_t index)
{
	VALIDATE_TLS_ENTRY(index, NULL);

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);
	return tinfo->slots[index].value;
}

int wlibc_tss_set(key_t index, const void *data)
{
	VALIDATE_TLS_ENTRY(index, -1);

	threadinfo *tinfo = TlsGetValue(_wlibc_threadinfo_index);
	tinfo->slots[index].value = (void *)data;
	return 0;
}

int wlibc_tss_delete(key_t index)
{
	VALIDATE_TLS_ENTRY(index, -1);

	_wlibc_tls_bitmap &= ~(1ull << index);
	_wlibc_tls_destructors[index] = NULL;
	return 0;
}
