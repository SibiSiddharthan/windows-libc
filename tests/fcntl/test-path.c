/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>

wchar_t *get_absolute_ntpath(int dirfd, const char *path);
static wchar_t cwd[32768]; // MAX_PATH for windows
static int length;

int test_null()
{
	wchar_t *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "NUL");
	ASSERT_WSTREQ(path, L"\\??\\NUL");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/null");
	ASSERT_WSTREQ(path, L"\\??\\NUL");
	free(path);

	return 0;
}

int test_con()
{
	wchar_t *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "CON");
	ASSERT_WSTREQ(path, L"\\??\\CON");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/tty");
	ASSERT_WSTREQ(path, L"\\??\\CON");
	free(path);

	return 0;
}

int test_relative()
{
	wchar_t *path;

	path = get_absolute_ntpath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length] = L'\\';
	cwd[length + 1] = L'a';
	cwd[length + 2] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length + 2] = L'\\';
	cwd[length + 3] = L'a';
	cwd[length + 4] = L'b';
	cwd[length + 5] = L'c';
	cwd[length + 6] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	// mix and forward slash with backward slashes
	path = get_absolute_ntpath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length + 2] = L'b';
	cwd[length + 3] = L'c';
	cwd[length + 4] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	// trailing slash
	cwd[length + 4] = L'\\';
	cwd[length + 5] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length] = L'\\';
	cwd[length + 1] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	cwd[length] = L'\0';

	return 0;
}

int test_at()
{
	int fd;
	wchar_t *path;

	fd = open("t-path", O_RDONLY);
	ASSERT_EQ(fd, 3);

	wcscat(cwd, L"\\t-path");

	path = get_absolute_ntpath(fd, ".");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	wcscat(cwd, L"\\abc");
	path = get_absolute_ntpath(fd, "abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute()
{

	int fd;
	wchar_t *path;

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc\\");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path, L"\\??\\C:");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path, L"\\??\\C:\\");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path, L"\\??\\C:\\");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path, L"\\??\\C:");
	free(path);

	// bad path
	path = get_absolute_ntpath(AT_FDCWD, "C:/..");
	ASSERT_NULL(path);

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	path = get_absolute_ntpath(fd, "C:/abc/");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc\\");
	free(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int main()
{
	wgetcwd(cwd + 4, 32764); // To add \??\ at the beginning
	memcpy(cwd, L"\\??\\", 8);
	// getcwd returns with forward slashes, convert to backslashes
	for (int i = 0; cwd[i] != L'\0'; i++)
	{
		if (cwd[i] == L'/')
		{
			cwd[i] = L'\\';
		}
	}
	length = wcslen(cwd);

	INITIAILIZE_TESTS();

	TEST(test_null());
	TEST(test_con());

	TEST(test_relative());

	mkdir("t-path", 0700);
	TEST(test_at());
	TEST(test_absolute());
	rmdir("t-path");

	VERIFY_RESULT_AND_EXIT();
}
