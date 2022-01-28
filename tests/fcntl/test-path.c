/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <internal/path.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// UNICODE_STRING *get_absolute_ntpath(int dirfd, const char *path);
// UNICODE_STRING *xget_absolute_ntpath(int dirfd, const char *path);

#define get_absolute_ntpath xget_absolute_ntpath

static wchar_t cwd[32768]; // MAX_PATH for windows
static wchar_t cdrive[1024];
static int length, cdrive_length;

int test_null()
{
	UNICODE_STRING *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "NUL");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\Null", path->Length);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/null");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\Null", path->Length);
	free_ntpath(path);

	return 0;
}

int test_con()
{
	UNICODE_STRING *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "CON");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/tty");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	free_ntpath(path);

	return 0;
}

int test_relative()
{
	UNICODE_STRING *path;

	path = get_absolute_ntpath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length] = L'\\';
	cwd[length + 1] = L'a';
	cwd[length + 2] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length + 2] = L'\\';
	cwd[length + 3] = L'a';
	cwd[length + 4] = L'b';
	cwd[length + 5] = L'c';
	cwd[length + 6] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	// mix and forward slash with backward slashes
	path = get_absolute_ntpath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length + 2] = L'b';
	cwd[length + 3] = L'c';
	cwd[length + 4] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	// trailing slash
	cwd[length + 4] = L'\\';
	cwd[length + 5] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length] = L'\\';
	cwd[length + 1] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	cwd[length] = L'\0';

	return 0;
}

int test_at()
{
	int fd;
	UNICODE_STRING *path;

	fd = open("t-path", O_RDONLY);
	ASSERT_EQ(fd, 3);

	wcscat(cwd, L"\\t-path");

	path = get_absolute_ntpath(fd, ".");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	wcscat(cwd, L"\\abc");
	path = get_absolute_ntpath(fd, "abc");
	ASSERT_WSTREQ(path->Buffer, cwd);
	free_ntpath(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute()
{

	int fd;
	UNICODE_STRING *path;

	wcscat(cdrive, L"\\abc");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	wcscat(cdrive, L"\\");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	wcscat(cdrive, L"\\");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	cdrive[cdrive_length] = L'\0';
	wcscat(cdrive, L"\\abc");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	cdrive[cdrive_length] = L'\0';
	wcscat(cdrive, L"\\");

	path = get_absolute_ntpath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	// bad path
	path = get_absolute_ntpath(AT_FDCWD, "C:/..");
	ASSERT_NULL(path);
	free_ntpath(path); // should be a nop

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	wcscat(cdrive, L"\\abc\\");
	path = get_absolute_ntpath(fd, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free_ntpath(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int main()
{
	char cwd_buf[32768];
	UNICODE_STRING *cwd_nt, *cdrive_nt;

	getcwd(cwd_buf, 32768);
	cwd_nt = xget_absolute_ntpath(AT_FDCWD, cwd_buf);

	printf("Current Working Directory (NT): %ls\n", cwd_nt->Buffer);
	memcpy(cwd, cwd_nt->Buffer, cwd_nt->MaximumLength);
	length = cwd_nt->Length / sizeof(wchar_t);

	cdrive_nt = xget_absolute_ntpath(AT_FDCWD, "C:");
	printf("C Drive (NT): %ls\n", cdrive_nt->Buffer);
	memcpy(cdrive, cdrive_nt->Buffer, cdrive_nt->MaximumLength);
	cdrive_length = cdrive_nt->Length / sizeof(wchar_t);

	free_ntpath(cwd_nt);
	free_ntpath(cdrive_nt);

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
