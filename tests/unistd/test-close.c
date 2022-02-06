/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <Windows.h>
#include <internal/fcntl.h>

int test_EBADF()
{
	int status = close(-1);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EBADF);
	return 0;
}

int test_okay_normal_file()
{
	int status;
	int fd;
	const char *filename = "t-close";
	HANDLE handle;
	DWORD error;
	BY_HANDLE_FILE_INFORMATION FILE_INFO;

	fd = open(filename, O_CREAT | O_RDONLY);
	ASSERT_EQ(fd, 3);
	handle = get_fd_handle(fd);
	status = close(fd);
	ASSERT_EQ(status, 0);

	// Check if the underlying handle is actually closed
	GetFileInformationByHandle(handle, &FILE_INFO);
	error = GetLastError();
	ASSERT_EQ(error, ERROR_INVALID_HANDLE);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_okay_directory()
{
	int status;
	int fd;
	HANDLE handle;
	DWORD error;
	BY_HANDLE_FILE_INFORMATION FILE_INFO;

	fd = open(".", O_RDONLY);
	ASSERT_EQ(fd, 3);
	handle = get_fd_handle(fd);
	status = close(fd);
	ASSERT_EQ(status, 0);

	// Check if the underlying handle is actually closed
	GetFileInformationByHandle(handle, &FILE_INFO);
	error = GetLastError();
	ASSERT_EQ(error, ERROR_INVALID_HANDLE);

	return 0;
}

void cleanup()
{
	remove("t-close");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_EBADF());
	TEST(test_okay_normal_file());
	TEST(test_okay_directory());

	VERIFY_RESULT_AND_EXIT();
}
