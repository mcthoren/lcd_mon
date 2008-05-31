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

#define lcd_wait 100000

#include <sys/ioctl.h>
#include <sys/param.h>
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
int		clear_lcd(int);
int		set_backlight(int);
int		line_two(int);

volatile sig_atomic_t bail = 0;
char cmd_0 = (int)0xFE, cmd_1 = (int)0x7C, cmd_on = 0x0C, cmd_clear = 0x01;
char cmd_bl = (int)157, cmd_l2 = (int)192;

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

int clear_lcd(int fd)
{
        write(fd, &cmd_0, sizeof(cmd_0));
        write(fd, &cmd_clear, sizeof(cmd_clear));
	usleep(lcd_wait);

	return(0);
}

int display_on(int fd)
{
        write(fd, &cmd_0, sizeof(cmd_0));
        write(fd, &cmd_on, sizeof(cmd_on));
	usleep(lcd_wait);

	return(0);
}

int set_backlight(int fd)
{
        write(fd, &cmd_1, sizeof(cmd_1));
        write(fd, &cmd_bl, sizeof(cmd_bl));
	usleep(lcd_wait);

	return(0);
}

int line_two(int fd)
{
        write(fd, &cmd_0, sizeof(cmd_0));
        write(fd, &cmd_l2, sizeof(cmd_l2));
	usleep(lcd_wait);

	return(0);
}


int
main(int argc, char *argv[])
{
	int fd;
	char *dev, devicename[32], *p, hostname[MAXHOSTNAMELEN];
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

	printf("-4\n"); fflush(stdout);

	if ((fd = open(dev, O_WRONLY)) < 0)
		err(1, "open: %s", dev);

	printf("-3\n"); fflush(stdout);

/*
	if (tcgetattr(fd, &portsave) < 0)
		err(1, "tcgetattr");

	if (cfsetspeed(&port, B9600) < 0)
		err(1, "cfsetspeed");
*/

	printf("-2\n"); fflush(stdout);

	signal(SIGINT, sigf);
	signal(SIGTERM, sigf);
	signal(SIGHUP, SIG_IGN);

	printf("-1\n"); fflush(stdout);

	if (gethostname(hostname, sizeof(hostname)))
		err(1, "gethostname");
	if ((p = strchr(hostname, '.')))
		*p = '\0';

	set_backlight(fd);
	clear_lcd(fd);
	display_on(fd);

	while (bail == 0) {
		printf("wee\n"); fflush(stdout);
		clear_lcd(fd);
		write(fd, "Hello ", 6);
		usleep(lcd_wait);
		line_two(fd);	
		write(fd, "Line 2 ", 7);
		usleep(lcd_wait);
		sleep(1);

		clear_lcd(fd);
		write(fd, &hostname, p - hostname);
		sleep(1);
	}
/*
	if(tcsetattr(fd, TCSANOW, &portsave) < 0)
		err(1, "tcsetattr restore");
*/

	close(fd);
	return(0);
}
