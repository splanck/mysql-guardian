/*
	Copyright (c) 2018-20 - Stephen Planck and Alistair Packer

	fileio.c - Contains functions that read and write to the filesystem.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"
#include "fileio.h"
#include "mysqlgd.h"
#include "guardian.h"

extern dbserver configServer;           // Struct to store config database server.
extern guardianconfig configSettings;   // Struct to store configuration settings for daemon.

int writeMailFile(char emailMsg[250], char filename[80]) {
	FILE *mailFile;

	mailFile = fopen(filename, "w");

	if(!mailFile)
		return 1;

	fprintf(mailFile, "%s\n", emailMsg);

	fclose(mailFile);

	return 0;
}

// Writes an entry into the mysql_guardian.log file. It accepts a char array
// as the string to be written to the log.
int writeLog(char logEntry[200], char filename[80]) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	FILE *logFile;

	logFile = fopen(filename, "a");

	if(!logFile)
		return 1;

	fprintf(logFile, "%d-%d-%d %d:%d:%d: ", tm.tm_year + 1900, tm.tm_mon + 1, 
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(logFile, "%s\n", logEntry);

	fclose(logFile);

	return 0;
}

// Calls writeLog function to write a log entry to the general log file.
// Returns true if an error occurred.
int writeToLog(char logEntry[200])
{
	return writeLog(logEntry, "mysqlgd.log");
}

// Calls writeLog function to write a log entry to the SQL command log file.
// Returns true if an error occurred.
int writeToSQLLog(char logEntry[200])
{
	return writeLog(logEntry, "mysqlgd-sql.log");
}

// Accepts a hostname, username, and password for the monitoring server and generates a
// .mysql-guardian_rc configuration file.
int createConfigFile(char *hostname, char *username, char *password) {
	FILE *cfgFile;

	cfgFile = fopen("/etc/mysqlgd.conf", "w");

	if(!cfgFile)
		return 1;

	freopen(NULL, "w+", cfgFile);

	fprintf(cfgFile, "HOSTNAME %s\n", hostname);
	fprintf(cfgFile, "USERNAME %s\n", username);
	fprintf(cfgFile, "PASSWORD %s\n", password);
	fprintf(cfgFile, "LOG_PATH /var/log\n");
	fprintf(cfgFile, "ONLINE_CHECK_INTERVAL 60\n");
	fprintf(cfgFile, "DATABASE_SERVER_CHECK_INTERVAL 120\n");
	fprintf(cfgFile, "DATABASE_CHECK_INTERVAL 120\n");
	fprintf(cfgFile, "INTEGRITY_CHECK_INTERVAL 500\n");
	fprintf(cfgFile, "SLOW_QUERY_MONITORING 1\n");
	fprintf(cfgFile, "DATABASE_BACKUP 1000\n");
	fprintf(cfgFile, "BACKUP_PATH /tmp\n");
	fprintf(cfgFile, "EXTENDED_LOGGING 1\n");
	fprintf(cfgFile, "DAILY_HEALTH_CHECK 1\n");
	fprintf(cfgFile, "CHECK_RETRY_ATTEMPTS 0\n");
	fprintf(cfgFile, "DESTINATION_EMAIL root@localhost\n");
	
	fclose(cfgFile);

	return 0;
}

// Reads configuration from /etc/mysqlgd.conf and passed values back as string pointers.
int readConfig() {
	char k[40], v[40];
	
	FILE *configFile;

	configFile = fopen("/etc/mysqlgd.conf", "r");

	if (!configFile)
		return 1;

	while(!feof(configFile)) {
		if(fscanf(configFile, "%s %s", k, v)) {
			ucase(k);

			processConfigKeyValuePair(k, v);
		}	
    }

	fclose(configFile);

  	return 0;
}

void processConfigKeyValuePair(char *k, char *v) {
	if(strcmp(k, "HOSTNAME") == 0) {
		char *hostname = malloc(80);
		strcpy(hostname, v);
		configServer.hostname = hostname;
	}

	if(strcmp(k, "USERNAME") == 0) {
		char *username = malloc(50);
		strcpy(username, v);
		configServer.username = username;
	}

	if(strcmp(k, "PASSWORD") == 0) {
		char *password = malloc(50);
		strcpy(password, v);
		configServer.password = password;
	}

	if(strcmp(k, "BACKUP_PATH") == 0) {
		char *backup_path = malloc(250);
		strcpy(backup_path, v);
		configSettings.backupPath = backup_path;
	}

	if(strcmp(k, "LOG_PATH") == 0) {
		char *log_path = malloc(250);
		strcpy(log_path, v);
		configSettings.logPath = log_path;
	}

	if(strcmp(k, "DESTINATION_EMAIL") == 0) {
		char *destination_email = malloc(100);
		strcpy(destination_email, v);
		configSettings.destinationEmail = destination_email;
	}

	if(strcmp(k, "CHECK_RETRY_ATTEMPTS") == 0) {
		int i = atoi(v);

		if(i >= 0 && i < 10)
			configSettings.checkRetries = i;
	}

	if(strcmp(k, "ONLINE_CHECK_INTERVAL") == 0) {
		int i = atoi(v);
				
		if(i > 0 && i < 99999)
			configSettings.onlineCheckInterval = i;
	}

	if(strcmp(k, "INTEGRITY_CHECK_INTERVAL") == 0) {
		int i = atoi(v);

		if(i > 0 && i < 99999)
			configSettings.integrityCheckInterval = i;
	}

	if(strcmp(k, "DATABASE_CHECK_INTERVAL") == 0) {
		int i = atoi(v);

		if(i > 0 && i < 99999)
			configSettings.databaseCheckInterval = i;
	}

	if(strcmp(k, "DATABASE_SERVER_CHECK_INTERVAL") == 0) {
		int i = atoi(v);

		if(i > 0 && i < 99999)
			configSettings.databaseServerCheckInterval = i;
	}

	if(strcmp(k, "SLOW_QUERY_MONITORING") == 0) {
		int i = atoi(v);

		if(i == 0 || i == 1)
			configSettings.slowQueryMonitoring = i;
	}

	if(strcmp(k, "EXTENDED_LOGGING") == 0) {
		int i = atoi(v);

		if(i == 0 || i == 1)
			configSettings.extendedLogging = i;
	}

	if(strcmp(k, "DAILY_HEALTH_CHECK") == 0) {
		int i = atoi(v);

		if(i == 0 || i == 1)
			configSettings.healthCheck = i;
	}

	if(strcmp(k, "DATABASE_BACKUP") == 0) {
		int i = atoi(v);

		if(i > 0 || i < 99999)
			configSettings.databaseBackup = i;
	}
}
