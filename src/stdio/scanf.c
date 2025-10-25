/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <internal/buffer.h>
#include <internal/convert.h>
#include <internal/varargs.h>

typedef uint8_t byte_t;

// Flags
#define SCAN_SUPPRESS_INPUT  0x01 // *
#define SCAN_ALLOCATE_STRING 0x02 // m
#define SCAN_UPPER_CASE      0x20 // 'X|G|A|E'
#define SCAN_GROUP_DIGITS    0x40 // '

// Modifiers
#define SCAN_MOD_NONE        0
#define SCAN_MOD_SHORT_SHORT 1 // hh
#define SCAN_MOD_SHORT       2 // h
#define SCAN_MOD_LONG        3 // l
#define SCAN_MOD_LONG_LONG   4 // ll

#define SCAN_MOD_MAX     5 // j
#define SCAN_MOD_SIZE    6 // z
#define SCAN_MOD_PTRDIFF 7 // t

#define SCAN_MOD_LONG_DOUBLE 8

#define IS_SPACE(x) ((x) == ' ' || ((x) >= '\t' && (x) <= '\r'))

#define GET_BIT(s, x)   (s[(x) / 64] >> ((x) % 64) & 1)
#define SET_BIT(s, x)   (s[(x) / 64] |= ((uint64_t)1 << ((x) % 64)))
#define UNSET_BIT(s, x) (s[(x) / 64] &= (~((uint64_t)1 << ((x) % 64))))

typedef enum _scan_type
{
	SCAN_INT_NUMBER = 1,

	SCAN_UINT_NUMBER,
	SCAN_UINT_BINARY,
	SCAN_UINT_OCTAL,
	SCAN_UINT_HEX,

	SCAN_DOUBLE_NORMAL,
	SCAN_DOUBLE_HEX,
	SCAN_DOUBLE_SCIENTIFIC,
	SCAN_DOUBLE_SCIENTIFIC_SHORT,

	SCAN_CHAR,
	SCAN_STRING,
	SCAN_SET,

	SCAN_POINTER,
	SCAN_RESULT,

	SCAN_UNKNOWN

} scan_type;

typedef struct _scan_config
{
	scan_type type;
	uint16_t modifier;
	uint16_t flags;
	uint32_t width;
	uint32_t index;
	size_t result;
	void *data;
	void *suppress;
	uint64_t set[2];
} scan_config;

static void parse_number(buffer_t *format, uint32_t *index)
{
	byte_t byte = 0;

	*index = 0;

	while ((byte = peekbyte(format, 0)) != '\0')
	{
		if (byte >= '0' && byte <= '9')
		{
			*index = (*index * 10) + (byte - '0');
			readbyte(format);
		}
		else
		{
			break;
		}
	}
}

