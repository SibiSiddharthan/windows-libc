/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_VARARGS_INTENRAL_H
#define WLIBC_VARARGS_INTENRAL_H

#include <internal/minmax.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ROUND_UP(x, y)             ((((x) + ((y) - 1)) / (y)) * (y))
#define VARIADIC_ARGS_DEFAULT_SIZE 64

typedef struct _variadic_args
{
	va_list list;
	uint32_t current_index;
	uint32_t read_count;
	void *args[VARIADIC_ARGS_DEFAULT_SIZE];

	uint32_t extra_capacity;
	void **extra_args;
} variadic_args;

static void variadic_args_init(variadic_args *args, va_list list)
{
	memset(args, 0, sizeof(variadic_args));
	args->list = list;
}

static void variadic_args_free(variadic_args *args)
{
	if (args->extra_args != NULL)
	{
		free(args->extra_args);
	}
}

static void *variadic_args_get(variadic_args *args, uint32_t index)
{
	if (index == 0)
	{
		// Current index will always be less than or equal to read count
		if (args->current_index < VARIADIC_ARGS_DEFAULT_SIZE)
		{
			if (args->read_count == args->current_index)
			{
				args->args[args->read_count++] = va_arg(args->list, void *);
			}

			return args->args[args->current_index++];
		}
		else
		{
			if (args->read_count == args->current_index)
			{
				if (args->extra_args == NULL || (args->read_count - VARIADIC_ARGS_DEFAULT_SIZE) == args->extra_capacity)
				{
					args->extra_capacity = MAX(4, args->extra_capacity * 2);
					args->extra_args = (void **)realloc(args->extra_args, sizeof(void *) * args->extra_capacity);

					if (args->extra_args == NULL)
					{
						return NULL;
					}
				}

				args->extra_args[args->read_count++ - VARIADIC_ARGS_DEFAULT_SIZE] = va_arg(args->list, void *);
			}

			return args->extra_args[args->current_index++ - VARIADIC_ARGS_DEFAULT_SIZE];
		}
	}

	if (index <= VARIADIC_ARGS_DEFAULT_SIZE)
	{
		// Read the args upto index
		for (uint32_t i = args->read_count; i < index; ++i)
		{
			args->args[args->read_count++] = va_arg(args->list, void *);
		}

		return args->args[index - 1];
	}

	// Read the args into the static storage first
	for (uint32_t i = args->read_count; i < VARIADIC_ARGS_DEFAULT_SIZE; ++i)
	{
		args->args[args->read_count++] = va_arg(args->list, void *);
	}

	if (args->extra_args == NULL || (index - VARIADIC_ARGS_DEFAULT_SIZE) > args->extra_capacity)
	{
		args->extra_capacity = MAX(4, ROUND_UP(index - VARIADIC_ARGS_DEFAULT_SIZE, 4));
		args->extra_args = (void **)realloc(args->extra_args, sizeof(void *) * args->extra_capacity);

		if (args->extra_args == NULL)
		{
			return NULL;
		}
	}

	// Read upto index args in extra space
	for (uint32_t i = args->read_count; i < index; ++i)
	{
		args->extra_args[args->read_count++ - VARIADIC_ARGS_DEFAULT_SIZE] = va_arg(args->list, void *);
	}

	return args->extra_args[index - VARIADIC_ARGS_DEFAULT_SIZE - 1];
}

#endif
