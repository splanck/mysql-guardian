/*
	Copyright (c) 2018 - Stephen Planck and Alistair Packer

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
	return writeLog(logEntry, "mysql-guardian.log");
}

// Calls writeLog function to write a log entry to the SQL command log file.
// Returns true if an error occurred.
int writeToSQLLog(char logEntry[200])
{
	return writeLog(logEntry, "mysql-guardian-sql.log");
}

// Accepts a hostname, username, and password for the monitoring server and generates a
// .mysql-guardian_rc configuration file.
int createConfigFile(char *hostname, char *username, char *password) {
	FILE *cfgFile;

	cfgFile = fopen(".mysql-guardian_rc", "a");

	if(!cfgFile)
		return 1;

	fprintf(cfgFile, "Hostname %s\n", hostname);
	fprintf(cfgFile, "Username %s\n", username);
	fprintf(cfgFile, "Password %s\n", password);
	
	fclose(cfgFile);

	return 0;
}

// Reads the .mysql-guardian_rc configuration file and stores its valies 
// into global variables.
int readConfig(char *hostname, char *username, char *password) {
	char host_buffer[1000];
	char username_buffer[1000];
	char password_buffer[1000];
	
	FILE *configFile;

	configFile = fopen("/etc/mysqlgd.conf", "r");

	if (!configFile)
		return 1;

	fgets(host_buffer, 1000, configFile);
	fgets(username_buffer, 1000, configFile);
	fgets(password_buffer, 1000, configFile);

	fclose(configFile);

  	char *str;

  	str = strtok(host_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(hostname, str);

	str = strtok(username_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(username, str);

	str = strtok(password_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(password, str);

  	return 0;
}

// Improved read config function that is not yet working.
int readConfig2() {
	char buffer[1000];
	char cfg_hostname[80];
	char cfg_username[25];
	char cfg_password[25];

	FILE *configFile;

	configFile = fopen(".mysql-guardian_rc", "r");

	if (!configFile)
		return 1;

	char *str;

	while(fgets(buffer, 1000, configFile)) {
		str = strtok(buffer, " ");

		if(strcmp(str, "Hostname"))
			strcpy(cfg_hostname, strtok(NULL, " "));
		else if(strcmp(str, "Username"))
			strcpy(cfg_username, strtok(NULL, " "));
		else if(strcmp(str, "Password"))
			strcpy(cfg_password, strtok(NULL, " "));
	}

	fclose(configFile);

  	return 0;
}
