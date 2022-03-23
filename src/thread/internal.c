/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/thread.h>
#include <stdlib.h>

DWORD _wlibc_threadinfo_index;
ULONGLONG _wlibc_tls_bitmap;
dtor_t _wlibc_tls_destructors[64];

void threads_init(void)
{
	// Initialize the bitmap to zero.
	_wlibc_tls_bitmap = 0;

	// Allocate the index.
	_wlibc_threadinfo_index = TlsAlloc();

	// Initialize the destructors.
	memset(_wlibc_tls_destructors, 0, sizeof(dtor_t) * 64);

	// Initialize the main thread's info structure.
	threadinfo *tinfo = (threadinfo *)malloc(sizeof(threadinfo));
	memset(tinfo, 0, sizeof(threadinfo));

	tinfo->handle = NtCurrentThread();
	tinfo->id = GetCurrentThreadId();

	// Put the threadinfo in the index we allocated.
	TlsSetValue(_wlibc_threadinfo_index, tinfo);
}

void threads_cleanup(void)
{
	threadinfo *tinfo = (threadinfo *)TlsGetValue(_wlibc_threadinfo_index);

	// Perform cleanup on the main thread.
	cleanup_tls(tinfo);

	// Free the main thread's info structure.
	free(tinfo);

	TlsFree(_wlibc_threadinfo_index);
}

void cleanup_tls(threadinfo *tinfo)
{
	if (_wlibc_tls_bitmap == 0)
	{
		// Quick return if no tls has been allocated.
		return;
	}

	// Iterate through all the active tls slots only once.
	// There is no point in repeating this procedure.
	for (int i = 0; i < 64; ++i)
	{
		// Check if tls slot is active.
		if (_wlibc_tls_bitmap & (1 << i))
		{
			// Check if value at slot is non zero (non NULL).
			if (tinfo->slots[i].value != NULL)
			{
				// Check if a destructor for the slot has been registered.
				if (_wlibc_tls_destructors[i] != NULL)
				{
					// Execute the destructor with the value at slot as its argument.
					_wlibc_tls_destructors[i](tinfo->slots[i].value);
				}
			}
		}
	}
}
