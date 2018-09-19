/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
	daemon.c - Implements the main daemon loop functionality.
    
    This file is part of MySQL Guardian.

    MySQL Guardian is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    any later version.

    MySQL Guardian is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MySQL Guardian. If not, see <https://www.gnu.org/licenses/>.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "daemon.h"

time_t last_server_check;
time_t last_integrity_check;

double server_check_delay;
double integrity_check_delay;

int startDaemon() {
	signal(SIGTERM, sig_handler);

	server_check_delay = 30;
	integrity_check_delay = 600;

	time(&last_server_check);
	time(&last_integrity_check);

	while(1) {
		serverCheck();
		integrityCheck();

		sleep(10);
	}

	return 0;
}

int serverCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_server_check);

	if(diff > server_check_delay) {
		syslog(LOG_INFO, "%s", "Time for server check.");
		time(&last_server_check);
	}

	return 0;
}

int integrityCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_integrity_check);

	if(diff > integrity_check_delay) {
		syslog(LOG_INFO, "%s", "Time for integrity checks.");
		time(&last_integrity_check);
	}

	return 0;
}

void sig_handler(int signum) {
	syslog(LOG_INFO, "%s", "MySQL Guardian daemon stopping...");
	exit(0);
}
