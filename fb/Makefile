include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive -I../render -I../Xext $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libfb.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libfb.a
	cp -f libfb.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c