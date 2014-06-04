/**********************************************************************\
* RPIGPIO.H
* By: Jesse McClure (c) 2014
* License: GPLv3
* See COPYING for license information
\**********************************************************************/

#ifndef __RPI_GPIO_H__
#define __RPI_GPIO_H__

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

const char *msg_fifo_name		= "/tmp/rpi-gpio/msg";
const char *event_fifo_name	= "/tmp/rpi-gpio/event";

const int RPiPinMask				= 0x000F;

const int RPiMsgMask				= 0x00F0;
const int RPiMsgSetOff			= 0x0000;
const int RPiMsgSetOn			= 0x0010;
const int RPiMsgQueryState		= 0x0020;
const int RPiMsgInit				= 0x0040;
const int RPiMsgStop				= 0x0080;

const int RPiEventMask			= 0x0F00;
const int RPiEventError			= 0x0100;
const int RPiEventChange		= 0x0200;
const int RPiEventState			= 0x0400;
const int RPiEventUnused		= 0x0800;

const int RPiDetailMask			= 0xF000;
const int RPiExported			= 0x1000;
const int RPiDirection			= 0x2000;
const int RPiFatalError			= 0x4000;
const int RPiUnused				= 0x8000;

int msg_fd, event_fd;

inline void open_gpio() {
	msg_fd = open(msg_fifo_name, O_WRONLY);
	event_fd = open(event_fifo_name, O_RDONLY);
}

inline void close_gpio() {
	close(msg_fd);
	close(event_fd);
}

inline int check_event(int sec, int usec) {
	int msg;
	struct timeval tv;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(event_fd, &fds);
	memset(&tv, 0, sizeof(struct timeval));
	tv.tv_sec = sec;
	tv.tv_usec = usec;
	select(msg_fd + 1, &fds, 0, 0, &tv);
	if (FD_ISSET(event_fd, &fds))
		read(event_fd, &msg, sizeof(int));
	else msg = 0;
	return msg;
}

inline void send_msg(int msg) {
	write(msg_fd, &msg, sizeof(int));
}

#endif /* __RPI_GPIO_H__ */
