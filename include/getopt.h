/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_GETOPT_H
#define WLIBC_GETOPT_H

#include <wlibc.h>

_WLIBC_BEGIN_DECLS

extern char *optarg; // Stores the parameter given to an option.
extern int optind;   // Next index of arv to be processed.
extern int opterr;   // Controls printing of error messages.
extern int optopt;   // Stores the unknown option.

struct option
{
	const char *name;
	int has_arg;
	int *flag;
	int val;
};

/* Names for the values of the 'has_arg' field of 'struct option'. */

#define no_argument       0
#define required_argument 1
#define optional_argument 2

WLIBC_API int wlibc_common_getopt(int argc, char *argv[], const char *optstring, const struct option *longopts, int *longindex);

WLIBC_INLINE int getopt(int argc, char *argv[], const char *optstring)
{
	return wlibc_common_getopt(argc, argv, optstring, NULL, NULL);
}

WLIBC_INLINE int getopt_long(int argc, char *argv[], const char *optstring, const struct option *longopts, int *longindex)
{
	return wlibc_common_getopt(argc, argv, optstring, longopts, longindex);
}

_WLIBC_END_DECLS

#endif
