/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_XATTR_H
#define WLIBC_SYS_XATTR_H

#include <wlibc.h>
#include <sys/types.h>
#include <fcntl.h>

_WLIBC_BEGIN_DECLS

#define XATTR_CREATE  1
#define XATTR_REPLACE 2
#define XATTR_DELETE  3

WLIBC_API int wlibc_common_setxattr(int fd, const char *restrict path, const char *restrict name, const void *restrict value, size_t size,
									int operation, int flags);
WLIBC_API ssize_t wlibc_common_getxattr(int fd, const char *restrict path, const char *restrict name, void *restrict value, size_t size,
										int flags);
WLIBC_API ssize_t wlibc_common_listxattr(int fd, const char *restrict path, char *restrict list, size_t size, int flags);


WLIBC_INLINE int setxattr(const char *restrict path, const char *restrict name, const void *restrict value, size_t size, int operation)
{
	return wlibc_common_setxattr(AT_FDCWD, path, name, value, size, operation, 0);
}

WLIBC_INLINE int lsetxattr(const char *restrict path, const char *restrict name, const void *restrict value, size_t size, int operation)
{
	return wlibc_common_setxattr(AT_FDCWD, path, name, value, size, operation, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int fsetxattr(int fd, const char *restrict name, const void *restrict value, size_t size, int operation)
{
	return wlibc_common_setxattr(fd, NULL, name, value, size, operation, AT_EMPTY_PATH);
}

WLIBC_INLINE ssize_t getxattr(const char *restrict path, const char *restrict name, void *restrict value, size_t size)
{
	return wlibc_common_getxattr(AT_FDCWD, path, name, value, size, 0);
}

WLIBC_INLINE ssize_t lgetxattr(const char *restrict path, const char *restrict name, void *restrict value, size_t size)
{
	return wlibc_common_getxattr(AT_FDCWD, path, name, value, size, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE ssize_t fgetxattr(int fd, const char *restrict name, void *restrict value, size_t size)
{
	return wlibc_common_getxattr(fd, NULL, name, value, size, AT_EMPTY_PATH);
}

WLIBC_INLINE ssize_t listxattr(const char *restrict path, char *restrict list, size_t size)
{
	return wlibc_common_listxattr(AT_FDCWD, path, list, size, 0);
}

WLIBC_INLINE ssize_t llistxattr(const char *restrict path, char *restrict list, size_t size)
{
	return wlibc_common_listxattr(AT_FDCWD, path, list, size, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE ssize_t flistxattr(int fd, char *list, size_t size)
{
	return wlibc_common_listxattr(fd, NULL, list, size, AT_EMPTY_PATH);
}

WLIBC_INLINE int removexattr(const char *restrict path, const char *restrict name)
{
	return wlibc_common_setxattr(AT_FDCWD, path, name, NULL, 0, XATTR_DELETE, 0);
}

WLIBC_INLINE int lremovexattr(const char *restrict path, const char *restrict name)
{
	return wlibc_common_setxattr(AT_FDCWD, path, name, NULL, 0, XATTR_DELETE, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int fremovexattr(int fd, const char *name)
{
	return wlibc_common_setxattr(fd, NULL, name, NULL, 0, XATTR_DELETE, AT_EMPTY_PATH);
}

_WLIBC_END_DECLS

#endif
