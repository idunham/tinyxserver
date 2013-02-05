include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libxtrap.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libxtrap.a
	cp -f libxtrap.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c