/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/convert.h>
#include <stdlib.h>
#include <string.h>

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/

struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT)
{
	struct timespec result;
	time_t epoch = LT.QuadPart - 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

LARGE_INTEGER timespec_to_LARGE_INTEGER(const struct timespec *time)
{
	LARGE_INTEGER L;
	L.QuadPart = time->tv_sec * 10000000 + time->tv_nsec / 100;
	L.QuadPart += 116444736000000000LL;
	return L;
}

#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')

static const uint8_t hex_lower_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static const uint8_t hex_upper_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// clang-format off
static const uint8_t hex_to_nibble_table[256] = 
{
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255,                       // 0 - 9
	255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255,         // A - F
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255,         // a - f
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
// clang-format on

typedef struct _digit_parse_state
{
	uint8_t period;
	uint8_t grouping;
	uint8_t fraction;
	uint8_t flags;
} digit_parse_state;

static uint8_t parse_digits(digit_parse_state *state, buffer_t *buffer, uint32_t *count)
{
	uint8_t byte = 0;

begin:
	byte = peekbyte(buffer, 0);

	if (byte == 0)
	{
		return 0;
	}

	if (IS_DIGIT(byte))
	{
		if (state->grouping && state->period == 3)
		{
			return 0;
		}

		readbyte(buffer);
		state->period += 1;
		*count += 1;

		return byte;
	}

	if (byte == ',')
	{
		if (state->flags & CONVERT_GROUP_DIGITS)
		{
			uint8_t b1 = 0, b2 = 0, b3 = 0;

			// consume trailing periods
			if (peekbyte(buffer, 1) == '\0')
			{
				readbyte(buffer);
				*count += 1;

				return 0;
			}

			if (state->fraction)
			{
				if (state->period != 3)
				{
					return 0;
				}

				state->grouping = 1;
				state->period = 0;

				readbyte(buffer);
				*count += 1;

				goto begin;
			}

			if (state->grouping)
			{
				if (state->period != 3)
				{
					return 0;
				}
			}
			else
			{
				if (state->period > 3)
				{
					return 0;
				}
			}

			state->grouping = 1;
			state->period = 0;

			b1 = peekbyte(buffer, 1);
			b2 = peekbyte(buffer, 2);
			b3 = peekbyte(buffer, 3);

			if (IS_DIGIT(b1) && IS_DIGIT(b2) && IS_DIGIT(b3))
			{
				readbyte(buffer);
				*count += 1;

				goto begin;
			}
		}
	}

	return 0;
}

uint32_t uint_to_hex_common(uint8_t buffer[32], uint8_t upper, uintmax_t x)
{
	const uint8_t *table = upper ? hex_upper_table : hex_lower_table;
	uint8_t temp[32] = {0};
	uint8_t pos = 0;

	do
	{
		temp[pos++] = table[x & 0x0F];
		x >>= 4;

	} while (x != 0);

	for (uint8_t i = 0; i < pos; ++i)
	{
		buffer[i] = temp[(pos - i) - 1];
	}

	return pos;
}

uint32_t uint_from_hex_common(buffer_t *buffer, uintmax_t *value)
{
	uint32_t count = 0;
	uint8_t byte = 0;

	*value = 0;

	while ((byte = peekbyte(buffer, 0)) != '\0')
	{
		uint8_t nibble = hex_to_nibble_table[byte];

		if (nibble == 255)
		{
			break;
		}

		*value = (*value << 4) + nibble;
		readbyte(buffer);
		count += 1;
	}

	return count;
}

uint32_t uint_to_oct_common(uint8_t buffer[32], uintmax_t x)
{
	uint8_t temp[32] = {0};
	uint8_t pos = 0;

	do
	{
		temp[pos++] = (x & 0x07) + '0';
		x >>= 3;

	} while (x != 0);

	for (uint8_t i = 0; i < pos; ++i)
	{
		buffer[i] = temp[pos - i - 1];
	}

	return pos;
}

uint32_t uint_from_oct_common(buffer_t *buffer, uintmax_t *value)
{
	uint32_t count = 0;
	uint8_t byte = 0;

	*value = 0;

	while ((byte = peekbyte(buffer, 0)) != '\0')
	{
		if (byte >= '0' && byte <= '7')
		{
			*value = (*value << 3) + (byte - '0');
			readbyte(buffer);
			count += 1;

			continue;
		}

		break;
	}

	return count;
}

uint32_t uint_to_bin_common(uint8_t buffer[64], uintmax_t x)
{
	uint8_t temp[32] = {0};
	uint8_t pos = 0;

	do
	{
		temp[pos++] = (x & 0x1) + '0';
		x >>= 1;

	} while (x != 0);

	for (uint8_t i = 0; i < pos; ++i)
	{
		buffer[i] = temp[pos - i - 1];
	}

	return pos;
}

uint32_t uint_from_bin_common(buffer_t *buffer, uintmax_t *value)
{
	uint32_t count = 0;
	uint8_t byte = 0;

	*value = 0;

	while ((byte = peekbyte(buffer, 0)) != '\0')
	{
		if (byte == '0' || byte == '1')
		{
			*value = (*value << 1) + (byte - '0');
			readbyte(buffer);
			count += 1;

			continue;
		}

		break;
	}

	return count;
}

uint32_t uint_to_dec_common(uint8_t buffer[32], uintmax_t x, uint32_t flags)
{
	uint8_t temp[32] = {0};
	uint8_t pos = 0;
	uint8_t sep = 0;

	do
	{
		if (sep == 3)
		{
			sep = 0;

			if (flags & CONVERT_GROUP_DIGITS)
			{
				temp[pos++] = ',';
			}
		}

		temp[pos++] = (x % 10) + '0';
		x /= 10;
		sep++;

	} while (x != 0);

	for (uint8_t i = 0; i < pos; ++i)
	{
		buffer[i] = temp[pos - i - 1];
	}

	return pos;
}

uint32_t uint_from_dec_common(buffer_t *buffer, uintmax_t *value, uint32_t flags)
{
	uint32_t count = 0;
	uint8_t byte = 0;

	digit_parse_state state = {.flags = (uint8_t)flags};

	*value = 0;
	byte = peekbyte(buffer, 0);

	if (byte == '+')
	{
		readbyte(buffer);
		count++;
	}

	while ((byte = parse_digits(&state, buffer, &count)) != '\0')
	{
		*value = (*value * 10) + (byte - '0');
	}

	return count;
}

uint32_t int_to_dec_common(uint8_t buffer[32], intmax_t x, uint32_t flags)
{
	uint8_t sign = 0;

	if (x == INT64_MIN)
	{
		memcpy(buffer, "-9223372036854775808", 20);
		return 20;
	}

	if (x < 0)
	{
		x = ~x + 1;
		sign = 1;
		*buffer++ = '-';
	}
	else
	{
		if (flags & CONVERT_FORCE_SIGN)
		{
			sign = 1;
			*buffer++ = '+';
		}
	}

	return uint_to_dec_common(buffer, x, flags) + sign;
}

uint32_t int_from_dec_common(buffer_t *buffer, intmax_t *value, uint32_t flags)
{
	uint32_t count = 0;
	uint8_t minus = 0;
	uint8_t byte = 0;

	digit_parse_state state = {.flags = (uint8_t)flags};

	*value = 0;
	byte = peekbyte(buffer, 0);

	if (byte == '+' || byte == '-')
	{
		if (byte == '-')
		{
			minus = 1;
		}

		readbyte(buffer);
		count++;
	}

	while ((byte = parse_digits(&state, buffer, &count)) != '\0')
	{
		if (minus)
		{
			*value = (*value * 10) - (byte - '0');
		}
		else
		{
			*value = (*value * 10) + (byte - '0');
		}
	}

	return count;
}

#define FLOAT32_EXP_BIAS 127
#define FLOAT64_EXP_BIAS 1023

#define FLOAT32_EXP_INF 255
#define FLOAT64_EXP_INF 2047

#define FLOAT32_EXP_MIN (-FLOAT32_EXP_BIAS)
#define FLOAT32_EXP_MAX (FLOAT32_EXP_INF - FLOAT32_EXP_BIAS)
#define FLOAT64_EXP_MIN (-FLOAT64_EXP_BIAS)
#define FLOAT64_EXP_MAX (FLOAT64_EXP_INF - FLOAT64_EXP_BIAS)

#define FLOAT32_AS_UINT32(x) (*(float *)&(x))
#define FLOAT64_AS_UINT64(x) (*(double *)&(x))

#define UINT32_AS_FLOAT32(x) (*(uint32_t *)&(x))
#define UINT64_AS_FLOAT64(x) (*(uint64_t *)&(x))

static uint32_t print_nan(uint8_t buffer[32], uint8_t upper)
{
	if (upper)
	{
		buffer[0] = 'N';
		buffer[1] = 'A';
		buffer[2] = 'N';
	}
	else
	{
		buffer[0] = 'n';
		buffer[1] = 'a';
		buffer[2] = 'n';
	}

	return 3;
}

static uint32_t print_inf(uint8_t buffer[32], uint8_t upper)
{
	if (upper)
	{
		buffer[0] = 'I';
		buffer[1] = 'N';
		buffer[2] = 'F';
	}
	else
	{
		buffer[0] = 'i';
		buffer[1] = 'n';
		buffer[2] = 'f';
	}

	return 3;
}

uint32_t float32_to_hex(uint8_t buffer[64], uint8_t upper, float x)
{
	uint32_t v = UINT32_AS_FLOAT32(x);
	uint8_t sign = 0;
	uint8_t exponent = 0;
	uint32_t mantissa = 0;
	uint32_t pos = 0;

	sign = (v >> 31) & 0x1;             // 1 bit
	exponent = ((v << 1) >> 24) & 0xFF; // 8 bits
	mantissa = v & 0x7FFFFF;            // 23 bits

	// Sign
	if (sign)
	{
		buffer[pos++] = '-';
	}

	// Check nan or inf
	if (exponent == FLOAT32_EXP_INF)
	{
		if (mantissa == 0)
		{
			pos += print_inf(buffer + pos, upper);
		}
		else
		{
			pos += print_nan(buffer + pos, upper);
		}

		return pos;
	}

	// Form '0x1.' or '0x0.'
	buffer[pos++] = '0';
	buffer[pos++] = upper ? 'X' : 'x';

	if (exponent != 0)
	{
		// Normal
		buffer[pos++] = '1';
	}
	else
	{
		// Subnormal
		buffer[pos++] = '0';
	}

	buffer[pos++] = '.';

	// Mantissa
	pos += uint_to_hex_common(buffer + pos, upper, mantissa);

	while (buffer[pos - 1] == '0')
	{
		if (buffer[pos - 2] == '.')
		{
			break;
		}

		--pos;
	}

	// Exponent
	buffer[pos++] = upper ? 'P' : 'p';

	if (exponent >= FLOAT32_EXP_BIAS)
	{
		buffer[pos++] = '+';
		pos += uint_to_dec_common(buffer + pos, exponent - FLOAT32_EXP_BIAS, 0);
	}
	else
	{
		buffer[pos++] = '-';
		pos += uint_to_dec_common(buffer + pos, FLOAT32_EXP_BIAS - exponent, 0);
	}

	return pos;
}

uint32_t float64_to_hex(uint8_t buffer[64], uint8_t upper, double x)
{
	uint64_t v = UINT64_AS_FLOAT64(x);
	uint8_t sign = 0;
	uint16_t exponent = 0;
	uint64_t mantissa = 0;
	uint32_t pos = 0;

	sign = (v >> 63) & 0x1;              // 1 bit
	exponent = ((v << 1) >> 53) & 0x7FF; // 11 bits
	mantissa = v & 0xFFFFFFFFFFFFF;      // 52 bits

	// Sign
	if (sign)
	{
		buffer[pos++] = '-';
	}

	// Check nan or inf
	if (exponent == FLOAT64_EXP_INF)
	{
		if (mantissa == 0)
		{
			pos += print_inf(buffer + pos, upper);
		}
		else
		{
			pos += print_nan(buffer + pos, upper);
		}

		return pos;
	}

	// Form '0x1.' or '0x0.'
	buffer[pos++] = '0';
	buffer[pos++] = upper ? 'X' : 'x';

	if (exponent != 0)
	{
		// Normal
		buffer[pos++] = '1';
	}
	else
	{
		// Subnormal
		buffer[pos++] = '0';
	}

	buffer[pos++] = '.';

	// Mantissa
	pos += uint_to_hex_common(buffer + pos, upper, mantissa);

	while (buffer[pos - 1] == '0')
	{
		if (buffer[pos - 2] == '.')
		{
			break;
		}

		--pos;
	}

	// Exponent
	buffer[pos++] = upper ? 'P' : 'p';

	if (exponent >= FLOAT64_EXP_BIAS)
	{
		buffer[pos++] = '+';
		pos += uint_to_dec_common(buffer + pos, exponent - FLOAT64_EXP_BIAS, 0);
	}
	else
	{
		buffer[pos++] = '-';
		pos += uint_to_dec_common(buffer + pos, FLOAT64_EXP_BIAS - exponent, 0);
	}

	return pos;
}

static uint32_t parse_float_inf_or_nan(buffer_t *buffer, double *value)
{
	uint8_t b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;

	b1 = peekbyte(buffer, 0);
	b2 = peekbyte(buffer, 1);
	b3 = peekbyte(buffer, 2);

	if ((TO_LOWER(b1) == 'i') && (TO_LOWER(b2) == 'n') && (TO_LOWER(b3) == 'f'))
	{
		uint64_t out = ((uint64_t)0x7FF << 52);
		*value = *(double *)&out;

		advance(buffer, 3);

		b1 = peekbyte(buffer, 0);
		b2 = peekbyte(buffer, 1);
		b3 = peekbyte(buffer, 2);
		b4 = peekbyte(buffer, 3);
		b5 = peekbyte(buffer, 4);

		if ((TO_LOWER(b1) == 'i') && (TO_LOWER(b2) == 'n') && (TO_LOWER(b3) == 'i') && (TO_LOWER(b4) == 't') && (TO_LOWER(b5) == 'y'))
		{
			advance(buffer, 5);
			return 8;
		}

		return 3;
	}

	if ((TO_LOWER(b1) == 'n') && (TO_LOWER(b2) == 'a') && (TO_LOWER(b3) == 'n'))
	{
		uint64_t out = 0x7FFFFFFFFFFFFFFF;
		*value = *(double *)&out;

		advance(buffer, 3);
		return 3;
	}

	return 0;
}

uint32_t float_from_hex_common(buffer_t *buffer, double *value)
{
	uint8_t byte = 0;
	uint32_t count = 0;
	uint32_t inf_or_nan = 0;

	uint8_t minus = 0;

	*value = 0;

	byte = peekbyte(buffer, 0);

	if (byte == '+' || byte == '-')
	{
		if (byte == '-')
		{
			minus = 1;
		}

		readbyte(buffer);
		count++;
	}

	inf_or_nan = parse_float_inf_or_nan(buffer, value);

	if (inf_or_nan)
	{
		if (minus)
		{
			*(uint64_t *)value |= ((uint64_t)1 << 63);
		}

		return count + inf_or_nan;
	}

	byte = readbyte(buffer);
	count++;

	if (byte != '0')
	{
		return 0;
	}

	byte = readbyte(buffer);
	count++;

	if (TO_LOWER(byte) != 'x')
	{
		return 0;
	}

	while ((byte = peekbyte(buffer, 0)) != '\0')
	{
		uint8_t nibble = hex_to_nibble_table[byte];

		if (nibble == 255)
		{
			break;
		}

		*value = (*value * 16.0) + nibble;

		readbyte(buffer);
		count++;
	}

	if (byte == '.')
	{
		double div = 16.0;

		readbyte(buffer);
		count++;

		while ((byte = peekbyte(buffer, 0)) != '\0')
		{
			uint8_t nibble = hex_to_nibble_table[byte];

			if (nibble == 255)
			{
				break;
			}

			*value = *value + (nibble / div);
			div *= 16.0;

			readbyte(buffer);
			count++;
		}
	}

	if (TO_LOWER(byte) == 'p')
	{
		uintmax_t exp = 0;
		uint8_t sign = 0;
		double factor = 1.0;
		double temp = 2.0;

		readbyte(buffer);
		count++;

		byte = peekbyte(buffer, 0);

		if (byte == '+' || byte == '-')
		{
			if (byte == '-')
			{
				sign = 1;
			}

			readbyte(buffer);
			count++;
		}

		count += uint_from_dec_common(buffer, &exp, 0);

		if (exp & 1)
		{
			factor = 2.0;
		}

		exp >>= 1;

		while (exp != 0)
		{
			temp *= temp;

			if (exp & 1)
			{
				factor *= temp;
			}

			exp >>= 1;
		}

		if (sign)
		{
			*value /= factor;
		}
		else
		{
			*value *= factor;
		}
	}

	if (minus)
	{
		*value *= -1.0;
	}

	return count;
}

uint32_t float_from_normal_common(buffer_t *buffer, double *value, uint32_t flags)
{
	uint8_t byte = 0;
	uint32_t count = 0;
	uint32_t inf_or_nan = 0;

	uint8_t minus = 0;
	uint8_t fraction = 0;
	uint8_t exponent = 0;

	digit_parse_state state = {0};

	*value = 0;

	byte = peekbyte(buffer, 0);

	if (byte == '-' || byte == '+')
	{
		if (byte == '-')
		{
			minus = 1;
		}

		readbyte(buffer);
		count++;
	}

	inf_or_nan = parse_float_inf_or_nan(buffer, value);

	if (inf_or_nan)
	{
		if (minus)
		{
			*(uint64_t *)value |= ((uint64_t)1 << 63);
		}

		return count + inf_or_nan;
	}

	state = (digit_parse_state){.flags = (uint8_t)flags};

	while ((byte = parse_digits(&state, buffer, &count)) != '\0')
	{
		*value = (*value * 10.0) + (double)(byte - '0');
	}

	byte = peekbyte(buffer, 0);

	if (byte == '.')
	{
		fraction = 1;

		readbyte(buffer);
		count += 1;
	}
	else
	{
		if (TO_LOWER(byte) == 'e')
		{
			exponent = 1;

			readbyte(buffer);
			count++;
		}
	}

	if (fraction)
	{
		double div = 10.0;

		state = (digit_parse_state){.fraction = 1, .flags = (uint8_t)flags};

		while ((byte = parse_digits(&state, buffer, &count)) != '\0')
		{
			*value += (double)(byte - '0') / div;
			div *= 10.0;
		}

		byte = peekbyte(buffer, 0);

		if (TO_LOWER(byte) == 'e')
		{
			exponent = 1;

			readbyte(buffer);
			count++;
		}
	}

	if (exponent)
	{
		uintmax_t exp = 0;
		uint8_t sign = 0;
		double factor = 1.0;
		double temp = 10.0;

		byte = peekbyte(buffer, 0);

		if (byte == '+' || byte == '-')
		{
			if (byte == '-')
			{
				sign = 1;
			}

			readbyte(buffer);
			count += 1;
		}

		// no grouping for exponents
		count += uint_from_dec_common(buffer, &exp, 0);

		if (exp & 1)
		{
			factor = 10.0;
		}

		exp >>= 1;

		while (exp != 0)
		{
			temp *= temp;

			if (exp & 1)
			{
				factor *= temp;
			}

			exp >>= 1;
		}

		if (sign)
		{
			*value /= factor;
		}
		else
		{
			*value *= factor;
		}
	}

	if (minus)
	{
		*value *= -1.0;
	}

	return count;
}

uint32_t pointer_encode(uint8_t buffer[32], void *ptr)
{
	uintptr_t value = (uintptr_t)ptr;
	uint8_t size = 0;

	*buffer++ = '0';
	*buffer++ = 'x';

#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant

	if (sizeof(void *) == 8)
	{
		value = _byteswap_uint64((unsigned long long)value);
		size = 8;
	}
	else if (sizeof(void *) == 4)
	{
		value = _byteswap_ulong((unsigned long)value);
		size = 4;
	}
	else // 2
	{
		value = _byteswap_ushort((unsigned short)value);
		size = 2;
	}

#pragma warning(pop)

	for (uint32_t i = 0; i < size; ++i)
	{
		uint8_t a, b;

		a = ((uint8_t *)&value)[i] / 16;
		b = ((uint8_t *)&value)[i] % 16;

		*buffer++ = hex_lower_table[a];
		*buffer++ = hex_lower_table[b];
	}

	return 2 + (2 * sizeof(void *));
}

uint32_t pointer_decode(buffer_t *buffer, void **value)
{
	uintmax_t result = 0;
	uint32_t count = 0;
	uint8_t byte = 0;

	*value = NULL;
	byte = peekbyte(buffer, 0);

	if (byte == '0')
	{
		readbyte(buffer);
		count += 1;

		byte = peekbyte(buffer, 0);

		if (TO_LOWER(byte) == 'x')
		{
			readbyte(buffer);
			count += 1;

			count += uint_from_hex_common(buffer, &result);
			*value = (void *)result;

			return count;
		}
	}

	return 0;
}

uint32_t utf8_octets(uint32_t codepoint)
{
	if (codepoint <= 0x7F)
	{
		return 1;
	}

	if (codepoint >= 0x80 && codepoint <= 0x07FF)
	{
		return 2;
	}

	if (codepoint >= 0x800 && codepoint <= 0xFFFF)
	{
		return 3;
	}

	if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
	{
		return 4;
	}

	return 0;
}

uint32_t utf8_encode(uint8_t buffer[32], uint32_t codepoint)
{
	if (codepoint <= 0x7F)
	{
		*buffer++ = codepoint & 0x7F; // 7 bits

		return 1;
	}

	if (codepoint >= 0x80 && codepoint <= 0x07FF)
	{
		*buffer++ = 0xC0 | ((codepoint >> 6) & 0x1F); // 5 bits
		*buffer++ = 0x80 | (codepoint & 0x3F);        // 6 bits

		return 2;
	}

	if (codepoint >= 0x800 && codepoint <= 0xFFFF)
	{
		*buffer++ = 0xE0 | ((codepoint >> 12) & 0x0F); // 4 bits
		*buffer++ = 0x80 | ((codepoint >> 6) & 0x3F);  // 6 bits
		*buffer++ = 0x80 | (codepoint & 0x3F);         // 6 bits

		return 3;
	}

	if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
	{
		*buffer++ = 0xF0 | ((codepoint >> 18) & 0x07); // 3 bits
		*buffer++ = 0x80 | ((codepoint >> 12) & 0x3F); // 6 bits
		*buffer++ = 0x80 | ((codepoint >> 6) & 0x3F);  // 6 bits
		*buffer++ = 0x80 | (codepoint & 0x3F);         // 6 bits

		return 4;
	}

	// Illegal Codepoint
	return 0;
}

uint32_t utf8_decode(void *buffer, uint8_t size, uint32_t *codepoint)
{
	uint8_t *in = buffer;
	uint8_t byte = 0;

	if (size == 0)
	{
		return 0;
	}

	byte = *in++;
	*codepoint = 0;

	if (byte <= 0x7F)
	{
		*codepoint = byte;
		return 1;
	}

	if ((byte & 0xE0) == 0xC0) // Ensure 11'0'xxxxx
	{
		if (size < 2)
		{
			return 0;
		}

		*codepoint |= (byte & 0x1F) << 6;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F);

		// Invalid Encoding
		if (*codepoint < 0x80)
		{
			return 0;
		}

		return 2;
	}

	if ((byte & 0xF0) == 0xE0) // Ensure 111'0'xxxx
	{
		if (size < 3)
		{
			return 0;
		}

		*codepoint |= (byte & 0x0F) << 12;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F) << 6;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F);

		// Surrogate pairs (Invalid codepoints)
		if (*codepoint >= 0xD800 && *codepoint <= 0xDFFF)
		{
			return 0;
		}

		// Invalid Encoding
		if (*codepoint < 0x800)
		{
			return 0;
		}

		return 3;
	}

	if ((byte & 0xF8) == 0xF0) // Ensure 1111'0'xxx
	{
		if (size < 4)
		{
			return 0;
		}

		*codepoint |= (byte & 0x07) << 18;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F) << 12;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F) << 6;
		byte = *in++;

		// Illegal Sequence
		if ((byte & 0xC0) != 0x80)
		{
			return 0;
		}

		*codepoint |= (byte & 0x3F);

		// Invalid Encoding (also catches surrogate pairs)
		if (*codepoint < 0x10000)
		{
			return 0;
		}

		return 4;
	}

	return 0;
}

