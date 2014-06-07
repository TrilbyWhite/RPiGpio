#!/bin/bash
## RPIOPERANT.BASHRC
## Description: controls RPi in auto-login configurations
## Author: Jesse McClure, copyright 2014
##   http://mccluresk9.com
## License: CC-BY-SA
##   https://creativecommons.org/licenses/by-sa/2.0/
##---------------------------------------------------------------------##


ID=$(whoami)
USB=/home/$ID/usb
MIN=15
OPERANT=RPiOperant
OPSCRIPT=$USB/RPiOperant.sh
ETHERNET=0

maintenance_mode() {
	## If connected, transmit IP address and wait for ssh access
	[[ $ETHERNET -eq 0 ]] && return
	url="http://mccluresk9.com/track_me.html?name=$ID"
	wget -o /tmp/wget.log -O /tmp/wget.out $url
	## Wait $1 minutes for the removal of shutdown.key
	touch /tmp/shutdown.key
	loops=0
	while [[ $loops -lt $1 ]]; do
		sleep 60
		loops=$(( loops + 1 ))
	done
	## If shutdown key remains, shutdown
	[[ -f /tmp/shutdown.key ]] && shutdown -h now
}

## Check if time is going to sync - if so, wait for it
loops=0
while [[ $loops -lt 5 ]]; do
	ping -c 1 google.com && break
	sleep 4
	loops=$(( loops + 1 ))
done
[[ $loops -lt 5 ]] && ETHERNET=1
if [[ $ETHERNET -eq 1 ]]; then
	while [[ $(date +%s) -lt 600 ]]; do sleep 1; done
fi

## Check for usb with operant.sh file
## If no proper usb is present, enter maintainence mode
[[ -f $OPSCRIPT ]] && source $OPSCRIPT || maintenance_mode $MIN

shutdown -h now

