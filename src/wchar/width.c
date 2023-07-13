/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <wchar.h>

int wlibc_wcwidth(wchar_t wc)
{
	// Test for NUL character.
	if (wc == 0)
	{
		return 0;
	}

	// Test for ASCII printable characters.
	if (wc >= 0x20 && wc < 0x7f)
	{
		return 1;
	}

	// Test for ASCII control characters.
	if (wc < 0xa0)
	{
		return -1;
	}

	// Test for surrogate pair values.
	// U+D800 to U+DBFF (high surrogate)
	// U+DC00 to U+DFFF (low surrogate)
	if (wc >= 0xd800 && wc <= 0xdfff)
	{
		return -1;
	}

	// Test for non spacing characters.
	if (wc >= 0x0300 && wc <= 0x036f)
	{
		return 0;
	}

	// TODO
	// Ignore other non spacing characters for now.
	// Ignore CJK (Chinese Japanese Korean) characters for now.

	return 1;
}

int wlibc_wcswidth(const wchar_t *wstr, size_t size)
{
	int str_width = 0;
	int char_width = 0;

	for (size_t i = 0; wstr[i] != L'\0' && i < size; ++i)
	{
		char_width = wlibc_wcwidth(wstr[i]);

		if (char_width == -1)
		{
			return -1;
		}

		str_width += char_width;
	}

	return str_width;
}
