/*
	Copyright (c) 2018 - Stephen Planck and Alistair Packer

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

#define VERSION "0.01"

void setupGUITool();
void processParams();
void commandHelp();
void initialiseLog();
void listServers();
void statusServers();
void addNewServer();
void includeExclude(char *serverName, int incflag);
void removeServer(char *serverName);
void initialiseSetup();
void cleanUpTasks();
void getConfig();
void guardianHeader();
void debugFunc();

typedef struct server dbserver;
typedef struct gconfig guardianconfig;

struct server {
	char *hostname;
	int port;
	char *username;
	char *password;
};

struct gconfig {
	int onlineCheckInterval;
	int databaseServerCheckInterval;
	int databaseCheckInterval;
	int integrityCheckInterval;
	int slowQueryMonitoring;
	int databaseBackup;
	int extendedLogging;
	int checkRetries;
	char *backupPath;
	char *logPath;
	char *destinationEmail;
};
