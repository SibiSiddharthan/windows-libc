/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_USTNAME_H
#define WLIBC_SYS_USTNAME_H

#include <wlibc.h>

_WLIBC_BEGIN_DECLS

#define WLIBC_UTSNAME_LENGTH 64

struct utsname
{
	/* Name of the implementation of the operating system.  */
	char sysname[WLIBC_UTSNAME_LENGTH];
	/* Name of this node on the network.  */
	char nodename[WLIBC_UTSNAME_LENGTH];
	/* Current release level of this implementation.  */
	char release[WLIBC_UTSNAME_LENGTH];
	/* Current version level of this release.  */
	char version[WLIBC_UTSNAME_LENGTH];
	/* Name of the hardware type the system is running on.  */
	char machine[WLIBC_UTSNAME_LENGTH];
	/* Name of the domain of this node on the network.  */
	char domainname[WLIBC_UTSNAME_LENGTH];
};

WLIBC_API int wlibc_uname(struct utsname *name);
WLIBC_INLINE int uname(struct utsname *name)
{
	return wlibc_uname(name);
}

_WLIBC_END_DECLS

#endif
