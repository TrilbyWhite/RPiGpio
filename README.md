# RPiGpio

**RPiGpio** - *Simplifies raspberry pi gpio access*

**RPiOperant** - *Example client program demonstrating capabilities*

Author: Jesse McClure, Copyright 2014
License: GPLv3 / CC-BY-SA

## Using RPioGpio

The RPiGpio program must be run as root - this can be accomplished by
launching from init at boot.  A systemd service file is included for
distros which use systemd (e.g., ArchlinuxARM).

RPiGpio creates to user-accessible fifo pipes in /tmp, one for send
messages to RPiGpio to change pin settings (change outputs) and the
other for receiving events from RPiGpio (inputs).

Interaction with these fifos can be handled by using the constants and
inline functions defined in RPiGpio.h (for compiled client programs) or
RPiGPio.sh (for shell script clients - this is not yet tested and may
not currently work as RPiGpio requires a process have the event fifo
open for reading before it proceeds).  RPiOperant provides an example of
a compiled client for song-preference test for songbirds using
RPiGpio.c.

The constants, macros, and functions available are as follows:

<dl>
<dt>RPiPinMask</dt><dd>
	bitwise *and* with a message to get a pin fit-field that the message
	applies to.
</dd><dt>RPiMsgMask</dt><dd>
	bitwise *and* with a message to get the message type.  Note that this
	is not likely of use for client programs.
</dd><dt>RPiMsgSetOff</dt><dd>
	bitwise *or* with your message to turn off outputs
</dd><dt>RPiMsgSetOn</dt><dd>
	bitwise *or* with your message to turn on outputs
</dd><dt>RPiMsgQueryState</dt><dd>
	bitwise *or* with your message to query the state of all pins
</dd><dt>RPiMsgInit</dt><dd>
	must be the first message sent when a client program starts.  Bitwise
	*or* any pin numbers that are to be used as inputs, otherwise all
	pins default to being outputs.
</dd><dt>RPiMsgStop</dt><dd>
	send this message before shutting down a client program.
</dd></dl>


NOTE: descriptions for the following coming soon

RPiEventMask
RPiEventError
RPiEventChange
RPiEventState
RPiEventUnused

RPiDetailMask
RPiExported
RPiDirection
RPiFatalError

RPiPin(x)




open_gpio();
void close_gpio();

int check_event(int sec, int usec);
flush_events();

void send_msg(int msg);

## Client programs

Client programs should do the following:

1. call *open_gpio* (compiled clients only)
1. call *send_msg* passing a RPiMsgInit message bitwise-or'ed with an
	RPiPin(pin) for every pin to be used as an input.
1. The following may be used as needed:
	- Send message:
		* RPiMsgStateOn/Off bitwise-or'ed with any RPiPin(num) you wish to
		  turn on/off.  Multiple output pins can be specified.
		* RPiMsgQueryState to trigger a response with the state of ever
		  pin.
	- Handle events:
		* call *check_event* with a limit of seconds and/or useconds
		  which will be used as the timeout value for a select() read.
		  The return value will be an integer bitfield specifying an
		  event or error message.
		* call *flush_events* to discard any messages currently in the
		  queue. (not implemented in RPiGpio.sh yet)
1. call *send_msg* with a RPiMsgStop message
1. call *close_gpio* (compiled clients only)


