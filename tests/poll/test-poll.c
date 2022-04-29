/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int test_poll_err()
{
	int result;
	struct pollfd fds[1];

	fds[0].fd = -1;
	fds[0].events = POLLIN | POLLOUT;

	result = poll(fds, 1, 0);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(fds[0].revents, POLLNVAL);

	return 0;
}

int test_poll_file()
{
	int result;
	int fd_rw, fd_w, fd_r;
	struct pollfd fds[3];
	const char *filename_rw = "t-poll-rw";
	const char *filename_w = "t-poll-w";
	const char *filename_r = "t-poll-r";

	fd_rw = open(filename_rw, O_RDWR | O_CREAT, 0700);
	fd_w = open(filename_w, O_WRONLY | O_CREAT, 0700);
	fd_r = open(filename_r, O_RDWR | O_CREAT, 0700);

	ASSERT_SUCCESS(close(fd_r));
	fd_r = open(filename_r, O_RDONLY);

	fds[0].fd = fd_rw;
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd_w;
	fds[1].events = POLLIN | POLLOUT;

	fds[2].fd = fd_r;
	fds[2].events = POLLIN | POLLOUT;

	result = poll(fds, 3, 0);
	ASSERT_EQ(result, 3);

	ASSERT_EQ(fds[0].revents, (POLLIN | POLLOUT));
	ASSERT_EQ(fds[1].revents, POLLOUT);
	ASSERT_EQ(fds[2].revents, POLLIN);

	ASSERT_SUCCESS(close(fd_rw));
	ASSERT_SUCCESS(close(fd_w));
	ASSERT_SUCCESS(close(fd_r));

	ASSERT_SUCCESS(unlink(filename_rw));
	ASSERT_SUCCESS(unlink(filename_w));
	ASSERT_SUCCESS(unlink(filename_r));

	return 0;
}

int test_poll_devices()
{
	int result;
	int fd_null, fd_tty;
	struct pollfd fds[2];

	fd_null = open("/dev/null", O_RDWR);
	fd_tty = open("/dev/tty", O_RDONLY);

	fds[0].fd = fd_null;
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd_tty;
	fds[1].events = POLLIN | POLLOUT;

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(fds[0].revents, (POLLIN | POLLOUT));
	ASSERT_EQ(fds[1].revents, 0);

	ASSERT_SUCCESS(close(fd_null));
	ASSERT_SUCCESS(close(fd_tty));

	return 0;
}

int test_poll_pipe()
{
	int result;
	int fd[2];
	struct pollfd fds[2];
	char buffer[16384]; // 16KB is the buffer of pipe created by `pipe`.

	ASSERT_SUCCESS(pipe(fd));

	fds[0].fd = fd[0];
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd[1];
	fds[1].events = POLLIN | POLLOUT;

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 1);

	// There is no data to be read yet.
	ASSERT_EQ(fds[0].revents, 0);
	ASSERT_EQ(fds[1].revents, POLLOUT);

	write(fd[1], buffer, 2048);

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 2);

	// There is now data to be read.
	ASSERT_EQ(fds[0].revents, POLLIN);
	ASSERT_EQ(fds[1].revents, POLLOUT);

	// Fill up the pipe buffer.
	write(fd[1], buffer, 16384 - 2048);

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 1);

	ASSERT_EQ(fds[0].revents, POLLIN);
	// No more data can be written unless read from the pipe.
	ASSERT_EQ(fds[1].revents, 0);

	// Read some data from the pipe buffer.
	read(fd[0], buffer, 2048);

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 2);

	// There is some data left in the buffer that can be read.
	ASSERT_EQ(fds[0].revents, POLLIN);
	// Some more data can be written to the pipe.
	ASSERT_EQ(fds[1].revents, POLLOUT);

	ASSERT_SUCCESS(close(fd[0]));
	ASSERT_SUCCESS(close(fd[1]));

	return 0;
}

int test_poll_pipe_err()
{
	int result;
	int fd[2];
	struct pollfd fds[2];

	// Close the read end of pipe.
	ASSERT_SUCCESS(pipe(fd));

	fds[0].fd = fd[0];
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd[1];
	fds[1].events = POLLIN | POLLOUT;

	ASSERT_SUCCESS(close(fd[0]));

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 2);

	ASSERT_EQ(fds[0].revents, POLLNVAL);
	ASSERT_EQ(fds[1].revents, (POLLOUT | POLLERR));

	ASSERT_SUCCESS(close(fd[1]));

	// Close the write end of pipe.
	ASSERT_SUCCESS(pipe(fd));

	fds[0].fd = fd[0];
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd[1];
	fds[1].events = POLLIN | POLLOUT;

	ASSERT_SUCCESS(close(fd[1]));

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 2);

	ASSERT_EQ(fds[0].revents, POLLHUP);
	ASSERT_EQ(fds[1].revents, POLLNVAL);

	ASSERT_SUCCESS(close(fd[0]));

	// Close the write end of pipe, but write some data to it first.
	ASSERT_SUCCESS(pipe(fd));
	write(fd[1], "hello", 5);

	fds[0].fd = fd[0];
	fds[0].events = POLLIN | POLLOUT;

	fds[1].fd = fd[1];
	fds[1].events = POLLIN | POLLOUT;

	ASSERT_SUCCESS(close(fd[1]));

	result = poll(fds, 2, 0);
	ASSERT_EQ(result, 2);

	ASSERT_EQ(fds[0].revents, (POLLIN | POLLHUP));
	ASSERT_EQ(fds[1].revents, POLLNVAL);

	ASSERT_SUCCESS(close(fd[0]));

	return 0;
}

int test_poll_wait()
{
	int result;
	int fd[2];
	struct pollfd fds[1];

	// Close the read end of pipe.
	ASSERT_SUCCESS(pipe(fd));

	fds[0].fd = fd[0];
	fds[0].events = POLLIN | POLLOUT;

	result = poll(fds, 1, 0);
	ASSERT_EQ(result, 0);

	// Just check whether a wait happens.
	result = poll(fds, 1, 20);
	ASSERT_EQ(result, 0);

	ASSERT_SUCCESS(close(fd[0]));
	ASSERT_SUCCESS(close(fd[1]));

	return 0;
}

void cleanup()
{
	remove("t-poll-rw");
	remove("t-poll-w");
	remove("t-poll-r");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_poll_err());
	TEST(test_poll_file());
	TEST(test_poll_devices());
	TEST(test_poll_pipe());
	TEST(test_poll_pipe_err());
	TEST(test_poll_wait());

	VERIFY_RESULT_AND_EXIT();
}
