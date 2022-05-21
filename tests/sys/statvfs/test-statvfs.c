/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <unistd.h>

void print_statvfs(const struct statvfs *statvfsbuf)
{
	printf("f_bsize (Filesystem block size): %lu\n", statvfsbuf->f_bsize);
	printf("f_frsize (Fragment size): %lu\n", statvfsbuf->f_frsize);
	printf("f_flag (Filesystem attributes): %lu\n", statvfsbuf->f_flag);
	printf("f_blocks (Total number of blocks): %llu\n", statvfsbuf->f_blocks);
	printf("f_bfree (Number of free blocks): %llu\n", statvfsbuf->f_bfree);
	printf("f_bavail (Number of free blocks for unprivileged users): %llu\n", statvfsbuf->f_bavail);
	printf("f_fsid (Filesystem ID): %lu\n", statvfsbuf->f_fsid);
	printf("f_namemax (Maximum path length): %lu\n", statvfsbuf->f_namemax);
	printf("f_fstypename (Filesystem type): %s\n", statvfsbuf->f_fstypename);
}

int test_statvfs()
{
	int status;
	struct statvfs statvfsbuf;
	char cwd[256];

	getcwd(cwd, 256);

	status = statvfs(cwd, &statvfsbuf);
	ASSERT_EQ(status, 0);

	print_statvfs(&statvfsbuf);

	return 0;
}

int test_fstatvfs()
{
	int status;
	int fd;
	struct statvfs statvfsbuf;
	char cwd[256];

	getcwd(cwd, 256);

	fd = open(cwd, O_PATH | O_DIRECTORY);
	ASSERT_EQ(fd, 3);

	status = fstatvfs(fd, &statvfsbuf);
	ASSERT_EQ(status, 0);

	print_statvfs(&statvfsbuf);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_statvfs());
	TEST(test_fstatvfs());
	VERIFY_RESULT_AND_EXIT();
}
