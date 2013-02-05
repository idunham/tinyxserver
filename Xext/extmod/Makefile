include	../../standard_definitions.mk

DEFS=

CFLAGS= -I../../include -I../../mi -I../../fb -I../../hw/kdrive -I../../Xext -I../ $(COMMONDEFS) $(DEFS)

#OBJS=$(wildcard *.c)
ifeq ($(SHAPE),-DSHAPE)
OBJS2=shape.c
endif

ifeq ($(DPMS),-DDPMSExtension)
OBJS3=dpms.c
endif

OBJS=$(OBJS2) $(OBJS3) bigreq.c EVI.c sampleEVI.c sync.c xf86dga.c xvdisp.c xvmod.c cup.c mitmisc.c saver.c xcmisc.c xf86misc.c xvmain.c modinit.c xf86dga2.c xf86vmode.c xvmc.c




libextmod.a:	$(OBJS:.c=.o) 
	ar cr libextmod.a $(OBJS:.c=.o) 
clean:
	rm -f *.o *.a
install:	libextmod.a
	cp -f libextmod.a $(LIBDIR)
depend:
	gccmakedep -- $(CFLAGS) -- *.c