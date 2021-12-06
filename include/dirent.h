/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_DIRENT_H
#define WLIBC_DIRENT_H

#include <wlibc.h>
#include <sys/types.h>
#include <fcntl.h>

_WLIBC_BEGIN_DECLS

#define DT_UNKNOWN 0
#define DT_FIFO    1  // pipe
#define DT_CHR     2  // character device
#define DT_DIR     4  // directory
#define DT_BLK     6  // block
#define DT_REG     8  // regular file
#define DT_LNK     10 // symbolic link
#define DT_SOCK    12 // socket
#define DT_WHT     14

// available data in struct dirent
#define _DIRENT_HAVE_D_INO
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_NAME

#define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
#define _D_ALLOC_NAMLEN(d) ((d)->d_namlen + 1)

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	uint16_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[260];
};

typedef struct WLIBC_DIR DIR;

WLIBC_API DIR *wlibc_opendir(const char *path);
WLIBC_INLINE DIR *opendir(const char *path)
{
	return wlibc_opendir(path);
}

WLIBC_API DIR *wlibc_fdopendir(int fd);
WLIBC_INLINE DIR *fdopendir(int fd)
{
	return wlibc_fdopendir(fd);
}

WLIBC_API int wlibc_closedir(DIR *dirstream);
WLIBC_INLINE int closedir(DIR *dirstream)
{
	return wlibc_closedir(dirstream);
}

WLIBC_API struct dirent *wlibc_readdir(DIR *dirstream);
WLIBC_API int wlibc_readdir_r(DIR *restrict dirstream, struct dirent *restrict entry, struct dirent **restrict result);
WLIBC_INLINE struct dirent *readdir(DIR *dirstream)
{
	return wlibc_readdir(dirstream);
}

WLIBC_INLINE int readdir_r(DIR *restrict dirstream, struct dirent *restrict entry, struct dirent **restrict result)
{
	return wlibc_readdir_r(dirstream, entry, result);
}

WLIBC_API void wlibc_rewinddir(DIR *dirstream);
WLIBC_INLINE void rewinddir(DIR *dirstream)
{
	return wlibc_rewinddir(dirstream);
}

WLIBC_API void wlibc_seekdir(DIR *dirstream, long long int pos);
WLIBC_INLINE void seekdir(DIR *dirstream, long long int pos)
{
	return wlibc_seekdir(dirstream, pos);
}

WLIBC_API off_t wlibc_telldir(DIR *dirstream);
WLIBC_INLINE off_t telldir(DIR *dirstream)
{
	return wlibc_telldir(dirstream);
}

WLIBC_API int wlibc_dirfd(DIR *dirstream);
WLIBC_INLINE int dirfd(DIR *dirstream)
{
	return wlibc_dirfd(dirstream);
}

WLIBC_API int wlibc_scandir(const char *name, struct dirent ***namelist, int (*selector)(const struct dirent *),
							int (*cmp)(const struct dirent **, const struct dirent **));
WLIBC_INLINE int scandir(const char *name, struct dirent ***namelist, int (*selector)(const struct dirent *),
						 int (*cmp)(const struct dirent **, const struct dirent **))
{
	return wlibc_scandir(name, namelist, selector, cmp);
}

WLIBC_API int scandirat(int dfd, const char *name, struct dirent ***namelist, int (*selector)(const struct dirent *),
						int (*cmp)(const struct dirent **, const struct dirent **));

WLIBC_API int wlibc_alphasort(const struct dirent **e1, const struct dirent **e2);
WLIBC_INLINE int alphasort(const struct dirent **e1, const struct dirent **e2)
{
	return wlibc_alphasort(e1, e2);
}

_WLIBC_END_DECLS

#endif
