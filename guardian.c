#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "guardian.h"
#include "mysql.h"
#include "fileio.h"
#include "interface.h"

#define VERSION "0.01"

int colourSupport = 0;
int canChangeColours = 0;
char db_hostname[80];
char db_username[25];
char db_password[25];

int main(int argc, char **argv) {
	initialiseLog();
	getConfig();

	setupTerminal();
	mainMenu();
	
	cleanUpTerminal();
	cleanUpTasks();

	return 0;
}

void initialiseLog() {
	char str[80];
	strcpy(str, "MySQL Guardian v");
	strcat(str, VERSION);
	
	printf("MySQL Guardian v%s\n\r", VERSION);

	if(writeToLog("---------------------------------------------------------")) {
		printf("Could not create log file. Exiting.\n\r");
		
		exit(1);
	}

	writeToLog(str);
	writeToLog("MySQL Guardian has started.");
}

void getConfig() {
	if(readConfig()) {
		printf("Could not read configuration file. Exiting,\n\r");

		exit(1);
	}
}

void cleanUpTasks() {
	writeToLog("MySQL Guardian has stopped.");
}