#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <my_global.h>
#include <mysql.h>

void getDBInfo() {
	mvprintw(1, 0, "%s", mysql_get_client_info());
}