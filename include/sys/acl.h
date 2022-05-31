/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_ACL_H
#define WLIBC_SYS_ACL_H

#include <wlibc.h>
#include <fcntl.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS
/*=== Data types ===*/

typedef char acl_type_t;
typedef char acl_tag_t;
typedef unsigned long acl_perm_t;
typedef unsigned char acl_flag_t;
typedef void *acl_qualifier_t;

typedef struct _wlibc_acl_t *acl_t;
typedef struct _wlibc_acl_entry_t *acl_entry_t;
typedef unsigned long *acl_permset_t;
typedef unsigned char *acl_flagset_t;

/*=== Constants ===*/

/* acl_perm_t values */

#define ACL_READ_DATA                 0x00000001
#define ACL_LIST_DIRECTORY            0x00000001
#define ACL_WRITE_DATA                0x00000002
#define ACL_ADD_FILE                  0x00000002
#define ACL_APPEND_DATA               0x00000004
#define ACL_ADD_SUBDIRECTORY          0x00000004
#define ACL_READ_EXTENDED_ATTRIBUTES  0x00000008
#define ACL_WRITE_EXTENDED_ATTRIBUTES 0x00000010
#define ACL_EXECUTE_DATA              0x00000020
#define ACL_TRAVERSE_DIRECTORY        0x00000020
#define ACL_DELETE_CHILD              0x00000040
#define ACL_READ_ATTRIBUTES           0x00000080
#define ACL_WRITE_ATTRIBUTES          0x00000100
#define ACL_DELETE                    0x00010000
#define ACL_READ_SECURITY             0x00020000
#define ACL_WRITE_SECURITY            0x00040000
#define ACL_CHANGE_OWNER              0x00080000
#define ACL_SYNCHRONIZE               0x00100000

#define ACL_SEARCH              ACL_TRAVERSE_DIRECTORY
#define ACL_READ_EXTATTRIBUTES  ACL_READ_EXTENDED_ATTRIBUTES
#define ACL_WRITE_EXTATTRIBUTES ACL_WRITE_EXTENDED_ATTRIBUTES

#define ACL_BASIC    (ACL_READ_ATTRIBUTES | ACL_WRITE_ATTRIBUTES | ACL_DELETE | ACL_READ_SECURITY | ACL_SYNCHRONIZE)
#define ACL_EXTENDED (ACL_WRITE_SECURITY | ACL_CHANGE_OWNER)
#define ACL_READ     (ACL_READ_DATA | ACL_READ_EXTENDED_ATTRIBUTES)
#define ACL_WRITE    (ACL_WRITE_DATA | ACL_APPEND_DATA | ACL_WRITE_EXTENDED_ATTRIBUTES | ACL_DELETE_CHILD)
#define ACL_EXECUTE  (ACL_EXECUTE_DATA)

#define ACL_VALID_PERMS 0x1F01FF

/* acl_flag_t values */

#define ACL_OBJECT_INHERIT       0x01
#define ACL_CONTAINER_INHERIT    0x02
#define ACL_NO_PROPAGATE_INHERIT 0x04
#define ACL_INHERIT_ONLY         0x08
#define ACL_INHERITED            0x10

#define ACL_VALID_FLAGS 0x1F

/* acl_tag_t values */

#define ACL_UNDEFINED_TAG  -1
#define ACL_EXTENDED_ALLOW 0
#define ACL_EXTENDED_DENY  1

/* acl_type_t values */

#define ACL_TYPE_ACCESS   0x0
#define ACL_TYPE_DEFAULT  0x0
#define ACL_TYPE_EXTENDED 0x0

/* ACL qualifier constants */

#define ACL_UNDEFINED_ID NULL

/* ACL Entry Constants */

#define ACL_FIRST_ENTRY 0
#define ACL_NEXT_ENTRY  1
#define ACL_LAST_ENTRY  2

/*=== ACL manipulation ===*/

WLIBC_API acl_t wlibc_acl_init(int count);
WLIBC_API acl_t wlibc_acl_dup(const acl_t acl);
WLIBC_API int wlibc_acl_free(acl_t object);
WLIBC_API int wlibc_acl_valid(const acl_t acl);

