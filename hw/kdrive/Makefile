include	../../standard_definitions.mk

DEFS=

CFLAGS= -I../../include -I../../mi -I../../fb -I../../Xext -I../../render $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

libICE.a:	$(OBJS:.c=.o) 
	ar cr libkdrive.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libkdrive.a
	cp -f libkdrive.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c