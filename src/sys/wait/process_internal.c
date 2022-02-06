/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/process.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

process_table *_wlibc_process_table = NULL;
pid_t _wlibc_process_table_size = 0;
pid_t _wlibc_child_process_count = 0;

CRITICAL_SECTION _wlibc_process_critical;

void process_init(void)
{
	_wlibc_process_table = (process_table *)malloc(sizeof(process_table) * 4);
	_wlibc_process_table_size = 4;
	_wlibc_child_process_count = 0;
	for (int i = 0; i < _wlibc_process_table_size; i++)
	{
		_wlibc_process_table[i].process_id = -1;
		_wlibc_process_table[i].process_handle = INVALID_HANDLE_VALUE;
	}

	InitializeCriticalSection(&_wlibc_process_critical);
}

void process_cleanup(void)
{
	free(_wlibc_process_table);
	DeleteCriticalSection(&_wlibc_process_critical);
}

bool is_child(pid_t pid)
{
	bool result = false;

	EnterCriticalSection(&_wlibc_process_critical);

	for (pid_t i = 0; i < _wlibc_process_table_size; i++)
	{
		if (pid == _wlibc_process_table[i].process_id)
		{
			result = true;
			break;
		}
	}

	LeaveCriticalSection(&_wlibc_process_critical);

	return result;
}

HANDLE get_child_handle(pid_t pid)
{
	HANDLE child = INVALID_HANDLE_VALUE;

	EnterCriticalSection(&_wlibc_process_critical);

	for (pid_t i = 0; i < _wlibc_process_table_size; i++)
	{
		if (pid == _wlibc_process_table[i].process_id)
		{
			child = _wlibc_process_table[i].process_handle;
			break;
		}
	}

	LeaveCriticalSection(&_wlibc_process_critical);
	return child;
}

unsigned int get_child_process_count()
{
	unsigned int child_count;
	EnterCriticalSection(&_wlibc_process_critical);
	child_count = _wlibc_child_process_count;
	LeaveCriticalSection(&_wlibc_process_critical);
	return child_count;
}

void add_child(pid_t pid, HANDLE child)
{
	EnterCriticalSection(&_wlibc_process_critical);

	pid_t i = 0;
	for (i = 0; i < _wlibc_process_table_size; i++)
	{
		if (_wlibc_process_table[i].process_id == -1)
		{
			_wlibc_process_table[i].process_id = pid;
			_wlibc_process_table[i].process_handle = child;
			break;
		}
	}

	// No more space in process table. Double it's size
	if (i == _wlibc_process_table_size)
	{
		process_table *temp = (process_table *)malloc(sizeof(process_table) * _wlibc_process_table_size * 2);
		memcpy(temp, _wlibc_process_table, sizeof(process_table) * _wlibc_process_table_size);
		free(_wlibc_process_table);
		_wlibc_process_table = temp;

		_wlibc_process_table[_wlibc_process_table_size].process_id = pid;
		_wlibc_process_table[_wlibc_process_table_size].process_handle = child;

		for (i = _wlibc_process_table_size + 1; i < 2 * _wlibc_process_table_size; i++)
		{
			_wlibc_process_table[i].process_id = -1;
			_wlibc_process_table[i].process_handle = INVALID_HANDLE_VALUE;
		}
		_wlibc_process_table_size *= 2;
	}

	++_wlibc_child_process_count;

	LeaveCriticalSection(&_wlibc_process_critical);
}

void delete_child(HANDLE process_handle)
{
	EnterCriticalSection(&_wlibc_process_critical);

	for (pid_t i = 0; i < _wlibc_child_process_count; i++)
	{
		if (process_handle == _wlibc_process_table[i].process_handle)
		{
			_wlibc_process_table[i].process_id = -1;
			_wlibc_process_table[i].process_handle = INVALID_HANDLE_VALUE;

			/* We don't care about the order in which children are spawned. To maintain a continuous array
			   of alive children, do a swap with the exited and waited child with a free slot in the array.
			*/
			pid_t j = i + 2;
			for (j = i + 2; j < _wlibc_process_table_size; j++)
			{
				if (_wlibc_process_table[j].process_handle == INVALID_HANDLE_VALUE)
				{
					break;
				}
			}

			if (j != i)
			{
				_wlibc_process_table[i] = _wlibc_process_table[j - 1];
				_wlibc_process_table[j - 1].process_id = -1;
				_wlibc_process_table[j - 1].process_handle = INVALID_HANDLE_VALUE;
			}

			break;
		}
	}
	--_wlibc_child_process_count;

	LeaveCriticalSection(&_wlibc_process_critical);
}
