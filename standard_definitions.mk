
####compiler####
#CC=/usr/bin/gcc
CC=musl-gcc

####compilerflags####
COMPFLAGS=-pipe -Os -mtune=i386 -Wall -D_BSD_SOURCE -D_GNU_SOURCE \
-fno-strength-reduce -nodefaultlibs -fno-strict-aliasing \
-ffunction-sections -fdata-sections -D__KERNEL_STRICT_NAMES \
-I/opt/musl/include -I.

####extensions####
#DPMS=-DDPMSExtension
#SCREENSAVER=-DSCREENSAVER
MIT-SHM=-DMITSHM
RENDER=-DRENDER
SHAPE=-DSHAPE
SYNC=-DXSYNC
TOG-CUP=-DTOGCUP
XCMISC=-DXCMISC
XTEST=-DXTEST
XTRAP=-DXTRAP
XV=-DXV
XKB_IN_SERVER=-DXKB_IN_SERVER
RANDR=-DRANDR
#XRECORD=-DXRECORD

####not working####
#XINPUT=-DXINPUT	#NOT WORKING!
#XKB=-DXKB	#NOT WORKING!


####others####
#SERVER_LOCK=-DSERVER_LOCK
#SMART_SCHEDULE=-DSMART_SCHEDULE
USE_RGB_TXT=-DUSE_RGB_TXT
#XDMCP=-DXDMCP
#PANORAMIX=-DPANORAMIX

#all deactivated creates 543K Xvesa
#all activated creates 728K Xvesa
COMMONDEFS=$(COMPFLAGS) \
-DNOERROR \
-D__i386__ \
-Dlinux \
-D_POSIX_SOURCE \
-D_BSD_SOURCE \
-DTOSHIBA_SMM \
-D_SVID_SOURCE \
-D_GNU_SOURCE \
-DX_LOCALE \
-DKDRIVESERVER \
-DGCCUSESGAS \
-DDDXOSINIT \
-DNOFONTSERVERACCESS \
-DNDEBUG \
-DNARROWPROTO \
-DPIXPRIV \
$(XTEST) \
-DFUNCPROTO=15 \
-DCOMPILEDDEFAULTFONTPATH=\"/usr/share/fonts/X11/misc/\" \
-DRGB_DB=\"/usr/share/X11/rgb\" \
-D_POSIX_C_SOURCE=2 \
$(DPMS) \
$(SYNC) \
$(PANORAMIX) \
$(SHAPE) \
$(TOG-CUP) \
$(MIT-SHM) \
$(RENDER) \
$(SCREENSAVER) \
$(SERVER_LOCK) \
$(SMART_SCHEDULE) \
$(XCMISC) \
$(XDMCP) \
$(XTRAP) \
$(XV) \
$(XINPUT) \
$(XKB) \
$(XKB_IN_SERVER) \
$(RANDR) \
$(XRECORD) \
$(USE_RGB_TXT)
#Puppy:
#-DCOMPILEDDEFAULTFONTPATH=\"/usr/X11R7/lib/X11/fonts/misc/\" \
#-DRGB_DB=\"/usr/X11R7/lib/X11/rgb\" \

LDFLAGS=-static -Wl,--gc-sections,--sort-common,-s

LINKDIR=-L/opt/musl/lib

LINKDIR=-L/opt/musl/lib

LIBDIR=/opt/musl/lib

INCDIR=/opt/musl/include


