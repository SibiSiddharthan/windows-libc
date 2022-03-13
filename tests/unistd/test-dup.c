/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <tests/test.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int test_dup()
{
	int fd, nfd;
	const char *filename = "t-dup";

	fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);
	nfd = dup(fd);
	ASSERT_EQ(nfd, 4);

	// Test if file pointers are working properly
	write(fd, "hello1", 6);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 6);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 6);
	write(nfd, "hello2", 6);
	ASSERT_EQ(lseek(fd, 0, SEEK_CUR), 12);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 12);

	ASSERT_SUCCESS(close(fd));

	write(nfd, "hello3", 6);
	ASSERT_EQ(lseek(nfd, 0, SEEK_CUR), 18);

	ASSERT_SUCCESS(close(nfd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_dup2()
{
	int fd, nfd1, nfd2, nfd3;
	const char *filename = "t-dup2";

	fd = creat(filename, 0700);
	ASSERT_NOTEQ(fd, -1); // valid fd
	nfd1 = dup2(fd, 9);
	ASSERT_EQ(nfd1, 9);
	nfd2 = dup2(nfd1, 11);
	ASSERT_EQ(nfd2, 11);
	nfd3 = dup2(nfd2, 11);
	ASSERT_EQ(nfd3, 11);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(nfd1));
	ASSERT_SUCCESS(close(nfd2));
	ASSERT_FAIL(close(nfd3)); // Both nfd2, nfd3 are the same. This should fail.

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_dup2_EBADF()
{
	int fd = dup2(5, 5);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EBADF);
	return 0;
}

int test_dup3_EINVAL()
{
	int fd = dup3(1, 1, 0);
	ASSERT_EQ(fd, -1);
	ASSERT_ERRNO(EINVAL);
	return 0;
}

int test_dup3_O_CLOEXEC()
{
	int fd, nfd;
	const char *filename = "t-dup3";

	fd = creat(filename, 0700);
	ASSERT_NOTEQ(fd, -1);

	nfd = dup3(fd, 8, O_CLOEXEC);
	ASSERT_EQ(nfd, 8);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(close(nfd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-dup");
	remove("t-dup2");
	remove("t-dup3");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_dup());
	TEST(test_dup2());
	TEST(test_dup2_EBADF());
	TEST(test_dup3_EINVAL());
	TEST(test_dup3_O_CLOEXEC());

	VERIFY_RESULT_AND_EXIT();
}
