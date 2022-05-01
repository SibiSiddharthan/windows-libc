/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <spawn.h>
#include <stdlib.h>
#include <stdlib-ext.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const char *auxilary_dir = "auxilary";

int prepare_input(const char *filename, const char *content)
{
	int fd;
	size_t length;
	ssize_t result;

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0700);

	length = strlen(content);
	result = write(fd, content, (int)length);

	ASSERT_EQ(result, length);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int check_output(const char *filename, const char *content)
{
	int fd;
	size_t length;
	ssize_t result;
	char buf[32];

	fd = open(filename, O_RDONLY);

	length = strlen(content);
	result = read(fd, buf, 32);

	ASSERT_SUCCESS(close(fd));

	ASSERT_EQ(result, length);
	ASSERT_MEMEQ(buf, content, (int)length);

	return 0;
}

void convert_forward_slashes_to_backward_slashes(char *str)
{
	for (int i = 0; str[i] != '\0'; ++i)
	{
		if (str[i] == '/')
		{
			str[i] = '\\';
		}
	}
}

int test_spawn_error()
{
	int status;
	pid_t pid;

	const char *program = "non-existent.exe";
	char *const argv[] = {(char *)program, NULL};

	errno = 0;
	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	errno = 0;
	status = posix_spawnp(&pid, "cmd", NULL, NULL, NULL, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	return 0;
}

int test_spawn_basic()
{
	int status;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program = "basic.exe";
	const char *program_noext = "basic";
	const char *input = "input.txt";
	const char *output = "output.txt";
	const char *content = "Hello World";
	char *const argv[] = {"basic.exe", NULL};
	char *const null_argv[] = {NULL};

	// Prepare the input.
	ASSERT_SUCCESS(prepare_input(input, content));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, input, O_RDONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, output, O_CREAT | O_WRONLY, 0700);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	ASSERT_SUCCESS(check_output(output, content));
	ASSERT_SUCCESS(remove(output));

	// Spawn the process again but this time give a program with no extension.
	status = posix_spawn(&pid, program_noext, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	ASSERT_SUCCESS(check_output(output, content));
	ASSERT_SUCCESS(remove(output));

	// Spawn the process again but this time give no arguments.
	status = posix_spawn(&pid, program, &actions, NULL, null_argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	ASSERT_SUCCESS(check_output(output, content));
	ASSERT_SUCCESS(remove(output));

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(remove(input));

	return 0;
}

int test_spawn_pipe()
{
	int status;
	int input_fds[2], output_fds[2];
	size_t length;
	ssize_t result;
	char buf[32];
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program = "basic.exe";
	const char *content = "Hello World!!!";
	char *const argv[] = {(char *)program, NULL};

	length = strlen(content);

	ASSERT_SUCCESS(pipe(input_fds));
	ASSERT_SUCCESS(pipe(output_fds));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, input_fds[0], STDIN_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, output_fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	// We don't need these anymore
	ASSERT_SUCCESS(close(input_fds[0]));
	ASSERT_SUCCESS(close(output_fds[1]));

	result = write(input_fds[1], content, length);
	ASSERT_EQ(result, length);
	ASSERT_SUCCESS(close(input_fds[1]));

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// Check output
	result = read(output_fds[0], buf, 32);
	ASSERT_EQ(result, length);
	ASSERT_MEMEQ(buf, content, (int)length);
	ASSERT_SUCCESS(close(output_fds[0]));

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_spawn_env()
{
	int status;
	int wstatus;
	pid_t pid;
	const char *program = "env.exe";
	char *argv[] = {(char *)program, NULL};
	char *env[] = {"TEST_ENV=Hello World", NULL};

	status = posix_spawn(&pid, program, NULL, NULL, argv, environ);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 1);

	// Pass the environment variable directly to the child process.
	status = posix_spawn(&pid, program, NULL, NULL, argv, env);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	// Set the environment of the parent process, which is then inherited.
	setenv("TEST_ENV", "Hello World", 0);

	status = posix_spawn(&pid, program, NULL, NULL, argv, environ);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	return 0;
}

int test_spawn_chdir()
{
	int status;
	int fds[2];
	char buffer[4096];
	char cwd[256];
	pid_t pid;
	posix_spawn_file_actions_t actions;
	const char *directory = "t-spawn-chdir";
	const char *program = "cwd.exe";
	char *argv[] = {(char *)program, NULL};

	ASSERT_SUCCESS(mkdir(directory, 0700));
	ASSERT_SUCCESS(pipe(fds));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addchdir(&actions, directory);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[1]));

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	getcwd(cwd, 256);
	strcat(cwd, "/");
	strcat(cwd, directory);

	read(fds[0], buffer, 4096);
	ASSERT_STREQ(buffer, cwd);

	ASSERT_SUCCESS(close(fds[0]));
	ASSERT_SUCCESS(rmdir(directory));

	return 0;
}

