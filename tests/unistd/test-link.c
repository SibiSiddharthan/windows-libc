/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void setup()
{
	int fd;
	fd = creat("t-link1", 0700);
	close(fd);
	fd = creat("t-link2", 0700);
	close(fd);
}

void test_ENOENT()
{
	int status = link("t-link3", "t-link4");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);
}

void test_EEXIST()
{
	int status = link("t-link1", "t-link2");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);
}

void test_okay()
{
	int status = link("t-link1", "t-link3");
	ASSERT_EQ(status, 0);
	status = unlink("t-link3");
	ASSERT_EQ(status, 0);
}

void cleanup()
{
	unlink("t-link1");
	unlink("t-link2");
}

int main()
{
	setup();
	test_ENOENT();
	test_EEXIST();
	test_okay();
	cleanup();
	return 0;
}
