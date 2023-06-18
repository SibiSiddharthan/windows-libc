/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/statfs.h>

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

int test_getmntinfo()
{
	int status;
	struct statfs *statfsbuf;

	status = getmntinfo(&statfsbuf, MNT_WAIT);
	ASSERT_NOTEQ(status, 0);

	printf("Got %d entries.\n", status);
	for (int i = 0; i < status; ++i)
	{
		print_statfs(&statfsbuf[i]);
		puts("\n");
	}

	free(statfsbuf);
	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_getmntinfo());
	VERIFY_RESULT_AND_EXIT();
}
