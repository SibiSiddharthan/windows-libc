#ifndef WLIBC_DLFCN_H
#define WLIBC_DLFCN_H
#include <macros.h>

_WLIBC_BEGIN_DECLS

extern unsigned long _last_dlfcn_error;
extern char *_dlfcn_error_message;

/* The flags argument to dlopen is unused. Below macros are taken from
  glibc for compatibilty */

//Copied from glibc
#define RTLD_LAZY 0x00001     /* Lazy function call binding.  */
#define RTLD_NOW 0x00002      /* Immediate function call binding.  */
#define RTLD_BINDING_MASK 0x3 /* Mask of binding time value.  */
#define RTLD_NOLOAD 0x00004   /* Do not load the object.  */
#define RTLD_DEEPBIND 0x00008 /* Use deep binding.  */
#define RTLD_GLOBAL 0x00100
#define RTLD_LOCAL 0x0000     /* Default*/
#define RTLD_NODELETE 0x01000 /* Do not delete object when closed.  */

WLIBC_API void *dlopen(const char *filename, int flags /* unused */);

WLIBC_API char *dlerror();

WLIBC_API int dlclose(void *handle);

WLIBC_API void *dlsym(void *handle, const char *symbol);

_WLIBC_END_DECLS

#endif