/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <thread.h>

int wlibc_tss_create(key_t *index, dtor_t destructor)
{
	// TODO
	return 0;
}

void *wlibc_tss_get(key_t index)
{
	// TODO
	return NULL;
}

int wlibc_tss_set(key_t index, const void *data)
{
	// TODO
	return 0;
}

int wlibc_tss_delete(key_t index)
{
	// TODO
	return 0;
}
