/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/registry.h>
#include <stdlib.h>
#include <unistd.h>

int wlibc_gethostname(char *name, size_t length)
{
	if (name == NULL || length <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	NTSTATUS status;
	UNICODE_STRING u16_hostname;
	UTF8_STRING u8_hostname;
	size_t size = 0;
	void *data = NULL;

	// Both of the queries should be the same, prefer 'Hostname' to 'ComputerName'
	data = get_registry_value(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", L"HostName", &size);
	if (data == NULL)
	{
		data = get_registry_value(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName",
								  L"ComputerName", &size);
		if (data == NULL)
		{
			// errno will be set by get_registry_value
			return -1;
		}
	}

	u16_hostname.Length = (USHORT)size;
	u16_hostname.MaximumLength = (USHORT)size;
	u16_hostname.Buffer = data;

	u8_hostname.Buffer = name;
	u8_hostname.MaximumLength = (USHORT)length;
	status = RtlUnicodeStringToUTF8String(&u8_hostname, &u16_hostname, FALSE);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, data);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
