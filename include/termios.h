/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_TERMIOS_H
#define WLIBC_TERMIOS_H

#include <wlibc.h>
#include <errno.h>

_WLIBC_BEGIN_DECLS

#define NCCS 20

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

struct termios
{
	tcflag_t c_iflag; // input mode flags
	tcflag_t c_oflag; // output mode flags
	tcflag_t c_cflag; // control mode flags
	tcflag_t c_lflag; // local mode flags
	speed_t c_ispeed; // input speed
	speed_t c_ospeed; // output speed
	cc_t c_cc[NCCS];  // control characters
};

// Input Flags
#define IGNBRK (1 << 0)  // Ignore break condition.
#define BRKINT (1 << 1)  // Signal interrupt on break.
#define IGNPAR (1 << 2)  // Ignore characters with parity errors.
#define PARMRK (1 << 3)  // Mark parity and framing errors.
#define INPCK  (1 << 4)  // Enable input parity check.
#define ISTRIP (1 << 5)  // Strip 8th bit off characters.
#define INLCR  (1 << 6)  // Map NL to CR on input.
#define IGNCR  (1 << 7)  // Ignore CR.
#define ICRNL  (1 << 8)  // Map CR to NL on input.
#define IXON   (1 << 9)  // Enable start/stop output control.
#define IXOFF  (1 << 10) // Enable start/stop input control.

// Output Falgs
#define OPOST  (1 << 0)  // Perform output processing.
#define ONLCR  (1 << 1)  // Map NL to CR-NL on output.
#define OXTABS TAB3      // Expand tabs to spaces.
#define ONOEOT (1 << 3)  // Discard EOT (^D) on output.
#define OCRNL  (1 << 4)  // Map CR to NL.
#define ONOCR  (1 << 5)  // Discard CR's when on column 0.
#define ONLRET (1 << 6)  // Move to column 0 on NL.
#define NLDLY  (7 << 8)  // NL delay.
#define NL0    (0 << 8)  // NL type 0.
#define NL1    (1 << 8)  // NL type 1.
#define TABDLY (7 << 10) // TAB delay.
#define TAB0   (0 << 10) // TAB delay type 0.
#define TAB1   (1 << 10) // TAB delay type 1.
#define TAB2   (2 << 10) // TAB delay type 2.
#define TAB3   (3 << 10) // Expand tabs to spaces.
#define CRDLY  (7 << 12) // CR delay.
#define CR0    (0 << 12) // CR delay type 0.
#define CR1    (1 << 12) // CR delay type 1.
#define CR2    (2 << 12) // CR delay type 2.
#define CR3    (3 << 12) // CR delay type 3.
#define FFDLY  (7 << 14) // FF delay.
#define FF0    (0 << 14) // FF delay type 0.
#define FF1    (1 << 14) // FF delay type 1.
#define BSDLY  (7 << 15) // BS delay.
#define BS0    (0 << 15) // BS delay type 0.
#define BS1    (1 << 15) // BS delay type 1.
#define VTDLY  (7 << 16) // VT delay.
#define VT0    (0 << 16) // VT delay type 0.
#define VT1    (1 << 16) // VT delay type 1.

// Control Flags
#define CIGNORE (1 << 0)                // Ignore these control flags.
#define CSIZE   (CS5 | CS6 | CS7 | CS8) // Number of bits per byte (mask).
#define CS5     0                       // 5 bits per byte.
#define CS6     (1 << 8)                // 6 bits per byte.
#define CS7     (1 << 9)                // 7 bits per byte.
#define CS8     (CS6 | CS7)             // 8 bits per byte.
#define CSTOPB  (1 << 10)               // Two stop bits instead of one.
#define CREAD   (1 << 11)               // Enable receiver.
#define PARENB  (1 << 12)               // Parity enable.
#define PARODD  (1 << 13)               // Odd parity instead of even.
#define HUPCL   (1 << 14)               // Hang up on last close.
#define CLOCAL  (1 << 15)               // Ignore modem status lines.

// Local Flags
#define ECHOKE (1 << 0)  // Visual erase for KILL.
#define ECHOE  (1 << 1)  // Visual erase for ERASE.
#define ECHOK  (1 << 2)  // Echo NL after KILL.
#define ECHO   (1 << 3)  // Enable echo.
#define ECHONL (1 << 4)  // Echo NL even if ECHO is off.
#define ISIG   (1 << 7)  // Enable signals.
#define ICANON (1 << 8)  // Do erase and kill processing.
#define IEXTEN (1 << 10) // Enable extended input processing.
#define TOSTOP (1 << 11) // Send SIGTTOU for background output.
#define NOFLSH (1 << 12) // Disable flush after interrupt.

