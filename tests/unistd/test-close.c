/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <internal/fcntl.h>

void test_EBADF()
{
	int status = close(-1);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EBADF);
}

void test_okay_normal_file()
{
	int fd = open("t-close", O_CREAT | O_RDONLY);
	HANDLE handle = get_fd_handle(fd);
	int status = close(fd);
	ASSERT_EQ(status, 0);
	BY_HANDLE_FILE_INFORMATION FILE_INFO;
	GetFileInformationByHandle(handle, &FILE_INFO);
	DWORD error = GetLastError();
	ASSERT_EQ(error, ERROR_INVALID_HANDLE);
	unlink("t-close");
}

void test_okay_directory()
{
	int fd = open(".", O_RDONLY);
	HANDLE handle = get_fd_handle(fd);
	int status = close(fd);
	ASSERT_EQ(status, 0);
	BY_HANDLE_FILE_INFORMATION FILE_INFO;
	GetFileInformationByHandle(handle, &FILE_INFO);
	DWORD error = GetLastError();
	ASSERT_EQ(error, ERROR_INVALID_HANDLE);
}

int main()
{
	test_EBADF();
	test_okay_normal_file();
	test_okay_directory();
	return 0;
}
