include	../standard_definitions.mk

#-DMITMISC -DXTRAP -DXCMISC -DFONTCACHE
DEFS= -DBIGREQS 

CFLAGS= -I../include -I../mi -I../fb -I../hw/kdrive -I../render $(COMMONDEFS) $(DEFS)

#OBJS=$(wildcard *.c)
ifeq ($(PANORAMIX),-DPANORAMIX)
OBJS1=panoramiX.c panoramiXSwap.c panoramiXprocs.c        
endif

ifeq ($(SHAPE),-DSHAPE)
OBJS2=shape.c
endif

ifeq ($(DPMS),-DDPMSExtension)
OBJS3=dpms.c
endif

OBJS=$(OBJS1) $(OBJS2) $(OBJS3) bigreq.c mbuf.c saver.c xf86dga2.c xtest.c cup.c mbufpx.c  xf86dga.c xvdisp.c mitmisc.c shm.c xf86misc.c xvmain.c dpmsstubs.c sleepuntil.c xf86vmode.c  xvmc.c EVI.c sync.c xprint.c xvmod.c fontcache.c  xcmisc.c xtest1dd.c mbufbf.c sampleEVI.c xf86bigfont.c xtest1di.c


libICE.a:	$(OBJS:.c=.o) 
	ar cr libext.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libext.a
	cp -f libext.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c