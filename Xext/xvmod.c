/* $XFree86: xc/programs/Xserver/Xext/xvmod.c,v 1.2 2001/03/05 04:51:55 mvojkovi Exp $ */

#include <X11/X.h>
#include "misc.h"
#include "scrnintstr.h"
#include "gc.h"
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>
#include "xvdix.h"
#include "xvmodproc.h"

void
XvRegister()
{
    XvScreenInitProc = XvScreenInit;
    XvGetScreenIndexProc = XvGetScreenIndex;
    XvGetRTPortProc = XvGetRTPort;
    XvMCScreenInitProc = XvMCScreenInit;
}

