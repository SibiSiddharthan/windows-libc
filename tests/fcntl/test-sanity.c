/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

/*
 * These test the validity of the undocumented API we are using (structure of RTL_USER_PROCESS_PARAMETERS).
 * If something does not work properly in the future, this test should probably fail.
 */

int main()
{
	int exit_code = 0;
	PRTL_USER_PROCESS_PARAMETERS process_params = NtCurrentPeb()->ProcessParameters;

	if (process_params->StandardInput != GetStdHandle(STD_INPUT_HANDLE))
	{
		printf("Error stdin is different: %zu <-> %zu\n", (intptr_t)process_params->StandardInput,
			   (intptr_t)GetStdHandle(STD_INPUT_HANDLE));
		++exit_code;
	}

	if (process_params->StandardOutput != GetStdHandle(STD_OUTPUT_HANDLE))
	{
		printf("Error stdout is different: %zu <-> %zu\n", (intptr_t)process_params->StandardOutput,
			   (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE));
		++exit_code;
	}

	if (process_params->StandardError != GetStdHandle(STD_ERROR_HANDLE))
	{
		printf("Error stderr is different: %zu <-> %zu\n", (intptr_t)process_params->StandardError,
			   (intptr_t)GetStdHandle(STD_ERROR_HANDLE));
		++exit_code;
	}

	wchar_t cwd[MAX_PATH];
	DWORD length = GetCurrentDirectoryW(MAX_PATH, cwd);
	cwd[length] = L'\\'; // Append trailing slash
	cwd[length + 1] = L'\0';

	if (wcscmp(process_params->CurrentDirectory.DosPath.Buffer, cwd) != 0)
	{
		printf("Current directories are different: %ls <-> %ls\n", process_params->CurrentDirectory.DosPath.Buffer, cwd);
		++exit_code;
	}

	STARTUPINFOW startupinfo;
	GetStartupInfoW(&startupinfo);

	if (process_params->RuntimeData.Length != startupinfo.cbReserved2)
	{
		printf("Length of runtime data is different: %hu <-> %hu\n", process_params->RuntimeData.Length, startupinfo.cbReserved2);
		++exit_code;
	}
	else
	{
		// do the next check only if previous check was successful
		if (memcmp(process_params->RuntimeData.Buffer, startupinfo.lpReserved2, process_params->RuntimeData.Length))
		{
			printf("Runtime data is different\n");
			++exit_code;
		}
	}

	return exit_code;
}
