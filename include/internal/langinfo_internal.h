/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef LANGINFO_INTERNAL_H
#define LANGINFO_INTERNAL_H

#define MAX_LANGINFO_INVOKE_COUNT 256
#define MAX_LANGINFO_LENGTH 32

extern char **__nl_langinfo_buf;

void langinfo_init();
void langinfo_cleanup();

#endif
