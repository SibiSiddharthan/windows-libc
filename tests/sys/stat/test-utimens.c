/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <Windows.h>

// Same as LARGE_INTEGER_to_timespec from convert.c
struct timespec get_current_timespec(LARGE_INTEGER LT)
{
	struct timespec result;
	time_t epoch = LT.QuadPart - 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

int test_utimens()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf;
	struct timespec times[2];
	const char *filename = "t-utimens";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Add one hour
	current_time.QuadPart += 36000000000;

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);
	status = utimens(filename, times);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_lutimens()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf_before, statbuf_after, statbuf_symlink;
	struct timespec times[2];
	const char *filename = "t-lutimens";
	const char *filename_symlink = "t-lutimens.sym";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename, &statbuf_before);
	ASSERT_EQ(status, 0);

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Subract one hour
	current_time.QuadPart += 36000000000;

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);
	status = lutimens(filename_symlink, times);
	ASSERT_EQ(status, 0);

	status = lstat(filename_symlink, &statbuf_symlink);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_symlink.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf_symlink.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf_symlink.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf_symlink.st_mtim.tv_nsec, times[1].tv_nsec);

	// Make sure the original is untouched
	status = stat(filename, &statbuf_after);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_after.st_atim.tv_sec, statbuf_before.st_atim.tv_sec);
	ASSERT_EQ(statbuf_after.st_atim.tv_nsec, statbuf_before.st_atim.tv_nsec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_sec, statbuf_before.st_mtim.tv_sec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_nsec, statbuf_before.st_mtim.tv_nsec);

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_futimens()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf;
	struct timespec times[2];
	const char *filename = "t-futimens";

	int fd = creat(filename, 0700);

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Subract one hour
	current_time.QuadPart -= 36000000000;

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);
	status = futimens(fd, times);
	ASSERT_EQ(status, 0);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_utimensat()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf_before, statbuf_after, statbuf_symlink;
	struct timespec times[2];
	const char *filename = "t-utimensat";
	const char *filename_symlink = "t-utimensat.sym";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename, &statbuf_before);
	ASSERT_EQ(status, 0);

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Add one hour
	current_time.QuadPart += 36000000000;

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);

	// Work like lutimens
	status = utimensat(AT_FDCWD, filename_symlink, times, AT_SYMLINK_NOFOLLOW);
	ASSERT_EQ(status, 0);

	status = lstat(filename_symlink, &statbuf_symlink);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_symlink.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf_symlink.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf_symlink.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf_symlink.st_mtim.tv_nsec, times[1].tv_nsec);

	// Make sure the original is untouched
	status = stat(filename, &statbuf_after);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_after.st_atim.tv_sec, statbuf_before.st_atim.tv_sec);
	ASSERT_EQ(statbuf_after.st_atim.tv_nsec, statbuf_before.st_atim.tv_nsec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_sec, statbuf_before.st_mtim.tv_sec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_nsec, statbuf_before.st_mtim.tv_nsec);

	// Dereference the symlink
	status = utimensat(AT_FDCWD, filename_symlink, times, 0);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf_after);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_after.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf_after.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_nsec, times[1].tv_nsec);

	ASSERT_SUCCESS(unlink(filename_symlink));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_fdutimens()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf;
	struct timespec times[2];
	const char *filename = "t-fdutimens";

	int fd = creat(filename, 0700);

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Subract one hour
	current_time.QuadPart -= 36000000000;

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);

	// fd should be preferred
	status = fdutimens(fd, "dummy", times);
	ASSERT_EQ(status, 0);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	// Subract another hour
	current_time.QuadPart -= 36000000000;
	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);

	// use filename now
	status = fdutimens(-1, filename, times);
	ASSERT_EQ(status, 0);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	// Subract another hour
	current_time.QuadPart -= 36000000000;
	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);

	status = fdutimens(AT_FDCWD, filename, times);
	ASSERT_EQ(status, 0);

	status = fstat(fd, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	status = fdutimens(AT_FDCWD, "dummy", times);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENOENT);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_UTIME_OMIT()
{
	int status;
	LARGE_INTEGER current_time;
	struct stat statbuf_before, statbuf_after;
	struct timespec times[2];
	const char *filename = "t-utimensat-omit";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	// get current time
	GetSystemTimeAsFileTime((FILETIME *)&current_time);
	// Add one hour
	current_time.QuadPart += 36000000000;

	status = stat(filename, &statbuf_before);
	ASSERT_EQ(status, 0);

	times[0] = get_current_timespec(current_time);
	times[1] = get_current_timespec(current_time);
	times[1].tv_nsec = UTIME_OMIT;
	status = utimensat(AT_FDCWD, filename, times, 0);
	ASSERT_EQ(status, 0);

	stat(filename, &statbuf_after);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(statbuf_after.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf_after.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_sec, statbuf_before.st_mtim.tv_sec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_nsec, statbuf_before.st_mtim.tv_nsec);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_UTIME_NOW()
{
	int status;
	struct timespec times[2];
	const char *filename = "t-utimensat-now";

	int fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	// just check whether the funtions succeeds
	status = utimensat(AT_FDCWD, filename, NULL, 0);
	ASSERT_EQ(status, 0);

	times[0].tv_nsec = UTIME_NOW;
	times[1].tv_nsec = UTIME_NOW;

	status = utimensat(AT_FDCWD, filename, NULL, 0);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-utimens");
	remove("t-lutimens");
	remove("t-lutimens.sym");
	remove("t-futimens");
	remove("t-utimensat");
	remove("t-utimensat.sym");
	remove("t-fdutimens");
	remove("t-utimensat-omit");
	remove("t-utimensat-now");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_utimens());
	TEST(test_lutimens());
	TEST(test_futimens());
	TEST(test_utimensat());
	TEST(test_fdutimens());
	TEST(test_UTIME_OMIT());
	TEST(test_UTIME_NOW());

	VERIFY_RESULT_AND_EXIT();
}
