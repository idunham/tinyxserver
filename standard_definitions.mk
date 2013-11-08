
####compiler####
CC=/usr/bin/gcc
#CC=/opt/musl/bin/musl-gcc

####compilerflags####
COMPFLAGS=-pipe -fPIC -Os -Wall -D_BSD_SOURCE -D_GNU_SOURCE -D__KERNEL_STRICT_NAMES  \
-fno-strength-reduce -nodefaultlibs -fno-strict-aliasing \
-ffunction-sections -fdata-sections \
-I. -I/usr/xsrc/pkg/usr/include


####extensions####
DPMS=-DDPMSExtension
SCREENSAVER=-DSCREENSAVER
MIT-SHM=-DMITSHM
RENDER=-DRENDER
#FIXME:if not defined dont define PANORAMIX=-DPANORAMIX
SHAPE=-DSHAPE
SYNC=-DXSYNC
TOG-CUP=-DTOGCUP
XCMISC=-DXCMISC
XTEST=-DXTEST
XTRAP=-DXTRAP
XV=-DXV
RANDR=-DRANDR
XRECORD=-DXRECORD
FONTCACHE=-DNOFONTSERVERACCESS	# -DFONTCACHE or -DNOFONTSERVERACCESS

####not working####
#XINPUT=-DXINPUT	#NOT WORKING!
#XKB=-DXKB/XKB_IN_SERVER=-DXKB	#NOT WORKING!


####others####
#SERVER_LOCK=-DSERVER_LOCK
#SMART_SCHEDULE=-DSMART_SCHEDULE
USE_RGB_TXT=-DUSE_RGB_TXT
#XDMCP=-DXDMCP
PANORAMIX=-DPANORAMIX

####where to look for fonts/colors####
# Puppy:
#FONTPATH=/usr/X11R7/lib/X11/fonts/misc/
#RGB=/usr/X11R7/lib/X11/rgb
# Debian:
FONTPATH=/usr/share/fonts/X11/misc/
RGB=/usr/share/X11/rgb

#Lazyux
#FONTPATH="/usr/share/fonts/misc/,/usr/share/fonts/truetype/,/usr/share/fonts/X11/100dpi/,/usr/share/fonts/X11/75dpi/\" 

#-DRGB_DB=\"/usr/share/X11/rgb.txt\" \

#all deactivated creates 543K Xvesa
#all activated creates 728K Xvesa
COMMONDEFS=$(COMPFLAGS) \
-DNOERROR \
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
$(FONTCACHE) \
-DNDEBUG \
-DNARROWPROTO \
-DPIXPRIV \
$(XTEST) \
-DFUNCPROTO=15 \
-DCOMPILEDDEFAULTFONTPATH=\"${FONTPATH}\" \
-DRGB_DB=\"${RGB}\" \
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
$(USE_RGB_TXT) \
$(PIXPRIV) \
$(X_LOCALE) \
$(XF86BIGFONT) \
$(BIGREQS) \
-D__KERNEL_STRICT_NAMES

LDFLAGS=-static -Wl,--gc-sections,--sort-common,-s
LINKDIR=-L/usr/xsrc/pkg/usr/lib

#LINKDIR=-L/opt/musl/lib
#LIBDIR=/opt/musl/lib
#INCDIR=/opt/musl/include

PREDIR=/usr
LIBDIR=$(DESTDIR)$(PREDIR)/lib
INCDIR=$(DESTIR)$(PREDIR)/include

