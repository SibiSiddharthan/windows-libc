/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/acl.h>
#include <sys/acl.h>

int wlibc_acl_add_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	VALIDATE_ACL_FLAGSET(flagset, -1);
	VALIDATE_ACL_FLAGS(flag, -1);

	*flagset |= flag;
	return 0;
}

int wlibc_acl_clear_flags(acl_flagset_t flagset)
{
	VALIDATE_ACL_FLAGSET(flagset, -1);

	*flagset = 0;
	return 0;
}

int wlibc_acl_delete_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	VALIDATE_ACL_FLAGSET(flagset, -1);
	VALIDATE_ACL_FLAGS(flag, -1);

	*flagset &= ~flag;
	return 0;
}

int wlibc_acl_get_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	VALIDATE_ACL_FLAGSET(flagset, -1);
	VALIDATE_ACL_FLAGS(flag, -1);

	return (*flagset & flag) == flag;
}

int wlibc_acl_get_flagset(acl_entry_t entry, acl_flagset_t *flagset)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_PTR(flagset, EINVAL, -1);

	*flagset = &entry->flags;

	return 0;
}

int wlibc_acl_set_flagset(acl_entry_t entry, acl_flagset_t flagset)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_ACL_FLAGSET(flagset, -1);
	VALIDATE_ACL_FLAGS(*flagset, -1);

	entry->flags = *flagset;

	return 0;
}
