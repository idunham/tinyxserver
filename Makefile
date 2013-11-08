
include standard_definitions.mk

DEFS= -DNOERROR

CFLAGS= -I.. -I../.. -I../../.. -I./include -I./common -I./render -I./Xext -I./lbx -I./fb -I./mi -I./miext/shadow -I./hw/kdrive -I./miext/layer -I./os $(COMMONDEFS) $(DEFS)

OBJS=
all: core Xvesa Xfbdev

core:
	cd render; make
	cd dix; make
	cd os; make
	cd mi; make
	cd Xext; make
	cd Xext/extmod; make
	cd XTrap; make
	cd hw/kdrive; make
	cd hw/kdrive/linux; make
	cd fb; make
	cd miext/shadow; make
	cd miext/layer; make
	cd randr; make
	cd record; make
	
Xvesa: core
	cd hw/kdrive/vesa; make
	$(CC) $(CFLAGs) $(DEFS) $(LDFLAGS) -o Xvesa dix/libdix.a os/libos.a hw/kdrive/vesa/libvesa.a miext/layer/liblayer.a hw/kdrive/libkdrive.a hw/kdrive/linux/liblinux.a miext/shadow/libshadow.a fb/libfb.a mi/libmi.a Xext/libext.a Xext/extmod/libextmod.a XTrap/libxtrap.a randr/librandr.a render/librender.a record/librecord.a $(LINKDIR) -lXfont -lXinerama -lX11 -lXdmcp -lz -lm
	
Xfbdev: core
	cd hw/kdrive/fbdev; make
	$(CC) $(CFLAGs) $(DEFS) $(LDFLAGS) -o Xfbdev dix/libdix.a os/libos.a hw/kdrive/fbdev/libfbdev.a miext/layer/liblayer.a hw/kdrive/libkdrive.a hw/kdrive/linux/liblinux.a miext/shadow/libshadow.a fb/libfb.a mi/libmi.a Xext/libext.a Xext/extmod/libextmod.a XTrap/libxtrap.a randr/librandr.a render/librender.a record/librecord.a $(LINKDIR) -lXfont -lXinerama -lX11 -lXdmcp -lz -lm	
	
clean:
	cd render; make clean
	cd dix; make clean
	cd os; make clean
	cd mi; make clean
	cd Xext; make clean
	cd Xext/extmod; make clean
	cd XTrap; make clean
	cd hw/kdrive; make clean
	cd hw/kdrive/linux; make clean
	cd fb; make clean; cd ..
	cd hw/kdrive/vesa; make clean
	cd miext/shadow; make clean
	cd miext/layer; make clean
	cd randr; make clean
	cd hw/kdrive/fbdev; make clean
	cd record; make clean
	rm -f Xvesa
	rm -f Xfbdev
install:

tarball:	clean
	./make-tarball.sh

