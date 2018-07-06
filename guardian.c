#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "guardian.h"

int colourSupport = 0;
int canChangeColours = 0;

int main() {
	setupTerminal();
	mainMenu();
	cleanUpTerminal();

	return 0;
}

int mainMenu() {
	clear();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_RED);
	
	move(0, 0);

	attron(COLOR_PAIR(1));
	bkgd(COLOR_PAIR(1));

	mvprintw(0, 47, "MySQL Guardian");

	attroff(COLOR_PAIR(1));
	
	int height = 10;
	int width = 30;
	int starty = 5;
	int startx = 40;

	WINDOW *menuWin = newwin(height, width, starty, startx);
	box(menuWin, 0, 0);
	
	refresh();

	wbkgd(menuWin, COLOR_PAIR(2));
	wrefresh(menuWin);

	keypad(menuWin, true);

	char* choices[] = {
  		"Verify a server is online",
  		"List databases on a server",
  		"Exit Program"
	};

	int choice = 0;
	int highlight = 0;

	while(1)
	{
		attron(COLOR_PAIR(1));

		for(int i = 0; i < 3; i++) {
			if(i == highlight)
				wattron(menuWin, A_REVERSE);

			mvwprintw(menuWin, i + 2, 1, choices[i]);
			wattroff(menuWin, A_REVERSE);
		}

		wrefresh(menuWin);
	
		choice = wgetch(menuWin);

		if(choice == KEY_UP)
			highlight--;
		else if(choice == KEY_DOWN)
			highlight++;

		if(highlight == 3)
			highlight = 0;

		if(highlight == -1)
			highlight = 3;

		if(choice == 10)
			break;

		attroff(COLOR_PAIR(1));
	}

	return 0;
}

void setupTerminal() {
	initscr();
	noecho();

	if(has_colors()) {
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