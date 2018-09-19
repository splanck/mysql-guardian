/*
	Copyright (c) 2018 - Stephen Planck and Alistair Packer

	fileio.c - Contains functions that read and write to the filesystem.

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
#include <time.h>
#include "mysqlgd.h"
#include "utility.h"

// Reads the .mysql-guardian_rc configuration file and stores its valies 
// into global variables.
int readConfig(char *hostname, char *username, char *password) {
	char host_buffer[1000];
	char username_buffer[1000];
	char password_buffer[1000];
	
	FILE *configFile;

	configFile = fopen("/etc/mysqlgd.conf", "r");

	if (!configFile)
		return 1;

	fgets(host_buffer, 1000, configFile);
	fgets(username_buffer, 1000, configFile);
	fgets(password_buffer, 1000, configFile);

	fclose(configFile);

  	char *str;

  	str = strtok(host_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(hostname, str);

	str = strtok(username_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(username, str);

	str = strtok(password_buffer, " ");
  	str = strtok(NULL, " ");
  	remove_char_from_string('\n', str);
	strcpy(password, str);

  	return 0;
}
