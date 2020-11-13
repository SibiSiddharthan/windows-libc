/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <test-macros.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wchar.h>

void setup()
{
	mkdir("t", 0700);

	int fd;
	fd = creat("t/a1", 0700);
	close(fd);
	fd = creat("t/a2", 0700);
	close(fd);
	fd = creat("t/a3", 0700);
	close(fd);
	fd = creat("t/a4", 0700);
	close(fd);
	fd = creat("t/a5", 0700);
	close(fd);
	fd = creat("t/a6", 0700);
	close(fd);

	mkdir("t/d", 0);

	symlink("a1", "t/s1");
	symlink("a2", "t/s2");
	symlink("d", "t/sd");
}

void cleanup()
{
	rmdir("t/sd");
	rmdir("t/d");
	unlink("t/s1");
	unlink("t/s2");
	unlink("t/a1");
	unlink("t/a2");
	unlink("t/a3");
	unlink("t/a4");
	unlink("t/a5");
	unlink("t/a6");
	rmdir("t");
}

void test_EBADF()
{
	struct dirent *d = readdir(NULL);
	ASSERT_NULL(d);
}

void test_readdir() // telldir is also tested
{
	/* We step through the directory and check whether the order of files
	   listed is correct (alphabetical), it's deduced type is correct.
	*/
	DIR *D = opendir("t");
	struct dirent *d = NULL;
	d = readdir(D);
	ASSERT_STREQ(d->d_name, ".");
	ASSERT_EQ(d->d_type, DT_DIR);
	ASSERT_EQ(telldir(D), 1);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "..");
	ASSERT_EQ(d->d_type, DT_DIR);
	ASSERT_EQ(telldir(D), 2);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a1");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 3);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a2");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 4);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a3");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 5);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a4");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 6);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a5");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 7);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a6");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 8);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "d");
	ASSERT_EQ(d->d_type, DT_DIR);
	ASSERT_EQ(telldir(D), 9);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "s1");
	ASSERT_EQ(d->d_type, DT_LNK);
	ASSERT_EQ(telldir(D), 10);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "s2");
	ASSERT_EQ(d->d_type, DT_LNK);
	ASSERT_EQ(telldir(D), 11);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "sd");
	ASSERT_EQ(d->d_type, DT_LNK);
	ASSERT_EQ(telldir(D), 12);
	d = readdir(D);
	ASSERT_NULL(d);
	ASSERT_EQ(telldir(D), 12);
	closedir(D);
}

void test_seekdir() // rewinddir is also tested here
{
	DIR *D = opendir("t");
	struct dirent *d = NULL;
	seekdir(D, 5);

	ASSERT_EQ(telldir(D), 5);
	d = readdir(D);
	ASSERT_STREQ(d->d_name, "a4");
	ASSERT_EQ(d->d_type, DT_REG);
	ASSERT_EQ(telldir(D), 6);

	seekdir(D, 12);
	d = readdir(D);
	ASSERT_NULL(d);

	rewinddir(D);
	ASSERT_EQ(telldir(D), 0);
	d = readdir(D);
	ASSERT_EQ(telldir(D), 1);
	ASSERT_STREQ(d->d_name, ".");
	closedir(D);
}

int main()
{
	setup();
	test_EBADF();
	test_readdir();
	test_seekdir();
	cleanup();
	return 0;
}
