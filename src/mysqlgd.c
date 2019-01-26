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
#include <sys/wait.h>
#include "utility.h"
#include "guardian.h"
#include "fileio.h"
#include "mysql.h"
#include "mysqlgd.h"
#include "checks.h"

pid_t parent_pid;

time_t last_server_check;
time_t last_integrity_check;
time_t last_database_check;
time_t last_database_server_check;
time_t last_slow_query_check;
time_t last_backup_check;
time_t last_task_check;

double server_check_delay;
double integrity_check_delay;
double database_check_delay;
double database_server_check_delay;
double slow_query_check_delay;
double backup_check_delay;
double task_check_delay;

extern dbserver configServer;			// Struct to store config database server.
extern guardianconfig configSettings;	// Struct to store configuration settings for daemon.

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

	parent_pid = getpid();

	getConfigd();

	if((chdir(configSettings.logPath)) < 0)
		exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	initDaemon();
}

// Reads MySQL monitoring server configuration into memory using readConfig()
void getConfigd() {
    configSettings.onlineCheckInterval = 60;
	configSettings.databaseCheckInterval = 120;
	configSettings.databaseServerCheckInterval = 120;
	configSettings.integrityCheckInterval = 500;

	char *hostname = malloc(80);
    char *username = malloc(25);
    char *password = malloc(25);
	char *backup_path = malloc(200);
	char *log_path = malloc(200);

    if(readConfig(hostname, username, password, backup_path, log_path)) {
		syslog(LOG_INFO, "%s", "Could not read /etc/mysqlgd.conf file. Exiting...");
        exit(1);
    }

    configServer.hostname = hostname;
    configServer.username = username;
    configServer.password = password;
	configSettings.backupPath = backup_path;
	configSettings.logPath = log_path;
}

// Initialises values for timers used to schedule checks.
void setupTimers() {
	server_check_delay = configSettings.onlineCheckInterval;
	database_server_check_delay = configSettings.databaseServerCheckInterval;
	database_check_delay = configSettings.databaseCheckInterval;
	integrity_check_delay = configSettings.integrityCheckInterval;
	backup_check_delay = configSettings.databaseBackup;
	slow_query_check_delay = 60;
	task_check_delay = 30;

	time(&last_server_check);
	time(&last_integrity_check);
	time(&last_database_server_check);
	time(&last_database_check);
	time(&last_backup_check);
	time(&last_slow_query_check);
	time(&last_task_check);
}

// Sets up handling of SIGTERM termination signal from kernel, sets intervals for checks,
// and performs a loop that runs the desired checks at the specified intervals.
int initDaemon() {
	signal(SIGTERM, sig_handler);

	if(configSettings.slowQueryMonitoring == 1)
		enableSlowQueryLogging();

	setupTimers();

	while(1) {
		doServerCheck();
		doDatabaseServerCheck();
		doDatabaseCheck();
		doIntegrityCheck();
		doDatabaseBackups();
		doTaskCheck();

		if(configSettings.slowQueryMonitoring == 1)
			doSlowQueryCheck();
	
		sleep(5);

		waitpid(-1, NULL, WNOHANG);
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
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time for server check.");

		checkServersOnline();

		time(&last_server_check);
	}

	return 0;
}

// Checks to see if its time to perform database server online checks, and if so, it calls the
// checkDatabaseServer function to run the checks. It also resets the check timer to
// calculate time next checks should be performed.
int doDatabaseServerCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_database_server_check);

	if(diff > database_server_check_delay) {
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time for database server check.");

		checkDatabaseServer();

		time(&last_database_server_check);
	}

	return 0;
}

// Checks to see if its time to perform database online checks, and if so, it calls the
// checkDatabaseOnline function to run the checks. It also resets the check timer to
// calculate time next checks should be performed.
int doDatabaseCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_database_check);

	if(diff > database_check_delay) {
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time for database online check.");

		checkDatabaseOnline();

		time(&last_database_check);
	}

	return 0;
}

