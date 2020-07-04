/*
    Copyright (c) 2018-19 - Stephen Planck and Alistair Packer
    
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
#include <mysql.h>
#include "utility.h"
#include "guardian.h"
#include "fileio.h"
#include "mysqlgd.h"
#include "mysql.h"

extern char db_error[1000];
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
        
        char *log = malloc(200);

        strcpy(log, "Error: ");
        strcat(log, db_error);
        writeToLog(log);

        free(log);

        return NULL;
    }  

    return conn;
}

// Accepts a pointer to a MYSQL connection, a SQL statement to execute against the
// database, and an error message to print to the log on failure. Executes the
// provided SQL statement and returns 0 on success or 1 on failure.
int executeQuery(MYSQL *conn, char *sql, char *errorMsg) {
    if(mysql_query(conn, sql)) {
        handleDBError(conn, errorMsg, sql);
        
        return 1;
    }
    else {
        writeToSQLLog(sql);

        return 0;
    }
}

// Handles database connection errors. Accepts a database connection, an error message,
// and a SQL statement as parameters. Closes the connection, writes the error message to
// the mysql-guardian.log file and writes the SQL statement to the mysql-guardian-sql.log
// file.
void handleDBError(MYSQL *conn, char *errorMsg, char *sql) {
    strcpy(db_error, mysql_error(conn));
    mysql_close(conn);
        
    if(errorMsg != NULL)
        writeToLog(errorMsg);

    if(sql != NULL)
        writeToSQLLog(sql);

    char *log = malloc(200);

    strcpy(log, "Error: ");
    strcat(log, db_error);

    writeToLog(log);

    free(log);
}

// Creates configuration database on the monitoring server.
int createConfigDB() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, NULL);

    if(conn == NULL)
        return 1;

    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);
    int errorCode = 0;

    strcpy(sqlcmd, "DROP DATABASE IF EXISTS mysql_guardian");
    strcpy(errorMsg, "Cannot create configuration database.");

    if(executeQuery(conn, sqlcmd, errorMsg) == 1) 
        errorCode = 1;

    strcpy(sqlcmd, "CREATE DATABASE mysql_guardian");
  	
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) 
        errorCode = 1;

  	mysql_close(conn);
  	
  	writeToLog("Created configuration database.");

    free(sqlcmd);
    free(errorMsg);

  	return errorCode;
}

int enableSlowQueryLogging() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, NULL);

    if(conn == NULL)
        return 1;

    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);
    int errorCode = 0;

    strcpy(sqlcmd, "SET GLOBAL slow_query_log = 1;");
    strcpy(errorMsg, "Cannot enable slow query logging.");
  	
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) 
        errorCode = 1;

	strcpy(sqlcmd, "SET GLOBAL log_output = \"TABLE\";");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1) 
        errorCode = 1;

	strcpy(sqlcmd, "SET GLOBAL long_query_time = 5;");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        errorCode = 1;

  	mysql_close(conn);
  	
  	writeToLog("Enabled slow query logging.");
    
    free(sqlcmd);
    free(errorMsg);

  	return errorCode;
}

// Creates the servers table on the monitoring server.
int createConfigTables() {
	dropOldTables();

    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");
    
    if(conn == NULL)
        return 1;
  	
    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);

  	strcpy(sqlcmd, "CREATE TABLE servers(id INT PRIMARY KEY AUTO_INCREMENT, ");
	strcat(sqlcmd, "hostname TEXT NOT NULL, port INT NOT NULL, username TEXT NOT NULL, ");
	strcat(sqlcmd, "password TEXT NOT NULL)");
    strcpy(errorMsg, "Cannot create server table in monitoring database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

    strcpy(sqlcmd, "CREATE TABLE users(id INT PRIMARY KEY AUTO_INCREMENT, username TEXT, ");
	strcat(sqlcmd, "password TEXT, email TEXT, admin BOOLEAN)");
    strcpy(errorMsg, "Cannot create users table in monitoring database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE server_checks(id INT NOT NULL, online_check INT, ");
	strcat(sqlcmd, "database_server_check INT, database_check INT, ");
	strcat(sqlcmd, "integrity_check INT, slow_query_monitoring INT, ");
	strcat(sqlcmd, "database_backup INT)");
	strcpy(errorMsg, "Cannot create server_checks table in monitoring database.");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE check_results(id INT PRIMARY KEY AUTO_INCREMENT, ");
	strcat(sqlcmd, "server_id INT NOT NULL, ");
	strcat(sqlcmd, "time timestamp NOT NULL, ");
	strcat(sqlcmd, "check_type INT NOT NULL, check_result INT NOT NULL, db_name TEXT)");
	strcpy(errorMsg, "Cannot create check_results table in monitoring database.");
    
	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE backup_history(id INT PRIMARY KEY AUTO_INCREMENT, ");
	strcat(sqlcmd, "server_id INT NOT NULL, time timestamp NOT NULL, ");
	strcat(sqlcmd, "db_name TEXT, filename TEXT)");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE tasks(id INT PRIMARY KEY AUTO_INCREMENT, task_id INT NOT NULL, ");
	strcat(sqlcmd, "server_id INT, db_name TEXT, param TEXT, status INT NOT NULL, ");
	strcat(sqlcmd, "time TIMESTAMP NOT NULL)");
	strcpy(errorMsg, "Cannot create taska table in monitoring database.");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE check_result_errors(id INT NOT NULL, error_msg TEXT)");
	strcpy(errorMsg, "Cannot create check_result_errors table in monitoring database.");
    
	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

	strcpy(sqlcmd, "CREATE TABLE health_checks(time TIMESTAMP NOT NULL)");
	strcpy(errorMsg, "Cannot create health_check table in monitoring database.");
 
	if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

    strcpy(sqlcmd, "INSERT INTO users(username, password, admin) VALUES('admin','admin',true)");
    strcpy(errorMsg, "Cannot create admin user account.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) {
        free(sqlcmd);
        free(errorMsg);
        
        return 1;
    }

  	writeToLog("Created configuration tables.");

  	mysql_close(conn);
    free(sqlcmd);
    free(errorMsg);
}

int dropOldTables() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");
    
    if(conn == NULL)
        return 1;
  	
    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);
    int errorCode = 0;
     
    strcpy(sqlcmd, "DROP TABLE IF EXISTS servers");

    if(executeQuery(conn, sqlcmd, NULL) == 1) 
        errorCode = 1;

    strcpy(sqlcmd, "DROP TABLE IF EXISTS users");

    if(executeQuery(conn, sqlcmd, NULL) == 1) 
        errorCode = 1;

	strcpy(sqlcmd, "DROP TABLE IF EXISTS check_results");

	if(executeQuery(conn, sqlcmd, NULL) == 1)
        errorCode = 1;

	strcpy(sqlcmd, "DROP TABLE IF EXISTS check_result_errors");

	if(executeQuery(conn, sqlcmd, NULL) == 1) 
        errorCode = 1;

	strcpy(sqlcmd, "DROP TABLE IF EXISTS tasks");

	if(executeQuery(conn, sqlcmd, NULL) == 1) 
        errorCode = 1;

  	mysql_close(conn);

    free(sqlcmd);
    free(errorMsg);

	return errorCode;
}	

int writeBackupHistory(int server_id, char *dbname, char *filename) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);
    int errorCode = 0;

    int length = snprintf(NULL, 0, "%d", server_id);
    char* strid = malloc(length + 1);
    snprintf(strid, length + 1, "%d", server_id);

	strcpy(sqlcmd, "INSERT INTO backup_history(server_id, db_name, filename) ");
	strcat(sqlcmd, "VALUES(");
	strcat(sqlcmd, strid);
	strcat(sqlcmd, ", '");
	strcat(sqlcmd, dbname);
	strcat(sqlcmd, "', '");
	strcat(sqlcmd, filename);
	strcat(sqlcmd, "')");

	free(strid);

    strcpy(errorMsg, "Cannot record database backup result.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1) 
        errorCode = 1;

  	mysql_close(conn);
    free(sqlcmd).
    free(errorMsg);

    return errorCode;
}

int writeCheckResult(int id, int type, int result, char *dbname, char *errorText) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

    char *sqlcmd = malloc(500);
    char *errorMsg = malloc(100);
    int errorCode = 0;

    int length = snprintf(NULL, 0, "%d", id);
    char* strid = malloc(length + 1);
    snprintf(strid, length + 1, "%d", id);

    length = snprintf(NULL, 0, "%d", type);
    char* strtype = malloc(length + 1);
    snprintf(strtype, length + 1, "%d", type);

    length = snprintf(NULL, 0, "%d", result);
    char* strresult = malloc(length + 1);
    snprintf(strresult, length + 1, "%d", result);

  	strcpy(sqlcmd, "INSERT INTO check_results(server_id, check_type, check_result, db_name) ");
	strcat(sqlcmd, "VALUES(");
	strcat(sqlcmd, strid);
	strcat(sqlcmd, ", ");
	strcat(sqlcmd, strtype);
	strcat(sqlcmd, ", ");
	strcat(sqlcmd, strresult);

	if(dbname) {
		strcat(sqlcmd, ", '");
		strcat(sqlcmd, dbname); 
  		strcat(sqlcmd, "')");
	}	
	else {
		strcat(sqlcmd, ", NULL)");
	}
	
	free(strid);
	free(strtype);
	free(strresult);

    strcpy(errorMsg, "Cannot record check result to database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

	if(errorText != NULL) {
		strcpy(sqlcmd, "SELECT LAST_INSERT_ID()");

		if(executeQuery(conn, sqlcmd, errorMsg) == 1)
			return 1;

		char c = 39;
		remove_char_from_string(c, errorText);
	
		MYSQL_RES *result = mysql_store_result(conn);
    	MYSQL_ROW row = mysql_fetch_row(result); 

    	int row_id = atoi(row[0]);

		length = snprintf(NULL, 0, "%d", row_id);
    	char* strid = malloc(length + 1);
    	snprintf(strid, length + 1, "%d", row_id);

    	mysql_free_result(result);

		strcpy(sqlcmd, "INSERT INTO check_result_errors(id, error_msg) VALUES("); 
		strcat(sqlcmd, strid);
		strcat(sqlcmd, ", '");
		strcat(sqlcmd, errorText);
		strcat(sqlcmd, "')");

		free(strid);

		if(executeQuery(conn, sqlcmd, errorMsg) == 1)
			errorCode = 1;
	}

  	mysql_close(conn);

    free(sqlcmd);
    free(errorMsg);

    return errorCode;
}

// Adds a new server into the servers table on the monitoring server.
int addServerToTable(char *hostname, int port, char *username, char *password) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

  	char sqlcmd[500];

    int length = snprintf(NULL, 0, "%d", port);
    char* strPort = malloc(length + 1);
    snprintf(strPort, length + 1, "%d", port);

  	strcpy(sqlcmd, "INSERT INTO servers(hostname, port, username, password) VALUES('");
  	strcat(sqlcmd, hostname);
  	strcat(sqlcmd, "', ");
  	strcat(sqlcmd, strPort);
  	strcat(sqlcmd, ", '");
  	strcat(sqlcmd, username);
  	strcat(sqlcmd, "', '");
  	strcat(sqlcmd, password);
  	strcat(sqlcmd, "')");

  	free(strPort);

	char errorMsg[100];
    strcpy(errorMsg, "Cannot add server to database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

	strcpy(sqlcmd, "SELECT LAST_INSERT_ID()");

	if(executeQuery(conn, sqlcmd, errorMsg) == 1)
		return 1;

	MYSQL_RES *result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result); 

    int row_id = atoi(row[0]);

	length = snprintf(NULL, 0, "%d", row_id);
    char* strid = malloc(length + 1);
    snprintf(strid, length + 1, "%d", row_id);

    mysql_free_result(result);

	strcpy(sqlcmd, "INSERT INTO server_checks (id, online_check, ");
	strcat(sqlcmd, "database_server_check, database_check, ");
	strcat(sqlcmd, "integrity_check, slow_query_monitoring, ");
	strcat(sqlcmd, "database_backup) ");
	strcat(sqlcmd, "VALUES("); 
	strcat(sqlcmd, strid);
	strcat(sqlcmd, ", 1, 1, 1, 1, 1, 1)");

	free(strid);

	if(executeQuery(conn, sqlcmd, errorMsg) == 1)
		return 1;

  	writeToLog("Server added to monitoring.");
	
  	mysql_close(conn);
}

int includeExcludeFromTable(int server_id, int exclude) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

  	char sqlcmd[500];
	char chk_switch[2];

	if(exclude == 1)
		strcpy(chk_switch, "1");
	else
		strcpy(chk_switch, "0");

    int length = snprintf(NULL, 0, "%d", server_id);
    char* strid = malloc(length + 1);
    snprintf(strid, length + 1, "%d", server_id);

  	strcpy(sqlcmd, "UPDATE server_checks ");
	strcat(sqlcmd, "SET online_check = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, ", database_server_check = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, ", database_check = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, ", integrity_check = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, ", slow_query_monitoring = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, ", database_backup = ");
	strcat(sqlcmd, chk_switch);
	strcat(sqlcmd, " WHERE id = ");
  	strcat(sqlcmd, strid);

	char errorMsg[100];
    strcpy(errorMsg, "Cannot exclude server from checks.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

	writeToLog("Server excluded from monitoring.");

	free(strid);
	
  	mysql_close(conn);

	return 0;
}

int removeServerFromTable(int server_id) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return 1;

  	char sqlcmd[500];

    int length = snprintf(NULL, 0, "%d", server_id);
    char* strid = malloc(length + 1);
    snprintf(strid, length + 1, "%d", server_id);

  	strcpy(sqlcmd, "DELETE FROM servers WHERE id = ");
  	strcat(sqlcmd, strid);

	char errorMsg[100];
    strcpy(errorMsg, "Cannot delete server to database.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

	strcpy(sqlcmd, "DELETE FROM server_checks WHERE id = ");
	strcat(sqlcmd, strid);

	if(executeQuery(conn, sqlcmd, errorMsg) == 1)
		return 1;

	writeToLog("Server removed from monitoring.");

	free(strid);
	
  	mysql_close(conn);

	return 0;
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
    MYSQL_ROW row = mysql_fetch_row(result); 

    int rows = atoi(row[0]);

    mysql_free_result(result);
    mysql_close(conn);

    return rows;
}

int getNextTask(struct mytask *task) {
	MYSQL *conn = connectDB(configServer.hostname, configServer.username,
		configServer.password, "mysql_guardian");

	if(conn == NULL)
		return -1;

	char sqlcmd[500];
	strcpy(sqlcmd, "SELECT id, task_id, server_id, db_name, param, status ");
	strcat(sqlcmd, "FROM tasks where status = 1 ORDER BY time ASC LIMIT 1");
	
    char errorMsg[100];
    strcpy(errorMsg, "Cannot retrieve next pending task from monitoring database.");

    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, NULL);

        return -1;
    }

    int num_rows = mysql_num_rows(result);
    
	if(num_rows > 0) {
    	MYSQL_ROW row = mysql_fetch_row(result);
    
	    task->id = atoi(row[0]);
		task->task_id = atoi(row[1]);
		task->server_id = atoi(row[2]);
		task->dbname = row[3];
		task->param = row[4];
		task->status = atoi(row[5]);
	}
 	
	mysql_free_result(result);
    mysql_close(conn);

	if(num_rows > 0)
		return 1;
	else
		return 0;
}

int updateTaskStatus(struct mytask *task) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, "mysql_guardian");
    
    if(conn == NULL)
        return -1;

	int id = task->id;
	int status = task->status;
	int length = 0;

	length = snprintf(NULL, 0, "%d", id);
    char* str_id = malloc(length + 1);
    snprintf(str_id, length + 1, "%d", id);

	length = snprintf(NULL, 0, "%d", status);
    char* str_status = malloc(length + 1);
    snprintf(str_status, length + 1, "%d", status);

    char sqlcmd[500];
    strcpy(sqlcmd, "UPDATE tasks SET status = ");
	strcat(sqlcmd, str_status);
	strcat(sqlcmd, " WHERE id = ");
	strcat(sqlcmd, str_id);

    char errorMsg[100];
    strcpy(errorMsg, "Cannot update status for task ID ");
	strcat(errorMsg, str_id);
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

	mysql_close(conn);

	return 0;
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
    strcpy(sqlcmd, "SELECT a.id, a.hostname, a.port, a.username, a.password, ");
	strcat(sqlcmd, "b.online_check, b.database_server_check, b.database_check, ");
	strcat(sqlcmd, "b.integrity_check, b.slow_query_monitoring, ");
	strcat(sqlcmd, "b.database_backup ");
	strcat(sqlcmd, "FROM servers a, server_checks b ");
	strcat(sqlcmd, "WHERE a.id = b.id;");

    char errorMsg[100];
    strcpy(errorMsg, "Cannot retrieve list of servers in monitoring.");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, NULL);

        return -1;
    }

    int num_fields = mysql_num_fields(result);
    
    MYSQL_ROW row;

    while (row = mysql_fetch_row(result)) { 
        int id = atoi(row[0]);
        char *hostname = row[1];
        int port = atoi(row[2]);
        char *username = row[3];
        char *password = row[4];
		int online_check = atoi(row[5]);
		int database_server_check = atoi(row[6]);
		int database_check = atoi(row[7]);
		int integrity_check = atoi(row[8]);
		int slow_query = atoi(row[9]);
		int database_backup = atoi(row[10]);

        addServerNode(id, hostname, port, username, password, online_check, database_server_check,
			database_check, integrity_check, slow_query, database_backup);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}

// Populates a server's database linked list. Accepts the target server struct as a
// parameter. Returns 0 on success and 1 on failure.
int populateServerDatabasesList(struct myserver *svr) {
    MYSQL *conn = connectDB(svr->hostname, svr->username, svr->password, NULL);

    if(conn == NULL)
        return 1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SHOW DATABASES");

    char errorMsg[100];
    strcpy(errorMsg, "Cannot retrieve list of databases for server ");
    strcat(errorMsg, svr->hostname);
    strcat(errorMsg, ".");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, sqlcmd);

        return 1;
    }

    int num_fields = mysql_num_fields(result);
    
    MYSQL_ROW row;

    while (row = mysql_fetch_row(result)) { 
        char *dbname = row[0];
        
		addDatabaseNode(svr, dbname);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}

// Populates a database's tables linked list. Accepts the target server and database struct 
// as parameters. Returns 0 on success and 1 on failure.
int populateDatabaseTablesList(struct myserver *svr, struct mydatabase *db) {
    MYSQL *conn = connectDB(svr->hostname, svr->username, svr->password, db->dbname);

    if(conn == NULL)
        return 1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SHOW TABLES");

    char errorMsg[100];
    strcpy(errorMsg, "Cannot retrieve list of tables for database: ");
	strcat(errorMsg, db->dbname);
	strcat(errorMsg, " on ");
    strcat(errorMsg, svr->hostname);
    strcat(errorMsg, ".");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, sqlcmd);

        return 1;
    }

    int num_fields = mysql_num_fields(result);
    
    MYSQL_ROW row;

    while (row = mysql_fetch_row(result)) { 
        char *tblname = row[0];
        
		addTableNode(db, tblname);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}

int checkDatabase(struct myserver *svr, struct mydatabase *db, char *db_err) {
    MYSQL *conn = connectDB(svr->hostname, svr->username, svr->password, db->dbname);

    if(conn == NULL) {
		writeToLog(db_error);
		strcpy(db_err, db_error);
        return 1;
	}
	else {
		mysql_close(conn);
		return 0;
	}
}

int checkTable(struct myserver *svr, struct mydatabase *db, struct mytable *tbl) {
    MYSQL *conn = connectDB(svr->hostname, svr->username, svr->password, db->dbname);

    if(conn == NULL)
        return -1;

    char sqlcmd[500];
    strcpy(sqlcmd, "CHECK TABLE ");
	strcat(sqlcmd, tbl->tblname);

    char errorMsg[100];
    strcpy(errorMsg, "Cannot open database to check table: ");
	strcat(errorMsg, tbl->tblname);
	strcat(errorMsg, " in ");
	strcat(errorMsg, db->dbname);
	strcat(errorMsg, " on ");
    strcat(errorMsg, svr->hostname);
    strcat(errorMsg, ".");
    
    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, sqlcmd);

        return -1;
    }

    int num_fields = mysql_num_fields(result);
    
    MYSQL_ROW row;

	int checkresult = 1;

    while (row = mysql_fetch_row(result)) { 
		if(strcmp(row[2], "status") == 0) {
			if(strcmp(row[3], "OK") == 0) {
				checkresult = 0;
			}
		}

		if(strcmp(row[2], "note") == 0) {
			if(strcmp(row[3], "The storage engine for the table doesn't support check") == 0) {
				checkresult = 2;
			}	
		}
    }

    mysql_free_result(result);
    mysql_close(conn);

	if(checkresult != 0) {
		strcpy(errorMsg, "Table check failed for ");
		strcat(errorMsg, tbl->tblname);
		strcat(errorMsg, " in ");
		strcat(errorMsg, db->dbname);
		strcat(errorMsg, " on ");
		strcat(errorMsg, svr->hostname);
    	strcat(errorMsg, ".");
   
		writeToLog(errorMsg); 
	}
	
    return checkresult;
}

int checkRecentIntegrityCheck(int server_id, char *dbname) {
    // Needs to be implemented.
    return 0;
}

int checkRecentBackup(int server_id, char *dbname) {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username,
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return -1;

	int length = snprintf(NULL, 0, "%d", server_id);
    char* str_server_id = malloc(length + 1);
    snprintf(str_server_id, length + 1, "%d", server_id);

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT * FROM backup_history WHERE server_id = ");
    strcat(sqlcmd, str_server_id);
    strcat(sqlcmd, " AND time > SUBDATE(NOW(), 2) ");
    strcat(sqlcmd, " AND db_name = '");
    strcat(sqlcmd, dbname);
    strcat(sqlcmd, "';");
    
    char errorMsg[100];
    strcpy(errorMsg, "Cannot determine if recent backups have been taken.");

    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, NULL);

        return -1;
    }

    int num_rows = mysql_num_rows(result);
    
    mysql_free_result(result);
    mysql_close(conn);

    if(num_rows > 0)
        return 1;
    else
        return 0;
}

int timeForHealthCheck() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username,
        configServer.password, "mysql_guardian");

    if(conn == NULL)
        return -1;

    char sqlcmd[500];
    strcpy(sqlcmd, "SELECT * FROM health_checks WHERE time >= CURDATE();");
    
    char errorMsg[100];
    strcpy(errorMsg, "Cannot determine if health check is due.");

    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return -1;

    MYSQL_RES *result = mysql_store_result(conn);

    if(result == NULL) {
        handleDBError(conn, errorMsg, NULL);

        return -1;
    }

    int num_rows = mysql_num_rows(result);
    
    mysql_free_result(result);
    mysql_close(conn);

    if(num_rows > 0)
        return 1;
    else
        return 0;
}

int recordHealthCheck() {
    MYSQL *conn = connectDB(configServer.hostname, configServer.username, 
        configServer.password, NULL);

    if(conn == NULL)
        return 1;

    char sqlcmd[50];
    char errorMsg[50];

    strcpy(sqlcmd, "INSERT INTO health_checks VALUES();");
    strcpy(errorMsg, "Cannot write health check to database.");

    if(executeQuery(conn, sqlcmd, errorMsg) == 1)
        return 1;

    mysql_close(conn);
    
    writeToLog("Health check written to database.");

    return 0;
}

// Accepts a usernamd and password as parameters and attempts to authenticate these
// credentials against the monitoring database's users table. Returns 0 if authentication
// was successful, otherwise returns 1.
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
    MYSQL_ROW row = mysql_fetch_row(result); 

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
