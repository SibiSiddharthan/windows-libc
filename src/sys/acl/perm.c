/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/acl.h>
#include <sys/acl.h>

int wlibc_acl_add_perm(acl_permset_t permset, acl_perm_t perm)
{
	VALIDATE_ACL_PERMSET(permset, -1);
	VALIDATE_ACL_PERMS(perm, -1);

	*permset |= perm;
	return 0;
}

int wlibc_acl_clear_perms(acl_permset_t permset)
{
	VALIDATE_ACL_PERMSET(permset, -1);

	*permset = 0;
	return 0;
}

int wlibc_acl_delete_perm(acl_permset_t permset, acl_perm_t perm)
{
	VALIDATE_ACL_PERMSET(permset, -1);
	VALIDATE_ACL_PERMS(perm, -1);

	*permset &= ~perm;
	return 0;
}

int wlibc_acl_get_perm(acl_permset_t permset, acl_perm_t perm)
{
	VALIDATE_ACL_PERMSET(permset, -1);
	VALIDATE_ACL_PERMS(perm, -1);

	return (*permset & perm) == perm;
}

int wlibc_acl_get_permset(acl_entry_t entry, acl_permset_t *permset)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_PTR(permset, EINVAL, -1);

	if (entry->type != ACL_EXTENDED_ALLOW && entry->type != ACL_EXTENDED_DENY)
	{
		// Unsupported ace type.
		errno = ENOTSUP;
		return -1;
	}

	*permset = &entry->mask;

	return 0;
}

int wlibc_acl_set_permset(acl_entry_t entry, acl_permset_t permset)
{
	VALIDATE_ACL_ENTRY(entry, -1);
	VALIDATE_ACL_PERMSET(permset, -1);
	VALIDATE_ACL_PERMS(*permset, -1);

	entry->mask = *permset;

	return 0;
}
