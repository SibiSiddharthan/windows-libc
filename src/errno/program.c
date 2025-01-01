/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>

char *program_invocation_name = NULL;
char *program_invocation_short_name = NULL;

void init_program_name(void)
{
	NTSTATUS status;
	UTF8_STRING u8_program;

	// Fetch path name from RTL_USER_PROCESS_PARAMETERS.
	// This will be freed automatically during program termination.
	status = RtlUnicodeStringToUTF8String(&u8_program, &NtCurrentPeb()->ProcessParameters->ImagePathName, TRUE);
	if (status != STATUS_SUCCESS)
	{
		// Exit the program due to insufficient memory.
		RtlExitUserProcess(STATUS_NO_MEMORY);
	}

	// Convert back slashes to forward slashes.
	int last_slash = 0;
	for (int i = 0; i < u8_program.Length; ++i)
	{
		if (u8_program.Buffer[i] == '\\')
		{
			u8_program.Buffer[i] = '/';
			last_slash = i + 1;
		}
	}

	// 'program_invocation_name' is the full path of the program.
	// 'program_invocation_short_name' is the last component of the program.
	program_invocation_name = u8_program.Buffer;
	program_invocation_short_name = u8_program.Buffer + last_slash;
}
