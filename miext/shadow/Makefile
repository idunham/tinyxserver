include	../../standard_definitions.mk

DEFS=

CFLAGS= -I../../include -I../../mi -I../../fb -I../../hw/kdrive -I../../render $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libshadow.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libshadow.a
	cp -f libshadow.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c