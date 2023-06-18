/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <sys/acl.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

acl_t wlibc_acl_init(int count)
{
	if (count < 0)
	{
		errno = EINVAL;
		return NULL;
	}

	// Allocate enough space for atleast 1 ACE.
	ULONG size = offsetof(struct _wlibc_acl_t, entries) + (count != 0 ? count : 1) * sizeof(struct _wlibc_acl_entry_t);
	PACL acl = (PACL)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, size);

	if (acl == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	// This will succeed.
	RtlCreateAcl(acl, size, ACL_REVISION);

	return (acl_t)acl;
}

acl_t wlibc_acl_dup(const acl_t acl)
{
	VALIDATE_ACL(acl, NULL);

	acl_t dup_acl = (acl_t)RtlAllocateHeap(NtCurrentProcessHeap(), 0, acl->size);
	if (dup_acl == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memcpy(dup_acl, acl, acl->size);

	return dup_acl;
}

int wlibc_acl_free(acl_t acl)
{
	VALIDATE_ACL(acl, -1);

	if (RtlFreeHeap(NtCurrentProcessHeap(), 0, acl) == FALSE)
	{
		return -1;
	}

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
	for (WORD i = 0; i < acl->count; ++i)
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
