/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_GRP_H
#define WLIBC_GRP_H

#include <wlibc.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

struct group
{
	char *gr_name;   // Groupname.
	char *gr_passwd; // Hashed passphrase, will be NULL.
	gid_t gr_gid;    // Group ID.
	char **gr_mem;   // Group members.
};

WLIBC_API struct group *wlibc_getgrent();
WLIBC_API void wlibc_endgrent();
WLIBC_API void wlibc_setgrent();

WLIBC_INLINE struct group *getgrent()
{
	return wlibc_getgrent();
}

WLIBC_INLINE void endgrent()
{
	wlibc_endgrent();
}

WLIBC_INLINE void setgrent()
{
	wlibc_setgrent();
}

WLIBC_API struct group *wlibc_getgrnam(const char *name);
WLIBC_API struct group *wlibc_getgrgid(gid_t gid);

WLIBC_INLINE struct group *getgrnam(const char *name)
{
	return wlibc_getgrnam(name);
}

WLIBC_INLINE struct group *getgrgid(gid_t gid)
{
	return wlibc_getgrgid(gid);
}

// Reentrant variants
WLIBC_API int wlibc_getgrent_r(struct group *restrict grp_entry, char *restrict buffer, size_t size, struct group **restrict result);
WLIBC_API int wlibc_getgrgid_r(gid_t gid, struct group *restrict grp_entry, char *restrict buffer, size_t size,
							   struct group **restrict result);
WLIBC_API int wlibc_getgrnam_r(const char *restrict name, struct group *restrict grp_entry, char *restrict buffer, size_t size,
							   struct group **restrict result);

WLIBC_INLINE int getgrent_r(struct group *restrict grp_entry, char *restrict buffer, size_t size, struct group **restrict result)
{
	return wlibc_getgrent_r(grp_entry, buffer, size, result);
}

WLIBC_INLINE int getgrnam_r(const char *restrict name, struct group *restrict grp_entry, char *restrict buffer, size_t size,
							struct group **restrict result)
{
	return wlibc_getgrnam_r(name, grp_entry, buffer, size, result);
}

WLIBC_INLINE int getgrgid_r(gid_t gid, struct group *restrict grp_entry, char *restrict buffer, size_t size, struct group **restrict result)
{
	return wlibc_getgrgid_r(gid, grp_entry, buffer, size, result);
}

_WLIBC_END_DECLS

#endif
