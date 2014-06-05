/**********************************************************************\
* RPIOPERANT.C
* By: Jesse McClure (c) 2014
* License: GPLv3
* See COPYING for license information
\**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "RPiGpio.h"

extern char **environ;


/***************************************\
|* FUNCTION PROTOTYPES
\***************************************/
static uint64_t bit_shuffle(int);
static int config();
static int die(const char *);
static int log_data(int, int, time_t, int);
static int logs_open();
static int logs_close();
static int play_song(int);
static int run_forced_trials(int);
static int run_free_trials(int);
static int time_check();


/***************************************\
|* GLOBAL DATA
\***************************************/
static const char *log_fname = "/home/operant/operant.log";
static const char *hbar =
"------------------------------------------------------------------\n";
static const char *hbar2 =
"==================================================================\n\n";
static const char *song_path, *data_path, *data_fname, *song[2];
static char song_name[2][64], full_path[256];
static int session_min = 60, intertrial_sec = 5, interbout_sec = 60,
		forced_trials = 6, free_trials = 80;
static time_t start_time, now;
static FILE *log_file, *data_file;


/***************************************\
|* MAIN ENTRANCE POINT
\***************************************/

int main(int argc, const char **argv) {
	/* read variables and init gpio */
	config();
	open_gpio();
	send_msg(RPiMsgInit | RPiPin(0) | RPiPin(1));
	/* set up data files: */
	start_time = time(NULL);
	logs_open();
	/* main loop: */
	time_t time_stamp;
	int bout;
	for (bout = 0; time_check(); bout++) {
		/* run trials for bout */
		run_forced_trials(bout);
		run_free_trials(bout);
		/* pause for interbout interval & keep event cache flushed */
		time_stamp = time(NULL);
		while (time_check() && now < time_stamp + interbout_sec) {
			flush_events();
			sleep(1);
		}
	}
	/* clean up and exit: */
	logs_close();
	send_msg(RPiMsgStop);
	sleep(1);
	close_gpio();
	return 0;
}


/***************************************\
|* FUNCTION DEFINTIONS
\***************************************/

uint64_t bit_shuffle(int n) {
	srandom(time(NULL));
	uint32_t r = random() * 0xFFFFFFFF;
	return ((r<<(n/2)) | ((~r) & ~(0xFFFFFFFF<<(n/2))));
}

int config() {
	/* get string variables */
	song_path = getenv("stimuls_path");
	data_path = getenv("data_path");
	data_fname = getenv("data_file");
	song[0] = getenv("stimulus1");
	song[1] = getenv("stimulus2");
	/* get numeric variables */
	const char *str;
	if ((str=getenv("session_duration"))) session_min = atoi(str);
	if ((str=getenv("intertrial_interval"))) intertrial_sec = atoi(str);
	if ((str=getenv("interbout_interval"))) interbout_sec = atoi(str);
	if ((str=getenv("forced_trials"))) forced_trials = atoi(str);
	if ((str=getenv("free_trials"))) free_trials = atoi(str);
	if (forced_trials > 64) forced_trials = 64;
	/* create song name strings */
	strncpy(song_name[0], song[0], 64);
	strncpy(song_name[1], song[1], 64);
	char *c = strrchr(song_name[0], '.');
	if (c) *c = '\0';
	c = strrchr(song_name[1], '.');
	if (c) *c = '\0';
	return 0;
}

int die(const char *str) {
	/* print message */
	fprintf(stderr,"[RPiOperant] %s\n", str);
	/* ensure next time_check triggers a shutdown */
	start_time = now - session_min * 60 - 1;
	return 0;
}

int log_data(int bout, int trial, time_t when, int side) {
	/* log response */
	fprintf(log_file, "[%d] bout=%d, trial=%d, side=%d\n",
			when, bout, trial, side);
	if (bout == -1) return;
	/* write to data file */
	fprintf(log_file, "%d,%d,%d,%s\n",when, bout, trial, song_name[side]);
}

int logs_open() {
	if (!(log_file=fopen(log_fname,"a")))
		die("unable to open log file");
	if (!(data_file=fopen(data_fname,"w")))
		die("unable to open data file");
	/* print headers: */
	if (data_file) {
		fprintf(data_file,"Time (s), Bout, Trial, Song\n");
	}
	if (log_file) {
		fprintf(log_file, hbar2);
		fprintf(log_file, "START SESSION:\n");
		fprintf(log_file, "\ttime=%d (%s)\n",start_time,ctime(&start_time));
		char **var;
		for (var = environ; var; var++)
			fprintf(log_file, "\t%s\n", *var);
		fprintf(log_file, hbar);
	}
	return 0;
}

int logs_close() {
	fclose(data_file);
	if (log_file) {
		time_check();
		fprintf(log_file, hbar);
		fprintf(log_file, "END SESSION: session duration=%d\n",
				now - start_time);
		fprintf(log_file, hbar2);
		fprintf(log_file, "\n\n");
	}
	fclose(log_file);
}

int play_song(int n) {
	// TODO Block while playing? If not, use fork()/execl instead.
	snprintf(full_path, 256, "/usr/bin/play play -q %s/%s",
			song_path, song[n]);
	system(full_path);
//	if (!fork()==0) {
//		snprintf(full_path, 256, "%s/%s", song_path, song[n]);
//		// TODO close file descriptors?
//		execl("/usr/bin/play", "play", "-q", full_path);
//		perror("Operant Song Playback");
//		exit(1);
//	}
}

int run_forced_trials(int bout) {
	int n, msg, side;
	uint64_t forced_side = bit_shuffle(forced_trials);
	time_t time_stamp;
	for (n = 0; time_check() && n < forced_trials; n++) {
		side = ((forced_side<<n) & 0x01);
		flush_events();
		/* turn on stimulus light */
		send_msg(RPiMsgSetOn | RPiPin(side+2));
		/* wait for trigger onset */
		while (time_check() &&
				(	(msg=check_event(1,0)) &
					(RPiEventState | RPiPin(side)) ) );
		/* turn off light + play song */
		time_stamp = time(NULL);
		send_msg(RPiMsgSetOff | RPiPin(side+2));
		log_data(-1, n, time_stamp - start_time, side);
		play_song(side);
	}
	return 0;
}

int run_free_trials(int bout) {
	int n, msg, side;
	time_t time_stamp;
	for (n = 0; time_check() && n < forced_trials; n++) {
		flush_events();
		/* turn on stimulus lights */
		send_msg(RPiMsgSetOn | RPiPin(2) | RPiPin(3));
		/* wait for trigger onset */
		while (time_check() &&
				(	(msg=check_event(1,0)) &
					(RPiEventState | RPiPin(0) | RPiPin(1)) ) );
		/* turn off light + play song */
		time_stamp = time(NULL);
		send_msg(RPiMsgSetOff | RPiPin(2) | RPiPin(3));
		log_data(bout, n, time_stamp - start_time, side);
		play_song(side);
	}
	return 0;
}

int time_check() {
	now = time(NULL);
	return ((now < start_time + session_min * 60));
}

