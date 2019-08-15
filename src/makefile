CC=gcc
CFLAGS=-lncurses `mysql_config --cflags --libs`
DEPS = guardian.h mysql.h fileio.h interface.h utility.h mysqlgd.h checks.h

%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -g -o ../build/mysqlgd guardian.c mysql.c fileio.c interface.c utility.c mysqlgd.c checks.c $(CFLAGS) 

install: all
	cp -f ../build/mysqlgd /usr/bin/mysqlgd
	chmod 755 /usr/bin/mysqlgd
	cp -f ../build/mysqlgd.service /etc/systemd/system/mysqlgd.service
	mkdir -p /usr/local/man/man1
	cp -f ../doc/mysqlgd.1 /usr/local/man/man1/mysqlgd.1
	gzip -f /usr/local/man/man1/mysqlgd.1

uninstall:
	rm -f /usr/bin/mysqlgd
	rm -f /etc/systemd/system/mysqlgd.service
	rm -f /etc/mysqlgd.conf
	rm -f /usr/local/man/man1/mysqlgd.1

.PHONY: all install uninstall
