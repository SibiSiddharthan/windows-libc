/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

const char *get_message()
{
	switch (errno)
	{
	case 0:
		return "No errno";
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case ESRCH:
		return "No such process";
	case EINTR:
		return "Interrupted function call";
	case EIO:
		return "Input/output error";
	case ENXIO:
		return "No such device or address";
	case E2BIG:
		return "Argument list too long";
	case ENOEXEC:
		return "Exec format error";
	case EBADF:
		return "Bad file descriptor";
	case ECHILD:
		return "No child processes";
	case EAGAIN:
		return "Resource temporarily unavailable";
	case ENOMEM:
		return "Out of memory";
	case EACCES:
		return "Permission denied";
	case EFAULT:
		return "Bad address";
	case EBUSY:
		return "Device or resource busy";
	case EEXIST:
		return "File exists";
	case EXDEV:
		return "Invalid cross-device link";
	case ENODEV:
		return "No such device";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EINVAL:
		return "Invalid argument";
	case ENFILE:
		return "Too many open files in system";
	case EMFILE:
		return "Too many open files";
	case ENOTTY:
		return "Not a terminal device";
	case EFBIG:
		return "File too large";
	case ENOSPC:
		return "No space left on device";
	case ESPIPE:
		return "Invalid seek";
	case EROFS:
		return "Read-only file system";
	case EMLINK:
		return "Too many links";
	case EPIPE:
		return "Broken pipe";
	case EDOM:
		return "Domain error";
	case ERANGE:
		return "Numerical result out of range";
	case EDEADLK:
		return "Resource deadlock avoided";
	case ENAMETOOLONG:
		return "Filename too long";
	case ENOLCK:
		return "No locks available";
	case ENOSYS:
		return "Function not implemented";
	case ENOTEMPTY:
		return "Directory not empty";
	case EADDRINUSE:
		return "Address in use";
	case EADDRNOTAVAIL:
		return "Address not available";
	case EAFNOSUPPORT:
		return "Address family not supported";
	case EALREADY:
		return "Connection already in progress";
	case EBADMSG:
		return "Bad message";
	case ECANCELED:
		return "Operation cancelled";
	case ECONNABORTED:
		return "Connection aborted";
	case ECONNREFUSED:
		return "Connection refused";
	case ECONNRESET:
		return "Connection reset";
	case EDESTADDRREQ:
		return "Destination address required";
	case EHOSTUNREACH:
		return "Host unreachable";
	case EIDRM:
		return "Identifier removed";
	case EINPROGRESS:
		return "Operation in progress";
	case EISCONN:
		return "Already connected";
	case ELOOP:
		return "Too many symbolic link levels";
	case EMSGSIZE:
		return "Message size";
	case ENETDOWN:
		return "Network down";
	case ENETRESET:
		return "Network reset";
	case ENETUNREACH:
		return "Network unreachable";
	case ENOBUFS:
		return "No buffer space";
	case ENODATA:
		return "No message available";
	case ENOLINK:
		return "Link has been severed";
	case ENOMSG:
		return "No message";
	case ENOPROTOOPT:
		return "No protocol option";
	case ENOSR:
		return "No stream resources";
	case ENOSTR:
		return "Not a stream";
	case ENOTCONN:
		return "Not connected";
	case ENOTRECOVERABLE:
		return "State not recoverable";
	case ENOTSOCK:
		return "Not a socket";
	case ENOTSUP:
		return "Not supported";
	case EOPNOTSUPP:
		return "Operation not supported";
	case EOTHER:
		return "Unknown error";
	case EOVERFLOW:
		return "Value too large for defined data type";
	case EOWNERDEAD:
		return "Owner dead";
	case EPROTO:
		return "Protocol error";
	case EPROTONOSUPPORT:
		return "Protocol not supported";
	case EPROTOTYPE:
		return "Wrong protocol type";
	case ETIME:
		return "Stream timeout";
	case ETIMEDOUT:
		return "Timed out";
	case ETXTBSY:
		return "Text file busy";
	case EWOULDBLOCK:
		return "Operation would block";
	default:
		return "Unknown error";
	}
}

// Write to stderr "message: <system-error based on errno>"
void wlibc_perror(char const *message)
{
	if (stderr == NULL || stderr->magic != 0x1)
	{
		errno = 22;
		return;
	}

	if (message)
	{
		int message_length = strlen(message);
		fwrite(message, 1, message_length, stderr);
	}

	fwrite(": ", 1, 2, stderr);

	const char *sys_message = get_message();
	int sys_length = strlen(sys_message);
	fwrite(sys_message, 1, sys_length, stderr);
}
