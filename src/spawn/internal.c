/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/spawn.h>

processinfo *_wlibc_process_table = NULL;
size_t _wlibc_process_table_size = 0;
size_t _wlibc_child_process_count = 0;

RTL_SRWLOCK _wlibc_process_table_srwlock;

void process_init(void)
{
	RtlInitializeSRWLock(&_wlibc_process_table_srwlock);

	_wlibc_process_table = (processinfo *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(processinfo) * 4);

	// Exit the process if this initialization routine fails.
	if (_wlibc_process_table == NULL)
	{
		RtlExitUserProcess(STATUS_NO_MEMORY);
	}

	_wlibc_process_table_size = 4;
	_wlibc_child_process_count = 0;

	// TODO
	for (size_t i = 0; i < _wlibc_process_table_size; i++)
	{
		_wlibc_process_table[i].id = 0;
		_wlibc_process_table[i].handle = NULL;
	}
}

void process_cleanup(void)
{
	RtlFreeHeap(NtCurrentProcessHeap(), 0, _wlibc_process_table);
}

void get_processinfo(pid_t pid, processinfo *pinfo)
{
	pinfo->handle = NULL;
	pinfo->id = 0;

	SHARED_LOCK_PROCESS_TABLE();

	for (size_t i = 0; i < _wlibc_process_table_size; i++)
	{
		if ((DWORD)pid == _wlibc_process_table[i].id)
		{
			pinfo->handle = _wlibc_process_table[i].handle;
			pinfo->id = _wlibc_process_table[i].id;
			break;
		}
	}

	SHARED_UNLOCK_PROCESS_TABLE();
}

int add_child(DWORD id, HANDLE child)
{
	EXCLUSIVE_LOCK_PROCESS_TABLE();

	size_t i = 0;
	for (i = 0; i < _wlibc_process_table_size; i++)
	{
		if (_wlibc_process_table[i].id == 0)
		{
			_wlibc_process_table[i].handle = child;
			_wlibc_process_table[i].id = id;
			break;
		}
	}

	// No more space in process table. Double it's size
	if (i == _wlibc_process_table_size)
	{
		void *temp = (processinfo *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, _wlibc_process_table,
													  sizeof(processinfo) * _wlibc_process_table_size * 2);

		if (temp == NULL)
		{
			goto fail;
		}

		_wlibc_process_table = temp;
		_wlibc_process_table[_wlibc_process_table_size].id = id;
		_wlibc_process_table[_wlibc_process_table_size].handle = child;

		for (i = _wlibc_process_table_size + 1; i < 2 * _wlibc_process_table_size; i++)
		{
			_wlibc_process_table[i].handle = NULL;
			_wlibc_process_table[i].id = 0;
		}

		_wlibc_process_table_size *= 2;
	}

	++_wlibc_child_process_count;

	EXCLUSIVE_UNLOCK_PROCESS_TABLE();

	return 0;

fail:
	errno = ENOMEM;
	return -1;
}

void delete_child(DWORD id)
{
	EXCLUSIVE_LOCK_PROCESS_TABLE();

	for (size_t i = 0; i < _wlibc_child_process_count; i++)
	{
		if (id == _wlibc_process_table[i].id)
		{
			_wlibc_process_table[i].handle = NULL;
			_wlibc_process_table[i].id = 0;

			/* We don't care about the order in which children are spawned. To maintain a continuous array
			   of alive children, do a swap with the exited and waited child with a free slot in the array.
			*/
			size_t j = i + 2;
			for (j = i + 2; j < _wlibc_process_table_size; j++)
			{
				if (_wlibc_process_table[j].handle == NULL)
				{
					break;
				}
			}

			if (j != i)
			{
				_wlibc_process_table[i] = _wlibc_process_table[j - 1];
				_wlibc_process_table[j - 1].handle = NULL;
				_wlibc_process_table[j - 1].id = 0;
			}

			break;
		}
	}
	--_wlibc_child_process_count;

	EXCLUSIVE_UNLOCK_PROCESS_TABLE();
}
