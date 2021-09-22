/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <test-macros.h>
#include <fcntl.h>
#include <unistd.h>

void test_read_write_basic()
{
	FILE *f = NULL;

	f = fopen("t-fileio", "w");
	char write_buffer[256];
	memset(write_buffer, '@', 256);
	size_t elements_written = fwrite(write_buffer, 1, 256, f);
	ASSERT_EQ(elements_written, 256);
	elements_written = fwrite(write_buffer, 2, 128, f);
	ASSERT_EQ(elements_written, 128);
	fclose(f);

	f = fopen("t-fileio", "r");
	char read_buffer[512];
	memset(read_buffer, '\0', 512);
	size_t elements_read = fread(read_buffer, 1, 256, f);
	ASSERT_EQ(elements_read, 256);
	ASSERT_EQ(strncmp(read_buffer, write_buffer, 256), 0);
	memset(read_buffer, 0, 512);
	elements_read = fread(read_buffer, 2, 128, f);
	ASSERT_EQ(strncmp(read_buffer, write_buffer, 256), 0);
	ASSERT_EQ(elements_read, 128);
	memset(read_buffer, 0, 512);
	elements_read = fread(read_buffer, 2, 128, f);
	ASSERT_EQ(elements_read, 0);
	ASSERT_EQ(feof(f), 1);
	fclose(f);

	unlink("t-fileio");
}

void prepare(const char *filename)
{
	int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);
	ssize_t result = write(fd, "abcdefghijklmnopqrstuvwxyz0123456789", 36);
	ASSERT_EQ(result, 36);
	close(fd);
}

void test_read_small_buffer_internal()
{
	FILE *f = fopen("t-fileio-read", "r");
	int result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	char buffer[32];
	size_t elements_read = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);

	// 2nd read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 16);
	ASSERT_EQ(ftell(f), 24);
	ASSERT_MEMEQ(buffer, "ijklmnopqrstuvwx", 16);

	// 3rd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 32);
	ASSERT_MEMEQ(buffer, "yz012345", 8);

	// 4th final read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 4);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "6789", 4);

	// 5th read nothing should happen
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 0);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "6789", 4); // The buffer should not be changed

	// Seek to the beginning of the file
	fseek(f, 0, SEEK_SET);
	ASSERT_EQ(ftell(f), 0);
	ASSERT_EQ(feof(f), 0);

	// Do big read
	elements_read = fread(buffer, 1, 24, f);
	ASSERT_EQ(elements_read, 24);
	ASSERT_EQ(ftell(f), 24);
	ASSERT_MEMEQ(buffer, "abcdefghijklmnopqrstuvwx", 24);

	// Again
	elements_read = fread(buffer, 1, 24, f);
	ASSERT_EQ(elements_read, 12);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "yz0123456789", 12);

	fclose(f);
}

void test_read_small_buffer_external()
{
	FILE *f = fopen("t-fileio-read", "r");
	char file_buffer[16];
	int result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	char buffer[32];
	size_t elements_read = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);
	ASSERT_MEMEQ(file_buffer, "abcdefghijklmnop", 16);

	// 2nd read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 16);
	ASSERT_EQ(ftell(f), 24);
	ASSERT_MEMEQ(buffer, "ijklmnopqrstuvwx", 16);
	ASSERT_MEMEQ(file_buffer, "qrstuvwxyz012345", 16);

	// 3rd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 32);
	ASSERT_MEMEQ(buffer, "yz012345", 8);
	ASSERT_MEMEQ(file_buffer, "qrstuvwxyz012345", 16);

	// 4th final read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 4);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "6789", 4);
	ASSERT_MEMEQ(file_buffer, "6789uvwxyz012345", 16);

	// 5th read nothing should happen
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 0);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "6789", 4); // The buffer should not be changed

	// Seek to the beginning of the file
	fseek(f, 0, SEEK_SET);
	ASSERT_EQ(ftell(f), 0);
	ASSERT_EQ(feof(f), 0);

	// Do big read
	elements_read = fread(buffer, 1, 24, f);
	ASSERT_EQ(elements_read, 24);
	ASSERT_EQ(ftell(f), 24);
	ASSERT_MEMEQ(buffer, "abcdefghijklmnopqrstuvwx", 24);
	ASSERT_MEMEQ(file_buffer, "qrstuvwxyz012345", 16);

	// Again
	elements_read = fread(buffer, 1, 24, f);
	ASSERT_EQ(elements_read, 12);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "yz0123456789", 12);
	ASSERT_MEMEQ(file_buffer, "6789uvwxyz012345", 16);

	fclose(f);
}

