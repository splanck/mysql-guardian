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
int createConfigTables();
int addServerToTable();
int getMonitoredServersCount();
int populateMonitoredServersList();
int populateServerDatabasesList(struct myserver *);
int authenticateUser(char *username, char *password);
void getDBVersion(char *dbversion);
