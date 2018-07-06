CC=gcc
CFLAGS=-lncurses
DEPS = guardian.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
mysql-guardian: guardian.c
	$(CC) -o mysql-guardian guardian.c $(CFLAGS)
