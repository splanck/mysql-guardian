/*
    Copyright (c) 2018-19 - Stephen Planck and Alistair Packer
    
    healthcheck.h - Header file for the healthcheck.c source file.
    
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

int isHealthCheckTime();
int hcServerOnline(struct myhealthcheck *pHC);
int hcDatabaseServerOnline(struct myhealthcheck *pHC, char *db_err, struct myserver *pServer);
