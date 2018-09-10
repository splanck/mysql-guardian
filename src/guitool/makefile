CC=gcc
CFLAGS=-lncurses `mysql_config --cflags --libs`
DEPS = guardian.h mysql.h fileio.h interface.h utility.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -o ../../build/mysql-guardian guardian.c mysql.c fileio.c interface.c utility.c $(CFLAGS) 
