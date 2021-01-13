/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-ext.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <sys/stat.h>

void test_file()
{
	int fd = creat("t-rename", 0700);
	close(fd);

	int status = rename("t-rename", "t-rename.r");
	ASSERT_EQ(status, 0);

	status = unlink("t-rename.r");
	ASSERT_EQ(status, 0);
	status = unlink("t-rename");
	ASSERT_EQ(status, -1);
}

void test_dir()
{
	mkdir("t-rename", 0700);

	int status = rename("t-rename", "t-rename.r");
	ASSERT_EQ(status, 0);

	status = rmdir("t-rename.r");
	ASSERT_EQ(status, 0);
	status = rmdir("t-rename");
	ASSERT_EQ(status, -1);
}

void test_overwrite()
{
	int fd = creat("t-rename1", 0700);
	close(fd);
	fd = creat("t-rename2", 0700);
	close(fd);

	int status = rename("t-rename1", "t-rename2");
	ASSERT_EQ(status, 0);

	status = unlink("t-rename2");
	ASSERT_EQ(status, 0);
	status = unlink("t-rename1");
	ASSERT_EQ(status, -1);
}

void test_symlink()
{
	int fd = creat("t-rename", 0700);
	close(fd);
	symlink("t-rename", "t-rename.sym");

	int status = rename("t-rename.sym", "t-rename.sym.r");
	ASSERT_EQ(status, 0);

	char buf[260];
	ssize_t length = readlink("t-rename.sym.r", buf, 260);
	buf[length] = '\0';
	ASSERT_STREQ(buf, "t-rename");

	status = unlink("t-rename.sym.r");
	ASSERT_EQ(status, 0);
	status = unlink("t-rename.sym");
	ASSERT_EQ(status, -1);
	status = unlink("t-rename");
}

void test_hardlink()
{
	int fd = creat("t-rename", 0700);
	close(fd);
	link("t-rename", "t-rename.lnk");

	int status = rename("t-rename", "t-rename.lnk");
	ASSERT_EQ(status, 0);

	status = unlink("t-rename");
	ASSERT_EQ(status, 0);
	status = unlink("t-rename.lnk");
	ASSERT_EQ(status, 0);
}

void test_existing_empty_dir()
{
	mkdir("t-rename1", 0700);
	mkdir("t-rename2", 0700);

	int status = rename("t-rename1", "t-rename2");
	ASSERT_EQ(status, 0);

	status = rmdir("t-rename2");
	ASSERT_EQ(status, 0);
	status = rmdir("t-rename1");
	ASSERT_EQ(status, -1);
}

int main()
{
	test_file();
	test_dir();
	test_overwrite();
	test_symlink();
	test_hardlink();
	test_existing_empty_dir();
	return 0;
}
