/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/registry.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int wlibc_gethostname(char *name, size_t length)
{
	if (name == NULL || length <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	size_t size = 0;
	void *data = NULL;
	UNICODE_STRING u16_hostname;
	UTF8_STRING u8_hostname;

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

	u16_hostname.Length = size;
	u16_hostname.MaximumLength = size;
	u16_hostname.Buffer = data;

	RtlUnicodeStringToUTF8String(&u8_hostname, &u16_hostname, TRUE);
	size = u8_hostname.MaximumLength;
	free(data);

	if (length < size)
	{
		errno = EINVAL;
		RtlFreeUTF8String(&u8_hostname);
		return -1;
	}

	memcpy(name, u8_hostname.Buffer, size);
	RtlFreeUTF8String(&u8_hostname);
	return 0;
}
