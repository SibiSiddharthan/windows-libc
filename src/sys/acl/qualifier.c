/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
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

	RtlCopySid(entry->size - offsetof(struct _wlibc_acl_entry_t, sid), &entry->sid, qualifier);
	// After copying the sid, set the size of the entry. The length of the sid will determine it.
	entry->size = offsetof(struct _wlibc_acl_entry_t, sid) + SECURITY_SID_SIZE(((PISID)qualifier)->SubAuthorityCount);

	return 0;
}