void test_read_buffer_change()
{
	FILE *f = fopen("t-fileio-read", "r");
	char file_buffer[16];
	int result;

	char buffer[32];
	size_t elements_read = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);

	// Make stream unbuffered
	result = setvbuf(f, NULL, _IONBF, 0);
	ASSERT_EQ(result, 0);

	// 2nd read
	elements_read = fread(buffer, 1, 4, f);
	ASSERT_EQ(elements_read, 4);
	ASSERT_EQ(ftell(f), 12);
	ASSERT_MEMEQ(buffer, "ijkl", 4);

	// Revert back to a buffered stream
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 3rd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 20);
	ASSERT_MEMEQ(buffer, "mnopqrst", 8);
	ASSERT_MEMEQ(file_buffer, "mnopqrstuvwxyz01", 16);

	// Change to using the internal buffer
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 4th read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 28);
	ASSERT_MEMEQ(buffer, "uvwxyz01", 8);

	// Change to using an external buffer
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 5th read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "23456789", 8);
	ASSERT_MEMEQ(file_buffer, "23456789", 8);

	fclose(f);
}

void cleanup(const char *filename)
{
	unlink(filename);
}

void write_check()
{
	int fd = open("t-fileio-write", O_RDONLY);
	char buffer[64];
	ssize_t result = read(fd, buffer, 64);
	ASSERT_EQ(result, 36);
	ASSERT_MEMEQ(buffer, "abcdefghijklmnopqrstuvwxyz0123456789", 36);
	close(fd);
	unlink("t-fileio-write");
}

void test_write_small_buffer_internal()
{
	FILE *f = NULL;
	size_t elements_written = 0;
	int result;

	f = fopen("t-fileio-write", "w");
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 1st write
	elements_written = fwrite("abcdefgh", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 8);

	// 2nd write
	elements_written = fwrite("ijklmnopqrstuvwx", 1, 16, f);
	ASSERT_EQ(elements_written, 16);
	ASSERT_EQ(ftell(f), 24);

	// 3rd write
	elements_written = fwrite("yz0123456789", 1, 12, f);
	ASSERT_EQ(elements_written, 12);
	ASSERT_EQ(ftell(f), 36);

	fclose(f);

	write_check();

	f = fopen("t-fileio-write", "w");
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// Big write
	elements_written = fwrite("abcdefghijklmnopqrstuvwxyz0123456789", 1, 36, f);
	ASSERT_EQ(elements_written, 36);
	ASSERT_EQ(ftell(f), 36);

	fclose(f);

	write_check();
}

