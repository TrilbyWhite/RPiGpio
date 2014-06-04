/**********************************************************************\
* RPIOPERANT.C
* By: Jesse McClure (c) 2014
* License: GPLv3
* See COPYING for license information
\**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "RPiGpio.h"

int main(int argc, const char **argv) {
int i, msg, pmsg;
	open_gpio();
	send_msg(RPiMsgInit | RPiPin(0));

for (i = 0; i < 5; i++) {
	send_msg(RPiMsgSetOn | RPiPin(7));
	while (!(msg=check_event(1,0)));
	while (!(msg=check_event(1,0)));
	send_msg(RPiMsgSetOff | RPiPin(7));
	while (!(msg=check_event(1,0)));
	while (!(msg=check_event(1,0)));
}
	send_msg(RPiMsgStop);
	sleep(1);
	close_gpio();
	return 0;
}
