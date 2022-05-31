/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <sys/acl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

acl_t wlibc_acl_init(int count)
{
	if (count < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	WORD max_entry_size = sizeof(struct _wlibc_acl_entry_t) + SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES);

	// Allocate enough space for atleast 1 ACE.
	// Allocate 76 bytes per ACE, maximum required.
	size_t size = offsetof(struct _wlibc_acl_t, entries) + (count != 0 ? count : 1) * max_entry_size;
	PACL acl = (PACL)malloc(size);
	RtlCreateAcl(acl, size, ACL_REVISION);
	return (acl_t)acl;
}

acl_t wlibc_acl_dup(const acl_t acl)
{
	VALIDATE_ACL(acl, NULL);

	size_t size = ((PACL)(acl))->AclSize;
	PACL dup_acl = (PACL)malloc(size);
	return (acl_t)dup_acl;
}

int wlibc_acl_free(acl_t acl)
{
	VALIDATE_ACL(acl, -1);

	free(acl);
	return 0;
}

int wlibc_acl_valid(const acl_t acl)
{
	VALIDATE_ACL(acl, -1);

	// Check whether the fields are correct
	if (acl->revision < MIN_ACL_REVISION || acl->revision > MAX_ACL_REVISION)
	{
		return -1;
	}
	// padding zeros
	if (acl->zero1 != 0 || acl->zero2 != 0)
	{
		return -1;
	}

	// Now iterate through the supposed ACES
	WORD acl_read = 0;
	for (int i = 0; i < acl->count; ++i)
	{
		struct _wlibc_acl_entry_t *entry = (struct _wlibc_acl_entry_t *)((char *)acl + offsetof(struct _wlibc_acl_t, entries) + acl_read);
		// Just validate the type and flags
		if (entry->type > ACCESS_MAX_MS_V5_ACE_TYPE || entry->flags > VALID_INHERIT_FLAGS)
		{
			return -1;
		}

		acl_read += entry->size;
	}

	return 0;
}
