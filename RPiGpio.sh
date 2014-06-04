## RPI-GPIO.SH
## Part of RpiGpio
## Author: Jesse McClure, copyright 2014
##   http://mccluresk9.com
## License: CC-BY-SA
##   https://creativecommons.org/licenses/by-sa/2.0/
##
## Source this file in your bash operant programs

msg_fifo_name=/tmp/rpi-gpio/msg
event_fifo_name=/tmp/rpi-gpio/event

RPiPinMask=$((16#000F))

RPiMsgMask=$((16#00F0))
RPiMsgSetOff=$((16#0000))
RPiMsgSetOn=$((16#0010))
RPiMsgQueryState=$((16#0020))
RPiMsgInit=$((16#0040))
RPiMsgStop=$((16#0080))

RPiEventMask=$((16#0F00))
RPiEventError=$((16#0100))
RPiEventChange=$((16#0200))
RPiEventState=$((16#0400))
RPiEventUnused=$((16#0800))

RPiDetailMask=$((16#F000))
RPiExported=$((16#1000))
RPiDirection=$((16#2000))
RPiFatalError=$((16#4000))
RPiUnused=$((16#8000))

check_event() {
	[[ -n "$1" ]] && cmd="read -t $1" || cmd="read"
	$cmd msg < $event_fifo_name
	[[ $? -ne 0 ]] && msg=0
	echo $msg
}

send_msg() {
	[[ "$1" -eq 0 ]] && return
	[[ $1 -gt 255 ]] && return
	echo $1 > $msg_fifo_name
}

