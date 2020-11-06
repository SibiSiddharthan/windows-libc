#ifndef WLIBC_MACROS_H
#define WLIBC_MACROS_H

#if BUILDING_WLIBC
#	ifdef WLIBC_DLL
#		define WLIBC_API __declspec(dllexport)
#	else
#		define WLIBC_API
#	endif
#else
#	define WLIBC_API /*__declspec(dllimport)*/
#endif

/* For C++ compilers compiling C code */
#ifdef __cplusplus
#	define _WLIBC_BEGIN_DECLS \
		extern "C"             \
		{
#	define _WLIBC_END_DECLS }
#else
#	define _WLIBC_BEGIN_DECLS
#	define _WLIBC_END_DECLS
#endif

#define WLIBC_INLINE __inline

#endif