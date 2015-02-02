#
CC	= gcc
AR      = ar
RANLIB  = ranlib
SIZE    = size

CFLAGS	= -O3 -Wall

LIB1     = libpigpio.a
OBJ1     = pigpio.o command.o

LIB2     = libpigpiod_if.a
OBJ2     = pigpiod_if.o command.o

LIB      = $(LIB1) $(LIB2)

ALL     = $(LIB) x_pigpio x_pigpiod_if pig2vcd pigpiod pigs

LL1      = -L. -lpigpio -lpthread -lrt

LL2      = -L. -lpigpiod_if -lpthread -lrt

all:	$(ALL)

x_pigpio:	x_pigpio.o $(LIB1)
	$(CC) -o x_pigpio x_pigpio.c $(LL1)

x_pigpiod_if:	x_pigpiod_if.o $(LIB2)
	$(CC) -o x_pigpiod_if x_pigpiod_if.c $(LL2)

pigpiod:	pigpiod.o $(LIB1)
	$(CC) -o pigpiod pigpiod.c $(LL1)

pigs:		pigs.o command.o
	$(CC) -o pigs pigs.c command.c

pig2vcd:	pig2vcd.o
	$(CC) -o pig2vcd pig2vcd.c

clean:
	rm -f *.o *.i *.s *~ $(ALL)

install:	$(LIB) 
	sudo install -m 0755 -d              /opt/pigpio/cgi
	sudo install -m 0755 -d              /usr/local/include
	sudo install -m 0644 pigpio.h        /usr/local/include
	sudo install -m 0644 pigpiod_if.h    /usr/local/include
	sudo install -m 0755 -d              /usr/local/lib
	sudo install -m 0644 libpigpio.a     /usr/local/lib
	sudo install -m 0644 libpigpiod_if.a /usr/local/lib
	sudo install -m 0755 -d              /usr/local/bin
	sudo install -m 0755 pig2vcd         /usr/local/bin
	sudo install -m 0755 pigpiod         /usr/local/bin
	sudo install -m 0755 pigs            /usr/local/bin
	sudo python setup.py install
	sudo install -m 0755 -d              /usr/local/man/man1
	sudo install -m 0644 *.1             /usr/local/man/man1
	sudo install -m 0755 -d              /usr/local/man/man3
	sudo install -m 0644 *.3             /usr/local/man/man3

uninstall:
	sudo rm -f /usr/local/include/pigpio.h
	sudo rm -f /usr/local/include/pigpiod_if.h
	sudo rm -f /usr/local/lib/libpigpio.a
	sudo rm -f /usr/local/lib/libpigpiod_if.a
	sudo rm -f /usr/local/bin/pig2vcd
	sudo rm -f /usr/local/bin/pigpiod
	sudo rm -f /usr/local/bin/pigs
	sudo rm -f /usr/local/man/man1/pig*.1
	sudo rm -f /usr/local/man/man3/pig*.3

$(LIB1):	$(OBJ1)
	$(AR) rcs $(LIB1) $(OBJ1)
	$(RANLIB) $(LIB1)
	$(SIZE)   $(LIB1)

$(LIB2):	$(OBJ2)
	$(AR) rcs $(LIB2) $(OBJ2)
	$(RANLIB) $(LIB2)
	$(SIZE)   $(LIB2)

# generated using gcc -MM *.c

command.o: command.c pigpio.h command.h
pig2vcd.o: pig2vcd.c pigpio.h
pigpio.o: pigpio.c pigpio.h command.h custom.cext
pigpiod.o: pigpiod.c pigpio.h
pigpiod_if.o: pigpiod_if.c pigpio.h command.h pigpiod_if.h
pigs.o: pigs.c pigpio.h command.h
x_pigpio.o: x_pigpio.c pigpio.h
x_pigpiod_if.o: x_pigpiod_if.c pigpiod_if.h pigpio.h

