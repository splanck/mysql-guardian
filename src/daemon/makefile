CC=gcc
CFLAGS=`mysql_config --cflags --libs`
DEPS = mysqlgd.h 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
mysql-guardian: mysqlgd.c 
	$(CC) -o ../../build/mysqlgd mysqlgd.c $(CFLAGS) 
