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

ALL     = $(LIB) checklib pig2vcd pigpiod pigs

LL      = -L. -lpigpio -lpthread -lrt

all:	$(ALL)

checklib:	checklib.o $(LIB1)
	$(CC) -o checklib checklib.c $(LL)

pigpiod:	pigpiod.o $(LIB1)
	$(CC) -o pigpiod pigpiod.c $(LL)

pigs:		pigs.o command.o
	$(CC) -o pigs pigs.c command.c

pig2vcd:	pig2vcd.o
	$(CC) -o pig2vcd pig2vcd.c

clean:
	rm -f *.o *.i *.s *~ $(ALL)

install:	$(LIB) 
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

uninstall:
	sudo rm -f /usr/local/include/pigpio.h
	sudo rm -f /usr/local/include/pigpiod_if.h
	sudo rm -f /usr/local/lib/libpigpio.a
	sudo rm -f /usr/local/lib/libpigpiod_if.a
	sudo rm -f /usr/local/bin/pig2vcd
	sudo rm -f /usr/local/bin/pigpiod
	sudo rm -f /usr/local/bin/pigs

$(LIB1):	$(OBJ1)
	$(AR) rcs $(LIB1) $(OBJ1)
	$(RANLIB) $(LIB1)
	$(SIZE)   $(LIB1)

$(LIB2):	$(OBJ2)
	$(AR) rcs $(LIB2) $(OBJ2)
	$(RANLIB) $(LIB2)
	$(SIZE)   $(LIB2)

# generated using gcc -MM *.c

checklib.o: checklib.c pigpio.h
command.o: command.c pigpio.h command.h
pig2vcd.o: pig2vcd.c pigpio.h
pigpio.o: pigpio.c pigpio.h command.h
pigpiod.o: pigpiod.c pigpio.h
pigpiod_if.o: pigpiod_if.c pigpio.h command.h pigpiod_if.h
pigs.o: pigs.c pigpio.h command.h

