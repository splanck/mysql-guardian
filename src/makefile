CC=gcc
CFLAGS=-lncurses `mysql_config --cflags --libs`
DEPS = guardian.h mysql.h fileio.h interface.h utility.h mysqlgd.h

%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -o ../build/mysql-guardian guardian.c mysql.c fileio.c interface.c utility.c mysqlgd.c $(CFLAGS) 
