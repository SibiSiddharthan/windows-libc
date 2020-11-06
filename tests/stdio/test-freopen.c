/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <stdio-hooks.h>
#include <fcntl.h>
#include <fcntl_internal.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>

void test_redirect_stdout()
{
	FILE *f = freopen("t-freopen", "w", stdout);
	printf("hello");
	fclose(f);

	int fd = open("t-freopen", O_RDONLY | O_EXCL);
	ASSERT_EQ(fd, 1);
	char rbuf[16];
	ssize_t llength = read(fd, rbuf, 16);
	ASSERT_EQ(llength, 5);
	rbuf[llength] = '\0';
	ASSERT_STREQ(rbuf, "hello");
	close(fd);

	unlink("t-freopen");
}

int main()
{
	test_redirect_stdout();
	return 0;
}