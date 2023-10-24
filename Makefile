all: uecsrxdb

uecsrxdb: rx.o rep.o db.o
	cc -lmysqlclient -L/usr/lib64/ -o uecsrxdb rx.o rep.o db.o

rx.o: rx.c
	cc -c rx.c
rep.o: rep.c
	cc -c rep.c
db.o: db.c
	cc -c db.c

install:
	install uecsrxdb /usr/local/bin/

clean:
	-/bin/rm *.o uecsrxdb *~
