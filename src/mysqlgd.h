/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
    mysqlgd.h - Header file for the mysqlgd.c source file.
    
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

typedef enum {
	c_serverOnline = 0,
	c_databaseServer = 1,
	c_databaseOnline = 2,
	c_integrityCheck = 3,
	c_databaseBackup = 4,
	c_slowQuery = 5
} checkType_t;

void startDaemon();
void getConfigd();
void setupTimers();
int initDaemon();
int doHealthCheck();
int isHealthCheckTime();
int doServerCheck();
int doDatabaseServerCheck();
int doDatabaseCheck();
int doIntegrityCheck();
int doSlowQueryCheck();
int doTaskCheck();
int doDatabaseBackups();
void checkFailure(struct myserver *svr, struct mydatabase *db, struct mytable *tbl, checkType_t chk, char *error, char *error_desc);
void sig_handler();
