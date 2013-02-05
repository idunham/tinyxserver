include	../../standard_definitions.mk

DEFS=

CFLAGS= -I../../include -I../../mi -I../../fb -I../../hw/kdrive -I../../render $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr liblayer.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	liblayer.a
	cp -f liblayer.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c