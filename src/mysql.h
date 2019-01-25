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

#include <mysql.h>

MYSQL* connectDB(char *hostname, char *username, char *password, char *database);
int executeQuery(MYSQL *conn, char *sql, char *errorMsg);
void handleDBError(MYSQL *conn, char *errorMsg, char *sql);
int createConfigDB();
int enableSlowQueryLogging();
int dropOldTables();
int createConfigTables();
int writeBackupHistory(int server_id, char *dbname, char *filename);
int writeCheckResult(int id, int type, int result, char *dbname, char *errorText);
int addServerToTable(char *hostname, int port, char *username, char *password);
int getMonitoredServersCount();
int getNextTask(struct mytask *task);
int updateTaskStatus(struct mytask *task);
int populateMonitoredServersList();
int populateServerDatabasesList(struct myserver *);
int populateDatabaseTablesList(struct myserver *, struct mydatabase *);
int checkDatabase(struct myserver *svr, struct mydatabase *db, char *db_err);
int checkTable(struct myserver *svr, struct mydatabase *db, struct mytable *tbl);
int authenticateUser(char *username, char *password);
void getDBVersion(char *dbversion);
