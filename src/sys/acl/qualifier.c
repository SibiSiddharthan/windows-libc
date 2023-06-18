/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <internal/error.h>
#include <sys/acl.h>
#include <stddef.h>

acl_qualifier_t wlibc_acl_get_qualifier(acl_entry_t entry)
{
	VALIDATE_ACL_ENTRY(entry, NULL);

	return (acl_qualifier_t) & (entry->sid);
}

int wlibc_acl_set_qualifier(acl_entry_t entry, const acl_qualifier_t qualifier)
{
	VALIDATE_ACL_ENTRY(entry, -1);

	NTSTATUS status;

	status = RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), &entry->sid, qualifier);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}
