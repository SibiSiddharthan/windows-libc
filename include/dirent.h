/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_DIRENT_H
#define WLIBC_DIRENT_H

#include <wlibc-macros.h>
#include <sys/types.h>
#include <wchar.h>

// forward declarations to avoid including windows.h
struct _WIN32_FIND_DATAW;
typedef struct _WIN32_FIND_DATAW WIN32_FIND_DATA;

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

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short int d_reclen;
	unsigned char d_type;
	char d_name[260];
};

struct wdirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short int d_reclen;
	unsigned char d_type;
	wchar_t d_name[260];
};

typedef struct
{
	int fd;
	void *d_handle;
	WIN32_FIND_DATA *data;
	size_t buffer_length;
	size_t size;   /* Total valid data in the block.  */
	size_t offset; /* Current offset into the block.  */
	off_t filepos; /* Position of next entry to read. */
	int errcode;   /* Delayed error code.             */
	struct dirent *_dirent;
	struct wdirent *_wdirent;
} DIR;

WLIBC_API DIR *wlibc_opendir(const char *name);
WLIBC_API DIR *wlibc_wopendir(const wchar_t *name);

WLIBC_INLINE DIR *opendir(const char *name)
{
	return wlibc_opendir(name);
}

WLIBC_INLINE DIR *wopendir(const wchar_t *wname)
{
	return wlibc_wopendir(wname);
}

WLIBC_API DIR *wlibc_fdopendir(int fd);

WLIBC_INLINE DIR *fdopendir(int fd)
{
	return wlibc_fdopendir(fd);
}

WLIBC_API int wlibc_closedir(DIR *dirp);

WLIBC_INLINE int closedir(DIR *dirp)
{
	return wlibc_closedir(dirp);
}

WLIBC_API struct dirent *wlibc_readdir(DIR *dirp);
WLIBC_API struct wdirent *wlibc_wreaddir(DIR *dirp);

WLIBC_INLINE struct dirent *readdir(DIR *dirp)
{
	return wlibc_readdir(dirp);
}

WLIBC_INLINE struct wdirent *wreaddir(DIR *dirp)
{
	return wlibc_wreaddir(dirp);
}

WLIBC_API void wlibc_rewinddir(DIR *dirp);

WLIBC_INLINE void rewinddir(DIR *dirp)
{
	return wlibc_rewinddir(dirp);
}

WLIBC_API void wlibc_seekdir(DIR *dirp, long long int pos);

WLIBC_INLINE void seekdir(DIR *dirp, long long int pos)
{
	return wlibc_seekdir(dirp, pos);
}

WLIBC_API off_t wlibc_telldir(DIR *dirp);

WLIBC_INLINE off_t telldir(DIR *dirp)
{
	return wlibc_telldir(dirp);
}

WLIBC_API int wlibc_dirfd(DIR *dirp);

WLIBC_INLINE int dirfd(DIR *dirp)
{
	return wlibc_dirfd(dirp);
}

WLIBC_API int wlibc_scandir(const char * name, struct dirent *** namelist, int (*selector)(const struct dirent *),
							int (*cmp)(const struct dirent **, const struct dirent **));
WLIBC_API int wlibc_wscandir(const wchar_t * wname, struct wdirent *** wnamelist, int (*selector)(const struct wdirent *),
							 int (*cmp)(const struct wdirent **, const struct wdirent **));

WLIBC_INLINE int scandir(const char * name, struct dirent *** namelist, int (*selector)(const struct dirent *),
						 int (*cmp)(const struct dirent **, const struct dirent **))
{
	return wlibc_scandir(name, namelist, selector, cmp);
}

WLIBC_INLINE int wscandir(const wchar_t * wname, struct wdirent *** wnamelist, int (*selector)(const struct wdirent *),
						  int (*cmp)(const struct wdirent **, const struct wdirent **))
{
	return wlibc_wscandir(wname, wnamelist, selector, cmp);
}

WLIBC_API int scandirat(int dfd, const char * name, struct dirent *** namelist, int (*selector)(const struct dirent *),
						int (*cmp)(const struct dirent **, const struct dirent **));

WLIBC_API int wlibc_alphasort(const struct dirent **e1, const struct dirent **e2);
WLIBC_API int wlibc_walphasort(const struct wdirent **e1, const struct wdirent **e2);

WLIBC_INLINE int alphasort(const struct dirent **e1, const struct dirent **e2)
{
	return wlibc_alphasort(e1, e2);
}

WLIBC_INLINE int walphasort(const struct wdirent **e1, const struct wdirent **e2)
{
	return wlibc_walphasort(e1, e2);
}

_WLIBC_END_DECLS

#endif
