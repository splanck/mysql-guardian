/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
    mysqlgd.c - Main source file for the mysqlgd daemon process.
    
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
#include "mysqlgd.h"
#include "guardian.h"
#include "fileio.h"
#include "utility.h"
#include "mysql.h"

extern dbserver configServer;      // Struct to store config database server.

time_t last_server_check;
time_t last_integrity_check;

double server_check_delay;
double integrity_check_delay;

extern struct myserver *pFirst;
extern struct myserver *pLast;

// Forked the current process to create the daemon process, writes successful start up
// to the system log, sets the current working directory, closes standard input, output
// and error, reads configuration from config file, and calls the init function to
// initiate daemon operations.
void startDaemon() {
	pid_t pid = fork();

	if(pid < 0)
		exit(EXIT_FAILURE);

	if(pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	openlog("mysqlgd", 0, LOG_DAEMON);
	syslog(LOG_INFO, "%s", "MySQL Guardian daemon starting...");

	pid_t sid = setsid();

	if(sid < 0)
		exit(EXIT_FAILURE);

	if((chdir("/")) < 0)
		exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	getConfigd();
	initDaemon();
}

// Reads MySQL monitoring server configuration into memory using getConfig()
void getConfigd() {
    char *hostname = malloc(80);
    char *username = malloc(25);
    char *password = malloc(25);

    if(readConfig(hostname, username, password)) {
		syslog(LOG_INFO, "%s", "Could not read /etc/mysqlgd.conf file. Exiting...");
        exit(1);
    }

    configServer.hostname = hostname;
    configServer.username = username;
    configServer.password = password;
}

// Sets up handling of SIGTERM termination signal from kernel, sets intervals for checks,
// and performs a loop that runs the desired checks at the specified intervals.
int initDaemon() {
	signal(SIGTERM, sig_handler);

	server_check_delay = 30;
	integrity_check_delay = 600;

	time(&last_server_check);
	time(&last_integrity_check);

	while(1) {
		doServerCheck();
		integrityCheck();

		sleep(10);
	}

	return 0;
}

// Checks to see if its time to perform server online checks, and if so, it calls the
// checkServersOnline function to run the checks. It also resets the check timer to
// calculate time next checks should be performed.
int doServerCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_server_check);

	if(diff > server_check_delay) {
		syslog(LOG_INFO, "%s", "Time for server check.");
		checkServersOnline();

		time(&last_server_check);
	}

	return 0;
}

// Retrieves the server linked list if it hasn't been retrieved already and loops through
// each server in the list. pingServer is called to perform the check and the result is
// recorded in both the system log and the check_results table in the monitoring database.
int checkServersOnline() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pTemp = pFirst;

	while(pTemp != NULL) {
		int success = pingServer(pTemp->hostname);

		if(!success)
			syslog(LOG_INFO, "%s %s", "Server online check succeeded for ", pTemp->hostname);
		else
			syslog(LOG_INFO, "%s %s", "Server online check failed for ", pTemp->hostname);

		writeCheckResult(pTemp->id, 1, success, NULL);
	
		pTemp = pTemp->next;
	}
}

int integrityCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_integrity_check);

	if(diff > integrity_check_delay) {
		syslog(LOG_INFO, "%s", "Time for integrity checks.");
		performIntegrityCheck();

		time(&last_integrity_check);
	}

	return 0;
}

int performIntegrityCheck() {
	return 0;
}

// Handles signal 15 from the kernel and performs tasks to prepare for shutdown. A shutdown
// message is written to the log and the daemon terminates normally. 
void sig_handler(int signum) {
	syslog(LOG_INFO, "%s", "MySQL Guardian daemon stopping...");
	exit(0);
}
