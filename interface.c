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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "mysql.h"
#include "interface.h"

extern int colourSupport;
extern int canChangeColours;
extern char db_error[1000];

char newHostname[80] = "";
int newPort = 3306;
char newUsername[80] = "";
char newPassword[80] = "";
int addServerHighlight;

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
	
	int height = 9;
	int width = 40;
	int starty = 5;
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
  		"Exit"
	};

	int choice = 0;
	int highlight = 0;

	while(1)
	{
		attron(COLOR_PAIR(1));

		for(int i = 0; i < 5; i++) {
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

		if(highlight == 5)
			highlight = 0;

		if(highlight == -1)
			highlight = 4;

		if(choice == 10)
			break;

		attroff(COLOR_PAIR(1));
	}

	if(highlight == 0)
		getDBInfo();

	if(highlight == 1)
		showConfig();

	if(highlight == 2) 
		createDB();

	if(highlight == 3)
		addServer();

	if(highlight == 4)
		return 0;

	mainMenu();
}

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
		int success = addServerToTable();
		
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
		
		for(int i = 0; i < 6; i++) {
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

void cleanUpTerminal() {
	endwin();
}