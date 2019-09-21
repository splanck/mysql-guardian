/*
    Copyright (c) 2018-19 - Stephen Planck and Alistair Packer
    
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

			strcpy(pHC->hostname, pServer->hostname);
			pHC->id = pServer->id;
			pHC->server_online = hcServerOnline(pHC);

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
