#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "mysql.h"
#include "interface.h"

extern int colourSupport;
extern int canChangeColours;
extern char db_error[1000];

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
	
	int height = 10;
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

	getch();

	mainMenu();
}

void addServer() {
	char serverName[80];

	askQuestion("Server Name: ", serverName);

	printw("%s", serverName);
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