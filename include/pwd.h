/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_PWD_H
#define WLIBC_PWD_H

#include <wlibc.h>
#include <sys/types.h>
#include <stdarg.h>

_WLIBC_BEGIN_DECLS

struct passwd
{
	char *pw_name;   // Username.
	char *pw_passwd; // Hashed passphrase, will be NULL.
	uid_t pw_uid;    // User ID.
	gid_t pw_gid;    // Group ID.
	char *pw_gecos;  // Comment.
	char *pw_dir;    // Home directory, will be NULL as well.
	char *pw_shell;  // Shell program.
};

WLIBC_API struct passwd *wlibc_getpwent();
WLIBC_API void wlibc_endpwent();
WLIBC_API void wlibc_setpwent();

WLIBC_INLINE struct passwd *getpwent()
{
	return wlibc_getpwent();
}

WLIBC_INLINE void endpwent()
{
	wlibc_endpwent();
}

WLIBC_INLINE void setpwent()
{
	wlibc_setpwent();
}

WLIBC_API struct passwd *wlibc_getpwnam(const char *name);
WLIBC_API struct passwd *wlibc_getpwuid(uid_t uid);

WLIBC_INLINE struct passwd *getpwnam(const char *name)
{
	return wlibc_getpwnam(name);
}

WLIBC_INLINE struct passwd *getpwuid(uid_t uid)
{
	return wlibc_getpwuid(uid);
}

// Reentrant variants
WLIBC_API int wlibc_getpwent_r(struct passwd *restrict pwd_entry, char *restrict buffer , size_t size ,
							   struct passwd **restrict result);
WLIBC_API int wlibc_getpwuid_r(uid_t uid, struct passwd *restrict pwd_entry, char *restrict buffer , size_t size ,
							   struct passwd **restrict result);
WLIBC_API int wlibc_getpwnam_r(const char *restrict name, struct passwd *restrict pwd_entry, char *restrict buffer ,
							   size_t size , struct passwd **restrict result);

WLIBC_INLINE int getpwent_r(struct passwd *restrict pwd_entry, char *restrict buffer , size_t size ,
							struct passwd **restrict result)
{
	return wlibc_getpwent_r(pwd_entry, buffer, size, result);
}

WLIBC_INLINE int getpwnam_r(const char *restrict name, struct passwd *restrict pwd_entry, char *restrict buffer ,
							size_t size , struct passwd **restrict result)
{
	return wlibc_getpwnam_r(name, pwd_entry, buffer, size, result);
}

WLIBC_INLINE int getpwuid_r(uid_t uid, struct passwd *restrict pwd_entry, char *restrict buffer , size_t size ,
							struct passwd **restrict result)
{
	return wlibc_getpwuid_r(uid, pwd_entry, buffer, size, result);
}

_WLIBC_END_DECLS

#endif
