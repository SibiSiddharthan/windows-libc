/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_VALIDATE_INTERNAL_H
#define WLIBC_VALIDATE_INTERNAL_H

#define VALIDATE_PTR(ptr, error, ret) \
	if (ptr == NULL)                  \
	{                                 \
		errno = error;                \
		return ret;                   \
	}

#define VALIDATE_STRING(str, error, ret) \
	if (str == NULL || str[0] == '\0')   \
	{                                    \
		errno = error;                   \
		return ret;                      \
	}

#endif
