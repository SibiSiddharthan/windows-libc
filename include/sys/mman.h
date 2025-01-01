/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_MMAN_H
#define WLIBC_SYS_MMAN_H

#include <wlibc.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

/* Return value of `mmap' in case of an error.  */
#define MAP_FAILED ((void *)-1)

/* Memory protection constants */
#define PROT_GROWSUP   0xf // Unsupported
#define PROT_GROWSDOWN 0xf // Unsupported
#define PROT_NONE      0x0 // Page no access.
#define PROT_READ      0x1 // Page readable.
#define PROT_WRITE     0x2 // Page writable.
#define PROT_EXEC      0x4 // Page executable.

/* Memory mapping constants */
#define MAP_SHARED          0x01 // Sharable mapping.
#define MAP_PRIVATE         0x02 // Private mapping.
#define MAP_SHARED_VALIDATE 0x04 // Share changes and validate.
#define MAP_HUGETLB         0x10 // Allocate using large pages.
#define MAP_ANONYMOUS       0x20 // Use the pagefile.
#define MAP_FILE            0x40 // Regular file.
#define MAP_EXECUTABLE      0x80 // Mark it as an executable.
#define MAP_ANON            MAP_ANONYMOUS

#define MAP_LOCKED          0x0 // Obsolete
#define MAP_GROWSDOWN       0x0 // Unsupported
#define MAP_DENYWRITE       0x0 // Unsupported
#define MAP_NORESERVE       0x0 // Unsupported
#define MAP_POPULATE        0x0 // Unsupported
#define MAP_NONBLOCK        0x0 // Unsupported
#define MAP_STACK           0x0 // Unsupported
#define MAP_SYNC            0x0 // Unsupported
#define MAP_FIXED_NOREPLACE 0x0 // Unsupported

/* Flags for msync */
#define MS_ASYNC      0 // Sync memory asynchronously (Unsupported).
#define MS_INVALIDATE 0 // Invalidate caches (Unsupported).
#define MS_SYNC       1 // Sync memory synchronously (Supported).

WLIBC_API void *wlibc_mmap(void *address, size_t size, int protection, int flags, int fd, off_t offset);

WLIBC_INLINE void *mmap(void *address, size_t size, int protection, int flags, int fd, off_t offset)
{
	return wlibc_mmap(address, size, protection, flags, fd, offset);
}

WLIBC_API int wlibc_munmap(void *address, size_t size /* unused */);

WLIBC_INLINE int munmap(void *address, size_t size /* unused */)
{
	return wlibc_munmap(address, size);
}

WLIBC_API int wlibc_mlock(const void *address, size_t size);

WLIBC_INLINE int mlock(const void *address, size_t size)
{
	return wlibc_mlock(address, size);
}

WLIBC_API int wlibc_munlock(const void *address, size_t size);

WLIBC_INLINE int munlock(const void *address, size_t size)
{
	return wlibc_munlock(address, size);
}

WLIBC_API int wlibc_mprotect(void *address, size_t size, int protection);

WLIBC_INLINE int mprotect(void *address, size_t size, int protection)
{
	return wlibc_mprotect(address, size, protection);
}

WLIBC_API int wlibc_msync(void *address, size_t size, int flags /* unused */);

WLIBC_INLINE int msync(void *address, size_t size, int flags /* unused */)
{
	return wlibc_msync(address, size, flags);
}

_WLIBC_END_DECLS

#endif
