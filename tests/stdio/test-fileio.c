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

static int prepare(const char *filename)
{
	int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0700);
	ssize_t result = write(fd, "abcdefghijklmnopqrstuvwxyz0123456789", 36);
	ASSERT_EQ(result, 36);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

static int write_check(const char *filename)
{
	char buffer[64];
	int fd = open(filename, O_RDONLY);
	ssize_t result = read(fd, buffer, 64);
	ASSERT_EQ(result, 36);
	ASSERT_MEMEQ(buffer, "abcdefghijklmnopqrstuvwxyz0123456789", 36);
	ASSERT_SUCCESS(close(fd));

	return 0;
}

int test_read_write_basic()
{
	FILE *f = NULL;
	size_t elements_read, elements_written;
	char read_buffer[512], write_buffer[256];
	const char *filename = "t-fileio-basic";

	f = fopen(filename, "w");
	memset(write_buffer, '@', 256);
	elements_written = fwrite(write_buffer, 1, 256, f);
	ASSERT_EQ(elements_written, 256);
	elements_written = fwrite(write_buffer, 2, 128, f);
	ASSERT_EQ(elements_written, 128);
	ASSERT_SUCCESS(fclose(f));

	f = fopen(filename, "r");
	memset(read_buffer, '\0', 512);
	elements_read = fread(read_buffer, 1, 256, f);
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
	ASSERT_SUCCESS(fclose(f));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_read_small_buffer_internal()
{
	int result;
	size_t elements_read;
	FILE *f;
	char buffer[32];
	const char *filename = "t-fileio-read-internal-buffer";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r");
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_read_small_buffer_external()
{
	int result;
	size_t elements_read;
	FILE *f;
	char file_buffer[16];
	char buffer[32];
	const char *filename = "t-fileio-read-external-buffer";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_read_buffer_change()
{
	int result;
	size_t elements_read;
	FILE *f;
	char file_buffer[16];
	char buffer[32];
	const char *filename = "t-fileio-read-varying-buffer";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r");

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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_write_small_buffer_internal()
{
	int result;
	size_t elements_written = 0;
	FILE *f = NULL;
	const char *filename = "t-fileio-write-internal-buffer";

	f = fopen(filename, "w");
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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(write_check(filename));
	ASSERT_SUCCESS(unlink(filename));

	f = fopen(filename, "w");
	result = setvbuf(f, NULL, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// Big write
	elements_written = fwrite("abcdefghijklmnopqrstuvwxyz0123456789", 1, 36, f);
	ASSERT_EQ(elements_written, 36);
	ASSERT_EQ(ftell(f), 36);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(write_check(filename));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_write_small_buffer_external()
{
	int result;
	size_t elements_written = 0;
	FILE *f = NULL;
	char file_buffer[16];
	const char *filename = "t-fileio-write-external-buffer";

	f = fopen(filename, "w");
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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(write_check(filename));
	ASSERT_SUCCESS(unlink(filename));

	f = fopen(filename, "w");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

	// Big write
	elements_written = fwrite("abcdefghijklmnopqrstuvwxyz0123456789", 1, 36, f);
	ASSERT_EQ(elements_written, 36);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_MEMEQ(file_buffer, "6789", 4);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(write_check(filename));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_write_buffer_change()
{
	int result;
	FILE *f = NULL;
	size_t elements_written = 0;
	char file_buffer[16];
	const char *filename = "t-fileio-write-varying-buffer";

	f = fopen(filename, "w");

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

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(write_check(filename));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_read_write()
{
	int fd, result;
	size_t elements_read, elements_written;
	ssize_t read_result;
	FILE *f;
	char file_buffer[16], buffer[32], check_buffer[64];
	const char *filename = "t-fileio-read-write";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r+");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

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

	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 36);
	ASSERT_MEMEQ(check_buffer, "abcdefgh@@@@mnopqrst########23456789", 36);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_read_write_seek()
{
	int fd, result;
	size_t elements_read, elements_written;
	ssize_t read_result;
	FILE *f;
	char file_buffer[16], buffer[32], check_buffer[64];
	const char *filename = "t-fileio-read-write-seek";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r+");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

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

	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 44);
	ASSERT_MEMEQ(check_buffer, "abcdefghijkl@@@@qrstuvwxyz012345####\0\0\0\0&&&&", 44);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_read_append()
{
	int fd, result;
	size_t elements_read, elements_written;
	ssize_t read_result;
	FILE *f;
	char file_buffer[16], buffer[32], check_buffer[64];
	const char *filename = "t-fileio-read-append";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "a+");
	result = setvbuf(f, file_buffer, _IOFBF, 16);
	ASSERT_EQ(result, 0);

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

	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	read_result = read(fd, check_buffer, 64);
	ASSERT_EQ(read_result, 48);
	ASSERT_MEMEQ(check_buffer, "abcdefghijklmnopqrstuvwxyz0123456789@@@@@@@@####", 48);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_getc()
{
	FILE *f;
	int result;
	char ch;
	const char *filename = "t-fileio-getc";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r");
	result = setvbuf(f, NULL, _IOFBF, 4);
	ASSERT_EQ(result, 0);

	// read 7 times -> refill buffer once
	ch = fgetc(f);
	ASSERT_EQ(ch, 'a');
	ASSERT_EQ(ftell(f), 1);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'b');
	ASSERT_EQ(ftell(f), 2);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'c');
	ASSERT_EQ(ftell(f), 3);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'd');
	ASSERT_EQ(ftell(f), 4);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'e');
	ASSERT_EQ(ftell(f), 5);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'f');
	ASSERT_EQ(ftell(f), 6);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'g');
	ASSERT_EQ(ftell(f), 7);

	fseek(f, -5, SEEK_END);
	ASSERT_EQ(ftell(f), 31);
	ch = fgetc(f);
	ASSERT_EQ(ch, '5');
	ASSERT_EQ(ftell(f), 32);
	ch = fgetc(f);
	ASSERT_EQ(ch, '6');
	ASSERT_EQ(ftell(f), 33);
	ch = fgetc(f);
	ASSERT_EQ(ch, '7');
	ASSERT_EQ(ftell(f), 34);
	ch = fgetc(f);
	ASSERT_EQ(ch, '8');
	ASSERT_EQ(ftell(f), 35);
	ch = fgetc(f);
	ASSERT_EQ(ch, '9');
	ASSERT_EQ(ftell(f), 36);
	ch = fgetc(f);
	ASSERT_EQ(ch, EOF);
	ASSERT_EQ(ftell(f), 36);
	ASSERT_EQ(feof(f), 1);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_putc()
{
	char ch;
	int fd, result;
	ssize_t read_result;
	FILE *f;
	char check_buffer[16];
	const char *filename = "t-fileio-putc";

	f = fopen(filename, "w");
	result = setvbuf(f, NULL, _IOFBF, 4);
	ASSERT_EQ(result, 0);

	// read 7 times -> refill buffer once
	ch = fputc('a', f);
	ASSERT_EQ(ch, 'a');
	ASSERT_EQ(ftell(f), 1);
	ch = fputc('b', f);
	ASSERT_EQ(ch, 'b');
	ASSERT_EQ(ftell(f), 2);
	ch = fputc('c', f);
	ASSERT_EQ(ch, 'c');
	ASSERT_EQ(ftell(f), 3);
	ch = fputc('d', f);
	ASSERT_EQ(ch, 'd');
	ASSERT_EQ(ftell(f), 4);
	ch = fputc('e', f);
	ASSERT_EQ(ch, 'e');
	ASSERT_EQ(ftell(f), 5);
	ch = fputc('f', f);
	ASSERT_EQ(ch, 'f');
	ASSERT_EQ(ftell(f), 6);
	ch = fputc('g', f);
	ASSERT_EQ(ch, 'g');
	ASSERT_EQ(ftell(f), 7);

	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	read_result = read(fd, check_buffer, 16);
	ASSERT_EQ(read_result, 7);
	ASSERT_MEMEQ(check_buffer, "abcdefg", 7);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_getc_putc()
{
	char ch;
	int fd, result;
	ssize_t read_result;
	FILE *f;
	char check_buffer[16];
	const char *filename = "t-fileio-getc-putc";

	ASSERT_SUCCESS(prepare(filename));

	f = fopen(filename, "r+");
	result = setvbuf(f, NULL, _IOFBF, 4);
	ASSERT_EQ(result, 0);

	ch = fgetc(f);
	ASSERT_EQ(ch, 'a');
	ASSERT_EQ(ftell(f), 1);
	ch = fputc('B', f);
	ASSERT_EQ(ch, 'B');
	ASSERT_EQ(ftell(f), 2);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'c');
	ASSERT_EQ(ftell(f), 3);
	ch = fputc('D', f);
	ASSERT_EQ(ch, 'D');
	ASSERT_EQ(ftell(f), 4);
	ch = fgetc(f);
	ASSERT_EQ(ch, 'e');
	ASSERT_EQ(ftell(f), 5);
	ch = fputc('F', f);
	ASSERT_EQ(ch, 'F');
	ASSERT_EQ(ftell(f), 6);

	ASSERT_SUCCESS(fclose(f));

	fd = open(filename, O_RDONLY);
	ASSERT_EQ(fd, 3);
	read_result = read(fd, check_buffer, 16);
	ASSERT_EQ(read_result, 16);
	ASSERT_MEMEQ(check_buffer, "aBcDeFgh", 8);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_gets()
{
	int fd;
	FILE *f;
	char buf[16];
	ssize_t result;
	const char *filename = "t-fileio-gets";

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0700);
	ASSERT_EQ(fd, 3);
	result = write(fd, "abcdefg\nhijkop", 14);
	ASSERT_EQ(result, 14);
	ASSERT_SUCCESS(close(fd));

	f = fopen(filename, "r");

	fgets(buf, 16, f);
	ASSERT_STREQ(buf, "abcdefg\n");
	ASSERT_EQ(ftell(f), 8);

	fgets(buf, 3, f);
	ASSERT_STREQ(buf, "hi");
	ASSERT_EQ(ftell(f), 10);

	fgets(buf, 16, f);
	ASSERT_STREQ(buf, "jkop");
	ASSERT_EQ(ftell(f), 14);
	ASSERT_EQ(feof(f), 1);

	ASSERT_SUCCESS(fclose(f));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

void cleanup()
{
	remove("t-fileio-basic");

	remove("t-fileio-read-internal-buffer");
	remove("t-fileio-read-external-buffer");
	remove("t-fileio-read-varying-buffer");

	remove("t-fileio-write-internal-buffer");
	remove("t-fileio-write-external-buffer");
	remove("t-fileio-write-varying-buffer");

	remove("t-fileio-read-write");
	remove("t-fileio-read-write-seek");
	remove("t-fileio-read-append");

	remove("t-fileio-getc");
	remove("t-fileio-putc");
	remove("t-fileio-getc-putc");
	remove("t-fileio-gets");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_read_write_basic());

	// read tests
	TEST(test_read_small_buffer_internal());
	TEST(test_read_small_buffer_external());
	TEST(test_read_buffer_change());

	// write tests
	TEST(test_write_small_buffer_internal());
	TEST(test_write_small_buffer_external());
	TEST(test_write_buffer_change());

	// read and write tests
	TEST(test_read_write());
	TEST(test_read_write_seek());
	TEST(test_read_append());

	TEST(test_getc());
	TEST(test_putc());
	TEST(test_getc_putc());

	TEST(test_gets());

	VERIFY_RESULT_AND_EXIT();
}
