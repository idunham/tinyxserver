include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../Xext $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libdix.a:	$(OBJS:.c=.o) 
	ar cr libdix.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libdix.a
	cp -f libdix.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c