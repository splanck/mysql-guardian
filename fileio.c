#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern char db_hostname[80];
extern char db_username[25];
extern char db_password[25];

int writeToLog(char logEntry[80]) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	FILE *logFile;

	logFile = fopen("mysql-guardian.log", "a");

	if(!logFile)
		return 1;

	fprintf(logFile, "%d-%d-%d %d:%d:%d: ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(logFile, "%s\n", logEntry);

	fclose(logFile);

	return 0;
}

int readConfig() {
	char host_buffer[1000];
	char username_buffer[1000];
	char password_buffer[1000];

	FILE *configFile;

	configFile = fopen(".mysql-guardian_rc", "r");

	if (!configFile)
		return 1;

	fgets(host_buffer, 1000, configFile);
	fgets(username_buffer, 1000, configFile);
	fgets(password_buffer, 1000, configFile);

	fclose(configFile);

  	char *str;

  	str = strtok(host_buffer, " ");
	strcpy(db_hostname, strtok(NULL, " "));
	str = strtok(username_buffer, " ");
	strcpy(db_username, strtok(NULL, " "));
	str = strtok(password_buffer, " ");
	strcpy(db_password, strtok(NULL, " "));

  	return 0;
}