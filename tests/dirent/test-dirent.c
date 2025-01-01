/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int setup()
{
	int fd;

	ASSERT_SUCCESS(mkdir("t", 0700));

	fd = creat("t/a1", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a2", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a3", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a4", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a5", 0700);
	ASSERT_SUCCESS(close(fd));
	fd = creat("t/a6", 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(mkdir("t/d", 0700));

	ASSERT_SUCCESS(symlink("a1", "t/s1"));
	ASSERT_SUCCESS(symlink("a2", "t/s2"));
	ASSERT_SUCCESS(symlink("d", "t/sd"));

	return 0;
}

int cleanup()
{
	ASSERT_SUCCESS(rmdir("t/sd"));
	ASSERT_SUCCESS(rmdir("t/d"));
	ASSERT_SUCCESS(unlink("t/s1"));
	ASSERT_SUCCESS(unlink("t/s2"));
	ASSERT_SUCCESS(unlink("t/a1"));
	ASSERT_SUCCESS(unlink("t/a2"));
	ASSERT_SUCCESS(unlink("t/a3"));
	ASSERT_SUCCESS(unlink("t/a4"));
	ASSERT_SUCCESS(unlink("t/a5"));
	ASSERT_SUCCESS(unlink("t/a6"));
	ASSERT_SUCCESS(rmdir("t"));

	return 0;
}

int test_EBADF()
{
	struct dirent *d = readdir(NULL);
	ASSERT_NULL(d);
	ASSERT_ERRNO(EBADF);

	return 0;
}

int test_readdir() // telldir is also tested
{
	/* We step through the directory and check whether the order of files
	   listed is correct (alphabetical), it's deduced type is correct.
	*/
	struct dirent *d = NULL;
	DIR *D = opendir("t");
	ASSERT_NOTNULL(D);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, ".");
	ASSERT_EQ(d->d_namlen, 1);
	ASSERT_EQ(d->d_type, DT_DIR);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "..");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_DIR);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a1");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a2");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a3");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a4");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a5");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a6");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "d");
	ASSERT_EQ(d->d_namlen, 1);
	ASSERT_EQ(d->d_type, DT_DIR);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "s1");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "s2");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	d = readdir(D);
	ASSERT_STREQ(d->d_name, "sd");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	d = readdir(D);
	ASSERT_NULL(d);

	ASSERT_SUCCESS(closedir(D));

	return 0;
}

int test_seekdir() // rewinddir is also tested here
{
	struct dirent *d = NULL;
	off_t offset;

	DIR *D = opendir("t");
	ASSERT_NOTNULL(D);

	d = readdir(D);
	d = readdir(D);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a1");
	ASSERT_EQ(d->d_type, DT_REG);

	offset = telldir(D);
	d = readdir(D);
	d = readdir(D);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a4");
	ASSERT_EQ(d->d_type, DT_REG);

	seekdir(D, offset);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a2");
	ASSERT_EQ(d->d_type, DT_REG);

	rewinddir(D);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, ".");
	ASSERT_EQ(d->d_type, DT_DIR);

	ASSERT_SUCCESS(closedir(D));

	return 0;
}

int test_readdir_r() // telldir is also tested
{
	/* We step through the directory and check whether the order of files
	   listed is correct (alphabetical), it's deduced type is correct.
	*/
	int status;
	int fd;
	struct dirent entry;
	struct dirent *d;

	fd = open("t", O_DIRECTORY);
	ASSERT_NOTEQ(fd, -1);
	DIR *D = fdopendir(fd);
	ASSERT_NOTNULL(D);

	status = readdir_r(NULL, &entry, &d);
	ASSERT_EQ(status, EBADF);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, ".");
	ASSERT_EQ(d->d_namlen, 1);
	ASSERT_EQ(d->d_type, DT_DIR);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "..");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_DIR);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a1");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a2");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a3");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a4");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a5");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "a6");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_REG);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "d");
	ASSERT_EQ(d->d_namlen, 1);
	ASSERT_EQ(d->d_type, DT_DIR);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "s1");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "s2");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_STREQ(d->d_name, "sd");
	ASSERT_EQ(d->d_namlen, 2);
	ASSERT_EQ(d->d_type, DT_LNK);

	status = readdir_r(D, &entry, &d);
	ASSERT_EQ(status, 0);
	ASSERT_NULL(d);

	ASSERT_SUCCESS(closedir(D));
	errno = 0;
	ASSERT_FAIL(close(fd));
	ASSERT_ERRNO(EBADF);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_EBADF());

	// If the setup or cleanup fails exit the program
	if (setup() == 0)
	{
		TEST(test_readdir());
		TEST(test_seekdir());
		TEST(test_readdir_r());
		if (cleanup() == 1)
		{
			printf("Cleanup failed\n");
			exit(1);
		}
	}
	else
	{
		printf("Setup failed\n");
		exit(1);
	}

	VERIFY_RESULT_AND_EXIT();
}