int test_spawn_fchdir()
{
	int status;
	int dirfd, fds[2];
	char buffer[4096];
	char cwd[256];
	pid_t pid;
	posix_spawn_file_actions_t actions;
	const char *directory = "t-spawn-fchdir";
	const char *program = "cwd.exe";
	char *argv[] = {(char *)program, NULL};

	ASSERT_SUCCESS(mkdir(directory, 0700));
	ASSERT_SUCCESS(pipe(fds));

	dirfd = open(directory, O_DIRECTORY);
	ASSERT_NOTEQ(dirfd, -1);

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_addfchdir(&actions, dirfd);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[1]));

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	getcwd(cwd, 256);
	strcat(cwd, "/");
	strcat(cwd, directory);

	read(fds[0], buffer, 4096);
	ASSERT_STREQ(buffer, cwd);

	ASSERT_SUCCESS(close(fds[0]));
	ASSERT_SUCCESS(close(dirfd))
	ASSERT_SUCCESS(rmdir(directory));

	return 0;
}

int test_spawn_inherit_wlibc()
{
	int status, wstatus;
	int fd_r, fd_w, fd_a, fd_rw, fd_ra;
	pid_t pid;
	const char *program = "inherit-wlibc.exe";
	const char *content = "Hello World From wlibc!!!";
	const char *file_r = "inherit-wlibc-file-r";
	const char *file_w = "inherit-wlibc-file-w";
	const char *file_a = "inherit-wlibc-file-a";
	const char *file_rw = "inherit-wlibc-file-rw";
	const char *file_ra = "inherit-wlibc-file-ra";

	char *argv[] = {(char *)program, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char br[4], bw[4], ba[4], brw[4], bra[4];

	ASSERT_SUCCESS(prepare_input(file_r, content));
	ASSERT_SUCCESS(prepare_input(file_w, content));
	ASSERT_SUCCESS(prepare_input(file_a, content));
	ASSERT_SUCCESS(prepare_input(file_rw, content));
	ASSERT_SUCCESS(prepare_input(file_ra, content));

	fd_r = open(file_r, O_RDONLY);
	ASSERT_NOTEQ(fd_r, -1);

	fd_w = open(file_w, O_WRONLY);
	ASSERT_NOTEQ(fd_w, -1);

	fd_a = open(file_a, O_WRONLY | O_APPEND);
	ASSERT_NOTEQ(fd_a, -1);

	fd_rw = open(file_rw, O_RDWR);
	ASSERT_NOTEQ(fd_rw, -1);

	fd_ra = open(file_ra, O_RDWR | O_APPEND);
	ASSERT_NOTEQ(fd_ra, -1);

	itoa(fd_r, br, 10);
	itoa(fd_w, bw, 10);
	itoa(fd_a, ba, 10);
	itoa(fd_rw, brw, 10);
	itoa(fd_ra, bra, 10);

	argv[1] = "5";
	argv[2] = br;
	argv[3] = bw;
	argv[4] = ba;
	argv[5] = brw;
	argv[6] = bra;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	ASSERT_SUCCESS(close(fd_r));
	ASSERT_SUCCESS(close(fd_w));
	ASSERT_SUCCESS(close(fd_a));
	ASSERT_SUCCESS(close(fd_rw));
	ASSERT_SUCCESS(close(fd_ra));

	ASSERT_SUCCESS(check_output(file_w, "@@@@@ World From wlibc!!!"));
	ASSERT_SUCCESS(check_output(file_a, "Hello World From wlibc!!!@@@@@"));
	ASSERT_SUCCESS(check_output(file_rw, "Hello@@@@@d From wlibc!!!"));
	ASSERT_SUCCESS(check_output(file_ra, "Hello World From wlibc!!!@@@@@"));

	ASSERT_SUCCESS(remove(file_r));
	ASSERT_SUCCESS(remove(file_w));
	ASSERT_SUCCESS(remove(file_a));
	ASSERT_SUCCESS(remove(file_rw));
	ASSERT_SUCCESS(remove(file_ra));

	return 0;
}

int test_spawn_inherit_wlibc_extra()
{
	int status, wstatus;
	int fd_p[2], fd_n;
	pid_t pid;
	const char *program = "inherit-wlibc.exe";
	char *argv[] = {(char *)program, NULL, NULL, NULL, NULL};
	char bpr[4], bpw[4], bn[4];

	ASSERT_SUCCESS(pipe(fd_p));

	fd_n = open("/dev/null", O_RDWR);
	ASSERT_NOTEQ(fd_n, -1);

	itoa(fd_n, bn, 10);
	itoa(fd_p[0], bpr, 10);
	itoa(fd_p[1], bpw, 10);

	// Test pipe inheritance.
	argv[1] = "2";
	argv[2] = bpr;
	argv[3] = bpw;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	// // Test dev(console, null) inheritance.
	argv[1] = "1";
	argv[2] = bn;
	argv[3] = NULL;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	ASSERT_SUCCESS(close(fd_n));
	ASSERT_SUCCESS(close(fd_p[0]));
	ASSERT_SUCCESS(close(fd_p[1]));

	return 0;
}

int test_spawn_inherit_msvcrt()
{
	int status, wstatus;
	int fd_r, fd_w, fd_a, fd_rw, fd_ra;
	pid_t pid;
	const char *program = "inherit-msvcrt.exe";
	const char *content = "Hello World From msvcrt!!!";
	const char *file_r = "inherit-msvcrt-file-r";
	const char *file_w = "inherit-msvcrt-file-w";
	const char *file_a = "inherit-msvcrt-file-a";
	const char *file_rw = "inherit-msvcrt-file-rw";
	const char *file_ra = "inherit-msvcrt-file-ra";

	char *argv[] = {(char *)program, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char br[4], bw[4], ba[4], brw[4], bra[4];

	ASSERT_SUCCESS(prepare_input(file_r, content));
	ASSERT_SUCCESS(prepare_input(file_w, content));
	ASSERT_SUCCESS(prepare_input(file_a, content));
	ASSERT_SUCCESS(prepare_input(file_rw, content));
	ASSERT_SUCCESS(prepare_input(file_ra, content));

	fd_r = open(file_r, O_RDONLY);
	ASSERT_NOTEQ(fd_r, -1);

	fd_w = open(file_w, O_WRONLY);
	ASSERT_NOTEQ(fd_w, -1);

	fd_a = open(file_a, O_WRONLY | O_APPEND);
	ASSERT_NOTEQ(fd_a, -1);

	fd_rw = open(file_rw, O_RDWR);
	ASSERT_NOTEQ(fd_rw, -1);

	fd_ra = open(file_ra, O_RDWR | O_APPEND);
	ASSERT_NOTEQ(fd_ra, -1);

	itoa(fd_r, br, 10);
	itoa(fd_w, bw, 10);
	itoa(fd_a, ba, 10);
	itoa(fd_rw, brw, 10);
	itoa(fd_ra, bra, 10);

	argv[1] = "5";
	argv[2] = br;
	argv[3] = bw;
	argv[4] = ba;
	argv[5] = brw;
	argv[6] = bra;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	ASSERT_SUCCESS(close(fd_r));
	ASSERT_SUCCESS(close(fd_w));
	ASSERT_SUCCESS(close(fd_a));
	ASSERT_SUCCESS(close(fd_rw));
	ASSERT_SUCCESS(close(fd_ra));

	ASSERT_SUCCESS(check_output(file_w, "##### World From msvcrt!!!"));
	ASSERT_SUCCESS(check_output(file_a, "Hello World From msvcrt!!!#####"));
	ASSERT_SUCCESS(check_output(file_rw, "Hello#####d From msvcrt!!!"));
	ASSERT_SUCCESS(check_output(file_ra, "Hello World From msvcrt!!!#####"));

	ASSERT_SUCCESS(remove(file_r));
	ASSERT_SUCCESS(remove(file_w));
	ASSERT_SUCCESS(remove(file_a));
	ASSERT_SUCCESS(remove(file_rw));
	ASSERT_SUCCESS(remove(file_ra));

	return 0;
}

int test_spawn_inherit_msvcrt_extra()
{
	int status, wstatus;
	int fd_p[2], fd_n;
	pid_t pid;
	const char *program = "inherit-msvcrt.exe";
	char *argv[] = {(char *)program, NULL, NULL, NULL, NULL};
	char bpr[4], bpw[4], bn[4];

	ASSERT_SUCCESS(pipe(fd_p));

	fd_n = open("NUL", O_RDWR);
	ASSERT_NOTEQ(fd_n, -1);

	itoa(fd_n, bn, 10);
	itoa(fd_p[0], bpr, 10);
	itoa(fd_p[1], bpw, 10);

	// Test pipe inheritance.
	argv[1] = "2";
	argv[2] = bpr;
	argv[3] = bpw;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	// // Test dev(console, null) inheritance.
	argv[1] = "1";
	argv[2] = bn;
	argv[3] = NULL;

	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 0);

	ASSERT_SUCCESS(close(fd_n));
	ASSERT_SUCCESS(close(fd_p[0]));
	ASSERT_SUCCESS(close(fd_p[1]));

	return 0;
}

int test_spawn_path()
{
	int status;
	int wstatus;
	pid_t pid;
	size_t PATH_length, AUXILARY_length;
	char *oldpath;
	char *oldpath_dup;
	char *newpath;
	char *auxilary_location;
	char auxilary_dir_buffer[256];
	const char *program = "simple.exe";
	char *argv[] = {(char *)program, NULL};

	// The program executable won't be found.
	status = posix_spawn(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);

	status = posix_spawnp(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);

	getcwd(auxilary_dir_buffer, 256);
	strcat(auxilary_dir_buffer, "/");
	strcat(auxilary_dir_buffer, auxilary_dir);

	auxilary_location = auxilary_dir_buffer;

	oldpath = getenv("PATH");
	ASSERT_NOTNULL(oldpath);

	PATH_length = strlen(oldpath);
	AUXILARY_length = strlen(auxilary_location);

	oldpath_dup = (char *)malloc(PATH_length + 1);
	memcpy(oldpath_dup, oldpath, PATH_length + 1);

	newpath = (char *)malloc(PATH_length + AUXILARY_length + 4); // ';' + '\0' + extra.

	// Prepend the PATH with path.exe's location.
	memset(newpath, 0, PATH_length + AUXILARY_length + 4);
	memcpy(newpath, auxilary_location, AUXILARY_length);
	newpath[AUXILARY_length] = ';';
	memcpy(newpath + AUXILARY_length + 1, oldpath_dup, PATH_length + 1);

	setenv("PATH", newpath, 1);

	// We have updated the PATH with the location of the program. This should work.
	status = posix_spawnp(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 4096);

	// Append the PATH with path.exe's location.
	memset(newpath, 0, PATH_length + AUXILARY_length + 4);
	memcpy(newpath, oldpath_dup, PATH_length);
	newpath[PATH_length] = ';';
	memcpy(newpath + PATH_length + 1, auxilary_location, AUXILARY_length + 1);

	setenv("PATH", newpath, 1);

	status = posix_spawnp(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 4096);

	// Append the PATH with path.exe's location. This time add an empty path separator. i.e ";;"
	memset(newpath, 0, PATH_length + AUXILARY_length + 4);
	memcpy(newpath, oldpath_dup, PATH_length);
	newpath[PATH_length] = ';';
	newpath[PATH_length + 1] = ';';
	memcpy(newpath + PATH_length + 2, auxilary_location, AUXILARY_length + 1);

	setenv("PATH", newpath, 1);

	status = posix_spawnp(&pid, program, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, &wstatus, 0);
	ASSERT_EQ(status, pid);
	ASSERT_EQ(wstatus, 4096);

	setenv("PATH", oldpath_dup, 1);

	free(oldpath_dup);
	free(newpath);

	return 0;
}

int test_spawn_args()
{
	int status;
	int fds[2];
	char buffer[4096];
	char *bufp = buffer;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program = "arg.exe";
	const char *normal_arg = "normal-arg";
	const char *arg_with_spaces = "arg with spaces";
	const char *quoted_arg = "\"quoted-arg\"";
	const char *quoted_arg_with_spaces = "\"quoted arg\"";
	const char *arg_with_slashes = "arg\\";
	const char *arg_with_slashes_in_between = "arg\\arg";
	const char *arg_with_slashes_and_spaces = "arg\\ space ";
	const char *arg_with_end_slash_and_spaces = "arg slash at end\\";
	const char *arg_with_end_slashes_and_spaces = "arg slash at end\\\\";
	const char *arg_with_quotes = "arg\"";
	const char *arg_with_quotes_and_spaces = "arg\" space\"";
	const char *arg_with_quotes_and_slashes = "arg\\\"";
	const char *arg_with_quotes_and_slashes_many = "arg\\\\ space\\\\\" \\\\\\\" \" \\";

	const char *argv[] = {program,
						  normal_arg,
						  arg_with_spaces,
						  quoted_arg,
						  quoted_arg_with_spaces,
						  arg_with_slashes,
						  arg_with_slashes_in_between,
						  arg_with_slashes_and_spaces,
						  arg_with_end_slash_and_spaces,
						  arg_with_end_slashes_and_spaces,
						  arg_with_quotes,
						  arg_with_quotes_and_spaces,
						  arg_with_quotes_and_slashes,
						  arg_with_quotes_and_slashes_many,
						  NULL};

	ASSERT_SUCCESS(pipe(fds));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program, &actions, NULL, (char *const *)argv, NULL);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[1]));

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	// All of argv will be written to stdout by the child.
	// Each argv will be separated by NULL.
	read(fds[0], bufp, 4096);

	ASSERT_STREQ(bufp, program);
	bufp += strlen(program) + 1;

	ASSERT_STREQ(bufp, normal_arg);
	bufp += strlen(normal_arg) + 1;

	ASSERT_STREQ(bufp, arg_with_spaces);
	bufp += strlen(arg_with_spaces) + 1;

	ASSERT_STREQ(bufp, quoted_arg);
	bufp += strlen(quoted_arg) + 1;

	ASSERT_STREQ(bufp, quoted_arg_with_spaces);
	bufp += strlen(quoted_arg_with_spaces) + 1;

	ASSERT_STREQ(bufp, arg_with_slashes);
	bufp += strlen(arg_with_slashes) + 1;

	ASSERT_STREQ(bufp, arg_with_slashes_in_between);
	bufp += strlen(arg_with_slashes_in_between) + 1;

	ASSERT_STREQ(bufp, arg_with_slashes_and_spaces);
	bufp += strlen(arg_with_slashes_and_spaces) + 1;

	ASSERT_STREQ(bufp, arg_with_end_slash_and_spaces);
	bufp += strlen(arg_with_end_slash_and_spaces) + 1;

	ASSERT_STREQ(bufp, arg_with_end_slashes_and_spaces);
	bufp += strlen(arg_with_end_slashes_and_spaces) + 1;

	ASSERT_STREQ(bufp, arg_with_quotes);
	bufp += strlen(arg_with_quotes) + 1;

	ASSERT_STREQ(bufp, arg_with_quotes_and_spaces);
	bufp += strlen(arg_with_quotes_and_spaces) + 1;

	ASSERT_STREQ(bufp, arg_with_quotes_and_slashes);
	bufp += strlen(arg_with_quotes_and_slashes) + 1;

	ASSERT_STREQ(bufp, arg_with_quotes_and_slashes_many);
	bufp += strlen(arg_with_quotes_and_slashes_many) + 1;

	ASSERT_SUCCESS(close(fds[0]));

	return 0;
}

