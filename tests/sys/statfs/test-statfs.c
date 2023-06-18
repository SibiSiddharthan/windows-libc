/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <unistd.h>

void print_statfs(const struct statfs *statfsbuf)
{
	printf("f_bsize (Filesystem block size): %lu\n", statfsbuf->f_bsize);
	printf("f_iosize (Optimal transfer size): %lu\n", statfsbuf->f_iosize);
	printf("f_flag (Filesystem attributes): %lu\n", statfsbuf->f_flag);
	printf("f_blocks (Total number of blocks): %llu\n", statfsbuf->f_blocks);
	printf("f_bfree (Number of free blocks): %llu\n", statfsbuf->f_bfree);
	printf("f_bavail (Number of free blocks for unprivileged users): %llu\n", statfsbuf->f_bavail);
	printf("f_fsid (Filesystem ID): %u-%u\n", statfsbuf->f_fsid.major, statfsbuf->f_fsid.minor);
	printf("f_namemax (Maximum path length): %lu\n", statfsbuf->f_namemax);
	printf("f_fstypename (Filesystem type): %s\n", statfsbuf->f_fstypename);
	printf("f_mntfromname (Mount from location): %s\n", statfsbuf->f_mntfromname);
	printf("f_mntonname (Mount to location): %s\n", statfsbuf->f_mntonname);
}

int test_statfs()
{
	int status;
	struct statfs statfsbuf;
	char cwd[256];

	getcwd(cwd, 256);

	status = statfs(cwd, &statfsbuf);
	ASSERT_EQ(status, 0);

	print_statfs(&statfsbuf);

	return 0;
}

int test_fstatfs()
{
	int status;
	int fd;
	struct statfs statfsbuf;
	char cwd[256];

	getcwd(cwd, 256);

	fd = open(cwd, O_PATH | O_DIRECTORY);
	ASSERT_EQ(fd, 3);

	status = fstatfs(fd, &statfsbuf);
	ASSERT_EQ(status, 0);

	print_statfs(&statfsbuf);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_statfs());
	TEST(test_fstatfs());
	VERIFY_RESULT_AND_EXIT();
}
