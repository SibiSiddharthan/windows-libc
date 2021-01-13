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

void test_current_timestamp()
{
	int fd = creat("t-utimens", 0700);
	close(fd);

	// get current time
	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	struct timespec times[2];
	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
	int status = utimensat(AT_FDCWD, "t-utimens", times, 0);
	ASSERT_EQ(status, 0);

	struct stat statbuf;
	stat("t-utimens", &statbuf);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	unlink("t-utimens");
}

// Do the same as above, with futimens
void test_current_timestamp_f()
{
	int fd = creat("t-futimens", 0700);

	// get current time
	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	struct timespec times[2];
	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
	int status = futimens(fd, times);
	ASSERT_EQ(status, 0);

	struct stat statbuf;
	fstat(fd, &statbuf);
	ASSERT_EQ(statbuf.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf.st_mtim.tv_sec, times[1].tv_sec);
	ASSERT_EQ(statbuf.st_mtim.tv_nsec, times[1].tv_nsec);

	close(fd);
	unlink("t-futimens");
}

void test_UTIME_OMIT()
{
	int fd = creat("t-utimens", 0700);
	close(fd);

	// get current time
	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);

	struct stat statbuf_before;
	stat("t-utimens", &statbuf_before);

	struct timespec times[2];
	times[0] = get_current_timespec(filetime);
	times[1] = get_current_timespec(filetime);
	times[1].tv_nsec = UTIME_OMIT;
	int status = utimensat(AT_FDCWD, "t-utimens", times, 0);
	ASSERT_EQ(status, 0);

	struct stat statbuf_after;
	stat("t-utimens", &statbuf_after);
	ASSERT_EQ(statbuf_after.st_atim.tv_sec, times[0].tv_sec);
	ASSERT_EQ(statbuf_after.st_atim.tv_nsec, times[0].tv_nsec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_sec, statbuf_before.st_mtim.tv_sec);
	ASSERT_EQ(statbuf_after.st_mtim.tv_nsec, statbuf_before.st_mtim.tv_nsec);

	unlink("t-utimens");
}

int main()
{
	test_current_timestamp();
	test_current_timestamp_f();
	test_UTIME_OMIT();
	return 0;
}
