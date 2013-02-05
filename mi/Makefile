include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive -I../render -I../Xext $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libmi.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libmi.a
	cp -f libmi.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c