int test_spawn_shebang_error()
{
	int status;
	pid_t pid;
	const char *program_cmd = "shebang.error";
	char *argv[] = {"shebang", NULL};

	ASSERT_SUCCESS(prepare_input(program_cmd, "#!"));

	errno = 0;
	status = posix_spawn(&pid, program_cmd, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOEXEC);

	ASSERT_SUCCESS(prepare_input(program_cmd, "#!   "));

	errno = 0;
	status = posix_spawn(&pid, program_cmd, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOEXEC);

	ASSERT_SUCCESS(prepare_input(program_cmd, "#!   \n"));

	errno = 0;
	status = posix_spawn(&pid, program_cmd, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOEXEC);

	ASSERT_SUCCESS(prepare_input(program_cmd, "hello"));

	errno = 0;
	status = posix_spawn(&pid, program_cmd, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOEXEC);

	ASSERT_SUCCESS(prepare_input(program_cmd, "#! non-existent.exe"));

	errno = 0;
	status = posix_spawn(&pid, program_cmd, NULL, NULL, argv, NULL);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	ASSERT_SUCCESS(remove(program_cmd));

	return 0;
}

int test_spawn_shebang_basic()
{
	int status;
	int fds[2];
	char arg0[4096];
	char arg1[4096];
	char content[4096];
	char buffer[4096];
	char *bufp;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program_arg = "arg.exe";
	const char *program_arg_noext = "arg";
	const char *program_cmd = "shebang.basic";
	char *argv[] = {"shebang", NULL};
	char *null_argv[] = {"shebang", NULL};

	ASSERT_SUCCESS(pipe(fds));

	getcwd(arg0, 4096);
	strcat(arg0, "/");
	strcat(arg0, program_arg);
	convert_forward_slashes_to_backward_slashes(arg0);

	getcwd(arg1, 4096);
	strcat(arg1, "/");
	strcat(arg1, program_cmd);
	convert_forward_slashes_to_backward_slashes(arg1);

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	// Prepare the shebang file.
	bufp = buffer;

	snprintf(content, 4096, "#! %s", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	// Do the test again, no executable extension given.
	bufp = buffer;

	snprintf(content, 4096, "#! %s", program_arg_noext);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	// Do the test again, try different line terminators.
	bufp = buffer;

	snprintf(content, 4096, "#! %s\n", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	// Do the test again, try with null_argv.
	bufp = buffer;

	snprintf(content, 4096, "#! %s\r\n", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, null_argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	// Do the test again, more whitespaces at the beginning.
	bufp = buffer;

	snprintf(content, 4096, "#!   \t %s\n", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	// Do the test again, more whitespaces at the end.
	bufp = buffer;

	snprintf(content, 4096, "#!%s   \n", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[0]));
	ASSERT_SUCCESS(close(fds[1]));
	ASSERT_SUCCESS(remove(program_cmd));

	return 0;
}

int test_spawn_shebang_args()
{
	int status;
	int fds[2];
	char executable[4096];
	char filename[4096];
	char content[4096];
	char buffer[4096];
	char *bufp;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program_arg = "arg.exe";
	const char *program_cmd = "shebang.args";

	ASSERT_SUCCESS(pipe(fds));

	getcwd(executable, 4096);
	strcat(executable, "/");
	strcat(executable, program_arg);
	convert_forward_slashes_to_backward_slashes(executable);

	getcwd(filename, 4096);
	strcat(filename, "/");
	strcat(filename, program_cmd);
	convert_forward_slashes_to_backward_slashes(filename);

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	// Basic arg check.
	bufp = buffer;
	char *argt1 = "hello";
	char *argv1[] = {"shebang", "args", NULL};

	snprintf(content, 4096, "#! %s %s", program_arg, argt1);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv1, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, executable);
	bufp += strlen(executable) + 1;

	ASSERT_STREQ(bufp, argt1);
	bufp += strlen(argt1) + 1;

	ASSERT_STREQ(bufp, filename);
	bufp += strlen(filename) + 1;

	ASSERT_STREQ(bufp, argv1[1]);
	bufp += strlen(argv1[1]) + 1;

	// Multiple args check.
	bufp = buffer;
	char *argt2 = "hello world";
	char *argv2[] = {"shebang", "arg1", "arg2", NULL};

	snprintf(content, 4096, "#! %s %s", program_arg, argt2);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv2, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, executable);
	bufp += strlen(executable) + 1;

	ASSERT_STREQ(bufp, argt2);
	bufp += strlen(argt2) + 1;

	ASSERT_STREQ(bufp, filename);
	bufp += strlen(filename) + 1;

	ASSERT_STREQ(bufp, argv2[1]);
	bufp += strlen(argv2[1]) + 1;

	ASSERT_STREQ(bufp, argv2[2]);
	bufp += strlen(argv2[2]) + 1;

	// Quoted args check.
	bufp = buffer;
	char *argt3 = "\"hello world\"";
	char *argv3[] = {"shebang", "arg1", "ar\"g2", NULL};

	snprintf(content, 4096, "#! %s %s", program_arg, argt3);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv3, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, executable);
	bufp += strlen(executable) + 1;

	ASSERT_STREQ(bufp, argt3);
	bufp += strlen(argt3) + 1;

	ASSERT_STREQ(bufp, filename);
	bufp += strlen(filename) + 1;

	ASSERT_STREQ(bufp, argv3[1]);
	bufp += strlen(argv3[1]) + 1;

	ASSERT_STREQ(bufp, argv3[2]);
	bufp += strlen(argv3[2]) + 1;

	// Bizzare args check 1.
	bufp = buffer;
	char *argt4 = "hello\"world";
	char *argv4[] = {"shebang", "arg1", NULL};

	snprintf(content, 4096, "#! %s %s", program_arg, argt4);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv4, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, executable);
	bufp += strlen(executable) + 1;

	ASSERT_STREQ(bufp, argt4);
	bufp += strlen(argt4) + 1;

	ASSERT_STREQ(bufp, filename);
	bufp += strlen(filename) + 1;

	ASSERT_STREQ(bufp, argv4[1]);
	bufp += strlen(argv4[1]) + 1;

	// Bizzare args check 2.
	bufp = buffer;
	char *argt5 = "\\\\ \\\"hello\"world\\\\";
	char *argv5[] = {"shebang", "arg1", NULL};

	snprintf(content, 4096, "#! %s %s", program_arg, argt5);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv5, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, executable);
	bufp += strlen(executable) + 1;

	ASSERT_STREQ(bufp, argt5);
	bufp += strlen(argt5) + 1;

	ASSERT_STREQ(bufp, filename);
	bufp += strlen(filename) + 1;

	ASSERT_STREQ(bufp, argv5[1]);
	bufp += strlen(argv5[1]) + 1;

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[0]));
	ASSERT_SUCCESS(close(fds[1]));
	ASSERT_SUCCESS(remove(program_cmd));

	return 0;
}