static void parse_scan_specifier(buffer_t *format, scan_config *config, variadic_args *args)
{
	uint32_t index = 0;
	byte_t byte = 0;
	size_t pos = 0;

	memset(config, 0, sizeof(scan_config));

	// argument
	pos = format->pos;
	parse_number(format, &index);

	if (index != 0)
	{
		if (peekbyte(format, 0) == '$')
		{
			config->index = index;
			readbyte(format);
		}
		else
		{
			format->pos = pos;
		}
	}
	else
	{
		format->pos = pos;
	}

	// flags
	while ((byte = peekbyte(format, 0)) != '\0')
	{
		if (byte == '*')
		{
			config->flags |= SCAN_SUPPRESS_INPUT;
			readbyte(format);
			continue;
		}
		if (byte == '\'')
		{
			config->flags |= SCAN_GROUP_DIGITS;
			readbyte(format);
			continue;
		}
		if (byte == 'm')
		{
			config->flags |= SCAN_ALLOCATE_STRING;
			readbyte(format);
			continue;
		}

		break;
	}

	// width
	parse_number(format, &index);
	config->width = index;

	// length modifiers
	switch (byte = peekbyte(format, 0))
	{
	case 'h':
	{
		readbyte(format);

		if (peekbyte(format, 0) == 'h')
		{
			config->modifier = SCAN_MOD_SHORT_SHORT;
			readbyte(format);
		}
		else
		{
			config->modifier = SCAN_MOD_SHORT;
		}
	}
	break;
	case 'l':
	{
		readbyte(format);

		if (peekbyte(format, 0) == 'l')
		{
			config->modifier = SCAN_MOD_LONG_LONG;
			readbyte(format);
		}
		else
		{
			config->modifier = SCAN_MOD_LONG;
		}
	}
	break;
	case 'L':
		readbyte(format);
		config->modifier = SCAN_MOD_LONG_DOUBLE;
		break;
	case 'j':
		readbyte(format);
		config->modifier = SCAN_MOD_MAX;
		break;
	case 'z':
		readbyte(format);
		config->modifier = SCAN_MOD_SIZE;
		break;
	case 't':
		readbyte(format);
		config->modifier = SCAN_MOD_PTRDIFF;
		break;
	}

	// conversion
	switch (byte = readbyte(format))
	{
	// integer
	case 'i':
	case 'd':
		config->type = SCAN_INT_NUMBER;
		break;
	case 'u':
		config->type = SCAN_UINT_NUMBER;
		break;
	case 'B':
		config->flags |= SCAN_UPPER_CASE;
	case 'b':
		config->type = SCAN_UINT_BINARY;
		break;
	case 'O':
		config->flags |= SCAN_UPPER_CASE;
	case 'o':
		config->type = SCAN_UINT_OCTAL;
		break;
	case 'X':
		config->flags |= SCAN_UPPER_CASE;
	case 'x':
		config->type = SCAN_UINT_HEX;
		break;

	// float
	case 'A':
		config->flags |= SCAN_UPPER_CASE;
	case 'a':
		config->type = SCAN_DOUBLE_HEX;
		break;
	case 'F':
		config->flags |= SCAN_UPPER_CASE;
	case 'f':
		config->type = SCAN_DOUBLE_NORMAL;
		break;
	case 'E':
		config->flags |= SCAN_UPPER_CASE;
	case 'e':
		config->type = SCAN_DOUBLE_SCIENTIFIC;
		break;
	case 'G':
		config->flags |= SCAN_UPPER_CASE;
	case 'g':
		config->type = SCAN_DOUBLE_SCIENTIFIC_SHORT;
		break;

	// misc
	case 'c':
		config->type = SCAN_CHAR;
		break;
	case 's':
		config->type = SCAN_STRING;
		break;
	case '[':
		config->type = SCAN_SET;
		break;
	case 'p':
		config->type = SCAN_POINTER;
		break;
	case 'n':
		config->type = SCAN_RESULT;
		break;

	default:
		config->type = SCAN_UNKNOWN;
		break;
	}

	// parse the set
	if (config->type == SCAN_SET)
	{
		byte_t first = 0;
		byte_t exclude = 0;
		byte_t closed = 0;

		while ((byte = peekbyte(format, 0)) != '\0')
		{
			if (first == 0)
			{
				first = 1;

				if (byte == ']')
				{
					SET_BIT(config->set, byte);

					readbyte(format);
					continue;
				}

				if (byte == '^')
				{
					exclude = 1;
					config->set[0] = 0xFFFFFFFFFFFFFFFF;
					config->set[1] = 0xFFFFFFFFFFFFFFFF;

					// check to see if next byte is ']'
					if (peekbyte(format, 1) == ']')
					{
						UNSET_BIT(config->set, ']');
						readbyte(format);
					}

					readbyte(format);
					continue;
				}
			}

			if (byte == ']')
			{
				closed = 1;
				readbyte(format);

				break;
			}

			// only ascii 128 character set
			if (byte < 128)
			{
				if (peekbyte(format, 1) == '-')
				{
					byte_t from = byte;
					byte_t to = peekbyte(format, 2);

					if (to != ']')
					{
						while (from <= to)
						{
							if (exclude)
							{
								UNSET_BIT(config->set, from);
							}
							else
							{
								SET_BIT(config->set, from);
							}

							++from;
						}

						advance(format, 3);
						continue;
					}
				}

				if (exclude)
				{
					UNSET_BIT(config->set, byte);
				}
				else
				{
					SET_BIT(config->set, byte);
				}
			}

			readbyte(format);
		}

		// set to unknown on invalid parse
		if (!closed)
		{
			config->type = SCAN_UNKNOWN;
		}
	}

	if (config->type != SCAN_UNKNOWN)
	{
		if (config->flags & SCAN_SUPPRESS_INPUT)
		{
			config->data = &config->suppress;
		}
		else
		{
			config->data = variadic_args_get(args, config->index);
		}
	}
}

static uint32_t consume_whitespaces(buffer_t *buffer)
{
	uint32_t count = 0;
	byte_t byte = 0;

	while ((byte = peekbyte(buffer, 0)) != '\0')
	{
		if (IS_SPACE(byte))
		{
			readbyte(buffer);
			count += 1;

			continue;
		}

		break;
	}

	return count;
}

