/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/registry.h>
#include <internal/error.h>
#include <errno.h>

void *get_registry_value(const wchar_t *restrict basekey, const wchar_t *restrict subkey, size_t *restrict outsize)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES object;
	UNICODE_STRING keyname;
	UNICODE_STRING valuename;
	HANDLE key;
	PKEY_VALUE_FULL_INFORMATION value_info = NULL;
	ULONG length = 512;
	void *data = NULL;

	RtlInitUnicodeString(&keyname, basekey);
	InitializeObjectAttributes(&object, &keyname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtOpenKeyEx(&key, KEY_QUERY_VALUE, &object, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	// Start with 512 bytes
	value_info = (PKEY_VALUE_FULL_INFORMATION)RtlAllocateHeap(NtCurrentProcessHeap(), 0, 512);
	if (value_info == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	RtlInitUnicodeString(&valuename, subkey);

	status = NtQueryValueKey(key, &valuename, KeyValueFullInformation, value_info, length, &length);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)
		{
			value_info = (PKEY_VALUE_FULL_INFORMATION)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, value_info, length);
			if (value_info == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			status = NtQueryValueKey(key, &valuename, KeyValueFullInformation, value_info, length, &length);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}
		else
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}
	}

	if (value_info->DataLength > 2) // L'\0'
	{
		data = RtlAllocateHeap(NtCurrentProcessHeap(), 0, value_info->DataLength);
		if (data == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		memcpy(data, (char *)value_info + value_info->DataOffset, value_info->DataLength);
	}

	*outsize = value_info->DataLength;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, value_info);
	// data will be freed by user
	return data;
}