int test_spawn_shebang_path()
{
	int status;
	int fds[2];
	char arg0[4096];
	char arg1[4096];
	char content[4096];
	char buffer[4096];
	char auxilary_dir_buffer[256];
	char *bufp = buffer;
	pid_t pid;
	posix_spawn_file_actions_t actions;

	const char *program_arg = "arg-shebang.exe";
	const char *program_cmd = "shebang.path";
	char *argv[] = {"shebang", NULL};

	getcwd(auxilary_dir_buffer, 256);
	strcat(auxilary_dir_buffer, "/");
	strcat(auxilary_dir_buffer, auxilary_dir);

	setenv("PATH", auxilary_dir_buffer, 1);

	// Prepare the shebang file.
	snprintf(content, 4096, "#! %s", program_arg);
	ASSERT_SUCCESS(prepare_input(program_cmd, content));

	getcwd(arg0, 4096);
	strcat(arg0, "/");
	strcat(arg0, auxilary_dir);
	strcat(arg0, "/");
	strcat(arg0, program_arg);
	convert_forward_slashes_to_backward_slashes(arg0);

	getcwd(arg1, 4096);
	strcat(arg1, "/");
	strcat(arg1, program_cmd);
	convert_forward_slashes_to_backward_slashes(arg1);

	ASSERT_SUCCESS(pipe(fds));

	status = posix_spawn_file_actions_init(&actions);
	ASSERT_EQ(status, 0);

	status = posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	ASSERT_EQ(status, 0);

	status = posix_spawn(&pid, program_cmd, &actions, NULL, argv, NULL);
	ASSERT_EQ(status, 0);

	status = waitpid(pid, NULL, 0);
	ASSERT_EQ(status, pid);

	read(fds[0], buffer, 4096);

	ASSERT_STREQ(bufp, arg0);
	bufp += strlen(arg0) + 1;

	ASSERT_STREQ(bufp, arg1);
	bufp += strlen(arg1) + 1;

	status = posix_spawn_file_actions_destroy(&actions);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(close(fds[0]));
	ASSERT_SUCCESS(close(fds[1]));
	ASSERT_SUCCESS(remove(program_cmd));

	return 0;
}

