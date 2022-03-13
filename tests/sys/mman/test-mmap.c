/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int test_file()
{
	int status;
	int fd;
	size_t size;
	void *address;
	char buffer[16];
	const char *filename = "t-mmap";

	fd = creat(filename, 0700);
	size = write(fd, "Hello World!", 12);
	ASSERT_EQ(size, 12);
	ASSERT_SUCCESS(close(fd));

	// size = getpagesize();

	fd = open(filename, O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	// Only read access allowed
	errno = 0;
	address = mmap(NULL, size, PROT_READ | PROT_WRITE, 0, fd, 0);
	ASSERT_EQ(address, MAP_FAILED);
	ASSERT_ERRNO(EACCES);

	address = mmap(NULL, size, PROT_READ, 0, fd, 0);
	ASSERT_NOTEQ(address, MAP_FAILED);

	// We don't need the fd anymore after doing mmap, so close it.
	ASSERT_SUCCESS(close(fd));

	status = mlock(address, size);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = mprotect(address, size, PROT_READ | PROT_WRITE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EACCES);

	// This shouldn't do anything
	status = mprotect(address, size, PROT_READ);
	ASSERT_EQ(status, 0);

	ASSERT_MEMEQ((char *)address, "Hello World!", 12);

	// On read-only section this shouldn't do anything.
	status = msync(address, size, MS_SYNC);
	ASSERT_EQ(status, 0);

	status = munlock(address, size);
	ASSERT_EQ(status, 0);

	status = munmap(address, size);
	ASSERT_EQ(status, 0);

	// Do it all over again, with read, write access.
	fd = open(filename, O_RDWR);
	ASSERT_NOTEQ(fd, -1);

	address = mmap(NULL, size, PROT_READ | PROT_WRITE, 0, fd, 0);
	ASSERT_NOTEQ(address, MAP_FAILED);

	ASSERT_SUCCESS(close(fd));

	status = mlock(address, size);
	ASSERT_EQ(status, 0);

	// This shouldn't really do anything.
	status = mprotect(address, size, PROT_READ | PROT_WRITE);
	ASSERT_EQ(status, 0);

	memcpy(address, "HELLO", 5);

	status = msync(address, size, MS_SYNC);
	ASSERT_EQ(status, 0);

	status = munlock(address, size);
	ASSERT_EQ(status, 0);

	status = munmap(address, size);
	ASSERT_EQ(status, 0);

	fd = open(filename, O_RDONLY);
	ASSERT_NOTEQ(fd, -1);

	size = read(fd, buffer, 16);
	ASSERT_EQ(size, 12);
	ASSERT_MEMEQ(buffer, "HELLO World!", 12);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_anonymous()
{
	int status;
	size_t size;
	void *address;

	size = getpagesize() * 4;

	address = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
	ASSERT_NOTEQ(address, MAP_FAILED);

	status = mlock(address, size);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = mprotect(address, size, PROT_EXEC);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EACCES);

	status = mprotect(address, size, PROT_READ);
	ASSERT_EQ(status, 0);

	status = munlock(address, size);
	ASSERT_EQ(status, 0);

	status = munmap(address, size);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_anonymous_large()
{
	int status;
	size_t size;
	void *address;

	size = getpagesize() * 1024; // 4MB

	address = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
	ASSERT_NOTEQ(address, MAP_FAILED);

	// The process's working set quoto must be increased for this to succeed.
	// TODO: setrlimit RLIMIT_MEMLOCK
	// status = mlock(address, size);
	// ASSERT_EQ(status, 0);

	status = mprotect(address, size, PROT_READ);
	ASSERT_EQ(status, 0);

	// status = munlock(address, size);
	// ASSERT_EQ(status, 0);

	status = munmap(address, size);
	ASSERT_EQ(status, 0);

	return 0;
}

void cleanup()
{
	remove("t-mmap");
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_file());
	TEST(test_anonymous());
	// Same as above test, but try with huge memory
	TEST(test_anonymous_large());

	VERIFY_RESULT_AND_EXIT();
}