void test_write_small_buffer_external()
{
	FILE *f = NULL;
	size_t elements_written = 0;
	int result;
	char file_buffer[16];

	f = fopen("t-fileio-write", "w");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 1st write
	elements_written = fwrite("abcdefgh", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(file_buffer, "abcdefgh", 8);

	// 2nd write
	elements_written = fwrite("ijklmnopqrstuvwx", 1, 16, f);
	ASSERT_EQ(elements_written, 16);
	ASSERT_EQ(ftell(f), 24);
	ASSERT_MEMEQ(file_buffer, "qrstuvwxijklmnop", 16);

	// 3rd write
	elements_written = fwrite("yz0123456789", 1, 12, f);
	ASSERT_EQ(elements_written, 12);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_MEMEQ(file_buffer, "6789uvwxyz012345", 16);

	fclose(f);

	write_check();

	f = fopen("t-fileio-write", "w");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// Big write
	elements_written = fwrite("abcdefghijklmnopqrstuvwxyz0123456789", 1, 36, f);
	ASSERT_EQ(elements_written, 36);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_MEMEQ(file_buffer, "6789", 4);

	fclose(f);

	write_check();
}

void test_write_buffer_change()
{
	FILE *f = NULL;
	size_t elements_written = 0;
	int result;
	char file_buffer[16];

	f = fopen("t-fileio-write", "w");

	// 1st write
	elements_written = fwrite("abcdefgh", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 8);

	// Make stream unbuffered
	result = setvbuf(f, NULL, _IONBF, 0);
	ASSERT_EQ(result, 0);

	// 2nd write
	elements_written = fwrite("ijkl", 1, 4, f);
	ASSERT_EQ(elements_written, 4);
	ASSERT_EQ(ftell(f), 12);

	// Revert back to a buffered stream
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 3rd read
	elements_written = fwrite("mnopqrst", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 20);
	ASSERT_MEMEQ(file_buffer, "mnopqrst", 8);

	// Change to using the internal buffer
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 4th read
	elements_written = fwrite("uvwxyz01", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 28);

	// Change to using an external buffer
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// 5th read
	elements_written = fwrite("23456789", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_MEMEQ(file_buffer, "23456789", 8);

	fclose(f);

	write_check();
}

void test_read_write()
{
	FILE *f = fopen("t-fileio-read-write", "r+");
	char file_buffer[16];
	int result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	char buffer[32];
	size_t elements_read = 0;
	size_t elements_written = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);
	ASSERT_MEMEQ(file_buffer, "abcdefghijklmnop", 16);

	// 1st write
	elements_written = fwrite("@@@@", 1, 4, f);
	ASSERT_EQ(elements_written, 4);
	ASSERT_EQ(ftell(f), 12);
	ASSERT_MEMEQ(file_buffer, "@@@@efghijklmnop", 16);

	// 2nd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 20);
	ASSERT_MEMEQ(buffer, "mnopqrst", 8);
	ASSERT_MEMEQ(file_buffer, "mnopqrstuvwxyz01", 16);

	// 2nd write
	elements_written = fwrite("########", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 28);
	ASSERT_MEMEQ(file_buffer, "########uvwxyz01", 16);

	// 3rd read
	elements_read = fread(buffer, 1, 16, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(buffer, "23456789", 8);
	ASSERT_MEMEQ(file_buffer, "23456789uvwxyz01", 16);

	fclose(f);

	int fd = open("t-fileio-read-write", O_RDONLY);
	char check_buffer[64];
	ssize_t read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 36);
	ASSERT_MEMEQ(check_buffer, "abcdefgh@@@@mnopqrst########23456789", 36);
	close(fd);
}

void test_read_write_seek()
{
	FILE *f = fopen("t-fileio-read-write-seek", "r+");
	char file_buffer[16];
	int result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	char buffer[32];
	size_t elements_read = 0;
	size_t elements_written = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);
	ASSERT_MEMEQ(file_buffer, "abcdefghijklmnop", 16);

	result = fseek(f, 4, SEEK_CUR);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ftell(f), 12);

	// 1st write
	elements_written = fwrite("@@@@", 1, 4, f);
	ASSERT_EQ(elements_written, 4);
	ASSERT_EQ(ftell(f), 16);
	ASSERT_MEMEQ(file_buffer, "@@@@efghijklmnop", 16);

	result = fseek(f, 4, SEEK_CUR);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ftell(f), 20);

	// 2nd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 28);
	ASSERT_MEMEQ(buffer, "uvwxyz01", 8);
	ASSERT_MEMEQ(file_buffer, "uvwxyz0123456789", 16);

	result = fseek(f, 4, SEEK_CUR);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ftell(f), 32);

	// 2nd write
	elements_written = fwrite("####", 1, 4, f);
	ASSERT_EQ(elements_written, 4);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_MEMEQ(file_buffer, "####yz0123456789", 16);

	// Don't seek here

	// 3rd read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 0);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(file_buffer, "####yz0123456789", 16);

	result = fseek(f, 4, SEEK_CUR);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ftell(f), 40);

	// 3rd write
	elements_written = fwrite("&&&&", 1, 4, f);
	ASSERT_EQ(elements_written, 4);
	ASSERT_EQ(ftell(f), 44);
	ASSERT_MEMEQ(file_buffer, "&&&&yz0123456789", 16);

	fclose(f);

	int fd = open("t-fileio-read-write-seek", O_RDONLY);
	char check_buffer[64];
	ssize_t read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 44);
	ASSERT_MEMEQ(check_buffer, "abcdefghijkl@@@@qrstuvwxyz012345####\0\0\0\0&&&&", 44);
	close(fd);
}