void cleanup()
{
	remove("input.txt");
	remove("output.txt");
	remove("t-spawn-chdir");
	remove("t-spawn-fchdir");

	remove("inherit-wlibc-file-r");
	remove("inherit-wlibc-file-w");
	remove("inherit-wlibc-file-a");
	remove("inherit-wlibc-file-rw");
	remove("inherit-wlibc-file-ra");

	remove("inherit-msvcrt-file-r");
	remove("inherit-msvcrt-file-w");
	remove("inherit-msvcrt-file-a");
	remove("inherit-msvcrt-file-rw");
	remove("inherit-msvcrt-file-ra");

	remove("shebang.error");
	remove("shebang.basic");
	remove("shebang.args");
	remove("shebang.path");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_spawn_error());
	TEST(test_spawn_basic());
	TEST(test_spawn_pipe());
	TEST(test_spawn_env());
	TEST(test_spawn_chdir());
	TEST(test_spawn_fchdir());
	TEST(test_spawn_inherit_wlibc());
	TEST(test_spawn_inherit_wlibc_extra());
	TEST(test_spawn_inherit_msvcrt());
	TEST(test_spawn_inherit_msvcrt_extra());
	TEST(test_spawn_path());
	TEST(test_spawn_args());
	TEST(test_spawn_shebang_error());
	TEST(test_spawn_shebang_basic());
	TEST(test_spawn_shebang_args());
	TEST(test_spawn_shebang_path());

	VERIFY_RESULT_AND_EXIT();
}
