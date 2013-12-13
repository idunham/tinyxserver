include	../standard_definitions.mk

DEFS=

CFLAGS= -I../include $(COMMONDEFS) $(DEFS)

OBJS=$(wildcard *.c)

xinit.o:	$(OBJS:.c=.o)
clean:
	rm -f *.o
depend:
	gccmakedep -- $(CFLAGS) -- *.c