/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <test-macros.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>

int wlibc_open2(const char *name, int oflags, ...);
#define open wlibc_open2

wchar_t *get_absolute_ntpath(int dirfd, const char *path);
static wchar_t cwd[32768]; // MAX_PATH for windows
static int length;

void test_null()
{
	wchar_t *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "NUL");
	ASSERT_WSTREQ(path, L"\\??\\NUL");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/null");
	ASSERT_WSTREQ(path, L"\\??\\NUL");
	free(path);
}

void test_con()
{
	wchar_t *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "CON");
	ASSERT_WSTREQ(path, L"\\??\\CON");
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/tty");
	ASSERT_WSTREQ(path, L"\\??\\CON");
	free(path);
}

void test_relative()
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
}

void test_at()
{
	int fd = open("t-path", O_RDONLY);
	wchar_t *path;
	wcscat(cwd, L"\\t-path");

	path = get_absolute_ntpath(fd, ".");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	wcscat(cwd, L"\\abc");
	path = get_absolute_ntpath(fd, "abc");
	ASSERT_WSTREQ(path, cwd);
	free(path);

	close(fd);
}

void test_absolute()
{

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

	int fd = open("t-path", O_RDONLY);
	// fd should be ignored
	path = get_absolute_ntpath(fd, "C:/abc/");
	ASSERT_WSTREQ(path, L"\\??\\C:\\abc\\");
	free(path);
	close(fd);
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

	test_null();
	test_con();

	test_relative();

	mkdir("t-path", 0777);
	test_at();
	test_absolute();
	rmdir("t-path");
}