// Checks to see if its time to perform integrity checks, and if so, it calls the
// performIntegrityCheckDB function to run the checks. It also resets the check timer to
// calculate time next checks should be performed.
int doIntegrityCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_integrity_check);

	if(diff > integrity_check_delay) {
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time for integrity checks.");

		performIntegrityCheckDB();

		time(&last_integrity_check);
	}

	return 0;
}

int doSlowQueryCheck() {
	return 0;
}

// Determines if its time to check for pending manual tasks, and if so, it spawns a new process
// to perform these tasks asynchronously. performTaskCheck function is called to check for
// pending tasks and perform them. 
int doTaskCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_task_check);

	if(diff > task_check_delay) {
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time to check for pending tasks..");

		pid_t pid = fork();

		if(pid < 0)
			return 1;	

		if(pid > 0) {
			time(&last_task_check);
			return 0;	
		}	
		else {
			pid_t sid = setsid();

			if(sid < 0)
				exit(EXIT_FAILURE);

			performTaskCheck();
			syslog(LOG_INFO, "%s", "Task check performed successfully.");
	
			exit(EXIT_SUCCESS);
		}	
	}

	return 0;
}

// Checks to see if it is time to perform automated database backups, and if so, it spawns a 
// new process to perform these backups asynchronously. The performDatabaseBackups function is
// called to perform the backups. 
int doDatabaseBackups() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_backup_check);

	if(diff > backup_check_delay) {
		if(configSettings.extendedLogging == 1)
			syslog(LOG_INFO, "%s", "Time to perform database backups.");
	
		pid_t pid = fork();

		if(pid < 0)
			return 1;	

		if(pid > 0) {
			time(&last_backup_check);
			return 0;	
		}	
		else {
			pid_t sid = setsid();

			if(sid < 0)
				exit(EXIT_FAILURE);

			performDatabaseBackups();
	
			if(configSettings.extendedLogging == 1)
				syslog(LOG_INFO, "%s", "Backup process completed.");

			exit(EXIT_SUCCESS);
		}	
	}

	return 0;
}

// The checkFailure function is called to process failed checks. Error messages are constructed
// to generate an alert for the failure. These alerts are written to the system log.
void checkFailure(struct myserver *svr, struct mydatabase *db, struct mytable *tbl, checkType_t chk,
	char *error, char *error_desc) {
	char error_msg[250] = "CHECK FAILURE: "; 

	if(chk == c_serverOnline) {
		strcat(error_msg, "Server ");
		strcat(error_msg, svr->hostname);
		strcat(error_msg, " is unresponsive and may be offline.");
	}
	else if(chk == c_databaseServer) {
		strcat(error_msg, "Cannot connect to database server on ");
		strcat(error_msg, svr->hostname);
		strcat(error_msg, ". Database server may be inaccessible.");
	}
	else if(chk == c_databaseOnline) {
		strcat(error_msg, "Connot access the ");
		strcat(error_msg, db->dbname);
		strcat(error_msg, " database on ");
		strcat(error_msg, svr->hostname);
		strcat(error_msg, ". Database may be offline.");
	}
	else if(chk == c_integrityCheck) {
		strcat(error_msg, "Integrity check failed on ");
		strcat(error_msg, tbl->tblname);
		strcat(error_msg, " from the ");
		strcat(error_msg, db->dbname);
		strcat(error_msg, " on ");
		strcat(error_msg, svr->hostname);
		strcat(error_msg, ".");	
	}
	else if(chk == c_databaseBackup) {
		strcat(error_msg, "The database backup of ");
		strcat(error_msg, db->dbname);
		strcat(error_msg, " on ");
		strcat(error_msg, svr->hostname);
		strcat(error_msg, " has failed.");
	}
	else if(chk == c_slowQuery) {
	}

	syslog(LOG_INFO, "%s", error_msg);
}

// Handles signal 15 from the kernel and performs tasks to prepare for shutdown. A shutdown
// message is written to the log and the daemon terminates normally. 
void sig_handler(int signum) {
	pid_t my_pid = getpid();

	if(my_pid == parent_pid) 
		syslog(LOG_INFO, "%s", "MySQL Guardian daemon stopping...");
	else
		syslog(LOG_INFO, "%s", "WARNING: Running backup process may have been terminated.");

	exit(0);
}
