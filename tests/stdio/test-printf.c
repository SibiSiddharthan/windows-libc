/*
   Copyright (c) 2024 - 2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdio.h>

#pragma warning(push)
#pragma warning(disable : 4244) // conversion from 'intmax_t' to 'int'
#pragma warning(disable : 4313) // format
#pragma warning(disable : 4474) // format
#pragma warning(disable : 4476) // format
#pragma warning(disable : 4477) // format
#pragma warning(disable : 4778) // format


int test_simple(void)
{
	int status = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "");
	status += CHECK_STRING(buffer, "");
	status += CHECK_RESULT(result, 0);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abcd");
	status += CHECK_STRING(buffer, "abcd");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%%");
	status += CHECK_STRING(buffer, "abc%");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%%abc");
	status += CHECK_STRING(buffer, "%abc");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%%%%%%");
	status += CHECK_STRING(buffer, "%%%");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%");
	status += CHECK_STRING(buffer, "%");
	status += CHECK_RESULT(result, 1);

	memset(buffer, 0, 256);
	result = snprintf(NULL, 0, "abcd");
	status += CHECK_RESULT(result, 4);

	return status;
}

int test_int(void)
{
	int status = 0;

	int result = 0;
	short outs = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%d", 10);
	status += CHECK_STRING(buffer, "10");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+d", 10);
	status += CHECK_STRING(buffer, "+10");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#d", 10);
	status += CHECK_STRING(buffer, "10");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%i %d", 10, 100);
	status += CHECK_STRING(buffer, "10 100");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%i %d", -10, -100);
	status += CHECK_STRING(buffer, "-10 -100");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$d %1$d", -10, -100);
	status += CHECK_STRING(buffer, "-10 -10");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.5d", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.5d%hn", 55, &outs);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);
	status += CHECK_RESULT(outs, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%5d", -55);
	status += CHECK_STRING(buffer, "  -55");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%2d", -55);
	status += CHECK_STRING(buffer, "-55");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%05d", -55);
	status += CHECK_STRING(buffer, "-0055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-05d", 55);
	status += CHECK_STRING(buffer, "55   ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-05d", -55);
	status += CHECK_STRING(buffer, "-55  ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+05d", 55);
	status += CHECK_STRING(buffer, "+0055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+02d", 55);
	status += CHECK_STRING(buffer, "+55");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-7.5d", 55);
	status += CHECK_STRING(buffer, "00055  ");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-7.5d", -55);
	status += CHECK_STRING(buffer, "-00055 ");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%- 7.5d", -55);
	status += CHECK_STRING(buffer, "-00055 ");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.5d", 55);
	status += CHECK_STRING(buffer, "   00055");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+8.5d", 55);
	status += CHECK_STRING(buffer, "  +00055");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+-8.5d", 55);
	status += CHECK_STRING(buffer, "+00055  ");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "% 8.5d", 55);
	status += CHECK_STRING(buffer, "   00055");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%+8.5d", -55);
	status += CHECK_STRING(buffer, "  -00055");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.1d", 55);
	status += CHECK_STRING(buffer, "      55");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "% 8.1d", 55);
	status += CHECK_STRING(buffer, "      55");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#2.5d", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%# 2.5d", 55);
	status += CHECK_STRING(buffer, " 00055");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#+2.5d", 55);
	status += CHECK_STRING(buffer, "+00055");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#+2.5d", -55);
	status += CHECK_STRING(buffer, "-00055");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-4.5d", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-+4.5d", 55);
	status += CHECK_STRING(buffer, "+00055");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'d", 5555555);
	status += CHECK_STRING(buffer, "5,555,555");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'d", -5555555);
	status += CHECK_STRING(buffer, "-5,555,555");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+d", 5555555);
	status += CHECK_STRING(buffer, "+5,555,555");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+.10d", 5555555);
	status += CHECK_STRING(buffer, "+0005,555,555");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+10d", 5555555);
	status += CHECK_STRING(buffer, "+5,555,555");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+13d", 5555555);
	status += CHECK_STRING(buffer, "   +5,555,555");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+013d", 5555555);
	status += CHECK_STRING(buffer, "+0005,555,555");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+13.10d", 5555555);
	status += CHECK_STRING(buffer, "+0005,555,555");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+13.8d", 5555555);
	status += CHECK_STRING(buffer, "  +05,555,555");
	status += CHECK_RESULT(result, 13);

	return status;
}

int test_uint(void)
{
	int status = 0;

	int result = 0;
	short outs = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%u", 10);
	status += CHECK_STRING(buffer, "10");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#u", 10);
	status += CHECK_STRING(buffer, "10");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%u %u", 10, 100);
	status += CHECK_STRING(buffer, "10 100");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$u %1$u", 10, 100);
	status += CHECK_STRING(buffer, "10 10");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.5u", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.5u%hn", 55, &outs);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);
	status += CHECK_RESULT(outs, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%5u", 55);
	status += CHECK_STRING(buffer, "   55");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%05u", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-05u", 55);
	status += CHECK_STRING(buffer, "55   ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-7.5u", 55);
	status += CHECK_STRING(buffer, "00055  ");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.5u", 55);
	status += CHECK_STRING(buffer, "   00055");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.1u", 55);
	status += CHECK_STRING(buffer, "      55");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#2.5u", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-4.5u", 55);
	status += CHECK_STRING(buffer, "00055");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'u", 5555555);
	status += CHECK_STRING(buffer, "5,555,555");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'.10u", 5555555);
	status += CHECK_STRING(buffer, "0005,555,555");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%'+10u", 5555555);
	status += CHECK_STRING(buffer, " 5,555,555");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%b", 10);
	status += CHECK_STRING(buffer, "1010");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#b", 10);
	status += CHECK_STRING(buffer, "0b1010");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#B", 10);
	status += CHECK_STRING(buffer, "0B1010");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%b %b", 10, 100);
	status += CHECK_STRING(buffer, "1010 1100100");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%b %b%hn", 10, 100, &outs);
	status += CHECK_STRING(buffer, "1010 1100100");
	status += CHECK_RESULT(result, 12);
	status += CHECK_RESULT(outs, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$b %1$b", 10, 100);
	status += CHECK_STRING(buffer, "1010 1010");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.9b", 55);
	status += CHECK_STRING(buffer, "000110111");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%9b", 55);
	status += CHECK_STRING(buffer, "   110111");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%09b", 55);
	status += CHECK_STRING(buffer, "000110111");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#09b", 55);
	status += CHECK_STRING(buffer, "0b0110111");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#.9b", 55);
	status += CHECK_STRING(buffer, "0b000110111");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%o", 10);
	status += CHECK_STRING(buffer, "12");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#o", 10);
	status += CHECK_STRING(buffer, "0o12");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#O", 10);
	status += CHECK_STRING(buffer, "0O12");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%o %o", 10, 100);
	status += CHECK_STRING(buffer, "12 144");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$o %1$o", 10, 100);
	status += CHECK_STRING(buffer, "12 12");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.3o", 55);
	status += CHECK_STRING(buffer, "067");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%3o", 55);
	status += CHECK_STRING(buffer, " 67");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%3o%hn", 55, &outs);
	status += CHECK_STRING(buffer, " 67");
	status += CHECK_RESULT(result, 3);
	status += CHECK_RESULT(outs, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%03o", 55);
	status += CHECK_STRING(buffer, "067");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#05o", 55);
	status += CHECK_STRING(buffer, "0o067");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#-05o", 55);
	status += CHECK_STRING(buffer, "0o67 ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#.5o", 55);
	status += CHECK_STRING(buffer, "0o00067");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%x", 10);
	status += CHECK_STRING(buffer, "a");
	status += CHECK_RESULT(result, 1);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#x", 10);
	status += CHECK_STRING(buffer, "0xa");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#X", 10);
	status += CHECK_STRING(buffer, "0XA");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%X", 10);
	status += CHECK_STRING(buffer, "A");
	status += CHECK_RESULT(result, 1);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%x %x", 10, 100);
	status += CHECK_STRING(buffer, "a 64");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$x %1$x", 10, 100);
	status += CHECK_STRING(buffer, "a a");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$X %1$x", 10, 100);
	status += CHECK_STRING(buffer, "A a");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%.3x", 55);
	status += CHECK_STRING(buffer, "037");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%3x", 55);
	status += CHECK_STRING(buffer, " 37");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%03x", 55);
	status += CHECK_STRING(buffer, "037");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#03x", 55);
	status += CHECK_STRING(buffer, "0x37");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#05x", 55);
	status += CHECK_STRING(buffer, "0x037");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#.5x", 55);
	status += CHECK_STRING(buffer, "0x00037");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-05x", 55);
	status += CHECK_STRING(buffer, "37   ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-#05x", 55);
	status += CHECK_STRING(buffer, "0x37 ");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-7.5x", 55);
	status += CHECK_STRING(buffer, "00037  ");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.5x", 55);
	status += CHECK_STRING(buffer, "   00037");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#6.5x", 55);
	status += CHECK_STRING(buffer, "0x00037");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%8.1x", 55);
	status += CHECK_STRING(buffer, "      37");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#8.1x", 55);
	status += CHECK_STRING(buffer, "    0x37");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%-4.5x", 55);
	status += CHECK_STRING(buffer, "00037");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#03x%hn", 55, &outs);
	status += CHECK_STRING(buffer, "0x37");
	status += CHECK_RESULT(result, 4);
	status += CHECK_RESULT(outs, 4);

	return status;
}

int test_float(void)
{
	int status = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%a", 0x1.5p+2);
	status += CHECK_STRING(buffer, "0x1.5p+2");
	status += CHECK_RESULT(result, 8);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%a", -0x1.5p-3);
	status += CHECK_STRING(buffer, "-0x1.5p-3");
	status += CHECK_RESULT(result, 9);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%a", 0x1.5ab9p-3);
	status += CHECK_STRING(buffer, "0x1.5ab9p-3");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%a", 0x1.5ab904p+11);
	status += CHECK_STRING(buffer, "0x1.5ab904p+11");
	status += CHECK_RESULT(result, 14);

	return status;
}

int test_char(void)
{
	int status = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%c", 'd');
	status += CHECK_STRING(buffer, "abcd");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%c%c", 'd', 'e');
	status += CHECK_STRING(buffer, "abcde");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%4c%4c", 'd', 'e');
	status += CHECK_STRING(buffer, "abc   d   e");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%4c%-5c", 'd', 'e');
	status += CHECK_STRING(buffer, "abc   de    ");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%#02.4c", 'd');
	status += CHECK_STRING(buffer, "abc d");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%lc", L'd');
	status += CHECK_STRING(buffer, "abcd");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%4lc%-5lc", L'd', L'e');
	status += CHECK_STRING(buffer, "abc   de    ");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%llc", U'😊');
	status += CHECK_STRING(buffer, "abc😊");
	status += CHECK_RESULT(result, 7);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%4llc", U'😊');
	status += CHECK_STRING(buffer, "abc   😊");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%hc", 'd');
	status += CHECK_STRING(buffer, "abcd");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%*c%-*c", 4, 'd', 5, 'e');
	status += CHECK_STRING(buffer, "abc   de    ");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%*c%-*.*c", 4, 'd', 5, 6, 'e');
	status += CHECK_STRING(buffer, "abc   de    ");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%2$*1$c%4$-*3$c", 4, 'd', 5, 'e');
	status += CHECK_STRING(buffer, "abc   de    ");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%2$*1$c%3$-*1$c", 4, 'd', 'e');
	status += CHECK_STRING(buffer, "abc   de   ");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%2$*1$c%3$-*1$.*4$c", 4, 'd', 'e', 5);
	status += CHECK_STRING(buffer, "abc   de   ");
	status += CHECK_RESULT(result, 11);

	return status;
}

int test_string()
{
	int status = 0;

	int result = 0;
	intmax_t out = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %s\n", "World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.6s\n", "World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.4s\n", "World");
	status += CHECK_STRING(buffer, "Hello Worl\n");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %6.3s\n", "World");
	status += CHECK_STRING(buffer, "Hello    Wor\n");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.*s\n", 5, "World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %2$.*1$s\n", 5, "World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %2$.*1$s\n%3$jn", 5, "World", &out);
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);
	status += CHECK_RESULT(out, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %ls\n", L"World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.6ls\n", L"World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.4ls\n", L"World");
	status += CHECK_STRING(buffer, "Hello Worl\n");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %6.3ls\n", L"World");
	status += CHECK_STRING(buffer, "Hello    Wor\n");
	status += CHECK_RESULT(result, 13);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %.*ls\n", 5, L"World");
	status += CHECK_STRING(buffer, "Hello World\n");
	status += CHECK_RESULT(result, 12);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %s\n%jn", "World😊", &out);
	status += CHECK_STRING(buffer, "Hello World😊\n");
	status += CHECK_RESULT(result, 16);
	status += CHECK_RESULT(out, 16);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %ls\n%jn", L"World😊", &out);
	status += CHECK_STRING(buffer, "Hello World😊\n");
	status += CHECK_RESULT(result, 16);
	status += CHECK_RESULT(out, 16);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %ls\n%jn", u"World😊", &out);
	status += CHECK_STRING(buffer, "Hello World😊\n");
	status += CHECK_RESULT(result, 16);
	status += CHECK_RESULT(out, 16);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "Hello %lls\n%jn", U"World😊", &out);
	status += CHECK_STRING(buffer, "Hello World😊\n");
	status += CHECK_RESULT(result, 16);
	status += CHECK_RESULT(out, 16);

	return status;
}

int test_pointer(void)
{
	int status = 0;

	int result = 0;
	void *ptr = (void *)0x800800800;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%p", ptr);
	status += CHECK_STRING(buffer, "abc0x0000000800800800");
	status += CHECK_RESULT(result, 21);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%-20p", ptr);
	status += CHECK_STRING(buffer, "abc0x0000000800800800  ");
	status += CHECK_RESULT(result, 23);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "abc%21p", ptr);
	status += CHECK_STRING(buffer, "abc   0x0000000800800800");
	status += CHECK_RESULT(result, 24);

	return status;
}

int test_result(void)
{
	int status = 0;

	int out2 = 0;
	int out1 = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%n", &out1);
	status += CHECK_STRING(buffer, "");
	status += CHECK_RESULT(result, 0);
	status += CHECK_RESULT(out1, 0);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "a%nbc%c%n", &out1, 'd', &out2);
	status += CHECK_STRING(buffer, "abcd");
	status += CHECK_RESULT(result, 4);
	status += CHECK_RESULT(out1, 1);
	status += CHECK_RESULT(out2, 4);

	return status;
}

int test_overflow()
{
	int status = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%llu", 18446744073709551615);
	status += CHECK_STRING(buffer, "18446744073709551615");
	status += CHECK_RESULT(result, 20);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%lu", 18446744073709551615);
	status += CHECK_STRING(buffer, "18446744073709551615");
	status += CHECK_RESULT(result, 20);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%u", 18446744073709551615);
	status += CHECK_STRING(buffer, "4294967295");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%u", 4294967295);
	status += CHECK_STRING(buffer, "4294967295");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hu", 4294967295);
	status += CHECK_STRING(buffer, "65535");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hu", 65535);
	status += CHECK_STRING(buffer, "65535");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hhu", 4294967295);
	status += CHECK_STRING(buffer, "255");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hhu", 255);
	status += CHECK_STRING(buffer, "255");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%lld", -9223372036854775808);
	status += CHECK_STRING(buffer, "-9223372036854775808");
	status += CHECK_RESULT(result, 20);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%ld", -9223372036854775808);
	status += CHECK_STRING(buffer, "-9223372036854775808");
	status += CHECK_RESULT(result, 20);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%lld", 9223372036854775807);
	status += CHECK_STRING(buffer, "9223372036854775807");
	status += CHECK_RESULT(result, 19);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%ld", 9223372036854775807);
	status += CHECK_STRING(buffer, "9223372036854775807");
	status += CHECK_RESULT(result, 19);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%d", -2147483648);
	status += CHECK_STRING(buffer, "-2147483648");
	status += CHECK_RESULT(result, 11);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%d", 2147483647);
	status += CHECK_STRING(buffer, "2147483647");
	status += CHECK_RESULT(result, 10);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hd", 32767);
	status += CHECK_STRING(buffer, "32767");
	status += CHECK_RESULT(result, 5);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hd", -32768);
	status += CHECK_STRING(buffer, "-32768");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hhd", 127);
	status += CHECK_STRING(buffer, "127");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%hhd", -128);
	status += CHECK_STRING(buffer, "-128");
	status += CHECK_RESULT(result, 4);

	return status;
}

int test_error()
{
	int status = 0;

	int result = 0;
	char buffer[256] = {0};

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%k");
	status += CHECK_STRING(buffer, "%k");
	status += CHECK_RESULT(result, 2);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%1$k");
	status += CHECK_STRING(buffer, "%1$k");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%*k");
	status += CHECK_STRING(buffer, "%*k");
	status += CHECK_RESULT(result, 3);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#2k");
	status += CHECK_STRING(buffer, "%#2k");
	status += CHECK_RESULT(result, 4);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#2.5k");
	status += CHECK_STRING(buffer, "%#2.5k");
	status += CHECK_RESULT(result, 6);

	memset(buffer, 0, 256);
	result = snprintf(buffer, 256, "%#2$1$.3$k");
	status += CHECK_STRING(buffer, "%#2$1$.3$k");
	status += CHECK_RESULT(result, 10);

	return status;
}

#pragma warning(pop)

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_simple());
	TEST(test_int());
	TEST(test_uint());
	TEST(test_float());
	TEST(test_char());
	TEST(test_string());
	TEST(test_pointer());
	TEST(test_result());
	TEST(test_error());

	VERIFY_RESULT_AND_EXIT();
}
