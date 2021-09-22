/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void test_redirect_stdout()
{
	FILE *f = freopen("t-freopen1", "w", stdout);
	printf("hello");
	ASSERT_EQ(fileno(f),1);
	fclose(f);

	int fd = open("t-freopen1", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, 1);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	rbuf[llength] = '\0';
	ASSERT_STREQ(rbuf, "hello");
	close(fd);

	unlink("t-freopen1");
}

int main()
{
	test_redirect_stdout();
	//test_reopen_same_file();
	return 0;
}
