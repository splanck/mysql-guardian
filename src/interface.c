/*
	Copyright (c) 2018 - Stephen Planck and Alistair Packer

	interface.c - Contains functions to implement ncurses user interface.

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
#include "interface.h"

extern int colourSupport;
extern int canChangeColours;
extern char db_error[1000];
extern dbserver configServer;

extern struct myserver *pFirst;
extern struct myserver *pLast;

char newHostname[80] = "";		// Stores new hostname for add server window
int newPort = 3306;				// Stores new port for add server window
char newUsername[80] = "";		// Stores new username for add server window
char newPassword[80] = "";		// Stores new password for add server window
int addServerHighlight;			// ID for currently highlighted field on add server window

// Displays main menu, implements lightbar navigation, and calls functions
// to execute selected menu item.
int mainMenu() {
	clear();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_RED);
	
	move(0, 0);

	attron(COLOR_PAIR(1));
	bkgd(COLOR_PAIR(1));

	mvprintw(0, 47, "MySQL Guardian");

	attroff(COLOR_PAIR(1));
	refresh();
	
	int height = 13;
	int width = 40;
	int starty = 3;
	int startx = 35;

	WINDOW *menuWin = newwin(height, width, starty, startx);
	box(menuWin, 0, 0);

	wbkgd(menuWin, COLOR_PAIR(2));
	wrefresh(menuWin);

	keypad(menuWin, true);

	char* choices[] = {
  		"Show MySQL Version",
  		"Show Current Configuration",
  		"Create Configuration Database",
  		"Add Server to Monitoring",
  		"Show Monitored Servers List",
		"Show Monitored Databases List",
		"Show Tables List",
  		"Check localhost",
  		"Exit"
	};

	int choice = 0;
	int highlight = 0;

	while(1)
	{
		attron(COLOR_PAIR(1));

		int i;

		for(i = 0; i < 9; i++) {
			if(i == highlight)
				wattron(menuWin, A_REVERSE);

			mvwprintw(menuWin, i + 2, 2, choices[i]);
			wattroff(menuWin, A_REVERSE);
		}

		wrefresh(menuWin);
	
		choice = wgetch(menuWin);

		if(choice == KEY_UP)
			highlight--;
		
		if(choice == KEY_DOWN)
			highlight++;

		if(highlight == 9)
			highlight = 0;

		if(highlight == -1)
			highlight = 8;

		if(choice == 10)
			break;

		attroff(COLOR_PAIR(1));
	}

	if(highlight == 0)
		showDBVersion();

	if(highlight == 1)
		showConfig();

	if(highlight == 2) 
		createDB();

	if(highlight == 3)
		addServer();

	if(highlight == 4)
		showServersList();

	if(highlight == 5)
		showDatabasesList();

	if(highlight == 6)
		showTablesList();

	if(highlight == 7)
		checkServerOnline();

	if(highlight == 8)
		return 0;

	mainMenu();
}

void checkServerOnline() {
	int checkSuccess = pingServer("localhost");

	if(checkSuccess == 0) {
		mvprintw(1, 0, "Server is online.\n\r");
	}
	else {
		mvprintw(1, 0, "Server is not reachable.\n\r");
	}

	getch();
}

// Count servers in monitoring and display the count.
void showServersList() {
	if(pFirst == NULL) {
		int success = populateMonitoredServersList();		
	}

	struct myserver *pTemp = pFirst;

	mvprintw(1, 0, "Server List:");
	
	int i = 0;

	while(pTemp != NULL) {
		mvprintw(i + 3, 0, "%s %d %s %s", pTemp->hostname, pTemp->port, pTemp->username,
			pTemp->password);

		pTemp = pTemp->next;
		i++;
	}
	
	getch();
}

void showDatabasesList() {
	if(pFirst == NULL) {
		int success = populateMonitoredServersList();		
	}

	struct myserver *pServer = pFirst;
	
	if(pServer->firstDatabase == NULL) {
		int success = populateServerDatabasesList(pServer);
		mvprintw(0, 0, "%d", success);
	}

	struct mydatabase *pTemp = pServer->firstDatabase;

	mvprintw(1, 0, "Tables List: %s", pServer->hostname);
	
	int i = 0;

	while(pTemp != NULL) {
		mvprintw(i + 3, 0, "%s", pTemp->dbname);

		pTemp = pTemp->next;
		i++;
	}

	getch();
}

void showTablesList() {
	if(pFirst == NULL) {
		int success = populateMonitoredServersList();		
	}

	struct myserver *pServer = pFirst;
	
	if(pServer->firstDatabase == NULL) {
		int success = populateServerDatabasesList(pServer);
	}

	if(pServer->firstDatabase->firstTable == NULL) {
		int success = populateDatabaseTablesList(pServer, pServer->firstDatabase);
	}

	struct mytable *pTemp = pServer->firstDatabase->firstTable;

	mvprintw(1, 0, "Tables List: %s %s", pServer->hostname, pServer->firstDatabase->dbname);
	
	int i = 0;

	while(pTemp != NULL) {
		mvprintw(i + 3, 0, "%s", pTemp->tblname);

		pTemp = pTemp->next;
		i++;
	}

	getch();
}


// Initialises new server variables, calls addServerMenu() to gather values,
// and calls addServerToTable() to add new server to servers table.
void addServer() {
	strcpy(newHostname, "");
	newPort = 3306;
	strcpy(newUsername, "");
	strcpy(newPassword, "");
	addServerHighlight = 0;

	init_pair(3, COLOR_WHITE, COLOR_MAGENTA);

	int action = addServerMenu();

	while(!action) {
		action = addServerMenu();
	}

	if(action == 1) {
		int success = addServerToTable(newHostname, newPort, newUsername, newPassword);
		
		if(success) {
			mvprintw(1, 0, "Could not add server to monitoring.\n\r");
			mvprintw(2, 0, "Error: %s", db_error);
		}
		else {
			mvprintw(1, 0, "Server added successfully.\n\r");	
		}

		getch();
	}
}

// Displays add server menu to connect data for new server, implements
// lightbar navigation, accepts values from the user, and stores them in
// global variables.
int addServerMenu() {
	int height = 8;
	int width = 80;
	int starty = 15;
	int startx = 20;

	WINDOW *addServerWin = newwin(height, width, starty, startx);
	box(addServerWin, 0, 0);

	wbkgd(addServerWin, COLOR_PAIR(3));

	keypad(addServerWin, true);

	char* choices[] = {
  		"Hostname",
  		"Port",
  		"Username",
  		"Password",
  		"OK",
  		"Cancel"
	};

	int choice = 0;

	while(1)
	{
		attron(COLOR_PAIR(1));
		
		mvwprintw(addServerWin, 1, 20, newHostname);
		mvwprintw(addServerWin, 2, 20, "%d", newPort);
		mvwprintw(addServerWin, 3, 20, newUsername);
		mvwprintw(addServerWin, 4, 20, newPassword);
		
		int i;

		for(i = 0; i < 6; i++) {
			if(i == addServerHighlight)
				wattron(addServerWin, A_REVERSE);

			if(i < 4) {
				mvwprintw(addServerWin, i + 1, 2, choices[i]);	
			}
			else if (i == 4) {
				mvwprintw(addServerWin, 6, 10, choices[i]);
			} 
			else if (i == 5) {
				mvwprintw(addServerWin, 6, 30, choices[i]);
			}
			
			wattroff(addServerWin, A_REVERSE);
		}

		if (addServerHighlight < 4)
			wmove(addServerWin, addServerHighlight + 1, 2);
		else if (addServerHighlight == 4)
			wmove(addServerWin, 6, 10);
		else if (addServerHighlight == 5)
			wmove(addServerWin, 6, 30);

		wrefresh(addServerWin);
	
		choice = wgetch(addServerWin);

		if(choice == KEY_UP)
			addServerHighlight--;
		
		if(choice == KEY_DOWN)
			addServerHighlight++;

		if(addServerHighlight == 6)
			addServerHighlight = 0;

		if(addServerHighlight == -1)
			addServerHighlight = 5;

		if(choice == 10)
			break;

		if(choice == 27)
			return 2;

		attroff(COLOR_PAIR(1));
	}

	if(addServerHighlight == 0) {
		mvwprintw(addServerWin, 1, 20, "                                            ");
		wmove(addServerWin, 1, 20);
		echo();
		wgetstr(addServerWin, newHostname);
		noecho();

		return 0;
	}

	if(addServerHighlight == 1) {
		mvwprintw(addServerWin, 2, 20, "                                            ");
		wmove(addServerWin, 2, 20);
		echo();
		wscanw(addServerWin, "%d", &newPort);
		noecho();

		return 0;
	}

	if(addServerHighlight == 2) {
		mvwprintw(addServerWin, 3, 20, "                                            ");
		wmove(addServerWin, 3, 20);
		echo();
		wgetstr(addServerWin, newUsername);
		noecho();

		return 0;
	}

	if(addServerHighlight == 3) {
		mvwprintw(addServerWin, 4, 20, "                                            ");
		wmove(addServerWin, 4, 20);
		echo();
		wgetstr(addServerWin, newPassword);
		noecho();

		return 0;
	}

	if(addServerHighlight == 4)
		return 1;

	if(addServerHighlight == 5)
		return 2;

	return 0;
}

// Calls createConfigDB() and createConfigTables() to create monitoring 
// database and server table. It then displays the results.
void createDB() {
	int dbsuccess = createConfigDB();

	if(dbsuccess) {
		mvprintw(1, 0, "Could not create configuration database.\n\r");
		mvprintw(2, 0, "Error: %s", db_error);
	}
	else {
		mvprintw(1, 0, "Configuration database created successfully.\n\r");	

		int tblsuccess = createConfigTables();

		if(tblsuccess) {
			mvprintw(2, 0, "Could not create configuration tables.\n\r");
			mvprintw(3, 0, "Error: %s", db_error);
		}
		else {
			mvprintw(2, 0, "Configuration tables created successfully.\n\r");	
		}
	}

	getch();
}

// Function to display a window to the user to ask a question. It accepts
// a char array with the desired question and a char pointer to store the 
// response. Function not used yet.
void askQuestion(char questionText[80], char *answer) {
	int height = 4;
	int width = 80;
	int starty = 15;
	int startx = 20;

	char input[80];

	init_pair(3, COLOR_WHITE, COLOR_MAGENTA);

	WINDOW *questionWin = newwin(height, width, starty, startx);
	box(questionWin, 0, 0);

	wbkgd(questionWin, COLOR_PAIR(3));

	wmove(questionWin, 1, 1);
	mvwprintw(questionWin, 1, 1, questionText);
	wrefresh(questionWin);

	echo();
	wgetstr(questionWin, input);
	noecho();

	strcpy(answer, input);
}

// Displays monitoring server configuration values to the screen.
void showConfig() {
	mvprintw(2, 0, "Hostname: %s", configServer.hostname);
	mvprintw(3, 0, "Username: %s", configServer.username);
	mvprintw(4, 0, "Password: %s", configServer.password);

	getch();
}

void showDBVersion() {
	char *dbversion = malloc(100);

	getDBVersion(dbversion);

	mvprintw(1, 0, "MySQL Server Version: %s", dbversion);

	getch();
}

// Signal handler for terminal window resizing. Needs implemented.
void resizeHandler(int sig)
{
	int nh, nw;
	getmaxyx(stdscr, nh, nw);  /* get the new screen size */
}

// Initialises ncurses library and determines terminal colour capabilities.
void setupTerminal() {
	initscr();
	noecho();

	if(has_colors()) 
	{
		colourSupport = 1;
		start_color();
	}

	if(can_change_color())
	{
		canChangeColours = 1;
	}
}

// Disables ncurses before exit.
void cleanUpTerminal() {
	endwin();
}