WLIBC_INLINE acl_t acl_init(int count)
{
	return wlibc_acl_init(count);
}

WLIBC_INLINE acl_t acl_dup(const acl_t acl)
{
	return wlibc_acl_dup(acl);
}

WLIBC_INLINE int acl_free(acl_t acl)
{
	return wlibc_acl_free(acl);
}

WLIBC_INLINE int acl_valid(const acl_t acl)
{
	return wlibc_acl_valid(acl);
}

/*=== Entry manipulation ===*/

WLIBC_API int wlibc_acl_copy_entry(acl_entry_t destination, acl_entry_t source);
WLIBC_API int wlibc_acl_create_entry(acl_t *acl, acl_entry_t *entry);
WLIBC_API int wlibc_acl_delete_entry(acl_t acl, acl_entry_t entry);
WLIBC_API int wlibc_acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry);

WLIBC_INLINE int acl_copy_entry(acl_entry_t destination, acl_entry_t source)
{
	return wlibc_acl_copy_entry(destination, source);
}

WLIBC_INLINE int acl_create_entry(acl_t *acl, acl_entry_t *entry)
{
	return wlibc_acl_create_entry(acl, entry);
}

WLIBC_INLINE int acl_delete_entry(acl_t acl, acl_entry_t entry)
{
	return wlibc_acl_delete_entry(acl, entry);
}

WLIBC_INLINE int acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry)
{
	return wlibc_acl_get_entry(acl, entry_id, entry);
}

/* Manipulate ACL entry permissions */

WLIBC_API int wlibc_acl_add_perm(acl_permset_t permset, acl_perm_t perm);
WLIBC_API int wlibc_acl_clear_perms(acl_permset_t permset);
WLIBC_API int wlibc_acl_delete_perm(acl_permset_t permset, acl_perm_t perm);
WLIBC_API int wlibc_acl_get_perm(acl_permset_t permset, acl_perm_t perm);

WLIBC_INLINE int acl_add_perm(acl_permset_t permset, acl_perm_t perm)
{
	return wlibc_acl_add_perm(permset, perm);
}

WLIBC_INLINE int acl_clear_perms(acl_permset_t permset)
{
	return wlibc_acl_clear_perms(permset);
}

WLIBC_INLINE int acl_delete_perm(acl_permset_t permset, acl_perm_t perm)
{
	return wlibc_acl_delete_perm(permset, perm);
}

WLIBC_INLINE int acl_get_perm(acl_permset_t permset, acl_perm_t perm)
{
	return wlibc_acl_get_perm(permset, perm);
}

WLIBC_API int wlibc_acl_get_permset(acl_entry_t entry, acl_permset_t *permset);
WLIBC_API int wlibc_acl_set_permset(acl_entry_t entry, acl_permset_t permset);

WLIBC_INLINE int acl_get_permset(acl_entry_t entry, acl_permset_t *permset)
{
	return wlibc_acl_get_permset(entry, permset);
}

WLIBC_INLINE int acl_set_permset(acl_entry_t entry, acl_permset_t permset)
{
	return wlibc_acl_set_permset(entry, permset);
}

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

WLIBC_INLINE int acl_calc_mask(acl_t *acl WLIBC_UNUSED)
{
	// Nop (Unsupported).
	return -1;
}

#pragma warning(pop)

/* Manipulate flags on ACLs and entries */
WLIBC_API int wlibc_acl_add_flag(acl_flagset_t flagset, acl_flag_t flag);
WLIBC_API int wlibc_acl_clear_flags(acl_flagset_t flagset);
WLIBC_API int wlibc_acl_delete_flag(acl_flagset_t flagset, acl_flag_t flag);
WLIBC_API int wlibc_acl_get_flag(acl_flagset_t flagset, acl_flag_t flag);

WLIBC_INLINE int acl_add_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	return wlibc_acl_add_flag(flagset, flag);
}

