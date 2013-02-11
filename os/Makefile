include	../standard_definitions.mk
#-DXSYNC -DFONTCACHE -DMITMISC -DXTRAP -DXCMISC
DEFS=-DXSERV_t -DTRANS_SERVER -DUNIXCONN -DTCPCONN -DHAS_FCHOWN -DDDXOSINIT -DADMPATH=\"/usr/adm/X\%smsgs\" -DBIGREQS 

CFLAGS= -I../include -I../common -I../render -I../Xext -I../lbx -I../fb -I../mi -I../miext/shadow -I../hw/kdrive -I../miext/layer $(COMMONDEFS) $(DEFS)

OBJS=WaitFor.c access.c connection.c io.c osinit.c utils.c auth.c mitauth.c secauth.c xdmcp.c decompress.c transport.c

libICE.a:	$(OBJS:.c=.o) 
	ar cr libos.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libos.a
	cp -f libos.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c