/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <err.h>
#include <string.h>
#include <unistd.h>

const char *filename = "t-err.log";

void setup()
{
	freopen(filename, "w+", stderr);
}

int cleanup()
{
	ASSERT_SUCCESS(fclose(stderr));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int check_log(const char *content)
{
	char buf[260];
	fseek(stderr, 0, SEEK_SET);
	ASSERT_EQ(ftell(stderr), 0);
	fread(buf, 1, 260, stderr);
	ASSERT_MEMEQ(buf, content, (int)strlen(content));

	// truncate file to make testing easier
	ASSERT_SUCCESS(ftruncate(fileno(stderr), 0));
	lseek(fileno(stderr), 0, SEEK_SET);
	return 0;
}

int test_warn()
{
	char buffer[512];
	errno = EINVAL;

	warn("sample warning");
	snprintf(buffer, 512, "test-err: %s: %s\n", "sample warning", strerror(EINVAL));
	ASSERT_SUCCESS(check_log(buffer))
	return 0;
}

int test_warnx()
{
	char buffer[512];
	warnx("sample warning");
	snprintf(buffer, 512, "test-err: %s\n", "sample warning");
	ASSERT_SUCCESS(check_log(buffer))
	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	setup();
	TEST(test_warn());
	TEST(test_warnx());
	cleanup();

	VERIFY_RESULT_AND_EXIT();
}