static uint32_t do_scan(buffer_t *buffer, scan_config *config)
{
	uint32_t result = 0;

	if (config->type == SCAN_INT_NUMBER)
	{
		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += i32_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SHORT:
			result += i16_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SHORT_SHORT:
			result += i8_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_LONG:
			result += i64_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_LONG_LONG:
			result += imax_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_MAX:
			result += imax_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SIZE:
			result += isize_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_PTRDIFF:
			result += iptr_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		default:
			result += i32_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		}

		return result;
	}

	if (config->type == SCAN_UINT_NUMBER)
	{
		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += u32_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SHORT:
			result += u16_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SHORT_SHORT:
			result += u8_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_LONG:
			result += u64_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_LONG_LONG:
			result += umax_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_MAX:
			result += umax_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_SIZE:
			result += usize_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_PTRDIFF:
			result += uptr_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		default:
			result += u32_from_dec(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		}

		return result;
	}

	if (config->type == SCAN_UINT_BINARY)
	{
		if (peekbyte(buffer, 0) == '0' && (TO_LOWER(peekbyte(buffer, 1)) == 'b'))
		{
			advance(buffer, 2);
			result += 2;
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += u32_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_SHORT:
			result += u16_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_SHORT_SHORT:
			result += u8_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_LONG:
			result += u64_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_LONG_LONG:
			result += umax_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_MAX:
			result += umax_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_SIZE:
			result += usize_from_bin(buffer, config->data);
			break;
		case SCAN_MOD_PTRDIFF:
			result += uptr_from_bin(buffer, config->data);
			break;
		default:
			result += u32_from_bin(buffer, config->data);
			break;
		}

		return result;
	}

	if (config->type == SCAN_UINT_OCTAL)
	{
		if (peekbyte(buffer, 0) == '0')
		{
			readbyte(buffer);
			result += 1;

			if (TO_LOWER(peekbyte(buffer, 0)) == 'o')
			{
				readbyte(buffer);
				result += 1;
			}
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += u32_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_SHORT:
			result += u16_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_SHORT_SHORT:
			result += u8_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_LONG:
			result += u64_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_LONG_LONG:
			result += umax_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_MAX:
			result += umax_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_SIZE:
			result += usize_from_oct(buffer, config->data);
			break;
		case SCAN_MOD_PTRDIFF:
			result += uptr_from_oct(buffer, config->data);
			break;
		default:
			result += u32_from_oct(buffer, config->data);
			break;
		}

		return result;
	}

	if (config->type == SCAN_UINT_HEX)
	{
		if (peekbyte(buffer, 0) == '0' && (TO_LOWER(peekbyte(buffer, 1)) == 'x'))
		{
			advance(buffer, 2);
			result += 2;
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += u32_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_SHORT:
			result += u16_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_SHORT_SHORT:
			result += u8_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_LONG:
			result += u64_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_LONG_LONG:
			result += umax_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_MAX:
			result += umax_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_SIZE:
			result += usize_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_PTRDIFF:
			result += uptr_from_hex(buffer, config->data);
			break;
		default:
			result += u32_from_hex(buffer, config->data);
			break;
		}

		return result;
	}

	if (config->type == SCAN_DOUBLE_NORMAL || config->type == SCAN_DOUBLE_SCIENTIFIC || config->type == SCAN_DOUBLE_SCIENTIFIC_SHORT)
	{
		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += float32_from_normal(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		case SCAN_MOD_LONG:
		case SCAN_MOD_LONG_LONG:
		case SCAN_MOD_LONG_DOUBLE:
			result += float64_from_normal(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		default:
			result += float32_from_normal(buffer, config->data, config->flags & SCAN_GROUP_DIGITS);
			break;
		}

		return result;
	}

	if (config->type == SCAN_DOUBLE_HEX)
	{
		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			result += float32_from_hex(buffer, config->data);
			break;
		case SCAN_MOD_LONG:
		case SCAN_MOD_LONG_LONG:
		case SCAN_MOD_LONG_DOUBLE:
			result += float64_from_hex(buffer, config->data);
			break;
		default:
			result += float32_from_hex(buffer, config->data);
			break;
		}

		return result;
	}

	if (config->type == SCAN_CHAR)
	{
		uint32_t codepoint = 0;

		if (pending(buffer) == 0)
		{
			return 0;
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			*(byte_t *)config->data = readbyte(buffer);
			result = 1;
			break;
		case SCAN_MOD_LONG:
		{
			byte_t data[8] = {0};
			uint32_t count = 0;

			result = utf8_decode(current(buffer), (uint8_t)pending(buffer), &codepoint);
			advance(buffer, result);

			if (result != 0)
			{
				if (codepoint > UINT16_MAX)
				{
					*(uint16_t *)config->data = UINT16_MAX;
					return result;
				}

				count = utf16_encode(data, codepoint);
				memcpy(config->data, data, count);
			}
		}
		break;
		case SCAN_MOD_LONG_LONG:
		{
			result = utf8_decode(current(buffer), (uint8_t)pending(buffer), &codepoint);
			advance(buffer, result);

			if (result != 0)
			{
				*(uint32_t *)config->data = codepoint;
			}
		}
		break;
		default:
			*(byte_t *)config->data = readbyte(buffer);
			result = 1;
			break;
		}

		return result;
	}

	if (config->type == SCAN_STRING)
	{
		byte_t byte = 0;
		buffer_t out = {0};

		if ((config->flags & SCAN_ALLOCATE_STRING) == 0)
		{
			out.data = config->data;
			out.size = UINT64_MAX;
		}
		else
		{
			out.write = memory_buffer_write;
		}

		uint32_t count = 0;
		uint32_t codepoint = 0;

		if (config->flags & SCAN_SUPPRESS_INPUT)
		{
			while ((byte = peekbyte(buffer, 0)) != '\0')
			{
				if (IS_SPACE(byte))
				{
					break;
				}

				readbyte(buffer);
				result += 1;
			}

			goto str_end;
		}

		while ((byte = peekbyte(buffer, 0)) != '\0')
		{
			if (IS_SPACE(byte))
			{
				break;
			}

			switch (config->modifier)
			{
			case SCAN_MOD_NONE:
				writebyte(&out, byte);
				readbyte(buffer);
				result += 1;
				break;
			case SCAN_MOD_LONG:
			{
				count = utf8_decode(current(buffer), (uint8_t)pending(buffer), &codepoint);
				advance(buffer, count);
				result += count;

				if (count == 0)
				{
					goto str_end;
				}

				count = utf16_encode(current(&out), codepoint);
				advance(&out, count);
			}
			break;
			case SCAN_MOD_LONG_LONG:
			{
				count = utf8_decode(current(buffer), (uint8_t)pending(buffer), &codepoint);
				advance(buffer, count);
				result += count;

				if (count == 0)
				{
					goto str_end;
				}

				writen(&out, &codepoint, 4);
			}
			break;
			default:
				writebyte(&out, byte);
				readbyte(buffer);
				result += 1;
				break;
			}
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			writebyte(&out, 0);
			break;
		case SCAN_MOD_LONG:
			writebyte(&out, 0);
			writebyte(&out, 0);
			break;
		case SCAN_MOD_LONG_LONG:
			writebyte(&out, 0);
			writebyte(&out, 0);
			writebyte(&out, 0);
			writebyte(&out, 0);
			break;
		default:
			writebyte(&out, 0);
			break;
		}

	str_end:

		if (config->flags & SCAN_ALLOCATE_STRING)
		{
			*(void **)(config->data) = out.data;
		}

		return result;
	}

	if (config->type == SCAN_SET)
	{
		byte_t byte = 0;
		buffer_t out = {0};

		if ((config->flags & SCAN_ALLOCATE_STRING) == 0)
		{
			out.data = config->data;
			out.size = UINT64_MAX;
		}
		else
		{
			out.write = memory_buffer_write;
		}

		if (config->flags & SCAN_SUPPRESS_INPUT)
		{
			while ((byte = peekbyte(buffer, 0)) != '\0')
			{
				if (byte > 127)
				{
					break;
				}

				if (!GET_BIT(config->set, byte))
				{
					break;
				}

				readbyte(buffer);
				result += 1;
			}

			goto set_end;
		}

		while ((byte = peekbyte(buffer, 0)) != '\0')
		{
			if (byte > 127)
			{
				break;
			}

			if (!GET_BIT(config->set, byte))
			{
				break;
			}

			switch (config->modifier)
			{
			case SCAN_MOD_NONE:
				writebyte(&out, byte);
				break;
			case SCAN_MOD_LONG:
				writebyte(&out, byte);
				writebyte(&out, 0);
				break;
			case SCAN_MOD_LONG_LONG:
				writebyte(&out, byte);
				writebyte(&out, 0);
				writebyte(&out, 0);
				writebyte(&out, 0);
				break;
			default:
				writebyte(&out, byte);
				break;
			}

			result += 1;
			readbyte(buffer);
		}

		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			writebyte(&out, 0);
			break;
		case SCAN_MOD_LONG:
			writebyte(&out, 0);
			writebyte(&out, 0);
			break;
		case SCAN_MOD_LONG_LONG:
			writebyte(&out, 0);
			writebyte(&out, 0);
			writebyte(&out, 0);
			writebyte(&out, 0);
			break;
		default:
			writebyte(&out, 0);
			break;
		}

	set_end:

		if (config->flags & SCAN_ALLOCATE_STRING)
		{
			*(void **)(config->data) = out.data;
		}

		return result;
	}

	if (config->type == SCAN_POINTER)
	{
		return pointer_decode(buffer, config->data);
	}

	if (config->type == SCAN_RESULT)
	{
		switch (config->modifier)
		{
		case SCAN_MOD_NONE:
			*(uint32_t *)config->data = (uint32_t)config->result;
			break;
		case SCAN_MOD_SHORT:
			*(uint16_t *)config->data = (uint16_t)config->result;
			break;
		case SCAN_MOD_SHORT_SHORT:
			*(uint8_t *)config->data = (uint8_t)config->result;
			break;
		case SCAN_MOD_LONG:
			*(uint64_t *)config->data = (uint64_t)config->result;
			break;
		case SCAN_MOD_LONG_LONG:
			*(uint64_t *)config->data = (uint64_t)config->result;
			break;
		case SCAN_MOD_MAX:
			*(uintmax_t *)config->data = (uintmax_t)config->result;
			break;
		case SCAN_MOD_SIZE:
			*(size_t *)config->data = (size_t)config->result;
			break;
		case SCAN_MOD_PTRDIFF:
			*(ptrdiff_t *)config->data = (ptrdiff_t)config->result;
			break;
		default:
			*(uint32_t *)config->data = (uint32_t)config->result;
			break;
		}

		return 0;
	}

	return 0;
}

