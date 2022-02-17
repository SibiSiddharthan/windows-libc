/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <thread.h>

int wlibc_thread_once(once_t *control, void (*init)(void))
{

	if(control == NULL || init == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	BOOL result;

	// This is pretty much identitical to `InitOnceExecuteOnce`. Except this return 0 on success.
	// Casting our init function to PINIT_ONCE_FN is alright as user has control over init.
	result = RtlRunOnceExecuteOnce((PRTL_RUN_ONCE)control, (PINIT_ONCE_FN)init, NULL, NULL);
	if (result != 0)
	{
		// No errors are defined for this.
		return -1;
	}

	return result;
}
