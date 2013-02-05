include	../standard_definitions.mk  

DEFS= 

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive -I../Xext $(COMMONDEFS) $(DEFS)  

OBJS=$(wildcard *.c)  

libICE.a:	$(OBJS:.c=.o) 
	ar cr librender.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	librender.a
	cp -f librender.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c