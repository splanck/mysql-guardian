/*
    Copyright (c) 2018-20 - Stephen Planck and Alistair Packer
    
    healthcheck.c - Main source file for the mysqlgd daemon process.
    
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
#include "healthcheck.h"

extern dbserver configServer;			// Struct to store config database server.
extern guardianconfig configSettings;	// Struct to store configuration settings for daemon.

extern struct myserver *pFirst;
extern struct myserver *pLast;
extern struct myhealthcheck *pFirstHC;
extern struct myhealthcheck *pLastHC;

int isHealthCheckTime() {
	if(timeForHealthCheck() == 1) {
		if(pFirst == NULL)
			populateMonitoredServersList();

		struct myserver *pServer = pFirst;

		while(pServer != NULL) {
			struct myhealthcheck *pHC = malloc(sizeof(struct myhealthcheck));	
			char *db_err = malloc(500);

			strcpy(pHC->hostname, pServer->hostname);

			pHC->id = pServer->id;
			pHC->server_online = hcServerOnline(pHC);
			pHC->database_online = hcDatabaseServerOnline(pHC, db_err, pServer);

			if(pHC->database_online == 0 && strcmp(db_err, NULL) == 1) {
				strcpy(pHC->database_online_err, db_err);
				strcpy(db_err, NULL);
			}

            pHC->recent_backup = hcRecentBackup(pHC, pServer);

			addHealthCheck(pHC);

			free(db_err);

			pServer = pServer->next;
		}
	}
}

int hcServerOnline(struct myhealthcheck *pHC) {
	int success = 0;
	int retries = 0;

	do {
		success = pingServer(pHC->hostname);

		if(success == 0 && retries < configSettings.checkRetries)
			retries++;
		else
			break;
	} while(retries <= configSettings.checkRetries);

	if(success)
		return 1;
	else
		return 0;
}

int hcDatabaseServerOnline(struct myhealthcheck *pHC, char *db_err, struct myserver *pServer) {
	int success = 0;
	int retries = 0;

	do {
		success = checkDatabase(pServer, NULL, db_err);

		if(success == 0 && retries < configSettings.checkRetries)
			retries++;
		else
			break;
	} while(retries <= configSettings.checkRetries);

	if(success)
		return 1;
	else
		return 0;
}

int hcRecentBackup(struct myhealthcheck *pHC, struct myserver *pServer) {
    int success = 0;
    int backup_found = 1;

    if(pServer->firstDatabase == NULL)
        populateServerDatabasesList(pServer);

    struct mydatabase *pDatabase = pServer->firstDatabase;

    while(pDatabase != NULL) {
        int backup = checkRecentBackup(pServer->id, pDatabase->dbname);

        if(backup != 1)
            success = 0; 

        pDatabase = pDatabase->next;
    }

    return success;
}

int hcIntegrityCheck(struct myhealthcheck *pHC, struct myserver *pServer) {
    int success = 0;

    if(pServer->firstDatabase == NULL)
        populateServerDatabasesList(pServer);

    struct mydatabase *pDatabase = pServer->firstDatabase;

    while(pDatabase != NULL) {
        int integrity = checkRecentIntegrityCheck(pServer->id, pDatabase->dbname);

        if(integrity != 1)
            success = 0; 

        pDatabase = pDatabase->next;
    }

    return success;
}
