CC=gcc
CFLAGS=-lncurses
DEPS = guardian.h mysql.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -o mysql-guardian guardian.c mysql.c $(CFLAGS) `mysql_config --cflags --libs`
