#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <my_global.h>
#include <mysql.h>
#include "fileio.h"

extern char db_hostname[80];
extern char db_username[80];
extern char db_password[80];
extern char db_error[1000];
extern char newHostname[80];
extern int newPort;
extern char newUsername[80];
extern char newPassword[80];

int createConfigDB() {
	MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		return 1;

  	if (mysql_real_connect(conn, db_hostname, db_username, db_password, 
		NULL, 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);

      	return 1;
  	}  

  	if (mysql_query(conn, "CREATE DATABASE mysql_guardian")) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);

      	writeToLog("Cannot create configuration database.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	
      	return 1;
  	}

  	mysql_close(conn);
  	
  	writeToLog("Created configuration database.");

  	return 0;
}

int createConfigTables() {
	MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		return 1;

  	if (mysql_real_connect(conn, db_hostname, db_username, db_password, 
		"mysql_guardian", 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	
      	return 1;
  	}  

  	if (mysql_query(conn, "DROP TABLE IF EXISTS Servers")) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	return 1;
  	}

  	if (mysql_query(conn, "CREATE TABLE Servers(Id INT, Hostname TEXT, Port INT, Username TEXT, Password TEXT)")) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot create configuration tables.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);

      	return 1;
  	}

  	writeToLog("Created configuration tables.");

  	mysql_close(conn);
}

int addServerToTable() {
	MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		return 1;

  	if (mysql_real_connect(conn, db_hostname, db_username, db_password, 
		"mysql_guardian", 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	
      	return 1;
  	}

  	char sqlInsert[500];

	int length = snprintf(NULL, 0, "%d", newPort);
	char* strPort = malloc(length + 1);
	snprintf(strPort, length + 1, "%d", newPort);

  	strcpy(sqlInsert, "INSERT INTO Servers VALUES(1, '");
  	strcat(sqlInsert, newHostname);
  	strcat(sqlInsert, "', ");
  	strcat(sqlInsert, strPort);
  	strcat(sqlInsert, ", '");
  	strcat(sqlInsert, newUsername);
  	strcat(sqlInsert, "', '");
  	strcat(sqlInsert, newPassword);
  	strcat(sqlInsert, "')");

  	free(strPort);

  	if (mysql_query(conn, sqlInsert)) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot add server to database.");
      	writeToLog(sqlInsert);
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);

      	return 1;
  	}

  	writeToLog("Server added to monitoring.");

  	mysql_close(conn);
}

void getDBInfo() {
	mvprintw(1, 0, "MySQL Server Version: %s", mysql_get_client_info());

	getch();
}

void showConfig() {
	mvprintw(2, 0, "Hostname: %s", db_hostname);
	mvprintw(3, 0, "Username: %s", db_username);
	mvprintw(4, 0, "Password: %s", db_password);

	getch();
}