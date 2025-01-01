/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

int test_select_err()
{
	int result;
	struct timeval timeout = {0, 0};
	fd_set readfds, writefds, exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	FD_SET(4, &readfds);
	FD_SET(4, &writefds);
	FD_SET(4, &exceptfds);

	errno = 0;
	result = select(5, &readfds, &writefds, &exceptfds, &timeout);
	ASSERT_EQ(result, -1);
	ASSERT_EQ(errno, EBADF);

	ASSERT_EQ(FD_ISSET(4, &readfds), 0);
	ASSERT_EQ(FD_ISSET(4, &writefds), 0);
	ASSERT_EQ(FD_ISSET(4, &exceptfds), 0);

	return 0;
}

int test_select_file()
{
	int result;
	struct timeval timeout = {0, 0};
	fd_set readfds, writefds, exceptfds;
	int fd_rw, fd_w, fd_r;
	const char *filename_rw = "t-select-rw";
	const char *filename_w = "t-select-w";
	const char *filename_r = "t-select-r";

	fd_rw = open(filename_rw, O_RDWR | O_CREAT, 0700);
	fd_w = open(filename_w, O_WRONLY | O_CREAT, 0700);
	fd_r = open(filename_r, O_RDWR | O_CREAT, 0700);

	ASSERT_SUCCESS(close(fd_r));
	fd_r = open(filename_r, O_RDONLY);

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	FD_SET(fd_rw, &readfds);
	FD_SET(fd_rw, &writefds);
	FD_SET(fd_rw, &exceptfds);

	FD_SET(fd_w, &readfds);
	FD_SET(fd_w, &writefds);
	FD_SET(fd_w, &exceptfds);

	FD_SET(fd_r, &readfds);
	FD_SET(fd_r, &writefds);
	FD_SET(fd_r, &exceptfds);

	result = select(10, &readfds, &writefds, &exceptfds, &timeout);
	ASSERT_EQ(result, 4); // 2rw, 1w, 1r.

	ASSERT_EQ(FD_ISSET(fd_rw, &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd_rw, &writefds), 1);
	ASSERT_EQ(FD_ISSET(fd_rw, &exceptfds), 0);

	ASSERT_EQ(FD_ISSET(fd_w, &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd_w, &writefds), 1);
	ASSERT_EQ(FD_ISSET(fd_w, &exceptfds), 0);

	ASSERT_EQ(FD_ISSET(fd_r, &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd_r, &writefds), 0);
	ASSERT_EQ(FD_ISSET(fd_r, &exceptfds), 0);

	ASSERT_SUCCESS(close(fd_rw));
	ASSERT_SUCCESS(close(fd_w));
	ASSERT_SUCCESS(close(fd_r));

	ASSERT_SUCCESS(unlink(filename_rw));
	ASSERT_SUCCESS(unlink(filename_w));
	ASSERT_SUCCESS(unlink(filename_r));

	return 0;
}

int test_select_devices()
{
	int result;
	struct timeval timeout = {0, 0};
	fd_set readfds, writefds;
	int fd_null, fd_tty;

	fd_null = open("/dev/null", O_RDWR);
	fd_tty = open("/dev/tty", O_RDONLY);

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(fd_null, &readfds);
	FD_SET(fd_null, &writefds);

	FD_SET(fd_tty, &readfds);
	FD_SET(fd_tty, &writefds);

	result = select(10, &readfds, &writefds, NULL, &timeout);
	ASSERT_EQ(result, 2);

	ASSERT_EQ(FD_ISSET(fd_null, &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd_null, &writefds), 1);

	ASSERT_EQ(FD_ISSET(fd_tty, &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd_tty, &writefds), 0);

	ASSERT_SUCCESS(close(fd_null));
	ASSERT_SUCCESS(close(fd_tty));

	return 0;
}

int test_select_pipe()
{
	int result;
	int fd[2];
	struct timeval timeout = {0, 0};
	fd_set readfds, writefds;
	char buffer[16384]; // 16KB is the buffer of pipe created by `pipe`.

	ASSERT_SUCCESS(pipe(fd));

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(fd[0], &readfds);
	FD_SET(fd[0], &writefds);

	FD_SET(fd[1], &readfds);
	FD_SET(fd[1], &writefds);

	result = select(10, &readfds, &writefds, NULL, &timeout);
	ASSERT_EQ(result, 1);

	// There is no data to be read yet.
	ASSERT_EQ(FD_ISSET(fd[0], &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd[0], &writefds), 0);

	ASSERT_EQ(FD_ISSET(fd[1], &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd[1], &writefds), 1);

	write(fd[1], buffer, 2048);

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(fd[0], &readfds);
	FD_SET(fd[0], &writefds);

	FD_SET(fd[1], &readfds);
	FD_SET(fd[1], &writefds);

	result = select(10, &readfds, &writefds, NULL, &timeout);
	ASSERT_EQ(result, 2);

	// There is now data to be read.
	ASSERT_EQ(FD_ISSET(fd[0], &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd[0], &writefds), 0);

	ASSERT_EQ(FD_ISSET(fd[1], &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd[1], &writefds), 1);

	// Fill up the pipe buffer.
	write(fd[1], buffer, 16384 - 2048);

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(fd[0], &readfds);
	FD_SET(fd[0], &writefds);

	FD_SET(fd[1], &readfds);
	FD_SET(fd[1], &writefds);

	result = select(10, &readfds, &writefds, NULL, &timeout);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(FD_ISSET(fd[0], &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd[0], &writefds), 0);

	// No more data can be written unless read from the pipe.
	ASSERT_EQ(FD_ISSET(fd[1], &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd[1], &writefds), 0);

	// Read some data from the pipe buffer.
	read(fd[0], buffer, 2048);

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(fd[0], &readfds);
	FD_SET(fd[0], &writefds);

	FD_SET(fd[1], &readfds);
	FD_SET(fd[1], &writefds);

	result = select(10, &readfds, &writefds, NULL, &timeout);
	ASSERT_EQ(result, 2);

	// There is some data left in the buffer that can be read.
	ASSERT_EQ(FD_ISSET(fd[0], &readfds), 1);
	ASSERT_EQ(FD_ISSET(fd[0], &writefds), 0);

	// Some more data can be written to the pipe.
	ASSERT_EQ(FD_ISSET(fd[1], &readfds), 0);
	ASSERT_EQ(FD_ISSET(fd[1], &writefds), 1);

	ASSERT_SUCCESS(close(fd[0]));
	ASSERT_SUCCESS(close(fd[1]));

	return 0;
}

void cleanup()
{
	remove("t-select-rw");
	remove("t-select-w");
	remove("t-select-r");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_select_err());
	TEST(test_select_file());
	TEST(test_select_devices());
	TEST(test_select_pipe());

	VERIFY_RESULT_AND_EXIT();
}
