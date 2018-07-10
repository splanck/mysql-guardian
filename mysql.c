#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <my_global.h>
#include <mysql.h>

extern char db_hostname[80];
extern char db_username[25];
extern char db_password[25];

void getDBInfo() {
	mvprintw(1, 0, "%s", mysql_get_client_info());
}

void showConfig() {
	mvprintw(2, 0, "Hostname: %s", db_hostname);
	mvprintw(3, 0, "Username: %s", db_username);
	mvprintw(4, 0, "Password: %s", db_password);
}