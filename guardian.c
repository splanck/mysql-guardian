/*
	Copyright (c) 2018 - Stephen Planck and Alistair Packer

	guardian.c - Contains startup and initialisation functions.

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
#include <ncurses.h>
#include <signal.h>
#include "guardian.h"
#include "mysql.h"
#include "fileio.h"
#include "interface.h"

#define VERSION "0.01"

int colourSupport = 0;		// True if terminal supports ncurses colour
int canChangeColours = 0;	// True if terminal supports ncurses change colour 
char db_error[1000];		// Global variable to store database error messages

dbserver configServer;		// Struct to store config database server.

// Calls functions to initialise the log, read configuration file, setup
// ncurses terminal, and display the main menu. Also performs clean up tasks
// upon exit.
int main(int argc, char **argv) {
	initialiseLog();
	getConfig();

	setupTerminal();

	signal(SIGWINCH, resizeHandler);
	
	mainMenu();
	
	cleanUpTerminal();
	cleanUpTasks();

	return 0;
}

// Writes start up message to log fiile.
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

// Reads MySQL monitoring server configuration into memory using getConfig()
void getConfig() {
	char *hostname = malloc(80);
	char *username = malloc(25);
	char *password = malloc(25);

	if(readConfig(hostname, username, password)) {
		printf("Could not read configuration file. Exiting,\n\r");
		
		exit(1);
	}

	configServer.hostname = hostname;
	configServer.username = username;
	configServer.password = password;
}

// Writes shutdown message to log file.
void cleanUpTasks() {
	writeToLog("MySQL Guardian has stopped.");
}