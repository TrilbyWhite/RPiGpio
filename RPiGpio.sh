## RPI-GPIO.SH
## Part of RpiGpio
## Author: Jesse McClure, copyright 2014
##   http://mccluresk9.com
## License: CC-BY-SA
##   https://creativecommons.org/licenses/by-sa/2.0/
##
## Source this file in your bash operant programs

msg_fifo_name=/tmp/rpi-gpio-msg
event_fifo_name=/tmp/rpi-gpio-event

RPiPinMask=$((16#00FF))

RPiMsgMask=$((16#0F00))
RPiMsgSetOff=$((16#0000))
RPiMsgSetOn=$((16#0100))
RPiMsgQueryState=$((16#0200))
RPiMsgInit=$((16#0400))
RPiMsgStop=$((16#0800))

RPiEventMask=$((16#F000))
RPiEventError=$((16#1000))
RPiEventChange=$((16#2000))
RPiEventState=$((16#4000))
RPiEventUnused=$((16#8000))

RPiDetailMask=$((16#F0000))
RPiExported=$((16#10000))
RPiDirection=$((16#20000))
RPiFatalError=$((16#40000))
RPiUnused=$((16#80000))

RPiPin() {
	bits=${1:-0}
	echo $(( 1 << $bits ))
}

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

