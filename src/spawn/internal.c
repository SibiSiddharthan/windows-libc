/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/spawn.h>
#include <stdlib.h>

process_table *_wlibc_process_table = NULL;
pid_t _wlibc_process_table_size = 0;
pid_t _wlibc_child_process_count = 0;

RTL_SRWLOCK _wlibc_process_table_srwlock;

void process_init(void)
{
	RtlInitializeSRWLock(&_wlibc_process_table_srwlock);

	_wlibc_process_table = (process_table *)malloc(sizeof(process_table) * 4);
	_wlibc_process_table_size = 4;
	_wlibc_child_process_count = 0;
	for (int i = 0; i < _wlibc_process_table_size; i++)
	{
		_wlibc_process_table[i].process_id = -1;
		_wlibc_process_table[i].process_handle = INVALID_HANDLE_VALUE;
	}
}

void process_cleanup(void)
{
	free(_wlibc_process_table);
}

bool is_child(pid_t pid)
{
	bool result = false;

	SHARED_LOCK_PROCESS_TABLE();

	for (pid_t i = 0; i < _wlibc_process_table_size; i++)
	{
		if (pid == _wlibc_process_table[i].process_id)
		{
			result = true;
			break;
		}
	}

	SHARED_UNLOCK_PROCESS_TABLE();

	return result;
}

HANDLE get_child_handle(pid_t pid)
{
	HANDLE child = INVALID_HANDLE_VALUE;

	SHARED_LOCK_PROCESS_TABLE();

	for (pid_t i = 0; i < _wlibc_process_table_size; i++)
	{
		if (pid == _wlibc_process_table[i].process_id)
		{
			child = _wlibc_process_table[i].process_handle;
			break;
		}
	}

	SHARED_UNLOCK_PROCESS_TABLE();
	return child;
}

unsigned int get_child_process_count()
{
	unsigned int child_count;
	SHARED_LOCK_PROCESS_TABLE();
	child_count = _wlibc_child_process_count;
	SHARED_UNLOCK_PROCESS_TABLE();
	return child_count;
}

void add_child(pid_t pid, HANDLE child)
{
	EXCLUSIVE_LOCK_PROCESS_TABLE();

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

	EXCLUSIVE_UNLOCK_PROCESS_TABLE();
}

void delete_child(HANDLE process_handle)
{
	EXCLUSIVE_LOCK_PROCESS_TABLE();

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

	EXCLUSIVE_UNLOCK_PROCESS_TABLE();
}
