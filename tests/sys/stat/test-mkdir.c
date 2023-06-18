/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <Windows.h>

int test_ENOENT()
{
	errno = 0;
	int status = mkdir("", 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
	return 0;
}

int test_mkdir()
{
	errno = 0;
	int status;
	const char *dirname = "t-mkdir";

	status = mkdir(dirname, 0700);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(GetFileAttributesA(dirname), FILE_ATTRIBUTE_DIRECTORY);

	// should fail with EEXIST
	status = mkdir(dirname, 0700);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

int test_mkdirat()
{
	errno = 0;
	int status;
	int dirfd;
	const char *dirname = "t-mkdirat";
	const char *dirname_child = "t-mkdirat-child";

	status = mkdir(dirname, 0700);
	ASSERT_EQ(status, 0);

	dirfd = open(dirname, O_RDONLY);
	ASSERT_EQ(dirfd,3);

	status = mkdirat(dirfd, dirname_child, 0700);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(GetFileAttributesA("t-mkdirat/t-mkdirat-child"), FILE_ATTRIBUTE_DIRECTORY);

	ASSERT_SUCCESS(unlinkat(dirfd,dirname_child,AT_REMOVEDIR));
	ASSERT_SUCCESS(close(dirfd));
	ASSERT_SUCCESS(rmdir(dirname));

	return 0;
}

void cleanup()
{
	remove("t-mkdir");
	remove("t-mkdirat/t-mkdirat-child");
	remove("t-mkdirat");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_ENOENT());
	TEST(test_mkdir());
	TEST(test_mkdirat());

	VERIFY_RESULT_AND_EXIT();
}
