/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_IO_H
#define WLIBC_IO_H

#include <wlibc.h>

_WLIBC_BEGIN_DECLS

/*
 * Below functions are no-op and defined here for compatibility. We always do binary IO with files.
 * By introducing our io.h we prevent including MSVC's io.h which redeclares a few symbols (rename, remove).
 */

WLIBC_INLINE int _setmode(int fd, int mode)
{
	return 0;
}

#define setmode _setmode

_WLIBC_END_DECLS

#endif
