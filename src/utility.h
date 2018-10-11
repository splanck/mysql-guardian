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

#include <netinet/ip_icmp.h>

#define PING_PKT_S 64
#define PORT_NO 0 
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1 

struct myserver {
    int id;
    char hostname[80];
    int port;
    char username[25];
    char password[25];
    struct mydatabase *firstDatabase;
    struct mydatabase *lastDatabase;
    struct myserver *next;
};

struct mydatabase {
    char dbname[50];
    struct mytable *firstTable;
    struct mytable *lastTable;
    struct mydatabase *next;
};

struct mytable {
    char tblname[50];
    struct mytable *next;
};

struct ping_packet {
    struct icmphdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmphdr)];
};

void addServerNode(int id, char *hostname, int port, char *username, char *password);
void addDatabaseNode(struct myserver *svr, char *dbname);
void addTableNode(struct mydatabase *db, char *tblname);
void remove_char_from_string(char c, char *str);
unsigned short checkSum(void *buffer, int len);
char* reverseDNSLookup(char *ip_addr);
int pingServer(char *hostname);
void ucase(char str[]);
