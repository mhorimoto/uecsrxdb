CFLAGS = -Wpointer-to-int-cast -g

all: uecsrxdb

uecsrxdb: rx.o rep.o # db.o
	cc $(CFLAGS) -o uecsrxdb rx.o rep.o
#	cc -lmysqlclient -L/usr/lib64/ $(CFLAGS) -o uecsrxdb rx.o rep.o db.o

rx.o: rx.c
	cc -c $(CFLAGS) rx.c
rep.o: rep.c
	cc -c $(CFLAGS) rep.c
#db.o: db.c
#	cc -c $(CFLAGS) db.c

install:
	install uecsrxdb /usr/local/bin/

clean:
	-/bin/rm *.o uecsrxdb *~
