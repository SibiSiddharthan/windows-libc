/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/acl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/acl.h>

int wlibc_acl_copy_entry(acl_entry_t destination, acl_entry_t source)
{
	VALIDATE_ACL_ENTRY(destination, -1);
	VALIDATE_ACL_ENTRY(source, -1);

	if (destination->size < source->size)
	{
		errno = ERANGE;
		return -1;
	}

	memcpy(destination, source, source->size);

	return 0;
}

int wlibc_acl_create_entry(acl_t *acl, acl_entry_t *entry)
{
	VALIDATE_PTR(acl, EINVAL, -1);
	VALIDATE_PTR(entry, EINVAL, -1);

	WORD used_size = offsetof(struct _wlibc_acl_t, entries);
	WORD max_entry_size = sizeof(struct _wlibc_acl_entry_t) + SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES);

	for (int i = 0; i < (*acl)->count; ++i)
	{
		*entry = (struct _wlibc_acl_entry_t *)((char *)acl + used_size);
		used_size += (*entry)->size;
	}

	if ((*acl)->size - used_size < max_entry_size)
	{
		// Allocate more space.
		acl_t temp = (acl_t)malloc(max_entry_size * (*acl)->count * 2);
		memset(temp, 0, max_entry_size * (*acl)->count * 2);
		memcpy(temp, *acl, (*acl)->size);
		free(*acl);

		*acl = temp;
	}

	// Store the address at the end of the previous entry.
	*entry = (struct _wlibc_acl_entry_t *)((char *)acl + used_size);

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

	// All acls we handle here are in relative form.
	// So check whether the address entry within the range of acl.
	if ((char *)entry + entry->size > (char *)(acl) + acl->size)
	{
		errno = EINVAL;
		return -1;
	}

	// Do a memmove overwriting the entry.
	memmove(entry, (char *)entry + entry->size, acl->size - entry->size - ((char *)entry - (char *)acl));

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
		*entry = (struct _wlibc_acl_entry_t *)((char *)acl + offsetof(struct _wlibc_acl_t, entries));
	}
	else if (entry_id == ACL_LAST_ENTRY)
	{
		WORD offset = 0;

		for (int i = 0; i < acl->count - 1; ++i)
		{
			*entry = (struct _wlibc_acl_entry_t *)((char *)acl + offsetof(struct _wlibc_acl_t, entries) + offset);
			offset += (*entry)->size;
		}

		*entry = (struct _wlibc_acl_entry_t *)((char *)acl + offsetof(struct _wlibc_acl_t, entries) + offset);
	}
	else // entry_id == ACL_NEXT_ENTRY
	{
		if (*entry == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		// Check if entry is part of the acl.
		if ((char *)*entry + (*entry)->size > (char *)(acl) + acl->size)
		{
			errno = EINVAL;
			return -1;
		}

		// If the entry is the last entry set entry to NULL.
		WORD offset = 0;
		WORD index = 0;
		struct _wlibc_acl_entry_t *acl_entry;

		for (int i = 0; i < acl->count; ++i)
		{
			acl_entry = (struct _wlibc_acl_entry_t *)((char *)acl + offsetof(struct _wlibc_acl_t, entries) + offset);
			offset += acl_entry->size;

			if (acl_entry == *entry)
			{
				index = i;
				break;
			}
		}

		if (index == acl->count - 1)
		{
			*entry = NULL;
			return 0;
		}

		// The next entry will be located by 'size' bytes from the current one.
		*entry = (struct _wlibc_acl_entry_t *)((char *)*entry + (*entry)->size);
	}

	return 0;
}
