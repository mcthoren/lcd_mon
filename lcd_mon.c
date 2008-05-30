/* $Header$ */

/*
 * Copyright (c) 2008 Thoren McDole <thoren.mcdole@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Much of this code is from OpenBSD programs and manpages. 
 * 
 */

#include <sys/ioctl.h>
#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

__dead void	usage(void);
void		sigf(int);

volatile sig_atomic_t bail = 0;

__dead void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s device\n", __progname);
	exit(1);
}

/* ARGSUSED */
void
sigf(int useless)
{
	bail = 1;
}

int
main(int argc, char *argv[])
{
	int fd;
	char *dev, devicename[32];
	struct termios port, portsave;

	bzero(&port, sizeof(port));

	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();


	dev = *argv;
	if (strncmp(_PATH_DEV, dev, sizeof(_PATH_DEV) - 1)) {
		(void)snprintf(devicename, sizeof(devicename),
			"%s%s", _PATH_DEV, dev);
		dev = devicename;
	}

	if ((fd = open(dev, O_WR)) < 0)
		err(1, "open: %s", dev);

	if (tcgetattr(fd, &portsave) < 0)
		err(1, "tcgetattr");

	if (cfsetspeed(&port, B9600) < 0)
		err(1, "cfsetspeed");

	signal(SIGINT, sigf);
	signal(SIGTERM, sigf);
	signal(SIGHUP, SIG_IGN);

	while (bail == 0) {
	
	}

	if(tcsetattr(fd, TCSANOW, &portsave) < 0)
		err(1, "tcsetattr restore");

	close(fd);
	return(0);
}
