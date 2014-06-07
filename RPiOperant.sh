##---------------------------------------------------------------------##
## RpiOperant Control Script
##---------------------------------------------------------------------##
## Description: control script for mccluresk9 operant systems
## Author: Jesse McClure, copyright 2014
##   http://mccluresk9.com
## License: CC-BY-SA
##   https://creativecommons.org/licenses/by-sa/2.0/
##---------------------------------------------------------------------##


##---------------------------------------------------------------------##
## Instructions
##---------------------------------------------------------------------##
## 1) Lines starting with two '#'s like this one are comments that
##    provide instructions or information.
## 2) Lines starting with only one '#' are commands that are currently
##    commented out meaning they do not run.
## 3) Commented out commands can be activated by removing the '#' from
##    the start of the line.
## 4) Currently active commands can be deactivated by adding a '#' to
##    the start of the line.
## 5) Some lines will have a '#' after a command.  Everything after the
##    '#' is ignored by the program - these are for further comments.
##---------------------------------------------------------------------##


## ENTER MAINTENANCE MODE
## This allows ssh access via the ethernet port.  The raspberry pi
## must be plugged into an internet connected ethernet cable for this
## to work.  The number (default=15) is the number of minutes the device
## will wait for an incomming connection.  If no connection is made in
## that time, it will power down.
#maintenance_mode 15

## SET PARAMETERS
## Export variables that will be used by the operant program
export log_file=$USB/data/operant.log
export stimulus_path=$USB/songs
export data_file=$USB/data/$(date +%Y%m%d_%H%M.csv)
export session_duration=60                  # duration in minutes
export intertrial_interval=30               # duration in seconds
export interbout_interval=60                # duration in seconds
export forced_trials=6                      # number per bout
export free_trials=80                       # number per bout
export stimulus1=song1.wav                  # song for side 1
export stimulus2=song2.wav                  # song for side 2

## Ensure data file name is unique
## This should only be needed if there is no internet connection to set
## the current date and time.  If this is the case, the above default
## filename should be changed.
dfile="${data_file%.*}"
dfile_count=0
while [[ -f ${data_path}/${data_file} ]]; do
	dfile_count=$(( $dfile_count + 1 ))
	data_file="${dfile}_${dfile_count}.csv"
done
export data_file

## RUN OPERANT PROGRAM
## Execute the main operant program
$OPERANT

