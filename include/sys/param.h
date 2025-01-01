/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_PARAM_H
#define WLIBC_SYS_PARAM_H

#include <wlibc.h>
#include <limits.h>

_WLIBC_BEGIN_DECLS

// Constants.
#ifndef NOGROUP
#	define NOGROUP 65535
#endif
#ifndef MAXPATHLEN
#	define MAXPATHLEN 32768
#endif
#ifndef MAXSYMLINKS
#	define MAXSYMLINKS 20 // CHECK
#endif
#ifndef NOFILE
#	define NOFILE 1048576
#endif
#ifndef MAXHOSTNAMELEN
#	define MAXHOSTNAMELEN 256
#endif
#ifndef NODEV
#	define NODEV ((dev_t)-1) // Non-existent device.
#endif

// Unit of `st_blocks'.
#ifndef DEV_BSIZE
#	define DEV_BSIZE 512
#endif

// Bit map related macros.
#define NBBY         CHAR_BIT
#define setbit(a, i) ((a)[(i) / NBBY] |= 1 << ((i) % NBBY))
#define clrbit(a, i) ((a)[(i) / NBBY] &= ~(1 << ((i) % NBBY)))
#define isset(a, i)  ((a)[(i) / NBBY] & (1 << ((i) % NBBY)))
#define isclr(a, i)  (((a)[(i) / NBBY] & (1 << ((i) % NBBY))) == 0)

// Macros for counting and rounding.
#define howmany(x, y) (((x) + ((y)-1)) / (y)) // Number of groups of y items made from a total of x items.
#define roundup(x, y) ((howmany(x, y)) * (y)) // Round up x to nearest multiple of y.
#define powerof2(x)   ((((x)-1) & (x)) == 0)  // Is x a power of 2.

// Macros for min/max.
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

_WLIBC_END_DECLS

#endif
