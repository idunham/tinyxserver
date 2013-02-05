include	../../../standard_definitions.mk

DEFS=

CFLAGS= -I../../../include -I../../../mi -I../../../miext/layer -I../../../fb -I../../../render -I../.. $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libvesa.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libvesa.a
	cp -f libvesa.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c