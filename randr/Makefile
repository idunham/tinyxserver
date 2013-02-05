include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

librandr.a:	$(OBJS:.c=.o) 
	ar cr librandr.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	librandr.a
	cp -f librandr.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c