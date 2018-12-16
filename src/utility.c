/*
    Copyright (c) 2018 - Stephen Planck and Alistair Packer

    utility.c - Contains utility functions used throughout the code.

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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "utility.h"

struct myserver *pFirst = NULL;
struct myserver *pLast = NULL;

// Adds a server to the end of the linked list based on the parameters passed.
void addServerNode(int id, char *hostname, int port, char *username, char *password) {
    struct myserver *pNewNode = malloc(sizeof(struct myserver));

    strcpy(pNewNode->hostname, hostname);
    strcpy(pNewNode->username, username);
    strcpy(pNewNode->password, password);
    
    pNewNode->id = id;
    pNewNode->port = port;
    pNewNode->next = NULL;
	pNewNode->firstDatabase = NULL;
	pNewNode->lastDatabase = NULL;

    if(pFirst == NULL) {
        pFirst = pLast = pNewNode;
    }
    else {
        pLast->next = pNewNode;
        pLast = pNewNode;
    }
}

// Adds a database to the end of a server's linked list of dataabases. Parameters are
// a pointer to the server and the database name.
void addDatabaseNode(struct myserver *svr, char *dbname) {
    struct mydatabase *pNewNode = malloc(sizeof(struct mydatabase));

    strcpy(pNewNode->dbname, dbname);

    pNewNode->firstTable = NULL;
    pNewNode->lastTable = NULL;
    pNewNode->next = NULL;

    if(svr->firstDatabase == NULL) {
        svr->firstDatabase = svr->lastDatabase = pNewNode;
    }
    else {
        svr->lastDatabase->next = pNewNode;
        svr->lastDatabase = pNewNode;
    }
}

// Adds a table to the end of a database's linked list of tables. Parameters are
// a pointer to the database and the table name.
void addTableNode(struct mydatabase *db, char *tblname) {
    struct mytable *pNewNode = malloc(sizeof(struct mytable));

    strcpy(pNewNode->tblname, tblname);

    pNewNode->next = NULL;

    struct mytable *first = db->firstTable;
    struct mytable *last = db->lastTable;

    if(first == NULL) {
        db->firstTable = db->lastTable = pNewNode;
    }
    else {
        db->lastTable->next = pNewNode;
        db->lastTable = pNewNode;
    }
}

// Utility functioon to remove characters from strings. Accepts a char as the
// character to be removed and a char pointer as source string.
void remove_char_from_string(char c, char *str) {
    int i = 0;
    int len = strlen(str) + 1;

    for(i = 0; i < len; i++) {
        if(str[i] == c) {
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }
}

// Calculates checksum for packet and returns the result.
unsigned short checkSum(void *buffer, int len) {    
    unsigned short *buf = buffer;
    unsigned int sum = 0;
    unsigned short result;
 
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    
    if (len == 1)
        sum += *(unsigned char*)buf;
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    
    result = ~sum;
    
    return result;
}

// Accepts a hostname, performs a DNS lookup and returns the IP address if
// found. Otherwise returns NULL.
char *DNSLookup(char *addr_host, struct sockaddr_in *addr_con) {
    struct hostent *host_entity;
    
    char *ip = (char*) malloc(NI_MAXHOST*sizeof(char));
 
    if ((host_entity = gethostbyname(addr_host)) == NULL)
        return NULL;
     
    strcpy(ip, inet_ntoa(*(struct in_addr *) host_entity->h_addr));
 
    (*addr_con).sin_family = host_entity->h_addrtype;
    (*addr_con).sin_port = htons (PORT_NO);
    (*addr_con).sin_addr.s_addr  = *(long*)host_entity->h_addr;
 
    return ip;
}

// Accepts an IP address, performs a reverse DNS lookup for the hostname,
// and returns it. Otherwise returns NULL.
char* reverseDNSLookup(char *ip_addr) {
    struct sockaddr_in temp_addr;    

    socklen_t len;

    char buf[NI_MAXHOST];
    char *ret_buf;
 
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);

    len = sizeof(struct sockaddr_in);
 
    if (getnameinfo((struct sockaddr *) &temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD))
        return NULL;

    ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char));
    strcpy(ret_buf, buf);
    
    return ret_buf;
}
 
// Pings a host to determine if it is online and returns 0 if it can be reached
// or returns a -1 it the server is unreachable.
int sendPing(int p_sockfd, struct sockaddr_in *p_addr, char *p_dom, char *p_ip, char *rev_host) {
    int success = 0;
    int retries = 0;
    int ttl_val = 64;
    int msg_count = 0;
    int addr_len = 1;
    int msg_received_count = 0;
     
    struct ping_packet pckt;
    struct sockaddr_in r_addr;
    struct timeval timeout;

	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
 
    if (setsockopt(p_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
        return -1;
 
    setsockopt(p_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
 
    while(1) {
        bzero(&pckt, sizeof(pckt));
         
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();
         
		int i;

        for (i = 0; i < sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i+'0';
         
        pckt.msg[1] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checkSum(&pckt, sizeof(pckt));
 
        if (sendto(p_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*) p_addr, 
            sizeof(*p_addr)) <= 0) {
            success = 0;
        }
 
        addr_len = sizeof(r_addr);
 
        if (recvfrom(p_sockfd, &pckt, sizeof(pckt), 0, 
             (struct sockaddr*)&r_addr, &addr_len) <= 0 && msg_count > 1) {
            retries++;
            success = 0;
        }
        else {
            if(!(pckt.hdr.type == 69 && pckt.hdr.code == 0)) {
                success = 0;
                retries++;
            }
            else {
                success = 1;
                
                return 0;
            }
        }

        if(retries == 3)
            return -1;
    }
}
 
// Accepts a server host name and checks to see if it is reachable. Returns
// 0 on success and 1 on failure.
int pingServer(char *hostname) {
    struct sockaddr_in addr_con;

    int sockfd;
    int addrlen = sizeof(addr_con);
    
    char *ip;
    char *reverse_host;
    char net_buf[NI_MAXHOST];
 
    ip = DNSLookup(hostname, &addr_con);
    
    if(ip == NULL)
        return 1;
 
    reverse_host = reverseDNSLookup(ip);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    if(sockfd < 0)
        return 1;
 
    if(sendPing(sockfd, &addr_con, reverse_host, ip, hostname) == 0) 
        return 0;
    else
        return 1;
}

// Accepts a character array and converts all letters to uppercase.
void ucase(char str[]) {
	int i = 0;
   
	while (str[i] != '\0') {
		if (str[i] >= 'a' && str[i] <= 'z') 
			str[i] = str[i] - 32;

		i++;
	}
}
