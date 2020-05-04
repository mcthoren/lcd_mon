/*
 * Copyright (c) 2008, 2020 Thoren McDole <thoren.mcdole@gmail.com>
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
 * XXX This code depends on DTR being looped to DCD, cuz I haven't setup
 * the serial port corectly yet :x 
 *
 * XXX I'm not saving or restoring port settings yet either :x
 */

/*
 * This code is meant to stream "useful" info to a little lcd panel
 * external to the machine. I'm using a little serial driven lcd screen
 * bolted onto a little usb uart breakout board. Links below:
 * http://www.sparkfun.com/commerce/product_info.php?products_id=461
 * http://www.sparkfun.com/commerce/product_info.php?products_id=718
 */

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sensors.h>
#include <sys/sysctl.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <termios.h>
#include <tzfile.h>
#include <unistd.h>

#define lcd_wait	100000
#define	WIDTH		16

__dead void	usage(void);
void		sigf(int);
int		clear_lcd(int);
int		display_on(int fd);
int		set_backlight(int);
int		line_two(int);
int		write_hostname(int);
int		write_time(int);
int		write_date(int);
int		write_load(int);
int		write_uptime(int);
int		write_temp(int, int, int);

volatile sig_atomic_t bail = 0;
char cmd_0 = (int) 0xFE, cmd_1 = (int) 0x7C, cmd_on = (int) 0x0C, cmd_clear = (int) 0x01;
char cmd_bl = (int) 157, cmd_l2 = (int) 192;

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

int write_hostname(int fd)
{
	char *p, hostname[MAXHOSTNAMELEN];

	if (gethostname(hostname, sizeof(hostname)))
		err(1, "gethostname");

	if ((p = strchr(hostname, '.')))
		*p = '\0';
	else if ((p = strchr(hostname, '\0')))
		*p = '\0';
	else err(1, "can't figure out hostname");

	clear_lcd(fd);
	write(fd, &hostname, p - hostname);

	return(0);
}

int write_time(int fd)
{
	int i;
	char *format, buf[WIDTH+1];
	time_t tval;

	format = "%H:%M:%S %Z";
	for(i=0;i<=WIDTH;i++) buf[i] = (int) 0x20;

	if (time(&tval) == -1) err(1, "time");
	(void)strftime(buf, sizeof(buf), format, localtime(&tval));

	write_hostname(fd);
	write(fd, " Time:", 6);
	line_two(fd);	
	write(fd, buf, 12);

	return(0);
}

int write_date(int fd)
{
	int i;
	char *format, buf[WIDTH+1];
	time_t tval;

	format = "%a %b %e %Y";
	for(i=0;i<=WIDTH;i++) buf[i] = (int) 0x20;

	if (time(&tval) == -1) err(1, "time");
	(void)strftime(buf, sizeof(buf), format, localtime(&tval));

	write_hostname(fd);
	write(fd, " Date:", 6);
	line_two(fd);	
	write(fd, buf, 15);

	return(0);
}

int write_load(int fd)
{
	int i;
	double loadav[3];
	char *p, load[WIDTH+1];

	for(i=0;i<=WIDTH;i++) load[i] = (int) 0x20;

	if (getloadavg(loadav, sizeof(loadav) / sizeof(loadav[0])) < 0)
		err(1, "getloadavg");

	snprintf(load, sizeof(load), "%.2f %.2f %.2f",
		loadav[0], loadav[1], loadav[2]);

	if ((p = strchr(load, '\0')))
		*p = ' ';

	write_hostname(fd);
	write(fd, " Load:", 6);
	line_two(fd);	
	write(fd, load, WIDTH);
	
	return(0);
}

int write_uptime(int fd)
{
	int i, mib[2];
	char *p, wtime[WIDTH+1];
	time_t uptime, now;
	int days, hrs, mins;
	struct timeval  boottime;
	size_t size;

	for(i=0;i<=WIDTH;i++) wtime[i] = (int) 0x20;

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) < 0)
		err(1, "sysctl");

	(void)time(&now);
	uptime = now - boottime.tv_sec;

	days = uptime / SECSPERDAY;
	uptime %= SECSPERDAY;
	hrs = uptime / SECSPERHOUR;
	uptime %= SECSPERHOUR;
	mins = uptime / SECSPERMIN;
	uptime %= SECSPERMIN;

	snprintf(wtime, sizeof(wtime), "%dd %02dh %02dm %02ds",
		days, hrs, mins, uptime);

	if ((p = strchr(wtime, '\0')))
		*p = ' ';

	write_hostname(fd);
	write(fd, " Uptime:", 8);
	line_two(fd);	
	write(fd, wtime, WIDTH);

	return(0);
}

int write_temp(int fd, int fam, int sens)
{
	int mib[5];
	size_t size;
	char wtemp[11], num[2];
	struct sensor	sensor;

	mib[0] = CTL_HW;
	mib[1] = HW_SENSORS;
	mib[2] = fam; /* 2,4 found by doing a ktrace of sysctl */
	mib[3] = SENSOR_TEMP; 
	mib[4] = sens; /* need to figure out how to do this properly */

	size = sizeof(sensor);
	if (sysctl(mib, 5, &sensor, &size, NULL, 0) < 0)
		err(1, "sysctl");

	snprintf(wtemp, sizeof(wtemp), "%02.2f degC", 
		(sensor.value - 273150000) / 1000000.0);

	 snprintf(num, sizeof(num), "%d", sens);

	write_hostname(fd);
	write(fd, " Temp", 5);
	write(fd, num, 1);
	write(fd, ":", 1);
	line_two(fd);   
	write(fd, wtemp, 10);

	return(0);
}


int
main(int argc, char *argv[])
{
	int fd;
	uint wait=3;
	char *dev, devicename[32];
	struct termios port;
/*	struct termios portsave; */

	bzero(&port, sizeof(port));
	srand((uint)time(0));

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

	if ((fd = open(dev, O_WRONLY)) < 0)
		err(1, "open: %s", dev);

/*
	if (tcgetattr(fd, &portsave) < 0)
		err(1, "tcgetattr");

	if (cfsetspeed(&port, B9600) < 0)
		err(1, "cfsetspeed");
*/

	signal(SIGINT, sigf);
	signal(SIGTERM, sigf);
	signal(SIGHUP, SIG_IGN);

	set_backlight(fd);
	clear_lcd(fd);
	display_on(fd);

	while (bail == 0) {
		write_time(fd);

		if(bail) break;
		sleep(wait);

		write_date(fd);

		if(bail) break;
		sleep(wait);

		write_uptime(fd);

		if(bail) break;
		sleep(wait);

		write_load(fd);

		if(bail) break;
		sleep(wait);

		write_temp(fd, 0, 0);

		if(bail) break;
		sleep(wait);

/*
		write_temp(fd, 0, 2);

		if(bail) break;
		sleep(wait);
*/

		write_temp(fd, 1, 0);

/*
		if(bail) break;
		sleep(wait);

		write_fan(fd, 1, 0);
*/

		if(bail) break;
		sleep(wait);
	}
/*
	if(tcsetattr(fd, TCSANOW, &portsave) < 0)
		err(1, "tcsetattr restore");
*/

	close(fd);
	return(0);
}