static uint32_t scan_arg(buffer_t *buffer, scan_config *config)
{
	uint32_t result = 0;
	size_t old_size = 0;

	if (config->type != SCAN_RESULT && config->type != SCAN_CHAR && config->type != SCAN_SET)
	{
		result += consume_whitespaces(buffer);
	}

	if (config->width > 0)
	{
		old_size = buffer->size;
		buffer->size = buffer->pos + config->width;
	}

	result += do_scan(buffer, config);

	if (config->width > 0)
	{
		buffer->size = old_size;
	}

	return result;
}

static int wlibc_scanf_internal(buffer_t *buffer, const char *format, va_list list)
{
	variadic_args args = {0};
	scan_config config = {0};
	buffer_t in = {.data = (void *)format, .pos = 0, .size = strnlen(format, 65536)};

	uint32_t processed = 0;
	uint32_t result = 0;
	uint32_t count = 0;
	byte_t byte = 0;

	variadic_args_init(&args, list);

	while ((byte = readbyte(&in)) != '\0')
	{
		if (byte == '%')
		{
			byte = peekbyte(&in, 0);

			if (byte == '\0')
			{
				break;
			}

			if (byte == '%')
			{
				if (peekbyte(buffer, 0) != byte)
				{
					break;
				}

				readbyte(&in);
				readbyte(buffer);

				processed += 1;

				continue;
			}

			parse_scan_specifier(&in, &config, &args);

			if (config.type == SCAN_UNKNOWN)
			{
				return -1;
			}

			config.result = processed;
			count = scan_arg(buffer, &config);

			if (buffer->error)
			{
				return -1;
			}

			if ((config.type != SCAN_RESULT) && ((config.flags & SCAN_SUPPRESS_INPUT) == 0))
			{
				if (count == 0)
				{
					if (config.type != SCAN_SET || peekbyte(&in, 0) == 0)
					{
						break;
					}
				}

				result += 1;
			}

			processed += count;

			continue;
		}
		else
		{
			if (IS_SPACE(byte))
			{
				processed += consume_whitespaces(buffer);
				continue;
			}

			if (peekbyte(buffer, 0) != byte)
			{
				break;
			}

			readbyte(buffer);
			processed += 1;
		}
	}

	variadic_args_free(&args);

	return (int)result;
}

int wlibc_vfscanf(FILE *restrict stream, const char *restrict format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (stream == NULL)
	{
		return -1;
	}

	return wlibc_scanf_internal(&(buffer_t){.data = NULL, .size = 0}, format, args);
}

int wlibc_vsscanf(const char *restrict str, const char *restrict format, va_list args)
{
	if (format == NULL || str == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	return wlibc_scanf_internal(&(buffer_t){.data = (void *)str, .size = strnlen(str, 65536)}, format, args);
}
