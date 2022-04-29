/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <internal/path.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static wchar_t cwd_dos[32768]; // MAX_PATH for windows
static wchar_t cwd_nt[32768];  // MAX_PATH for windows
static wchar_t cdrive[1024];
static int cwd_dos_length, cwd_nt_length, cdrive_length;

int test_null()
{
	UNICODE_STRING *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "NUL");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\Null", path->Length);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/null");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\Null", path->Length);
	free(path);

	return 0;
}

int test_con()
{
	UNICODE_STRING *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "CON");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/tty");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	free(path);

	return 0;
}

int test_relative_nt()
{
	UNICODE_STRING *path;

	path = get_absolute_ntpath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length] = L'\\';
	cwd_nt[cwd_nt_length + 1] = L'a';
	cwd_nt[cwd_nt_length + 2] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length + 2] = L'\\';
	cwd_nt[cwd_nt_length + 3] = L'a';
	cwd_nt[cwd_nt_length + 4] = L'b';
	cwd_nt[cwd_nt_length + 5] = L'c';
	cwd_nt[cwd_nt_length + 6] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	// mix and forward slash with backward slashes
	path = get_absolute_ntpath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length + 2] = L'b';
	cwd_nt[cwd_nt_length + 3] = L'c';
	cwd_nt[cwd_nt_length + 4] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	// trailing slash
	cwd_nt[cwd_nt_length + 4] = L'\\';
	cwd_nt[cwd_nt_length + 5] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length] = L'\\';
	cwd_nt[cwd_nt_length + 1] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	cwd_nt[cwd_nt_length] = L'\0';

	return 0;
}

int test_relative_dos()
{
	UNICODE_STRING *path;

	path = get_absolute_dospath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length] = L'\\';
	cwd_dos[cwd_dos_length + 1] = L'a';
	cwd_dos[cwd_dos_length + 2] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length + 2] = L'\\';
	cwd_dos[cwd_dos_length + 3] = L'a';
	cwd_dos[cwd_dos_length + 4] = L'b';
	cwd_dos[cwd_dos_length + 5] = L'c';
	cwd_dos[cwd_dos_length + 6] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	// mix and forward slash with backward slashes
	path = get_absolute_dospath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length + 2] = L'b';
	cwd_dos[cwd_dos_length + 3] = L'c';
	cwd_dos[cwd_dos_length + 4] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	// trailing slash
	cwd_dos[cwd_dos_length + 4] = L'\\';
	cwd_dos[cwd_dos_length + 5] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length] = L'\\';
	cwd_dos[cwd_dos_length + 1] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	cwd_dos[cwd_dos_length] = L'\0';

	return 0;
}

int test_at()
{
	int fd;
	UNICODE_STRING *path;

	fd = open("t-path", O_RDONLY);
	ASSERT_EQ(fd, 3);

	wcscat(cwd_nt, L"\\t-path");
	path = get_absolute_ntpath(fd, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	wcscat(cwd_dos, L"\\t-path");
	path = get_absolute_dospath(fd, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	wcscat(cwd_nt, L"\\abc");
	path = get_absolute_ntpath(fd, "abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	free(path);

	wcscat(cwd_dos, L"\\abc");
	path = get_absolute_dospath(fd, "abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	free(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute_nt()
{

	int fd;
	UNICODE_STRING *path;

	wcscat(cdrive, L"abc");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	wcscat(cdrive, L"\\");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	cdrive[cdrive_length] = L'\0';
	wcscat(cdrive, L"abc");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	path = get_absolute_ntpath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	// bad path
	path = get_absolute_ntpath(AT_FDCWD, "C:/..");
	ASSERT_NULL(path);
	free(path); // should be a nop

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	wcscat(cdrive, L"abc\\");
	path = get_absolute_ntpath(fd, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	free(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute_dos()
{

	int fd;
	UNICODE_STRING *path;

	path = get_absolute_dospath(AT_FDCWD, "C:/abc");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc\\");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	free(path);

	path = get_absolute_dospath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	free(path);

	// bad path
	path = get_absolute_dospath(AT_FDCWD, "C:/..");
	ASSERT_NULL(path);
	free(path); // should be a nop

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	path = get_absolute_dospath(fd, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc\\");
	free(path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int main()
{
	char cwd_buf[32768];
	UNICODE_STRING *pcwd_dos, *pcwd_nt, *pcdrive_nt;

	getcwd(cwd_buf, 32768);

	pcwd_dos = get_absolute_dospath(AT_FDCWD, cwd_buf);
	printf("Current Working Directory (DOS): %ls\n", pcwd_dos->Buffer);
	memcpy(cwd_dos, pcwd_dos->Buffer, pcwd_dos->MaximumLength);
	cwd_dos_length = pcwd_dos->Length / sizeof(wchar_t);

	pcwd_nt = get_absolute_ntpath(AT_FDCWD, cwd_buf);
	printf("Current Working Directory (NT): %ls\n", pcwd_nt->Buffer);
	memcpy(cwd_nt, pcwd_nt->Buffer, pcwd_nt->MaximumLength);
	cwd_nt_length = pcwd_nt->Length / sizeof(wchar_t);

	pcdrive_nt = get_absolute_ntpath(AT_FDCWD, "C:");
	printf("C Drive (NT): %ls\n", pcdrive_nt->Buffer);
	memcpy(cdrive, pcdrive_nt->Buffer, pcdrive_nt->MaximumLength);
	cdrive_length = pcdrive_nt->Length / sizeof(wchar_t);

	free(pcwd_dos);
	free(pcwd_nt);
	free(pcdrive_nt);

	INITIAILIZE_TESTS();

	TEST(test_null());
	TEST(test_con());

	TEST(test_relative_nt());
	TEST(test_relative_dos());

	mkdir("t-path", 0700);

	TEST(test_at());
	TEST(test_absolute_nt());
	TEST(test_absolute_dos());

	rmdir("t-path");

	VERIFY_RESULT_AND_EXIT();
}
