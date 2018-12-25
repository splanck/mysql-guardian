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
#include "utility.h"
#include "guardian.h"
#include "fileio.h"
#include "mysql.h"
#include "mysqlgd.h"

extern dbserver configServer;			// Struct to store config database server.
extern guardianconfig configSettings;	// Struct to store configuration settings for daemon.

time_t last_server_check;
time_t last_integrity_check;
time_t last_database_check;
time_t last_database_server_check;
time_t last_slow_query_check;
time_t last_backup_check;

double server_check_delay;
double integrity_check_delay;
double database_check_delay;
double database_server_check_delay;
double slow_query_check_delay;
double backup_check_delay;

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
    configSettings.onlineCheckInterval = 60;
	configSettings.databaseCheckInterval = 120;
	configSettings.databaseServerCheckInterval = 120;
	configSettings.integrityCheckInterval = 500;

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

void setupTimers() {
	server_check_delay = configSettings.onlineCheckInterval;
	database_server_check_delay = configSettings.databaseServerCheckInterval;
	database_check_delay = configSettings.databaseCheckInterval;
	integrity_check_delay = configSettings.integrityCheckInterval;
	slow_query_check_delay = 60;
	backup_check_delay = configSettings.databaseBackup;

	time(&last_server_check);
	time(&last_integrity_check);
	time(&last_database_server_check);
	time(&last_database_check);
	time(&last_slow_query_check);
	time(&last_backup_check);
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

		if(configSettings.slowQueryMonitoring == 1)
			doSlowQueryCheck();
	
		sleep(5);
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

	struct myserver *pServer = pFirst;

	while(pServer != NULL) {
		if(pServer->online_check == 1) {
			char *db_err = malloc(200);

			int success = pingServer(pServer->hostname);

			if(!success) {
				syslog(LOG_INFO, "%s %s", "Server online check succeeded for", pServer->hostname);
				db_err = NULL;
			}	
			else {
				syslog(LOG_INFO, "%s %s", "Server online check failed for", pServer->hostname);

				strcpy(db_err, "Could not reach the host ");
				strcat(db_err, pServer->hostname);
			}

			writeCheckResult(pServer->id, 1, success, NULL, db_err);

			free(db_err);
		}	

		pServer = pServer->next;
	}

	return 0;
}

int doDatabaseServerCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_database_server_check);

	if(diff > database_server_check_delay) {
		syslog(LOG_INFO, "%s", "Time for database server check.");
		checkDatabaseServer();

		time(&last_database_server_check);
	}

	return 0;
}

int checkDatabaseServer() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pServer = pFirst;

	while(pServer != NULL) {
		if(pServer->database_server_check == 1) {
			char *db_err = malloc(500);

			int success = checkDatabase(pServer, NULL, db_err);

			if(!success)
				syslog(LOG_INFO, "%s %s", "Database server online check succeeded for", 
					pServer->hostname);
			else
				syslog(LOG_INFO, "%s %s", "Database Server online check failed for", 
					pServer->hostname);

			if(!success)
				db_err = NULL;

			writeCheckResult(pServer->id, 2, success, NULL, db_err);

			free(db_err);
		}

		pServer = pServer->next;
	}

	return 0;
}

int doDatabaseCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_database_check);

	if(diff > database_check_delay) {
		syslog(LOG_INFO, "%s", "Time for database online check.");
		checkDatabaseOnline();

		time(&last_database_check);
	}

	return 0;
}

int checkDatabaseOnline() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pServer = pFirst;

	int success = 0;

	while(pServer != NULL) {
		if(pServer->database_check == 1) {
			char *db_err = malloc(500);

			if(pServer->firstDatabase == NULL) 
				success = populateServerDatabasesList(pServer);	

			struct mydatabase *pDatabase = pServer->firstDatabase;

			while(pDatabase != NULL) {
				int success = checkDatabase(pServer, pDatabase, db_err);

				if(!success)
					syslog(LOG_INFO, "%s %s %s %s", "Database online check succeeded for", 
						pDatabase->dbname, "on", pServer->hostname);
				else
					syslog(LOG_INFO, "%s %s %s %s", "Database Server online check failed for", 
						pDatabase->dbname, "on", pServer->hostname);

				if(!success)
					db_err = NULL;

				writeCheckResult(pServer->id, 3, success, pDatabase->dbname, db_err);
	
				pDatabase = pDatabase->next;
			}

			free(db_err);
		}

		pServer = pServer->next;
	}

	return 0;
}

int doIntegrityCheck() {
	time_t time_now;
	time(&time_now);

	double diff = difftime(time_now, last_integrity_check);

	if(diff > integrity_check_delay) {
		syslog(LOG_INFO, "%s", "Time for integrity checks.");
		performIntegrityCheckDB();

		time(&last_integrity_check);
	}

	return 0;
}

int performIntegrityCheckDB() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pServer = pFirst;

	while(pServer != NULL) {
		if(pServer->integrity_check == 1) {
			if(pServer->firstDatabase == NULL)
				populateServerDatabasesList(pServer); 

			struct mydatabase *pDatabase = pServer->firstDatabase;

			while(pDatabase != NULL) {
				if((strcmp(pDatabase->dbname, "information_schema") == 0) ||
					(strcmp(pDatabase->dbname, "performance_schema") == 0)) {
					syslog(LOG_INFO, "%s %s %s %s. %s", 
						"Table check not valid for system database", 
						pDatabase->dbname, "on", pServer->hostname, "Skipping.");
				}
				else {
					syslog(LOG_INFO, "%s %s %s %s", "Checking tables in", pDatabase->dbname, "on", 
						pServer->hostname);

					performIntegrityCheckTable(pServer, pDatabase);
				}

				pDatabase = pDatabase->next;
			}
		}

		pServer = pServer->next;
	}

	return 0;
}

int performIntegrityCheckTable(struct myserver *pServer, struct mydatabase *pDatabase) {
	if(pDatabase->firstTable == NULL)
		populateDatabaseTablesList(pServer, pDatabase);	

	struct mytable *pTable = pDatabase->firstTable;
		
	while(pTable != NULL) {
		int success = checkTable(pServer, pDatabase, pTable);

		if(success == 0) 
			syslog(LOG_INFO, "%s %s", pTable->tblname, "check was successful.");
		else if(success == 1) 
			syslog(LOG_INFO, "%s %s", pTable->tblname, "check returned errors.");
		else if(success == 2)
			syslog(LOG_INFO, "%s %s", pTable->tblname, 
				"is using a storage engine that does not support table checking.");
		else if(success == -1)
			syslog(LOG_INFO, "%s %s", "Could not perform table check on", pTable->tblname);

		pTable = pTable->next;
	}

	return 0;
}

int doSlowQueryCheck() {

}

int doDatabaseBackups() {

}

// Handles signal 15 from the kernel and performs tasks to prepare for shutdown. A shutdown
// message is written to the log and the daemon terminates normally. 
void sig_handler(int signum) {
	syslog(LOG_INFO, "%s", "MySQL Guardian daemon stopping...");
	exit(0);
}
