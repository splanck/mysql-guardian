/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
    checks.c - Source file that performs the checks for the daemon. 
    
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

extern dbserver configServer;			// Struct to store config database server.
extern guardianconfig configSettings;	// Struct to store configuration settings for daemon.

extern struct myserver *pFirst;
extern struct myserver *pLast;

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
				checkFailure(pServer, NULL, NULL, c_serverOnline, "Server Offline",
					"Could not reach host.");

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

int checkDatabaseServer() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pServer = pFirst;

	while(pServer != NULL) {
		if(pServer->database_server_check == 1) {
			char *db_err = malloc(500);

			int success = checkDatabase(pServer, NULL, db_err);

			if(!success) {
				syslog(LOG_INFO, "%s %s", "Database server online check succeeded for", 
					pServer->hostname);
			}
			else {
				syslog(LOG_INFO, "%s %s", "Database Server online check failed for", 
					pServer->hostname);

				checkFailure(pServer, NULL, NULL, c_databaseServer, "Database server check failed",
					"Could not connect to database server.");
			}

			if(!success)
				db_err = NULL;

			writeCheckResult(pServer->id, 2, success, NULL, db_err);

			free(db_err);
		}

		pServer = pServer->next;
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

				if(!success) {
					syslog(LOG_INFO, "%s %s %s %s", "Database online check succeeded for", 
						pDatabase->dbname, "on", pServer->hostname);
				}
				else {
					syslog(LOG_INFO, "%s %s %s %s", "Database Server online check failed for", 
						pDatabase->dbname, "on", pServer->hostname);

					checkFailure(pServer, pDatabase, NULL, c_databaseOnline, "Database offline",
						"Could not open database.");
				}

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

		if(success == 0) {
			syslog(LOG_INFO, "%s %s", pTable->tblname, "check was successful.");
		} 
		else if(success == 1) { 
			syslog(LOG_INFO, "%s %s", pTable->tblname, "check returned errors.");

			checkFailure(pServer, pDatabase, pTable, c_integrityCheck, "Integrity Check Failed", 
				"");
		} 
		else if(success == 2) {
			syslog(LOG_INFO, "%s %s", pTable->tblname, 
				"is using a storage engine that does not support table checking.");
		} 
		else if(success == -1) {
			syslog(LOG_INFO, "%s %s", "Could not perform table check on", pTable->tblname);
		}

		pTable = pTable->next;
	}

	return 0;
}

int performDatabaseBackups() {
	if(pFirst == NULL)
		populateMonitoredServersList();

	struct myserver *pServer = pFirst;

	while(pServer != NULL) {
		if(pServer->database_backup == 1) {
			if(pServer->firstDatabase == NULL)
				populateServerDatabasesList(pServer); 

			struct mydatabase *pDatabase = pServer->firstDatabase;

			while(pDatabase != NULL) {
				if((strcmp(pDatabase->dbname, "information_schema") == 0) ||
					(strcmp(pDatabase->dbname, "performance_schema") == 0)) {
					syslog(LOG_INFO, "%s %s %s %s. %s", "Cannot backup system database", 
						pDatabase->dbname, "on", pServer->hostname, "Skipping.");
				}
				else {
					syslog(LOG_INFO, "%s %s %s %s.", "Calling backup of", 
						pDatabase->dbname, "database on", pServer->hostname);

					backupDatabase(pServer, pDatabase);

					pDatabase = pDatabase->next;
				}	
			}
		}

		pServer = pServer->next;
	}

	return 0;
}

int backupDatabase(struct myserver *svr, struct mydatabase *db) {
	syslog(LOG_INFO, "%s %s %s %s.", "Performing backup of", 
		db->dbname, "database on", svr->hostname);

	char cmd[250];
	char path[200];

	int length = snprintf(NULL, 0, "%d", svr->port);
	char* strPort = malloc(length + 1);
	snprintf(strPort, length + 1, "%d", svr->port);

	char *backupTime = malloc(80);
	getCurrentTime(backupTime);

	remove_char_from_string('/', backupTime);

	strcpy(path, configSettings.backupPath);
	strcat(path, "/");
	strcat(path, svr->hostname);
	strcat(path, "_");
	strcat(path, db->dbname);
	strcat(path, "_");
	strcat(path, backupTime);
	strcat(path, ".sql");

	strcpy(cmd, "mysqldump --host ");
	strcat(cmd, svr->hostname);
	strcat(cmd, " -P ");
	strcat(cmd, strPort);
	strcat(cmd, " -u ");
	strcat(cmd, svr->username);
	strcat(cmd, " -p");
	strcat(cmd, svr->password);
	strcat(cmd, " ");
	strcat(cmd, db->dbname);
	strcat(cmd, " > ");
	strcat(cmd, path);

	syslog(LOG_INFO, "%s", cmd);

	int result = system(cmd);

	if(result == 0) {
		writeBackupHistory(svr->id, db->dbname, path);
	}
	else {
		checkFailure(svr, db, NULL, c_databaseBackup, "Backup Failed.", 
			"Database could not be backed up.");
	}

	return result;
}

int performTaskCheck() {
	struct mytask *task = malloc(sizeof(struct mytask));

	int task_exists = getNextTask(task);

	if(task_exists == 1) {
		syslog(LOG_INFO, "%s %d %d.", "Found new task", task->server_id, task->task_id);

		task->status = 2;
		updateTaskStatus(task);
	}

	free(task);

	return 0;
}
