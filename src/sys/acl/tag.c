/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/acl.h>
#include <sys/acl.h>
#include <errno.h>

int wlibc_acl_get_tag_type(acl_entry_t entry, acl_tag_t *tag)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_PTR(tag, EINVAL, -1);

	// If the type is other than allow or deny, set tag to be undefined.
	// NOTE: This is only because we only support these 2 types. Other types if set
	// will work properly when copying, etc.
	// Just don't set tag when it is undefined.
	if(entry->type == ACL_EXTENDED_ALLOW || entry->type == ACL_EXTENDED_DENY)
	{
		*tag = entry->type;
	}
	else
	{
		*tag = ACL_UNDEFINED_TAG;
	}

	return 0;
}

int wlibc_acl_set_tag_type(acl_entry_t entry, acl_tag_t tag)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_ACL_TAG(tag, -1);

	entry->type = tag;
	return 0;
}
