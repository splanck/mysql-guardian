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
#include "utility.h"
#include "mysql.h"
#include "fileio.h"
#include "interface.h"
#include "mysqlgd.h"

#define VERSION "0.01"

int colourSupport = 0;			// True if terminal supports ncurses colour
int canChangeColours = 0;		// True if terminal supports ncurses change colour 
char db_error[1000];			// Global variable to store database error messages
int g_argc = 0;
char **g_argv = NULL;

dbserver configServer;			// Struct to store config database server.
guardianconfig configSettings;	// Struct to store configuration settings for daemon.

// Calls functions to initialise the log, read configuration file, setup
// ncurses terminal, and display the main menu. Also performs clean up tasks
// upon exit.
int main(int argc, char **argv) {
	g_argc = argc;
	g_argv = argv;

	processParams();

	cleanUpTerminal();
	cleanUpTasks();

	return 0;
}

void processParams() {
	if(g_argc > 1) {
		if(strcmp(g_argv[1], "--demonize") == 0) {
			startDaemon();
		}
		else if(strcmp(g_argv[1], "--init") == 0) {
			initialiseSetup();
		}
		else if(strcmp(g_argv[1], "--help") == 0) {
			commandHelp();
		}
		else if(strcmp(g_argv[1], "--gui") == 0) {
			setupGUITool();
		}
		else if(strcmp(g_argv[1], "--debug") == 0) {
			debugFunc();
		}
		else {
			printf("Invalid argument. Type mysql-guardian --help for list of valid parameters.\n");
			exit(1);
		}
	}
	else {
		commandHelp();
	}
}

void debugFunc() {
	getConfigd();
	initDaemon();
}

void commandHelp() {
	printf("MySQL Guardian v%s\n\n", VERSION);
	printf("--init\t\tCreate new /etc/mysqlgd.conf configuration file.\n");
	printf("--gui\t\tLaunch the MySQL Guardian console control centre application.\n");
	printf("--demonize\tLaunch the MySQL Guardian daemon to run in the background.\n");
	printf("--help\t\tDisplay the help screen.\n");
	exit(0);
}

void setupGUITool() {
	signal(SIGWINCH, resizeHandler);
	
	initialiseLog();
	getConfig();
	setupTerminal();
	mainMenu();
}

void initialiseSetup() {
	char hostname[80] = "";
	char username[80] = "";		
	char password[80] = "";		

	printf("MySQL Guardian %s\n\n", VERSION);
	printf("Configure Monitoring Server\n\n");
	printf("Server Hostname: ");
	scanf("%s", &hostname);

	printf("Root Username: ");
	scanf("%s", &username);

	printf("Root Password: ");
	scanf("%s", &password);

	if(createConfigFile(hostname, username, password) == 1) {
		printf("\nCould not create configuration file.\n");
		return;
	}

	getConfig();

	int dbsuccess = createConfigDB();

	if(dbsuccess) {
		printf("\nCould not create configuration database.\n");
	}
	else {
		printf("\nConfiguration database created successfully.\n");

		int tblsuccess = createConfigTables();

		if(tblsuccess)
			printf("Could not create configuration tables.\n");
		else
			printf("Configuration tables created successfully.\n");
	}
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
	char *backup_path = malloc(200);
	char *log_path = malloc(200);

	if(readConfig(hostname, username, password, backup_path, log_path)) {
		printf("Could not read configuration file. Use the -init parameter to create one.\n\r");
		printf("Exiting,\n\r");
		
		exit(1);
	}

	configServer.hostname = hostname;
	configServer.username = username;
	configServer.password = password;

	configSettings.backupPath = backup_path;
	configSettings.logPath = log_path;
	configSettings.onlineCheckInterval = 60;
	configSettings.integrityCheckInterval = 500;
}

// Writes shutdown message to log file.
void cleanUpTasks() {
	writeToLog("MySQL Guardian has stopped.");
}
