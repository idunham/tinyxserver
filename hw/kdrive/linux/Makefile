include	../../../standard_definitions.mk

DEFS=

CFLAGS= -I../../../include -I../../../mi -I../../../fb -I../../../hw/kdrive -I../../../render $(COMMONDEFS) $(DEFS)
OBJS=$(wildcard *.c)
#OBJS=keyboard.c linux.c mouse.c ps2.c bus.c ms.c agp.c 

libICE.a:	$(OBJS:.c=.o) 
	ar cr liblinux.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	liblinux.a
	cp -f liblinux.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c