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
#include <errno.h>

const char *msg_fifo_name		= "/tmp/rpi-gpio-msg";
const char *event_fifo_name	= "/tmp/rpi-gpio-event";

const int RPiPinMask				= 0x00FF;

const int RPiMsgMask				= 0x0F00;
const int RPiMsgSetOff			= 0x0000;
const int RPiMsgSetOn			= 0x0100;
const int RPiMsgQueryState		= 0x0200;
const int RPiMsgInit				= 0x0400;
const int RPiMsgStop				= 0x0800;

const int RPiEventMask			= 0xF000;
const int RPiEventError			= 0x1000;
const int RPiEventChange		= 0x2000;
const int RPiEventState			= 0x4000;
const int RPiEventUnused		= 0x8000;

const int RPiDetailMask			= 0xF0000;
const int RPiExported			= 0x10000;
const int RPiDirection			= 0x20000;
const int RPiFatalError			= 0x40000;

#define RPiPin(x)	(1<<x)

FILE *debug;

int msg_fd, event_fd;

inline void open_gpio() {
	msg_fd = open(msg_fifo_name, O_WRONLY | O_NONBLOCK);
	event_fd = open(event_fifo_name, O_RDONLY | O_NONBLOCK);
}

inline void close_gpio() {
	close(msg_fd);
	close(event_fd);
}

inline int check_event(int sec, int usec) {
fprintf(debug,"\t\t\tcheck_event\n");
	int msg;
	struct timeval tv;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(event_fd, &fds);
fprintf(debug,"\t\t\tmemset\n");
	memset(&tv, 0, sizeof(struct timeval));
	tv.tv_sec = sec;
	tv.tv_usec = usec;
fprintf(debug,"\t\t\tselect:\n");
	int ret;
	ret = select(event_fd + 1, &fds, 0, 0, &tv);
fprintf(debug,"\t\t\t\tret: %d\n",ret);
if (ret == -1) {
fprintf(debug,"\t\t\t\terr: %d\n",errno);
fprintf(debug,"\t\t\t\terr: %s\n",strerror(errno));
}
	if (FD_ISSET(event_fd, &fds)) {
fprintf(debug,"\t\t\tFD_ISSET\n");
		while (read(event_fd, &msg, sizeof(int)) > 0);
fprintf(debug,"\t\t\t\tread\n");
	}
	else msg = 0;
fprintf(debug,"\t\t\tmsg: %d\n",msg);
	return msg;
}

inline void flush_events() {
	int msg;
	while (read(event_fd, &msg, sizeof(int)) > 0);
}

inline void send_msg(int msg) {
	write(msg_fd, &msg, sizeof(int));
}

#endif /* __RPI_GPIO_H__ */
