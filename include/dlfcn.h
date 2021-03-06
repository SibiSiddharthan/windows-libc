/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_DLFCN_H
#define WLIBC_DLFCN_H

#include <wlibc-macros.h>

_WLIBC_BEGIN_DECLS

/* The flags argument to dlopen is unused. Below macros are
   provided for compatibilty */
#define RTLD_LAZY   0x0 // Lazy function call binding.
#define RTLD_NOW    0x0 // Immediate function call binding.
#define RTLD_GLOBAL 0x0 // Load symbols in global namespace
#define RTLD_LOCAL  0x0 // Do not load symbols in global namespace

WLIBC_API void *wlibc_dlopen(const char *filename, int flags /* unused */);

WLIBC_INLINE void *dlopen(const char *filename, int flags /* unused */)
{
	return wlibc_dlopen(filename, flags);
}

WLIBC_API char *wlibc_dlerror();

WLIBC_INLINE void *dlerror()
{
	return wlibc_dlerror();
}

WLIBC_API int wlibc_dlclose(void *handle);

WLIBC_INLINE int dlclose(void *handle)
{
	return wlibc_dlclose(handle);
}

WLIBC_API void *wlibc_dlsym(void *handle, const char *symbol);

WLIBC_INLINE void *dlsym(void *handle, const char *symbol)
{
	return wlibc_dlsym(handle, symbol);
}

_WLIBC_END_DECLS

#endif
