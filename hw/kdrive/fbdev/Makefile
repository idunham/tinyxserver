include	../../../standard_definitions.mk

DEFS=

CFLAGS= -I../../../include -I../../../mi -I../../../miext/layer -I../../../fb -I../hw/kdrive -I../../../render $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libfbdev.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libfbdev.a
	cp -f libfbdev.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c