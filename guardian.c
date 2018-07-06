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

	int height = 10;
	int width = 60;
	int starty = 5;
	int startx = 10;

	WINDOW *menuWin = newwin(height, width, starty, startx);
	refresh();

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