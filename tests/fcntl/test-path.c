/*
   Copyright (c) 2020-2025 Sibi Siddharthan

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
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/null");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\Null", path->Length);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	return 0;
}

int test_con()
{
	UNICODE_STRING *path = NULL;

	path = get_absolute_ntpath(AT_FDCWD, "CON");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "/dev/tty");
	ASSERT_MEMEQ((char *)path->Buffer, (char *)L"\\Device\\ConDrv\\Console", path->Length);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	return 0;
}

int test_relative_nt()
{
	UNICODE_STRING *path;

	path = get_absolute_ntpath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length] = L'\\';
	cwd_nt[cwd_nt_length + 1] = L'a';
	cwd_nt[cwd_nt_length + 2] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length + 2] = L'\\';
	cwd_nt[cwd_nt_length + 3] = L'a';
	cwd_nt[cwd_nt_length + 4] = L'b';
	cwd_nt[cwd_nt_length + 5] = L'c';
	cwd_nt[cwd_nt_length + 6] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	// mix and forward slash with backward slashes
	path = get_absolute_ntpath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length + 2] = L'b';
	cwd_nt[cwd_nt_length + 3] = L'c';
	cwd_nt[cwd_nt_length + 4] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	// trailing slash
	cwd_nt[cwd_nt_length + 4] = L'\\';
	cwd_nt[cwd_nt_length + 5] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length] = L'\\';
	cwd_nt[cwd_nt_length + 1] = L'\0';
	path = get_absolute_ntpath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_nt[cwd_nt_length] = L'\0';

	return 0;
}

int test_relative_dos()
{
	UNICODE_STRING *path;

	path = get_absolute_dospath(AT_FDCWD, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_dos[cwd_dos_length] = L'\\';
	cwd_dos[cwd_dos_length + 1] = L'a';
	cwd_dos[cwd_dos_length + 2] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_dos[cwd_dos_length + 2] = L'\\';
	cwd_dos[cwd_dos_length + 3] = L'a';
	cwd_dos[cwd_dos_length + 4] = L'b';
	cwd_dos[cwd_dos_length + 5] = L'c';
	cwd_dos[cwd_dos_length + 6] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a/abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "a\\abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "a/./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	// mix and forward slash with backward slashes
	path = get_absolute_dospath(AT_FDCWD, "a\\./abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_dos[cwd_dos_length + 2] = L'b';
	cwd_dos[cwd_dos_length + 3] = L'c';
	cwd_dos[cwd_dos_length + 4] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "a/../abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "abc/.");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "abc/./.");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	// trailing slash
	cwd_dos[cwd_dos_length + 4] = L'\\';
	cwd_dos[cwd_dos_length + 5] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "abc/./");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "abc/././");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_dos[cwd_dos_length] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/..");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cwd_dos[cwd_dos_length] = L'\\';
	cwd_dos[cwd_dos_length + 1] = L'\0';
	path = get_absolute_dospath(AT_FDCWD, "abc/../");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

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
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	wcscat(cwd_dos, L"\\t-path");
	path = get_absolute_dospath(fd, ".");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	wcscat(cwd_nt, L"\\abc");
	path = get_absolute_ntpath(fd, "abc");
	ASSERT_WSTREQ(path->Buffer, cwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	wcscat(cwd_dos, L"\\abc");
	path = get_absolute_dospath(fd, "abc");
	ASSERT_WSTREQ(path->Buffer, cwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

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
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	wcscat(cdrive, L"\\");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cdrive[cdrive_length] = L'\0';
	wcscat(cdrive, L"abc");

	path = get_absolute_ntpath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	cdrive[cdrive_length] = L'\0';

	path = get_absolute_ntpath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_ntpath(AT_FDCWD, "C:/..");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	wcscat(cdrive, L"abc\\");
	path = get_absolute_ntpath(fd, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, cdrive);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute_dos()
{

	int fd;
	UNICODE_STRING *path;

	path = get_absolute_dospath(AT_FDCWD, "C:/abc");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:\\abc");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/..");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/../");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/abc/.");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	path = get_absolute_dospath(AT_FDCWD, "C:/..");
	ASSERT_WSTREQ(path->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);
	// fd should be ignored
	path = get_absolute_dospath(fd, "C:/abc/");
	ASSERT_WSTREQ(path->Buffer, L"C:\\abc\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, path);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_absolute_cygwin_path()
{

	int fd;
	UNICODE_STRING *dospath, *ntpath;

	wcscat(cdrive, L"abc");

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/abc");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\abc");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	wcscat(cdrive, L"\\");

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc/");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/abc/");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\abc\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	cdrive[cdrive_length] = L'\0';

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc/..");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/abc/..");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc/../");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/abc/../");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	cdrive[cdrive_length] = L'\0';
	wcscat(cdrive, L"abc");

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc/.");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/abc/.");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\abc");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	cdrive[cdrive_length] = L'\0';

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	ntpath = get_absolute_ntpath(AT_FDCWD, "/c");
	dospath = get_absolute_dospath(AT_FDCWD, "/c");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	// bad path
	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/..");
	dospath = get_absolute_dospath(AT_FDCWD, "/c/..");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	// fd should be ignored
	wcscat(cdrive, L"abc\\");
	ntpath = get_absolute_ntpath(AT_FDCWD, "/c/abc/");
	dospath = get_absolute_dospath(fd, "/c/abc/");
	ASSERT_WSTREQ(ntpath->Buffer, cdrive);
	ASSERT_WSTREQ(dospath->Buffer, L"C:\\abc\\");
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_root()
{
	UNICODE_STRING *cd_ntpath, *cd_dospath, *root_ntpath, *root_dospath;
	char cd[256];

	getcwd(cd, 256);
	cd[3] = '\0';

	cd_ntpath = get_absolute_ntpath(AT_FDCWD, cd);
	cd_dospath = get_absolute_ntpath(AT_FDCWD, cd);

	root_ntpath = get_absolute_ntpath(AT_FDCWD, "/");
	root_dospath = get_absolute_ntpath(AT_FDCWD, "\\");

	ASSERT_WSTREQ(root_ntpath->Buffer, cd_ntpath->Buffer);
	ASSERT_WSTREQ(root_dospath->Buffer, cd_dospath->Buffer);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, cd_ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, cd_dospath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, root_ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, root_dospath);

	return 0;
}

int test_tmp()
{
	UNICODE_STRING *ntpath, *dospath;

	ntpath = get_absolute_ntpath(AT_FDCWD, "/tmp/file");
	dospath = get_absolute_dospath(AT_FDCWD, "/tmp/file");

	printf("Temporary File (NT)  : %ls\n", ntpath->Buffer);
	printf("Temporary File (DOS) : %ls\n", dospath->Buffer);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	return 0;
}

int test_pipe()
{
	UNICODE_STRING *ntpath;

	ntpath = get_absolute_ntpath(AT_FDCWD, "\\\\.\\pipe\\mypipe");
	ASSERT_WSTREQ(ntpath->Buffer, L"\\Device\\NamedPipe\\mypipe");

	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
	return 0;
}

int test_dev_fd()
{
	int fd;
	char str[64] = {0};
	UNICODE_STRING *ntpath_1, *ntpath_2;

	fd = open("t-path", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	sprintf(str, "/dev/fd/%d", fd);

	ntpath_1 = get_absolute_ntpath(AT_FDCWD, "t-path");
	ntpath_2 = get_absolute_ntpath(AT_FDCWD, str);

	ASSERT_WSTREQ(ntpath_2->Buffer, ntpath_1->Buffer);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath_1);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath_2);
	close(fd);

	return 0;
}

int test_fd_dospath()
{
	int fd;
	int pipefd[2];
	UNICODE_STRING *dospath;

	// Devices
	fd = open("/dev/null", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	dospath = get_fd_dospath(fd);
	ASSERT_WSTREQ(dospath->Buffer, L"NUL");

	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);
	close(fd);

	fd = open("/dev/tty", O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	dospath = get_fd_dospath(fd);
	ASSERT_WSTREQ(dospath->Buffer, L"CON");

	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);
	close(fd);

	// Named pipe
	named_pipe("\\\\.\\pipe\\mypipe", pipefd);
	ASSERT_NOTEQ(pipefd[0], -1);
	ASSERT_NOTEQ(pipefd[1], -1);

	dospath = get_fd_dospath(pipefd[0]);
	ASSERT_WSTREQ(dospath->Buffer, L"\\\\.\\pipe\\mypipe");

	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	dospath = get_fd_dospath(pipefd[1]);
	ASSERT_WSTREQ(dospath->Buffer, L"\\\\.\\pipe\\mypipe");

	RtlFreeHeap(NtCurrentProcessHeap(), 0, dospath);

	close(pipefd[0]);
	close(pipefd[1]);

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

	RtlFreeHeap(NtCurrentProcessHeap(), 0, pcwd_dos);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, pcwd_nt);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, pcdrive_nt);

	INITIAILIZE_TESTS();

	TEST(test_null());
	TEST(test_con());

	TEST(test_relative_nt());
	TEST(test_relative_dos());

	mkdir("t-path", 0700);

	TEST(test_at());
	TEST(test_absolute_nt());
	TEST(test_absolute_dos());

	cdrive[cdrive_length] = L'\0';
	TEST(test_absolute_cygwin_path());

	TEST(test_root());
	TEST(test_tmp());
	TEST(test_pipe());
	TEST(test_dev_fd());
	TEST(test_fd_dospath());

	rmdir("t-path");

	VERIFY_RESULT_AND_EXIT();
}
