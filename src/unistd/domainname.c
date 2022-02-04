/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/registry.h>
#include <stdlib.h>
#include <unistd.h>

int wlibc_getdomainname(char *name, size_t length)
{
	if (name == NULL || length <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	NTSTATUS status;
	UNICODE_STRING u16_domainname;
	UTF8_STRING u8_domainname;
	size_t size = 0;
	void *data = NULL;

	data = get_registry_value(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", L"Domain", &size);
	if (data == NULL)
	{
		// Fallback to 'localdomain'
		if (length < 12)
		{
			errno = EINVAL;
			return -1;
		}

		memcpy(name, "localdomain", 12);
		return 0;
	}

	u16_domainname.Length = (USHORT)size;
	u16_domainname.MaximumLength = (USHORT)size;
	u16_domainname.Buffer = data;

	u8_domainname.Buffer = name;
	u8_domainname.MaximumLength = (USHORT)length;
	status = RtlUnicodeStringToUTF8String(&u8_domainname, &u16_domainname, FALSE);
	free(data);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
