/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

int test_recursive_lock()
{
	int status;
	int fd;
	const char *filename = "t-flock-recursive";

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0700);

	status = flock(fd, LOCK_EX);
	ASSERT_EQ(status, 0);

	errno = 0;
	// LOCK_NB flags needs to OR'd otherwise this would deadlock
	status = flock(fd, LOCK_EX | LOCK_NB);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EWOULDBLOCK);

	status = flock(fd, LOCK_UN);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = flock(fd, LOCK_UN);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOLCK);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_exclusive_lock()
{
	int status;
	int fd, dfd;
	ssize_t result;
	char rbuf[16];
	const char *filename = "t-flock-exclusive";

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0700);
	result = write(fd, "hello", 5);
	ASSERT_EQ(result, 5);
	lseek(fd, 0, SEEK_SET);

	status = flock(fd, LOCK_EX);
	ASSERT_EQ(status, 0);

	// The open should still succeed
	dfd = open(filename, O_RDWR);
	ASSERT_NOTEQ(dfd, -1);

	// This read should fail
	errno = 0;
	result = read(dfd, rbuf, 16);
	ASSERT_EQ(result, -1);
	ASSERT_ERRNO(EIO);

	memset(rbuf, 0, 16);
	result = read(fd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	status = flock(fd, LOCK_UN);
	ASSERT_EQ(status, 0);

	// After unlocking this should succeed now
	memset(rbuf, 0, 16);
	result = read(dfd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(dfd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_shared_lock()
{
	int status;
	int fd, dfd;
	ssize_t result;
	char rbuf[16];
	const char *filename = "t-flock-shared";

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0700);
	result = write(fd, "hello", 5);
	ASSERT_EQ(result, 5);
	lseek(fd, 0, SEEK_SET);

	status = flock(fd, LOCK_SH);
	ASSERT_EQ(status, 0);

	// The open will succeed
	dfd = open(filename, O_RDWR);
	ASSERT_NOTEQ(dfd, -1);

	// Both the reads should succeed
	memset(rbuf, 0, 16);
	result = read(dfd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	memset(rbuf, 0, 16);
	result = read(fd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	// Do it again this time dfd also holds a shared lock
	lseek(fd, 0, SEEK_SET);
	lseek(dfd, 0, SEEK_SET);

	status = flock(dfd, LOCK_SH);
	ASSERT_EQ(status, 0);

	memset(rbuf, 0, 16);
	result = read(dfd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	memset(rbuf, 0, 16);
	result = read(fd, rbuf, 16);
	ASSERT_EQ(result, 5);
	ASSERT_MEMEQ(rbuf, "hello", 5);

	status = flock(fd, LOCK_UN);
	ASSERT_EQ(status, 0);

	status = flock(dfd, LOCK_UN);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(dfd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-flock-recursive");
	remove("t-flock-exclusive");
	remove("t-flock-shared");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_recursive_lock());
	TEST(test_exclusive_lock());
	TEST(test_shared_lock());

	VERIFY_RESULT_AND_EXIT();
}