// Control characters
#define VEOF     0  // End-of-file character [ICANON].
#define VEOL     1  // End-of-line character [ICANON].
#define VEOL2    2  // Second EOL character [ICANON].
#define VERASE   3  // Erase character [ICANON].
#define VWERASE  4  // Word-erase character [ICANON].
#define VKILL    5  // Kill-line character [ICANON].
#define VREPRINT 6  // Reprint-line character [ICANON].
#define VINTR    8  // Interrupt character [ISIG].
#define VQUIT    9  // Quit character [ISIG].
#define VSUSP    10 // Suspend character [ISIG].
#define VDSUSP   11 // Delayed suspend character [ISIG].
#define VSTART   12 // Start (X-ON) character [IXON, IXOFF].
#define VSTOP    13 // Stop (X-OFF) character [IXON, IXOFF].
#define VLNEXT   14 // Literal-next character [IEXTEN].
#define VDISCARD 15 // Discard character [IEXTEN].
#define VMIN     16 // Minimum number of bytes read at once [!ICANON].
#define VTIME    17 // Time-out value (tenths of a second) [!ICANON].
#define VSTATUS  18 // Status character [ICANON].

// Baud Rates
#define B0     0
#define B50    50
#define B75    75
#define B110   110
#define B134   134
#define B150   150
#define B200   200
#define B300   300
#define B600   600
#define B1200  1200
#define B1800  1800
#define B2400  2400
#define B4800  4800
#define B9600  9600
#define B7200  7200
#define B14400 14400
#define B19200 19200
#define B28800 28800
#define B38400 38400

// tcsetattr() actions
#define TCSANOW   0 // Only supported
#define TCSADRAIN 1
#define TCSAFLUSH 2

// tcflow() actions
#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3

// tcflush() queues
#define TCIFLUSH  0
#define TCOFLUSH  1
#define TCIOFLUSH 2

WLIBC_API int wlibc_tcgetattr(int fd, struct termios *termio);
WLIBC_API int wlibc_tcsetattr(int fd, int action, const struct termios *termio);

WLIBC_INLINE int tcgetattr(int fd, struct termios *termio)
{
	return wlibc_tcgetattr(fd, termio);
}

WLIBC_INLINE int tcsetattr(int fd, int action, const struct termios *termio)
{
	return wlibc_tcsetattr(fd, action, termio);
}

WLIBC_API int wlibc_tcsendbreak(int fd, int duration);
WLIBC_API int wlibc_tcdrain(int fd);
WLIBC_API int wlibc_tcflow(int fd, int action);
WLIBC_API int wlibc_tcflush(int fd, int queue);

WLIBC_INLINE int tcsendbreak(int fd, int duration)
{
	return wlibc_tcsendbreak(fd, duration);
}

WLIBC_INLINE int tcdrain(int fd)
{
	return wlibc_tcdrain(fd);
}

WLIBC_INLINE int tcflow(int fd, int action)
{
	return wlibc_tcflow(fd, action);
}

WLIBC_INLINE int tcflush(int fd, int queue)
{
	return wlibc_tcflush(fd, queue);
}

// Input Baud Rate
WLIBC_INLINE speed_t cfgetispeed(const struct termios *termio)
{
	if (termio == NULL)
	{
		errno = EINVAL;
		return -1u;
	}

	return termio->c_ispeed;
}

WLIBC_INLINE int cfsetispeed(struct termios *termio, speed_t speed)
{
	if (termio == NULL)
	{
		errno = EINVAL;
		return -1u;
	}

	termio->c_ispeed = speed;
	return 0;
}

// Output Baud Rate
WLIBC_INLINE speed_t cfgetospeed(const struct termios *termio)
{
	if (termio == NULL)
	{
		errno = EINVAL;
		return -1u;
	}

	return termio->c_ospeed;
}

WLIBC_INLINE int cfsetospeed(struct termios *termio, speed_t speed)
{
	if (termio == NULL)
	{
		errno = EINVAL;
		return -1u;
	}

	termio->c_ospeed = speed;
	return 0;
}

_WLIBC_END_DECLS

#endif
