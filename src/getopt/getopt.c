/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Global variable initialization
char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = '?';

int wlibc_common_getopt(int argc, char *argv[], const char *optstring, const struct option *longopts, int *longindex)
{
	static int index = 0;
	static bool processing_short_option = false;
	static bool processing_long_option = false;

	bool stop_at_first_nonoption = false;
	bool print_errors = true;

	if (optstring == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (opterr == 0)
	{
		print_errors = false;
	}

	if (optstring[0] == '+' || getenv("POSIXLY_CORRECT") != NULL)
	{
		stop_at_first_nonoption = true;
	}

	// Preferred reset value according to POSIX.
	if (optind == 0)
	{
		index = 0;
		optind = 1;
	}

	if (optind == argc)
	{
		// End of processing
		index = 0;
		return -1;
	}

	for (int i = optind; i < argc; ++i)
	{
		char *argument = argv[i];

		if (index == 0)
		{
			processing_long_option = false;
			processing_short_option = false;

			if (argument[index] == '-' && argument[index + 1] == '-')
			{
				if (argument[index + 2] == '\0')
				{
					// End of options.
					return -1;
				}
				else
				{
					index += 2;
					processing_long_option = true;
				}
			}
			else if (argument[index] == '-' && argument[index + 1] != '\0')
			{
				index += 1;
				processing_short_option = true;
			}
		}

		if (!(processing_long_option || processing_short_option))
		{
			bool exchange_arguments = false;

			if (stop_at_first_nonoption)
			{
				// Not an error.
				return -1;
			}

			// Check if there are options after this argument.
			for (int j = i + 1; j < argc; ++j)
			{
				if (argv[j][0] == '-')
				{
					exchange_arguments = true;
					break;
				}
			}

			// If so, move the non argument to the end.
			if (exchange_arguments)
			{
				for (int j = i + 1; j < argc; ++j)
				{
					argv[j - 1] = argv[j];
				}

				argv[argc - 1] = argument;
				--i;
			}

			continue;
		}

		if (processing_long_option)
		{
			// Long options
			if (longopts == NULL)
			{
				index = 0;
				++optind;
				return optstring[0] == ':' ? ':' : '?'; // Ambiguous
			}

			size_t argument_length;
			char *long_option = &argument[index];
			char *equal_pos = strchr(long_option, '=');

			if (equal_pos == NULL)
			{
				argument_length = strlen(long_option);
			}
			else
			{
				argument_length = equal_pos - long_option;
			}

			for (int j = 0; longopts[j].name != NULL; ++j)
			{
				size_t option_length = strlen(longopts[j].name);

				if (argument_length == option_length && strncmp(long_option, longopts[j].name, argument_length) == 0)
				{
					if (longindex != NULL)
					{
						*longindex = j;
					}

					// Always move to next argv in case of long options.
					index = 0;
					++optind;

					switch (longopts[j].has_arg)
					{
					case no_argument:
					{
						if (equal_pos != NULL)
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '--%s' does not allow a paramter\n", program_invocation_name, long_option);
							}
							return optstring[0] == ':' ? ':' : '?';
						}

						optarg = NULL;
					}
					break;
					case required_argument:
					{
						if (equal_pos != NULL)
						{
							// --arg=value
							optarg = (equal_pos + 1);
						}
						else
						{
							// --arg value
							char *parameter = argv[optind];
							if (parameter == NULL || parameter[0] == '-')
							{
								if (print_errors)
								{
									fprintf(stderr, "%s: '--%s' requires a parameter\n", program_invocation_name, long_option);
								}

								return optstring[0] == ':' ? ':' : '?';
							}

							optarg = parameter;
							++optind;
						}
					}
					break;
					case optional_argument:
					{
						if (equal_pos != NULL)
						{
							// --arg=value
							optarg = (equal_pos + 1);
						}
						else
						{
							// --arg value
							char *parameter = argv[optind];
							if (parameter != NULL && parameter[0] != '-')
							{
								optarg = parameter;
								++optind;
							}
							else
							{
								optarg = NULL;
							}
						}
					}
					break;
					}

					if (longopts[j].flag == NULL)
					{
						return longopts[j].val;
					}
					else
					{
						*longopts[j].flag = longopts[j].val;
						return 0;
					}
				}
			}

			// Option not found.
			if (print_errors)
			{
				fprintf(stderr, "%s: unrecognized option '--%s'\n", program_invocation_name, long_option);
			}

			return -1;
		}

		if (processing_short_option)
		{
			// Short options
			if (optstring[0] == '\0')
			{
				// Error
				return -1;
			}

			char option = argument[index];
			char *start = (char *)(optstring[0] == '+' ? &optstring[1] : &optstring[0]);
			char *found = strchr(start, option);

			index += 1;

			if (argument[index] == '\0')
			{
				// Move to next argv;
				++optind;
				index = 0;
			}

			if (found != NULL)
			{

				if (*(found + 1) == ':' && *(found + 2) == ':')
				{
					// Optional parameter
					if (index != 0)
					{
						// -ovalue
						optarg = &argument[index];
						++optind;
						index = 0;
					}
					else
					{
						// -o value
						argument = argv[optind];
						if (argument != NULL)
						{
							optarg = argument;
							++optind;
						}
						else
						{
							optarg = NULL;
						}
					}
				}
				else if (*(found + 1) == ':')
				{
					// Mandatory parameter
					if (index != 0)
					{
						optarg = &argument[index];
						++optind;
						index = 0;
					}
					else
					{
						argument = argv[optind];
						if (argument != NULL)
						{
							optarg = argument;
							++optind;
						}
						else
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '-%c' requires an argument\n", program_invocation_name, option);
							}

							optarg = NULL;
							return optstring[0] == ':' ? ':' : '?';
						}
					}
				}
				else
				{
					// -f
					optarg = NULL;
				}
				return option;
			}
			else
			{
				if (print_errors)
				{
					fprintf(stderr, "%s: unrecognized option '-%c'\n", program_invocation_name, option);
				}

				optarg = NULL;
				optopt = option;
				return optstring[0] == ':' ? ':' : '?';
			}
		}
	}

	// Unreachable
	return -1;
}
