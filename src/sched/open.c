/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>

HANDLE open_process(DWORD pid, ACCESS_MASK access)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	CLIENT_ID client_id;
	HANDLE handle = 0;

	if (pid == 0)
	{
		// Return the current process.
		return NtCurrentProcess();
	}

	InitializeObjectAttributes(&object, NULL, 0, NULL, NULL);
	client_id.UniqueProcess = (HANDLE)(LONG_PTR)pid;
	client_id.UniqueThread = 0;

	status = NtOpenProcess(&handle, access, &object, &client_id);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return NULL;
	}

	// Successful handle creation.
	return handle;
}
