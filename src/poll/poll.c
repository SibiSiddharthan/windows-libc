/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <intrin.h>

int do_poll(const fdinfo *pinfo, struct pollfd *fds, nfds_t nfds, const struct timespec *timeout)
{
	LARGE_INTEGER duetime = {0}, current = {0};
	int result = 0;

	if (timeout != NULL)
	{
		GetSystemTimeAsFileTime((LPFILETIME)&current);
		duetime.QuadPart = current.QuadPart + timeout->tv_sec * 10000000 + timeout->tv_nsec / 100;
	}

	while (1)
	{
		for (nfds_t i = 0; i < nfds; ++i)
		{
			switch (pinfo[i].type)
			{
			case INVALID_HANDLE:
			{
				// Don't set errno to EBADF in this case.
				fds[i].revents = POLLNVAL;
				break;
			}

			case CONSOLE_HANDLE:
			{
				BOOL status;
				DWORD events_read = 0;
				INPUT_RECORD record[4];

				status = PeekConsoleInputW(pinfo[i].handle, record, 4, &events_read);
				if (status == 0)
				{
					fds[i].revents = POLLNVAL;
					break;
				}

				if (pinfo[i].flags & O_RDWR)
				{
					fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM);
					if (events_read > 0)
					{
						fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM);
					}
				}
				else if (pinfo[i].flags & O_WRONLY)
				{
					fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM);
				}
				else // pinfo[i].flags == O_RDONLY
				{
					if (events_read > 0)
					{
						fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM);
					}
				}
				break;
			}

			case NULL_HANDLE:
			case FILE_HANDLE:
			case DIRECTORY_HANDLE:
			{
				// Don't worry if there is data or not.
				if (pinfo[i].flags & O_RDWR)
				{
					fds[i].revents = fds[i].events & (POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM);
				}
				else if (pinfo[i].flags & O_WRONLY)
				{
					fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM);
				}
				else // pinfo[i].flags == O_RDONLY
				{
					fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM);
				}
				break;
			}

			case PIPE_HANDLE:
			{
				NTSTATUS status;
				IO_STATUS_BLOCK io;
				FILE_PIPE_LOCAL_INFORMATION pipe_info;

				status =
					NtQueryInformationFile(pinfo[i].handle, &io, &pipe_info, sizeof(FILE_PIPE_LOCAL_INFORMATION), FilePipeLocalInformation);
				if (status != STATUS_SUCCESS)
				{
					fds[i].revents = POLLNVAL;
					break;
				}

				if (pinfo[i].flags & O_RDWR)
				{
					if (pipe_info.WriteQuotaAvailable > 0)
					{
						fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM);
					}
					if (pipe_info.ReadDataAvailable > 0)
					{
						fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM);
					}
				}
				else if (pinfo[i].flags & O_WRONLY)
				{
					if (pipe_info.WriteQuotaAvailable > 0)
					{
						fds[i].revents = fds[i].events & (POLLOUT | POLLWRNORM);
					}
					// Polling a write end of a pipe where the read end is closed.
					if (pipe_info.NamedPipeState == FILE_PIPE_DISCONNECTED_STATE || pipe_info.NamedPipeState == FILE_PIPE_CLOSING_STATE)
					{
						fds[i].revents |= POLLERR;
					}
				}
				else // pinfo[i].flags == O_RDONLY
				{
					if (pipe_info.ReadDataAvailable > 0)
					{
						fds[i].revents = fds[i].events & (POLLIN | POLLRDNORM);
					}
					// Polling a read end of a pipe where the write end is closed.
					if (pipe_info.NamedPipeState == FILE_PIPE_DISCONNECTED_STATE || pipe_info.NamedPipeState == FILE_PIPE_CLOSING_STATE)
					{
						fds[i].revents |= POLLHUP;
					}
				}

				break;
			}
			}

			if (fds[i].revents != 0)
			{
				++result;
			}
		}

		if (result == 0)
		{
			if (timeout != NULL)
			{
				GetSystemTimeAsFileTime((LPFILETIME)&current);
			}

			if (current.QuadPart <= duetime.QuadPart)
			{
				// Retry again.
				_mm_pause();
				continue;
			}
		}

		break;
	}

	return result;
}

int wlibc_common_poll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
{
	int result;
	sigset_t oldmask;
	fdinfo *pinfo;

	if (fds == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	pinfo = (fdinfo *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(fdinfo) * nfds);
	if (pinfo == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	// TODO Avoid locking every single time.
	for (nfds_t i = 0; i < nfds; ++i)
	{
		fds[i].revents = 0;
		get_fdinfo(fds[i].fd, &pinfo[i]);
	}

	if (sigmask)
	{
		sigprocmask(SIG_SETMASK, sigmask, &oldmask);
	}

	result = do_poll(pinfo, fds, nfds, timeout);

	if (sigmask)
	{
		sigprocmask(SIG_SETMASK, &oldmask, NULL);
	}

	RtlFreeHeap(NtCurrentProcessHeap(), 0, pinfo);

	return result;
}
