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
static uint8_t input=0;

static uint8_t gpio_running=0;
static char path[GPIO_PATH_LEN];
static const char *off="0";
static const char *on="1";
static const char *bcm_num[8] = {
	"17", "18", "21", "22", "23", "24", "25", "4" };


void clean_exit(int msg) {
	int i, gpio_fd;
	/* unexport all pins */
	gpio_fd=open(GPIO_BASE "unexport", O_WRONLY);
	for (i = 0; i < 8; i++)
		write(gpio_fd,bcm_num[i], sizeof(bcm_num[i]));
	close(gpio_fd);
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
	int i, pin, ret, dir_err = 0, gpio_fd;
	if (!(gpio_fd=open(GPIO_BASE "export", O_WRONLY)))
		gpio_error(RPiEventError | msg);
	/* export pins */
	for (i = 0; i < 8; i++) {
		/* export pin number */
		pin = (1<<i);
		if (!(write(gpio_fd,bcm_num[i],sizeof(bcm_num[i])) > 0)) continue;
		export |= pin;
	}
	close(gpio_fd);
	if (export != RPiPinMask) send(RPiEventError | RPiMsgInit | export);
	/* set pin directions */
	for (i = 0; i < 8; i++) {
		pin = (1<<i);
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/direction",
				bcm_num[i]);
		if (!(gpio_fd=open(path, O_WRONLY))) {
			dir_err |= pin;
			continue;
		}
		/* set direction */
		if (msg & pin) ret = write(gpio_fd,"in\n", sizeof("out\n"));
		else ret = write(gpio_fd,"out\n", sizeof("out\n"));
		if (ret < 0) dir_err |= pin;
		close(gpio_fd);
	}
	if (dir_err) gpio_error(RPiEventError | RPiDirection | dir_err);
	return (input = (msg & export));
}

int gpio_end(int msg) {
	int i, pin, gpio_fd;
	if (!(gpio_fd=open(GPIO_BASE "unexport", O_WRONLY)))
		gpio_error(RPiEventError | msg);
	for (i = 0; i < 8; i++) {
		/* unexport pin number */
		pin = (1<<i);
		if (!(write(gpio_fd,bcm_num[i],sizeof(bcm_num[i])) > 0)) continue;
		export &= ~pin;
	}
	close(gpio_fd);
	if (export) gpio_error(RPiEventError | RPiMsgStop | export);
	return export;
}

int gpio_get(int msg) {
	int i, pin, get, ret = 0, gpio_fd;
	for (i = 0; i < 8; i++) {
		pin = (1<<i);
		if (!(pin & msg)) continue;
		if (!(pin & input)) {
			if (msg & RPiMsgMask)
				send(RPiEventError | RPiDirection |
						RPiMsgQueryState | pin);
			continue;
		}
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/value",
				bcm_num[i]);
		if (!(gpio_fd=open(path, O_RDONLY))) {
			if (msg & RPiMsgMask)
				send(RPiEventError | (RPiMsgMask & msg) | pin);
			continue;
		}
		char cget = '0';
		if (read(gpio_fd, &cget, sizeof(char)) == sizeof(char))
			ret |= ((cget == '1' ? 1 : 0)<<pin);
		else if (msg & RPiMsgMask)
			send(RPiEventError | (RPiMsgMask & msg) | pin);
		close(gpio_fd);
	}
	if (RPiMsgMask & msg) send(RPiEventState | (msg & ret));
	return (ret & RPiPinMask);
}

void gpio_set(int msg) {
	int i, pin, gpio_fd;
	char *on="1\n", *off="0\n", *str=NULL;
	for (i = 0; i < 8; i++) {
		pin = (1<<i);
		if (!(pin & msg)) continue;
		if (pin & input) {
			send(RPiEventError | RPiDirection | (RPiMsgMask & msg) | pin);
			continue;
		}
		snprintf(path, GPIO_PATH_LEN, GPIO_BASE "gpio%s/value",
				bcm_num[i]);
		if (!(gpio_fd=open(path, O_WRONLY))) {
			send(RPiEventError | (RPiMsgMask & msg) | pin);
			continue;
		}
		str = (RPiMsgSetOn & msg ? on : off);
		if (write(gpio_fd, str, strlen(str)) < 0)
			send(RPiEventError | (RPiMsgMask & msg) | pin);
		close(gpio_fd);
	}
}

void signal_handler(int sig) {
	clean_exit(0);
}

int main(int argc, const char **argv) {
	/* create two fifos and open */
	mkfifo(msg_fifo_name, 0666);
	chmod(msg_fifo_name, 0666);
	mkfifo(event_fifo_name, 0666);
	chmod(event_fifo_name, 0666);
	/* register signal handler */
	signal(SIGTERM, &signal_handler);
	signal(SIGINT, &signal_handler);
	/* loop until sigterm at shutdown */
	msg_fd = open(msg_fifo_name, O_RDONLY | O_NONBLOCK);
	for (;;) {
fprintf(stderr,"[RPiGpio] starting session\n");
		/* wait for init msg */
		int msg = 0, pin;
		struct timeval tv;
		fd_set fds;
		uint8_t previous=0, current;
fprintf(stderr,"[RPiGpio] waiting for init\n");
		while (!(msg & RPiMsgInit))
			read(msg_fd, &msg, sizeof(int));
		event_fd = open(event_fifo_name, O_WRONLY | O_NONBLOCK);
fprintf(stderr,"[RPiGPio] recieved init\n");
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
		close(event_fd); event_fd = 0;
fprintf(stderr,"[RPiGpio] ending session\n");
fprintf(stderr,"----------------------------------\n");
		gpio_end(msg);
	}
	/* double check that we've unexported everything */
	clean_exit(0);
	return 0;
}