WLIBC_INLINE int acl_clear_flags(acl_flagset_t flagset)
{
	return wlibc_acl_clear_flags(flagset);
}

WLIBC_INLINE int acl_delete_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	return wlibc_acl_delete_flag(flagset, flag);
}

WLIBC_INLINE int acl_get_flag(acl_flagset_t flagset, acl_flag_t flag)
{
	return wlibc_acl_get_flag(flagset, flag);
}

WLIBC_API int wlibc_acl_get_flagset(acl_entry_t entry, acl_flagset_t *flagset);
WLIBC_API int wlibc_acl_set_flagset(acl_entry_t entry, acl_flagset_t flagset);

WLIBC_INLINE int acl_get_flagset(acl_entry_t entry, acl_flagset_t *flagset)
{
	return wlibc_acl_get_flagset(entry, flagset);
}

WLIBC_INLINE int acl_set_flagset(acl_entry_t entry, acl_flagset_t flagset)
{
	return wlibc_acl_set_flagset(entry, flagset);
}

/* Manipulate ACL entry tag type and qualifier */

WLIBC_API acl_qualifier_t wlibc_acl_get_qualifier(acl_entry_t entry);
WLIBC_API int wlibc_acl_set_qualifier(acl_entry_t entry, const acl_qualifier_t qualifier);

WLIBC_INLINE acl_qualifier_t acl_get_qualifier(acl_entry_t entry)
{
	return wlibc_acl_get_qualifier(entry);
}

WLIBC_INLINE int acl_set_qualifier(acl_entry_t entry, const acl_qualifier_t qualifier)
{
	return wlibc_acl_set_qualifier(entry, qualifier);
}

WLIBC_API int wlibc_acl_get_tag_type(acl_entry_t entry, acl_tag_t *tag);
WLIBC_API int wlibc_acl_set_tag_type(acl_entry_t entry, acl_tag_t tag);

WLIBC_INLINE int acl_get_tag_type(acl_entry_t entry, acl_tag_t *tag)
{
	return wlibc_acl_get_tag_type(entry, tag);
}

WLIBC_INLINE int acl_set_tag_type(acl_entry_t entry, acl_tag_t tag)
{
	return wlibc_acl_set_tag_type(entry, tag);
}

/*=== Format translation ===*/

#if 0
extern ssize_t acl_copy_ext(void *buf, acl_t acl, ssize_t size);
extern acl_t acl_copy_int(const void *buf);
extern acl_t acl_from_text(const char *buf);
extern ssize_t acl_size(acl_t acl);
extern char *acl_to_text(acl_t acl, ssize_t *len);
#endif

/*=== Object manipulation ===*/

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

WLIBC_API acl_t wlibc_acl_get(int fd, const char *path, int flags);
WLIBC_API int wlibc_acl_set(int fd, const char *path, acl_t acl, int flags);

WLIBC_INLINE acl_t acl_get_fd(int fd)
{
	return wlibc_acl_get(fd, NULL, AT_EMPTY_PATH);
}

WLIBC_INLINE acl_t acl_get_file(const char *path, acl_type_t type WLIBC_UNUSED)
{
	return wlibc_acl_get(-1, path, 0);
}

WLIBC_INLINE acl_t acl_get_link(const char *path, acl_type_t type WLIBC_UNUSED)
{
	return wlibc_acl_get(-1, path, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int acl_set_fd(int fd, acl_t acl)
{
	return wlibc_acl_set(fd, NULL, acl, AT_EMPTY_PATH);
}

WLIBC_INLINE int acl_set_file(const char *path, acl_type_t type WLIBC_UNUSED, acl_t acl)
{
	return wlibc_acl_set(-1, path, acl, 0);
}

WLIBC_INLINE int acl_set_link(const char *path, acl_type_t type WLIBC_UNUSED, acl_t acl)
{
	return wlibc_acl_set(-1, path, acl, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int acl_delete_def_file(const char *path WLIBC_UNUSED)
{
	// Nop (Unsupported).
	return -1;
}

#pragma warning(pop)

_WLIBC_END_DECLS

#endif
