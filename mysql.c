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
#include "utility.h"

extern char db_error[1000];
extern char newHostname[80];
extern int newPort;
extern char newUsername[80];
extern char newPassword[80];
extern dbserver configServer;

// Accepts a hostname, username, password, and database name and returns a pointer 
// to a new MYSQL connnection. Pass in a NULL value for the database name if you
// don't want to connect to a specific database. Function returns NULL if connection
// was unsuccessful.
MYSQL* connectDB(char *hostname, char *username, char *password, char *database) {
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) 
        return NULL;

    if (mysql_real_connect(conn, hostname, username, password, database, 0, NULL, 0) == NULL) {
        strcpy(db_error, mysql_error(conn));
        mysql_close(conn);
        
        writeToLog("Cannot connect to MySQL Server.");
        
        char log[200];
        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);

        return NULL;
    }  

    return conn;
}

// Accepts a pointer to a MYSQL connection, a SQL statement to execute against the
// database, and an error message to print to the log on failure. Executes the
// provided SQL statement and returns 0 on success or 1 on failure.
int executeQuery(MYSQL *conn, char *sql, char *errorMsg) {
    if (mysql_query(conn, sql)) {
        strcpy(db_error, mysql_error(conn));
        mysql_close(conn);

        if(errorMsg != NULL)
            writeToLog(errorMsg);

        char log[200];
        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);
        writeToSQLLog(sql);
        
        return 1;
    }
    else {
        writeToSQLLog(sql);

        return 0;
    }
}

// Creates configuration database on the monitoring server.
int createConfigDB() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, NULL);

    if(conn == NULL)
        return 1;

    char sqlcmd[500];
    char errorMsg[100];

    strcpy(sqlcmd, "CREATE DATABASE mysql_guardian");
    strcpy(errorMsg, "Cannot create configuration database.");
  	
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

  	mysql_close(conn);
  	
  	writeToLog("Created configuration database.");

  	return 0;
}

// Creates the servers table on the monitoring server.
int createConfigTables() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");
    
    if(conn == NULL)
        return 1;
  	
    char sqlcmd[500];
    char errorMsg[100];
    
    strcpy(sqlcmd, "DROP TABLE IF EXISTS servers");

    if(executeQuery(conn, sqlcmd, NULL) == 1)
        return 1;

    strcpy(sqlcmd, "DROP TABLE IF EXISTS users");

    if(executeQuery(conn, sqlcmd, NULL) == 1)
        return 1;

  	strcpy(sqlcmd, "CREATE TABLE servers(id INT PRIMARY KEY AUTO_INCREMENT, hostname TEXT, port INT, username TEXT, password TEXT)");
    strcpy(errorMsg, "Cannot create server table in monitoring database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    strcpy(sqlcmd, "CREATE TABLE users(id INT PRIMARY KEY AUTO_INCREMENT, username TEXT, password TEXT, admin BOOLEAN)");
    strcpy(errorMsg, "Cannot create users table in monitoring database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    strcpy(sqlcmd, "INSERT INTO users(username, password, admin) VALUES('admin','admin',true')");
    strcpy(errorMsg, "Cannot create admin user account.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

  	writeToLog("Created configuration tables.");

  	mysql_close(conn);
}

// Adds a new server into the servers table on the monitoring server.
int addServerToTable() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

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

    char errorMsg[100];
    strcpy(errorMsg, "Cannot add server to database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

  	writeToLog("Server added to monitoring.");
	
  	mysql_close(conn);
}

// Determines the number of servers in the monitoring database and returns the
// value as an int.
int getMonitoredServersCount() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT COUNT(ID) FROM servers");
    
    char errorMsg[100];
    strcpy(errorMsg, "Cannot determine number of servers in monitoring table.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    MYSQL_RES *result = mysql_store_result(conn);

    MYSQL_ROW row;
    row = mysql_fetch_row(result); 

    int rows = atoi(row[0]);

    mysql_free_result(result);
    mysql_close(conn);

    return rows;
}

// Retrieves a list of servers in monitoring from the servers table in the 
// monitoring database and populates a linked list by calling the addServerNode function.
// Return 0 if successful and -1 if an error occurs.
int populateMonitoredServersList() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");
    
    if(conn == NULL)
        return -1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT id, hostname, port, username, password FROM servers");

    char errorMsg[100];
    strcpy(errorMsg, "Cannot retrieve list of servers in monitoring.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        strcpy(db_error, mysql_error(conn));
        mysql_close(conn);
        
        writeToLog("Cannot retrieve list of servers in monitoring.");

        char log[200];
        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);

        return -1;
    }

    int num_fields = mysql_num_fields(result);
    
    MYSQL_ROW row;

    while (row = mysql_fetch_row(result)) 
    { 
        int id = atoi(row[0]);
        char *hostname = row[1];
        int port = atoi(row[2]);
        char *username = row[3];
        char *password = row[4];

        addServerNode(id, hostname, port, username, password);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}

int authenticateUser(char *username, char *password) {
    int authenticated = 0;

    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT COUNT(ID) FROM users WHERE username = '");
    strcat(sqlcmd, username);
    strcat(sqlcmd, "' AND password = '");
    strcat(sqlcmd, password);
    strcat(sqlcmd, "'");
    
    char errorMsg[100];
    strcpy(errorMsg, "Cannot authenticate user login.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    MYSQL_RES *result = mysql_store_result(conn);

    MYSQL_ROW row;
    row = mysql_fetch_row(result); 

    if(atoi(row[0]) > 0)
        authenticated = 1;

    mysql_free_result(result);
    mysql_close(conn);

    return authenticated;
}

// Gets and displays the MySQL server version to the screen.
void getDBVersion(char *dbversion) {
    strcpy(dbversion, mysql_get_client_info());
}