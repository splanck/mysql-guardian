CC=gcc
CFLAGS=-lncurses `mysql_config --cflags --libs`
DEPS = guardian.h mysql.h fileio.h interface.h utility.h mysqlgd.h

%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -g -o ../build/mysql-guardian guardian.c mysql.c fileio.c interface.c utility.c mysqlgd.c $(CFLAGS) 

install: all
	cp -f ../build/mysql-guardian /usr/bin/mysql-guardian
	chmod 755 /usr/bin/mysql-guardian
	cp -f ../build/mysqlgd.service /etc/systemd/system/mysqlgd.service

uninstall:
	rm -f /usr/bin/mysql-guardian
	rm -f /etc/systemd/system/mysqlgd.service
	rm -f /etc/mysqlgd.conf

.PHONY: all install uninstall
