/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
    mysql.c - Contains functions that directly interact with the database.
    
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
#include <my_global.h>
#include <mysql.h>
#include "guardian.h"
#include "fileio.h"

extern char db_error[1000];
extern char newHostname[80];
extern int newPort;
extern char newUsername[80];
extern char newPassword[80];
extern dbserver configServer;

// Creates configuration database on the monitoring server.
int createConfigDB() {
    MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		  return 1;

  	if (mysql_real_connect(conn, configServer.hostname, configServer.username, 
  		  configServer.password, NULL, 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);

      	return 1;
  	}  

    char sqlcmd[500];
    strcpy(sqlcmd, "CREATE DATABASE mysql_guardian");
  	
  	if (mysql_query(conn, sqlcmd)) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);

      	writeToLog("Cannot create configuration database.");

      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	writeToSQLLog(sqlcmd);
      	
      	return 1;
  	}

  	writeToSQLLog(sqlcmd);

  	mysql_close(conn);
  	
  	writeToLog("Created configuration database.");

  	return 0;
}

// Creates the servers table on the monitoring server.
int createConfigTables() {
    MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		  return 1;

  	if (mysql_real_connect(conn, configServer.hostname, configServer.username, 
  		  configServer.password, "mysql_guardian", 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	
      	return 1;
  	}  

  	char sqlcmd[500];
    strcpy(sqlcmd, "DROP TABLE IF EXISTS servers");

  	if (mysql_query(conn, sqlcmd)) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	writeToSQLLog(sqlcmd);
      	
      	return 1;
  	}

  	writeToSQLLog(sqlcmd);

  	strcpy(sqlcmd, "CREATE TABLE servers(id INT PRIMARY KEY AUTO_INCREMENT, hostname TEXT, port INT, username TEXT, password TEXT)");

  	if (mysql_query(conn, sqlcmd)) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot create configuration tables.");

      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	writeToSQLLog(sqlcmd);

      	return 1;
  	}

  	writeToLog("Created configuration tables.");
    writeToSQLLog(sqlcmd);

  	mysql_close(conn);
}

// Adds a new server into the servers table on the monitoring server.
int addServerToTable() {
    MYSQL *conn = mysql_init(NULL);

  	if (conn == NULL) 
  		return 1;

  	if (mysql_real_connect(conn, configServer.hostname, configServer.username, 
  		  configServer.password, "mysql_guardian", 0, NULL, 0) == NULL) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot connect to MySQL Server.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	
      	return 1;
  	}

  	char sqlcmd[500];

    int length = snprintf(NULL, 0, "%d", newPort);
    char* strPort = malloc(length + 1);
    snprintf(strPort, length + 1, "%d", newPort);

  	strcpy(sqlcmd, "INSERT INTO servers(hostname, port, username, password) VALUES('");
  	strcat(sqlcmd, newHostname);
  	strcat(sqlcmd, "', ");
  	strcat(sqlcmd, strPort);
  	strcat(sqlcmd, ", '");
  	strcat(sqlcmd, newUsername);
  	strcat(sqlcmd, "', '");
  	strcat(sqlcmd, newPassword);
  	strcat(sqlcmd, "')");

  	free(strPort);

  	if (mysql_query(conn, sqlcmd)) {
      	strcpy(db_error, mysql_error(conn));
      	mysql_close(conn);
      	
      	writeToLog("Cannot add server to database.");
      	
      	char log[200];
      	strcpy(log, "Error: ");
      	strcat(log, db_error);
      	writeToLog(log);
      	writeToSQLLog(sqlcmd);

      	return 1;
  	}

  	writeToLog("Server added to monitoring.");
    writeToSQLLog(sqlcmd);
	
  	mysql_close(conn);
}

// Determines the number of servers in the monitoring database and returns the
// value as an int.
int getMonitoredServers() {
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) 
        return 1;

    if (mysql_real_connect(conn, configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian", 0, NULL, 0) == NULL) {
        strcpy(db_error, mysql_error(conn));
        mysql_close(conn);
        
        writeToLog("Cannot connect to MySQL Server.");
        
        char log[200];
        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);
        
        return 1;
    }

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT COUNT(ID) FROM servers");

    if (mysql_query(conn, sqlcmd)) {
        strcpy(db_error, mysql_error(conn));
        mysql_close(conn);
        
        writeToLog("Cannot determine number of servers in monitoring table.");

        char log[200];
        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);
        writeToSQLLog(sqlcmd);

        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    MYSQL_ROW row;
    row = mysql_fetch_row(result); 

    int rows = atoi(row[0]);

    mysql_free_result(result);

    writeToSQLLog(sqlcmd);

    mysql_close(conn);

    return rows;
}

// Gets and displays the MySQL server version to the screen.
void getDBVersion(char *dbversion) {
    strcpy(dbversion, mysql_get_client_info());
}