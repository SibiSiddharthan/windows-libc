/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/registry.h>
#include <internal/validate.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>

static void u16_value_to_u8_value(char *destination, wchar_t *source, size_t size)
{
	NTSTATUS status;
	UTF8_STRING u8_data;
	UNICODE_STRING u16_data;

	u16_data.Length = (USHORT)size;
	u16_data.MaximumLength = (USHORT)size;
	u16_data.Buffer = source;

	u8_data.MaximumLength = WLIBC_UTSNAME_LENGTH;
	u8_data.Buffer = destination;

	status = RtlUnicodeStringToUTF8String(&u8_data, &u16_data, FALSE);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, source);

	// If we encounter an error in converting UTF-16 to UTF-8(most likely insufficient buffer space.), set errno.
	if(status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
	}
}

int wlibc_uname(struct utsname *name)
{
	VALIDATE_PTR(name, EINVAL, -1);

	NTSTATUS status;
	size_t size;

	// sysname;
	wchar_t *sysname = get_registry_value(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName", &size);
	if (sysname == NULL)
	{
		// errno will be set by get_registry_value
		return -1;
	}
	u16_value_to_u8_value(name->sysname, sysname, size);

	// version
	wchar_t *version =
		get_registry_value(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"DisplayVersion", &size);
	if (version == NULL)
	{
		// errno will be set by get_registry_value
		return -1;
	}
	u16_value_to_u8_value(name->version, version, size);

	// version
	wchar_t *release = get_registry_value(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"BuildLabEx", &size);
	if (release == NULL)
	{
		// errno will be set by get_registry_value
		return -1;
	}
	u16_value_to_u8_value(name->release, release, size);

	// machine
	char *machine = "";
	SYSTEM_PROCESSOR_INFORMATION proc_info;
	status = NtQuerySystemInformation(SystemProcessorInformation, &proc_info, sizeof(SYSTEM_PROCESSOR_INFORMATION), NULL);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	switch (proc_info.ProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_MIPS:
		machine = "mips";
		break;
	case PROCESSOR_ARCHITECTURE_ALPHA:
	case PROCESSOR_ARCHITECTURE_ALPHA64:
		machine = "alpha";
		break;
	case PROCESSOR_ARCHITECTURE_PPC:
		machine = "powerpc";
		break;
	case PROCESSOR_ARCHITECTURE_SHX:
		machine = "sh";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		machine = "ia64";
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
		machine = "x86";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		machine = "x86_64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		machine = "arm";
		break;
	case PROCESSOR_ARCHITECTURE_ARM64:
		machine = "arm64";
		break;
	default:
		machine = "Unknown";
	}
	strcpy(name->machine, machine);

	// nodename
	status = wlibc_gethostname(name->nodename, WLIBC_UTSNAME_LENGTH);
	if (status == -1)
	{
		// errno will be set by wlibc_gethostname
		return -1;
	}

	// dommainname
	status = wlibc_getdomainname(name->domainname, WLIBC_UTSNAME_LENGTH);
	if (status == -1)
	{
		// errno will be set by wlibc_getdomainname
		return -1;
	}

	return 0;
}