uint32_t utf16_octets(uint32_t codepoint)
{
	if (codepoint <= 0xFFFF)
	{
		return 2;
	}

	if (codepoint <= 0x10FFFF)
	{
		return 4;
	}

	return 0;
}

uint32_t utf16_encode(uint8_t buffer[32], uint32_t codepoint)
{
	uint32_t v = 0;
	uint16_t high = 0, low = 0;
	uint16_t *out = (uint16_t *)buffer;

	// Invalid Codepoint
	if (codepoint > 0x10FFFF)
	{
		return 0;
	}

	if (codepoint <= 0xFFFF)
	{
		memcpy(buffer, &codepoint, 2);
		return 2;
	}

	v = codepoint - 0x10000;

	// High 16 bits
	high |= 0xD800 | ((v >> 10) & 0x3FF);

	// Low 16 bits
	low |= 0xDC00 | (v & 0x3FF);

	out[0] = high;
	out[1] = low;

	return 4;
}

uint32_t utf16_decode(void *buffer, uint8_t size, uint32_t *codepoint)
{
	uint16_t *in = buffer;
	uint16_t high = 0;
	uint16_t low = 0;

	if (size < 2)
	{
		return 0;
	}

	*codepoint = 0;
	high = *in++;

	if (high < 0xD800 || high > 0xDFFF)
	{
		*codepoint = high;
		return 2;
	}

	if (high >= 0xD800 && high <= 0xDBFF)
	{
		if (size < 4)
		{
			return 0;
		}

		low = *in++;

		if (low >= 0xDC00 && low <= 0xDFFF)
		{
			*codepoint |= (high & 0x3FF) << 10;
			*codepoint |= low & 0x3FF;
			*codepoint += 0x10000;

			return 4;
		}
	}

	// Invalid Sequence
	return 0;
}
