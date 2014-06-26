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
PLAYBACK=RPiPlayback
ETHERNET=0

maintenance_mode() {
	[[ $ETHERNET -eq 0 ]] && return
	touch /tmp/shutdown.key
	loops=0
	while [[ $loops -lt $1 ]]; do
		sleep 60
		loops=$(( loops + 1 ))
	done
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
if [[ -d $USB/scripts ]]; then
	for f in $USB/scripts/*-operant.sh; do
		sudo systemctl start RPiGpio
		tr -d '\15\32' < $f > /tmp/script
		source /tmp/script
		sync
		sudo systemctl stop RPiGpio
		sleep 1
	done
else
	maintenance_mode $MIN
fi

sudo systemctl poweroff

