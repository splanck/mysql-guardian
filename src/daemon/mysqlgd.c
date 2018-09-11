/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer
    
    mysqlgd.c - Main source file for the mysqlgd daemon process.
    
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

int main() {
	pid_t pid = fork();

	if(pid < 0)
		exit(EXIT_FAILURE);

	if(pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	openlog("mysqlgd", 0, LOG_DAEMON);
	syslog(LOG_INFO, "%s", "MySQL Guardian daemon starting...");

	pid_t sid = setsid();

	if(sid < 0)
		exit(EXIT_FAILURE);

	if((chdir("/")) < 0)
		exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	int heartbeat = 0;

	while(1) {
		syslog(LOG_INFO, "%d", heartbeat);
		heartbeat++;
		sleep(30);
	}
}
