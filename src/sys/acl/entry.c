/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/acl.h>

int wlibc_acl_copy_entry(acl_entry_t destination, acl_entry_t source)
{
	VALIDATE_ACL_ENTRY(destination, -1);
	VALIDATE_ACL_ENTRY(source, -1);

	memcpy(destination, source, sizeof(struct _wlibc_acl_entry_t));

	return 0;
}

int wlibc_acl_create_entry(acl_t *acl, acl_entry_t *entry)
{
	VALIDATE_PTR(acl, EINVAL, -1);
	VALIDATE_PTR(entry, EINVAL, -1);

	WORD used_size = offsetof(struct _wlibc_acl_t, entries) + (*acl)->count * sizeof(struct _wlibc_acl_entry_t);

	if ((*acl)->size - used_size < sizeof(struct _wlibc_acl_entry_t))
	{
		// Allocate more space.
		WORD allocated_size = sizeof(struct _wlibc_acl_entry_t) * (*acl)->count * 2;
		acl_t temp = (acl_t)RtlReAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, *acl, allocated_size);

		if (temp == NULL)
		{
			errno = ENOMEM;
			return -1;
		}

		*acl = temp;
		(*acl)->size = allocated_size;
	}

	// Store the address at the end of the previous entry.
	*entry = (struct _wlibc_acl_entry_t *)((char *)*acl + used_size);
	(*entry)->size = sizeof(struct _wlibc_acl_entry_t);

	// We have allocated a new entry, increment the count.
	++(*acl)->count;

	return 0;
}

int wlibc_acl_delete_entry(acl_t acl, acl_entry_t entry)
{
	VALIDATE_ACL(acl, -1);
	VALIDATE_ACL_ENTRY(entry, -1);

	// No aces in acl.
	if (acl->count == 0)
	{
		errno = ESRCH;
		return -1;
	}

	// Check whether the entry is part of the acl.
	WORD index = 0;
	for (WORD i = 0; i < acl->count; ++i)
	{

		if (entry == &(acl->entries[i]))
		{
			index = i;
			break;
		}

		// entry is not part of acl.
		if (i == acl->count - 1)
		{
			errno = ESRCH;
			return -1;
		}
	}

	// Last entry
	if (index == acl->count - 1)
	{
		// Just zero the entry.
		memset(&(acl->entries[acl->count - 1]), 0, sizeof(struct _wlibc_acl_entry_t));
	}
	else
	{
		// Do a memmove overwriting the entry.
		memmove(&(acl->entries[index]), &(acl->entries[index + 1]), sizeof(struct _wlibc_acl_entry_t) * (acl->count - index - 1));
		// Zero the last entry.
		memset(&(acl->entries[acl->count - 1]), 0, sizeof(struct _wlibc_acl_entry_t));
	}

	// Decrement the number of aces stored here.
	--acl->count;

	return 0;
}

int wlibc_acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry)
{
	VALIDATE_ACL(acl, -1);
	VALIDATE_PTR(entry, EINVAL, -1);

	if (entry_id != ACL_FIRST_ENTRY && entry_id != ACL_NEXT_ENTRY && entry_id != ACL_LAST_ENTRY)
	{
		errno = EINVAL;
		return -1;
	}

	if (entry_id == ACL_FIRST_ENTRY)
	{
		*entry = &(acl->entries[0]);
	}
	else if (entry_id == ACL_LAST_ENTRY)
	{
		*entry = &(acl->entries[acl->count - 1]);
	}
	else // entry_id == ACL_NEXT_ENTRY
	{
		if (*entry == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		// Check whether the entry is part of the acl.
		WORD index = 0;
		for (WORD i = 1; i < acl->count; ++i) // Skip the first entry.
		{
			if (*entry == &(acl->entries[i]))
			{
				index = i;
				break;
			}

			// entry is not part of acl.
			if (i == acl->count - 1)
			{
				errno = ESRCH;
				return -1;
			}
		}

		// If the entry is the last entry set entry to NULL.
		if (index == acl->count - 1)
		{
			*entry = NULL;
			return 0;
		}

		*entry = &(acl->entries[index]);
	}

	return 0;
}
