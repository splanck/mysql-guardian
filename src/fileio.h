/*
	Copyright (c) 2018-20 - Stephen Planck and Alistair Packer

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

int writeMailFile(char emailMsg[250], char filename[80]);
int writeLog(char logEntry[200], char filename[80]);
int writeToLog(char logEntry[200]);
int writeToSQLLog(char logEntry[200]);
int createConfigFile(char *hostname, char *username, char *password);
int readConfig();
void processConfigKeyValuePair(char *k, char *v);
