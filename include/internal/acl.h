/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_ACL_INTERNAL_H
#define WLIBC_ACL_INTERNAL_H

#include <internal/nt.h>
#include <internal/validate.h>
#include <errno.h>

#define ACL_VALID_PERMS 0x1F01FF
#define ACL_VALID_FLAGS 0x1F

#define MAX_AUTHORITIES 15

// SID (constant size)
typedef struct _wlibc_acl_qualifier_t
{
	BYTE revision;
	BYTE count;
	BYTE root[6];
	DWORD authorities[MAX_AUTHORITIES];
} wlibc_acl_qualifier_t;

// ACE (constant size)
typedef struct _wlibc_acl_entry_t
{
	BYTE type;
	BYTE flags;
	WORD size;
	ACCESS_MASK mask;
	wlibc_acl_qualifier_t sid;
} wlibc_acl_entry_t;

// ACL (variable size)
typedef struct _wlibc_acl_t
{
	BYTE revision;
	BYTE zero1;
	WORD size;
	WORD count;
	WORD zero2;
	wlibc_acl_entry_t entries[1];
} wlibc_acl_t;

#define VALIDATE_ACL(acl, status)             VALIDATE_PTR(acl, EINVAL, status)
#define VALIDATE_ACL_ENTRY(entry, status)     VALIDATE_PTR(entry, EINVAL, status)
#define VALIDATE_ACL_PERMSET(permset, status) VALIDATE_PTR(permset, EINVAL, status)
#define VALIDATE_ACL_FLAGSET(flagset, status) VALIDATE_PTR(flagset, EINVAL, status)

#define VALIDATE_ACL_PERMS(perm, status) \
	{                                    \
		if ((perm) & (~ACL_VALID_PERMS)) \
		{                                \
			errno = EINVAL;              \
			return status;               \
		}                                \
	}

#define VALIDATE_ACL_FLAGS(flag, status) \
	{                                    \
		if ((flag) & (~ACL_VALID_FLAGS)) \
		{                                \
			errno = EINVAL;              \
			return status;               \
		}                                \
	}

#define VALIDATE_ACL_TAG(tag, status)                                      \
	{                                                                      \
		if (((tag) != ACL_EXTENDED_ALLOW) && ((tag) != ACL_EXTENDED_DENY)) \
		{                                                                  \
			errno = EINVAL;                                                \
			return status;                                                 \
		}                                                                  \
	}

#endif
