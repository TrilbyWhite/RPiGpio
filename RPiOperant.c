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
	open_gpio();

	int msg;
	msg = check_event(1, 0);

	close_gpio();
	return 0;
}
