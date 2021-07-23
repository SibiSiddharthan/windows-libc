/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_DLFCN_INTERNAL_H
#define WLIBC_DLFCN_INTERNAL_H

extern unsigned long _last_dlfcn_error;
extern char *_dlfcn_error_message;

void dlfcn_init();
void dlfcn_cleanup();

#endif