void test_read_append()
{
	FILE *f = fopen("t-fileio-read-append", "a+");
	char file_buffer[16];
	int result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	char buffer[32];
	size_t elements_read = 0;
	size_t elements_written = 0;

	// 1st read
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 8);
	ASSERT_EQ(ftell(f), 8);
	ASSERT_MEMEQ(buffer, "abcdefgh", 8);
	ASSERT_MEMEQ(file_buffer, "abcdefghijklmnop", 16);

	// append now

	elements_written = fwrite("@@@@@@@@", 1, 8, f);
	ASSERT_EQ(elements_written, 8);
	ASSERT_EQ(ftell(f), 44);
	ASSERT_MEMEQ(file_buffer, "@@@@@@@@ijklmnop", 16);

	// 2nd read should be empty
	elements_read = fread(buffer, 1, 8, f);
	ASSERT_EQ(elements_read, 0);
	ASSERT_EQ(ftell(f), 44);
	ASSERT_EQ(feof(f), 1);
	ASSERT_MEMEQ(file_buffer, "@@@@@@@@ijklmnop", 16);

	// Do 2 simultaneous appends
	elements_written = fwrite("##", 1, 2, f);
	ASSERT_EQ(elements_written, 2);
	ASSERT_EQ(ftell(f), 46);
	ASSERT_MEMEQ(file_buffer, "##@@@@@@ijklmnop", 16);
	elements_written = fwrite("##", 1, 2, f);
	ASSERT_EQ(elements_written, 2);
	ASSERT_EQ(ftell(f), 48);
	ASSERT_MEMEQ(file_buffer, "####@@@@ijklmnop", 16);

	fclose(f);

	int fd = open("t-fileio-read-append", O_RDONLY);
	char check_buffer[64];
	ssize_t read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 48);
	ASSERT_MEMEQ(check_buffer, "abcdefghijklmnopqrstuvwxyz0123456789@@@@@@@@####", 48);
	close(fd);
}

int main()
{
	test_read_write_basic();

	// read tests
	prepare("t-fileio-read");
	test_read_small_buffer_internal();
	test_read_small_buffer_external();
	test_read_buffer_change();
	cleanup("t-fileio-read");

	// write tests
	test_write_small_buffer_internal();
	test_write_small_buffer_external();
	test_write_buffer_change();

	// read and write tests
	prepare("t-fileio-read-write");
	test_read_write();
	cleanup("t-fileio-read-write");

	// // read and write with seek
	prepare("t-fileio-read-write-seek");
	test_read_write_seek();
	cleanup("t-fileio-read-write-seek");

	prepare("t-fileio-read-append");
	test_read_append();
	cleanup("t-fileio-read-append");

	return 0;
}
