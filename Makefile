#
# Set CROSS_PREFIX to prepend to all compiler tools at once for easier
# cross-compilation.
CROSS_PREFIX =
CC           = $(CROSS_PREFIX)gcc
AR           = $(CROSS_PREFIX)ar
RANLIB       = $(CROSS_PREFIX)ranlib
SIZE         = $(CROSS_PREFIX)size
STRIP        = $(CROSS_PREFIX)strip
SHLIB        = $(CC) -shared
STRIPLIB     = $(STRIP) --strip-unneeded

CFLAGS	+= -O3 -Wall -pthread

LIB1     = libpigpio.so
OBJ1     = pigpio.o command.o

LIB2     = libpigpiod_if.so
OBJ2     = pigpiod_if.o command.o

LIB3     = libpigpiod_if2.so
OBJ3     = pigpiod_if2.o command.o

LIB      = $(LIB1) $(LIB2) $(LIB3)

ALL     = $(LIB) x_pigpio x_pigpiod_if x_pigpiod_if2 pig2vcd pigpiod pigs

LL1      = -L. -lpigpio -pthread -lrt

LL2      = -L. -lpigpiod_if -pthread -lrt

LL3      = -L. -lpigpiod_if2 -pthread -lrt

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
libdir = $(prefix)/lib
mandir = $(prefix)/man

all:	$(ALL)

lib:	$(LIB)

pigpio.o: pigpio.c pigpio.h command.h custom.cext
	$(CC) $(CFLAGS) -fpic -c -o pigpio.o pigpio.c

pigpiod_if.o: pigpiod_if.c pigpio.h command.h pigpiod_if.h
	$(CC) $(CFLAGS) -fpic -c -o pigpiod_if.o pigpiod_if.c

pigpiod_if2.o: pigpiod_if2.c pigpio.h command.h pigpiod_if2.h
	$(CC) $(CFLAGS) -fpic -c -o pigpiod_if2.o pigpiod_if2.c

command.o: command.c pigpio.h command.h
	$(CC) $(CFLAGS) -fpic -c -o command.o command.c

x_pigpio:	x_pigpio.o $(LIB1)
	$(CC) -o x_pigpio x_pigpio.o $(LL1)

x_pigpiod_if:	x_pigpiod_if.o $(LIB2)
	$(CC) -o x_pigpiod_if x_pigpiod_if.o $(LL2)

x_pigpiod_if2:	x_pigpiod_if2.o $(LIB3)
	$(CC) -o x_pigpiod_if2 x_pigpiod_if2.o $(LL3)

pigpiod:	pigpiod.o $(LIB1)
	$(CC) -o pigpiod pigpiod.o $(LL1)
	$(STRIP) pigpiod

pigs:		pigs.o command.o
	$(CC) -o pigs pigs.o command.o
	$(STRIP) pigs

pig2vcd:	pig2vcd.o
	$(CC) -o pig2vcd pig2vcd.o
	$(STRIP) pig2vcd

clean:
	rm -f *.o *.i *.s *~ $(ALL)

install:	$(ALL)
	install -m 0755 -d                $(DESTDIR)/opt/pigpio/cgi
	install -m 0755 -d                $(DESTDIR)$(includedir)
	install -m 0644 pigpio.h          $(DESTDIR)$(includedir)
	install -m 0644 pigpiod_if.h      $(DESTDIR)$(includedir)
	install -m 0644 pigpiod_if2.h     $(DESTDIR)$(includedir)
	install -m 0755 -d                $(DESTDIR)$(libdir)
	install -m 0755 libpigpio.so      $(DESTDIR)$(libdir)
	install -m 0755 libpigpiod_if.so  $(DESTDIR)$(libdir)
	install -m 0755 libpigpiod_if2.so $(DESTDIR)$(libdir)
	install -m 0755 -d                $(DESTDIR)$(bindir)
	install -m 0755 pig2vcd           $(DESTDIR)$(bindir)
	install -m 0755 pigpiod           $(DESTDIR)$(bindir)
	install -m 0755 pigs              $(DESTDIR)$(bindir)
	if which python2; then python2 setup.py install; fi
	if which python3; then python3 setup.py install; fi
	install -m 0755 -d                $(DESTDIR)$(mandir)/man1
	install -m 0644 *.1               $(DESTDIR)$(mandir)/man1
	install -m 0755 -d                $(DESTDIR)$(mandir)/man3
	install -m 0644 *.3               $(DESTDIR)$(mandir)/man3
	ldconfig

uninstall:
	rm -f $(DESTDIR)$(includedir)/pigpio.h
	rm -f $(DESTDIR)$(includedir)/pigpiod_if.h
	rm -f $(DESTDIR)$(includedir)/pigpiod_if2.h
	rm -f $(DESTDIR)$(libdir)/libpigpio.so
	rm -f $(DESTDIR)$(libdir)/libpigpiod_if.so
	rm -f $(DESTDIR)$(libdir)/libpigpiod_if2.so
	rm -f $(DESTDIR)$(bindir)/pig2vcd
	rm -f $(DESTDIR)$(bindir)/pigpiod
	rm -f $(DESTDIR)$(bindir)/pigs
	if which python2; then python2 setup.py install --record /tmp/pigpio >/dev/null; xargs rm -f < /tmp/pigpio >/dev/null; fi
	if which python3; then python3 setup.py install --record /tmp/pigpio >/dev/null; xargs rm -f < /tmp/pigpio >/dev/null; fi
	rm -f $(DESTDIR)$(mandir)/man1/pig*.1
	rm -f $(DESTDIR)$(mandir)/man3/pig*.3
	ldconfig

$(LIB1):	$(OBJ1)
	$(SHLIB) -o $(LIB1) $(OBJ1)
	$(STRIPLIB) $(LIB1)
	$(SIZE)     $(LIB1)

$(LIB2):	$(OBJ2)
	$(SHLIB) -o $(LIB2) $(OBJ2)
	$(STRIPLIB) $(LIB2)
	$(SIZE)     $(LIB2)

$(LIB3):	$(OBJ3)
	$(SHLIB) -o $(LIB3) $(OBJ3)
	$(STRIPLIB) $(LIB3)
	$(SIZE)     $(LIB3)

# generated using gcc -MM *.c

pig2vcd.o: pig2vcd.c pigpio.h
pigpiod.o: pigpiod.c pigpio.h
pigs.o: pigs.c pigpio.h command.h pigs.h
x_pigpio.o: x_pigpio.c pigpio.h
x_pigpiod_if.o: x_pigpiod_if.c pigpiod_if.h pigpio.h
x_pigpiod_if2.o: x_pigpiod_if2.c pigpiod_if2.h pigpio.h

