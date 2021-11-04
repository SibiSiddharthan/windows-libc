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
#include <Windows.h>
#include <time.h>

// Same as FILETIME_to_timespec from stat.c
struct timespec get_current_timespec(FILETIME FT)
{
	struct timespec result;
	time_t epoch = ((time_t)FT.dwHighDateTime << 32) + FT.dwLowDateTime;
	epoch -= 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

int test_current_timestamp()
{
	int status;
	SYSTEMTIME systemtime;
	FILETIME filetime;
	struct stat statbuf;
	struct timespec times[2];
	const char *filename = "t-utimensat";

	int fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);
	ASSERT_SUCCESS(close(fd));

	// get current time
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
	status = utimensat(AT_FDCWD, filename, times, 0);
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

int test_current_timestamp_with_futimens()
{
	int status;
	SYSTEMTIME systemtime;
	FILETIME filetime;
	struct stat statbuf;
	struct timespec times[2];
	const char *filename = "t-futimens";

	int fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);

	// get current time
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
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

int test_UTIME_OMIT()
{
	int status;
	SYSTEMTIME systemtime;
	FILETIME filetime;
	struct stat statbuf_before, statbuf_after;
	struct timespec times[2];
	const char *filename = "t-utimensat-omit";

	int fd = creat(filename, 0700);
	ASSERT_EQ(fd, 3);
	ASSERT_SUCCESS(close(fd));

	// get current time
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	status = stat(filename, &statbuf_before);
	ASSERT_EQ(status, 0);

	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
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

void cleanup()
{
	remove("t-utimensat");
	remove("t-futimens");
	remove("t-utimensat-omit");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_current_timestamp());
	TEST(test_current_timestamp_with_futimens());
	TEST(test_UTIME_OMIT());

	VERIFY_RESULT_AND_EXIT();
}
