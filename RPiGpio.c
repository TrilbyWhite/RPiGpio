/**********************************************************************\
* RPIGPIO.C - Gpio access for Raspberry Pi
*
* Author: Jesse McClure, copyright 2014
* License: GPLv3
*
*    This program is free software: you can redistribute it and/or
*    modify it under the terms of the GNU General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see
*    <http://www.gnu.org/licenses/>.
*
\**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "RPiGpio.h"

#define GPIO_PATH_LEN	64
#define GPIO_BASE			"/sys/class/gpio/"

static uint8_t export=0;
static uint8_t output=0;

static uint8_t gpio_running=0;
static char path[GPIO_PATH_LEN];
static int sigterm = 0;
static const char *off="0";
static const char *on="1";
static const char *bcm_num[8] = {
	"17", "18", "21", "22", "23", "24", "25", "4" };


void clean_exit(int msg) {
	int pin;
	FILE *gpio_fp;
	/* unexport all pins */
	gpio_fp=fopen(GPIO_BASE "unexport", "w");
	for (pin = 0; pin < 8; pin++) fprintf(gpio_fp,bcm_num[pin]);
	/* unlink the fifos */
	close(event_fd); close(msg_fd);
	unlink(event_fifo_name);
	unlink(msg_fifo_name);
	exit(msg);
}

void send(int msg) {
	write(event_fd, &msg, sizeof(int));
}

void gpio_error(int msg) {
	send(RPiFatalError | msg);
	clean_exit(msg);
}

int gpio_init(int msg) {
	int pin, ret, dir_err = 0;
	FILE *gpio_fp, *fp;
	if (!(gpio_fp=fopen(GPIO_BASE "export", "w")))
		gpio_error(RPiEventError | msg);
	for (pin = 0; pin < 8; pin++) {
		/* export pin number */
		if (!(fprintf(gpio_fp,bcm_num[pin]) > 0)) continue;
		export |= (1<<pin);
		/* open direction file */
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/direction",
				bcm_num[pin]);
		if (!(fp=fopen(path, "w"))) {
			dir_err |= (1<<pin);
			continue;
		}
		/* set direction */
		if (msg & (1<<pin)) ret = fprintf(fp,"out");
		else ret = fprintf(fp,"in");
		if (ret < 0) dir_err |= (1<<pin);
		fclose(fp);
	}
	fclose(gpio_fp);
	if (export != 0xFF) send(RPiEventError | RPiMsgInit | export);
	if (dir_err) gpio_error(RPiEventError | RPiDirection | dir_err);
	return (output = (msg & export));
}

int gpio_end(int msg) {
	int pin;
	FILE *gpio_fp;
	if (!(gpio_fp=fopen(GPIO_BASE "unexport", "w")))
		gpio_error(RPiEventError | msg);
	for (pin = 0; pin < 8; pin++) {
		/* unexport pin number */
		if (!(fprintf(gpio_fp,bcm_num[pin]) > 0)) continue;
		export &= ~(1<<pin);
	}
	if (export) gpio_error(RPiEventError | RPiMsgStop | export);
	return export;
}

int gpio_get(int msg) {
	int pin, get, ret = 0;
	FILE *gpio_fp;
	for (pin = 0; pin < 8; pin++) {
		if (!(pin & msg)) continue;
		if ((pin & output)) {
			if (msg & RPiMsgMask)
				send(RPiEventError | RPiDirection |
						RPiMsgQueryState | pin);
			continue;
		}
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/value",
				bcm_num[pin]);
		if (!(gpio_fp=fopen(path, "r"))) {
			if (msg & RPiMsgMask)
				send(RPiEventError | (RPiMsgMask & msg) | pin);
			continue;
		}
		if (fscanf(gpio_fp, "%d", &get) == 1)
			ret |= ((get > 0 ? 1 : 0)<<pin);
		else if (msg & RPiMsgMask)
			send(RPiEventError | (RPiMsgMask & msg) | pin);
		fclose(gpio_fp);
	}
	if (RPiMsgMask & msg) send(RPiEventState | (msg & ret));
	return (ret & RPiPinMask);
}

void gpio_set(int msg) {
	int pin;
	FILE *gpio_fp;
	for (pin = 0; pin < 8; pin++) {
		if (!(pin & msg)) continue;
		if (!(pin & output)) {
			send(RPiEventError | RPiDirection | (RPiMsgMask & msg) | pin);
			continue;
		}
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/value",
				bcm_num[pin]);
		if (!(gpio_fp=fopen(path, "w"))) {
			send(RPiEventError | (RPiMsgMask & msg) | pin);
			continue;
		}
		if (fprintf(gpio_fp, "%d", (RPiMsgSetOn | msg ? 1 : 0)) < 0)
			send(RPiEventError | (RPiMsgMask & msg) | pin);
		fclose(gpio_fp);
	}
}

void signal_handler(int sig) {
	clean_exit(0);
}

int main(int argc, const char **argv) {
	/* create two fifos and open */
	mkfifo(msg_fifo_name, 0666);
	mkfifo(event_fifo_name, 0666);
	msg_fd = open(msg_fifo_name, O_RDONLY);
	event_fd = open(event_fifo_name, O_WRONLY);
	/* register signal handler */
	signal(SIGTERM, &signal_handler);
	/* loop until sigterm at shutdown */
	while (sigterm == 0) {
		/* wait for init msg */
		int msg = 0, pin;
		struct timeval tv;
		fd_set fds;
		uint8_t previous=0, current;
		while (!(msg & RPiMsgInit))
			read(msg_fd, &msg, sizeof(int));
		gpio_init(msg);
		/* loop until stop msg */
		current = gpio_get(RPiPinMask);
		while (!(msg & RPiMsgStop)) {
			FD_ZERO(&fds);
			FD_SET(msg_fd, &fds);
			memset(&tv, 0, sizeof(struct timeval));
			tv.tv_usec = 500;
			select(msg_fd + 1, &fds, 0, 0, &tv);
			if (FD_ISSET(msg_fd, &fds)) {
				read(msg_fd, &msg, sizeof(int));
				if (msg & RPiMsgStop) break;
				else if (msg & RPiMsgQueryState) gpio_get(msg);
				else gpio_set(msg);
			}
			previous = current;
			if ( (current=gpio_get(RPiPinMask)) != previous )
				send(RPiEventChange | (current ^ previous));
		}
		gpio_end(msg);
	}
	/* double check that we've unexported everything */
	clean_exit(0);
	return 0;
}

