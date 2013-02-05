include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../Xext $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr librecord.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	librecord.a
	cp -f librecord.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c