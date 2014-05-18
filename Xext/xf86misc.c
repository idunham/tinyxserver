/* $XFree86: xc/programs/Xserver/Xext/xf86misc.c,v 3.35 2001/08/15 16:25:20 paulo Exp $ */

/*
 * Copyright (c) 1995, 1996  The XFree86 Project, Inc
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "servermd.h"
#define _XF86MISC_SERVER_
#undef _XF86MISC_SAVER_COMPAT_
#include <X11/extensions/xf86mscstr.h>
#include "swaprep.h"
#include "../hw/xfree86/common/xf86.h"
#include <X11/Xfuncproto.h>
#include "xf86miscproc.h"

#if 0
#include <X11/Xtrans.h>
#include "../os/osdep.h"
#include <X11/Xauth.h>
#ifndef USL
#include <sys/socket.h>
#endif /* USL */
#endif

#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

static int miscErrorBase;

static void XF86MiscResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry* /* extEntry */
#endif
);

static DISPATCH_PROC(ProcXF86MiscDispatch);
static DISPATCH_PROC(ProcXF86MiscQueryVersion);
static DISPATCH_PROC(ProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscSetGrabKeysState);
#ifdef _XF86MISC_SAVER_COMPAT_
static DISPATCH_PROC(ProcXF86MiscGetSaver);
static DISPATCH_PROC(ProcXF86MiscSetSaver);
#endif
static DISPATCH_PROC(SProcXF86MiscDispatch);
static DISPATCH_PROC(SProcXF86MiscQueryVersion);
static DISPATCH_PROC(SProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscSetGrabKeysState);
#ifdef _XF86MISC_SAVER_COMPAT_
static DISPATCH_PROC(SProcXF86MiscGetSaver);
static DISPATCH_PROC(SProcXF86MiscSetSaver);
#endif

static unsigned char XF86MiscReqCode = 0;

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

#define MISCERR(x)	(miscErrorBase + x)

void
XFree86MiscExtensionInit(void)
{
    ExtensionEntry* extEntry;

    DEBUG_P("XFree86MiscExtensionInit");

    if (!xf86GetModInDevEnabled())
	return;

    if (
	(extEntry = AddExtension(XF86MISCNAME,
				XF86MiscNumberEvents,
				XF86MiscNumberErrors,
				ProcXF86MiscDispatch,
				SProcXF86MiscDispatch,
				XF86MiscResetProc,
				StandardMinorOpcode))) {
	XF86MiscReqCode = (unsigned char)extEntry->base;
	miscErrorBase = extEntry->errorBase;
    }
}

/*ARGSUSED*/
static void
XF86MiscResetProc (extEntry)
    ExtensionEntry* extEntry;
{
}

static int
ProcXF86MiscQueryVersion(client)
    register ClientPtr client;
{
    xXF86MiscQueryVersionReply rep;
    register int n;

    DEBUG_P("XF86MiscQueryVersion");

    REQUEST_SIZE_MATCH(xXF86MiscQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XF86MISC_MAJOR_VERSION;
    rep.minorVersion = XF86MISC_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.majorVersion, n);
    	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXF86MiscQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

#ifdef _XF86MISC_SAVER_COMPAT_
/* THESE HAVE NOT BEEN CONVERTED TO THE NEW DESIGN */

/*
 * This will go away, but remains for now for compatibility with older
 * clients.
 */
static int
ProcXF86MiscSetSaver(client)
    register ClientPtr client;
{
    REQUEST(xXF86MiscSetSaverReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86MiscSetSaverReq);

    if (stuff->suspendTime < 0)
	return BadValue;
    if (stuff->offTime < 0)
	return BadValue;

    return (client->noClientException);
}

/*
 * This will go away, but remains for now for compatibility with older
 * clients.
 */
static int
ProcXF86MiscGetSaver(client)
    register ClientPtr client;
{
    REQUEST(xXF86MiscGetSaverReq);
    xXF86MiscGetSaverReply rep;
    register int n;
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86MiscGetSaverReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.suspendTime = 0;
    rep.offTime = 0;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.suspendTime, n);
    	swapl(&rep.offTime, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetSaverReply), (char *)&rep);
    return (client->noClientException);
}

#endif /* _XF86MISC_SAVER_COMPAT_ */

static int
ProcXF86MiscGetMouseSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetMouseSettingsReply rep;
    char *devname;
    pointer mouse;
    register int n;

    DEBUG_P("XF86MiscGetMouseSettings");

    REQUEST_SIZE_MATCH(xXF86MiscGetMouseSettingsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;

    if (!MiscExtGetMouseSettings(&mouse, &devname))
	return BadValue;

    rep.mousetype =	  MiscExtGetMouseValue(mouse, MISC_MSE_PROTO);
    rep.baudrate =	  MiscExtGetMouseValue(mouse, MISC_MSE_BAUDRATE);
    rep.samplerate =	  MiscExtGetMouseValue(mouse, MISC_MSE_SAMPLERATE);
    rep.resolution =	  MiscExtGetMouseValue(mouse, MISC_MSE_RESOLUTION);
    rep.buttons =	  MiscExtGetMouseValue(mouse, MISC_MSE_BUTTONS);
    rep.emulate3buttons = MiscExtGetMouseValue(mouse, MISC_MSE_EM3BUTTONS);
    rep.emulate3timeout = MiscExtGetMouseValue(mouse, MISC_MSE_EM3TIMEOUT);
    rep.chordmiddle =	  MiscExtGetMouseValue(mouse, MISC_MSE_CHORDMIDDLE);
    rep.flags =		  MiscExtGetMouseValue(mouse, MISC_MSE_FLAGS);
    rep.devnamelen = (devname? strlen(devname): 0);
    rep.length = (sizeof(xXF86MiscGetMouseSettingsReply) -
		  sizeof(xGenericReply) + ((rep.devnamelen+3) & ~3)) >> 2;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.mousetype, n);
    	swapl(&rep.baudrate, n);
    	swapl(&rep.samplerate, n);
    	swapl(&rep.resolution, n);
    	swapl(&rep.buttons, n);
    	swapl(&rep.emulate3buttons, n);
    	swapl(&rep.emulate3timeout, n);
    	swapl(&rep.chordmiddle, n);
    	swapl(&rep.flags, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetMouseSettingsReply), (char *)&rep);
    MiscExtDestroyStruct(mouse, MISC_POINTER);
    
    if (rep.devnamelen)
        WriteToClient(client, rep.devnamelen, devname);
    return (client->noClientException);
}

static int
ProcXF86MiscGetKbdSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetKbdSettingsReply rep;
    pointer kbd;
    register int n;

    DEBUG_P("XF86MiscGetKbdSettings");

    REQUEST_SIZE_MATCH(xXF86MiscGetKbdSettingsReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (!MiscExtGetKbdSettings(&kbd))
	return BadValue;

    rep.kbdtype =	MiscExtGetKbdValue(kbd, MISC_KBD_TYPE);
    rep.rate =		MiscExtGetKbdValue(kbd, MISC_KBD_RATE);
    rep.delay =		MiscExtGetKbdValue(kbd, MISC_KBD_DELAY);
    rep.servnumlock =	MiscExtGetKbdValue(kbd, MISC_KBD_SERVNUMLOCK);
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.kbdtype, n);
    	swapl(&rep.rate, n);
    	swapl(&rep.delay, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetKbdSettingsReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86MiscSetMouseSettings(client)
    register ClientPtr client;
{
    MiscExtReturn ret;
    pointer mouse;
    REQUEST(xXF86MiscSetMouseSettingsReq);

    DEBUG_P("XF86MiscSetMouseSettings");

    REQUEST_SIZE_MATCH(xXF86MiscSetMouseSettingsReq);

    if (xf86GetVerbosity() > 1) {
	ErrorF("SetMouseSettings - type: %d brate: %d srate: %d chdmid: %d\n",
		stuff->mousetype, stuff->baudrate,
		stuff->samplerate, stuff->chordmiddle);
	ErrorF("                   em3but: %d em3tim: %d res: %d flags: %d\n",
		stuff->emulate3buttons, stuff->emulate3timeout,
		stuff->resolution, stuff->flags);
    }

    if ((mouse = MiscExtCreateStruct(MISC_POINTER)) == (pointer) 0)
	return BadAlloc;

    MiscExtSetMouseValue(mouse, MISC_MSE_PROTO,		stuff->mousetype);
    MiscExtSetMouseValue(mouse, MISC_MSE_BAUDRATE,	stuff->baudrate);
    MiscExtSetMouseValue(mouse, MISC_MSE_SAMPLERATE,	stuff->samplerate);
    MiscExtSetMouseValue(mouse, MISC_MSE_RESOLUTION,	stuff->resolution);
    MiscExtSetMouseValue(mouse, MISC_MSE_BUTTONS,	stuff->buttons);
    MiscExtSetMouseValue(mouse, MISC_MSE_EM3BUTTONS,	stuff->emulate3buttons);
    MiscExtSetMouseValue(mouse, MISC_MSE_EM3TIMEOUT,	stuff->emulate3timeout);
    MiscExtSetMouseValue(mouse, MISC_MSE_CHORDMIDDLE,	stuff->chordmiddle);
    MiscExtSetMouseValue(mouse, MISC_MSE_FLAGS,		stuff->flags);

    switch ((ret = MiscExtApply(mouse, MISC_POINTER))) {
        case MISC_RET_SUCCESS:      break;
	case MISC_RET_BADVAL:       return BadValue;
	case MISC_RET_BADMSEPROTO:  return MISCERR(XF86MiscBadMouseProtocol);
	case MISC_RET_BADBAUDRATE:  return MISCERR(XF86MiscBadMouseBaudRate);
	case MISC_RET_BADFLAGS:     return MISCERR(XF86MiscBadMouseFlags);
        case MISC_RET_BADCOMBO:     return MISCERR(XF86MiscBadMouseCombo);
        case MISC_RET_NOMODULE:     return MISCERR(XF86MiscNoModule);
        default:
	    ErrorF("Unexpected return from MiscExtApply(POINTER) = %d\n", ret);
	    return BadImplementation;
    }

    if (xf86GetVerbosity() > 1)
	ErrorF("SetMouseSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscSetKbdSettings(client)
    register ClientPtr client;
{
    MiscExtReturn ret;
    pointer kbd;
    REQUEST(xXF86MiscSetKbdSettingsReq);

    DEBUG_P("XF86MiscSetKbdSettings");

    REQUEST_SIZE_MATCH(xXF86MiscSetKbdSettingsReq);

    if (xf86GetVerbosity() > 1)
	ErrorF("SetKbdSettings - type: %d rate: %d delay: %d snumlk: %d\n",
		stuff->kbdtype, stuff->rate,
		stuff->delay, stuff->servnumlock);

    if ((kbd = MiscExtCreateStruct(MISC_KEYBOARD)) == (pointer) 0)
	return BadAlloc;

    MiscExtSetKbdValue(kbd, MISC_KBD_TYPE,		stuff->kbdtype);
    MiscExtSetKbdValue(kbd, MISC_KBD_RATE,		stuff->rate);
    MiscExtSetKbdValue(kbd, MISC_KBD_DELAY,		stuff->delay);
    MiscExtSetKbdValue(kbd, MISC_KBD_SERVNUMLOCK,	stuff->servnumlock);

    switch ((ret = MiscExtApply(kbd, MISC_KEYBOARD))) {
	case MISC_RET_SUCCESS:      break;
	case MISC_RET_BADVAL:       return BadValue;
	case MISC_RET_BADKBDTYPE:   return MISCERR(XF86MiscBadKbdType);
	default:
	    ErrorF("Unexpected return from MiscExtApply(KEYBOARD) = %d\n", ret);
	    return BadImplementation;
    }

    if (xf86GetVerbosity() > 1)
	ErrorF("SetKbdSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscSetGrabKeysState(client)
    register ClientPtr client;
{
    int n, status;
    xXF86MiscSetGrabKeysStateReply rep;
    REQUEST(xXF86MiscSetGrabKeysStateReq);

    DEBUG_P("XF86MiscSetGrabKeysState");

    REQUEST_SIZE_MATCH(xXF86MiscSetGrabKeysStateReq);

    if ((status = MiscExtSetGrabKeysState(client, stuff->enable)) == 0) {
	if (xf86GetVerbosity() > 1)
	    ErrorF("SetGrabKeysState - %s\n",
		   stuff->enable ? "enabled" : "disabled");
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.status = status;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.status, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscSetGrabKeysStateReply), (char *)&rep);

    return (client->noClientException);
}

static int
ProcXF86MiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86MiscQueryVersion:
	return ProcXF86MiscQueryVersion(client);
#ifdef _XF86MISC_SAVER_COMPAT_
    case X_XF86MiscGetSaver:
	return ProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return ProcXF86MiscSetSaver(client);
#endif
    case X_XF86MiscGetMouseSettings:
	return ProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return ProcXF86MiscGetKbdSettings(client);
    default:
	if (!xf86GetModInDevEnabled())
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86GetModInDevAllowNonLocal() || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return ProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return ProcXF86MiscSetKbdSettings(client);
		case X_XF86MiscSetGrabKeysState:
		    return ProcXF86MiscSetGrabKeysState(client);
	        default:
		    return BadRequest;
	    }
	} else
	    return miscErrorBase + XF86MiscModInDevClientNotLocal;
    }
}

static int
SProcXF86MiscQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXF86MiscQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcXF86MiscQueryVersion(client);
}

#ifdef _XF86MISC_SAVER_COMPAT_
static int
SProcXF86MiscGetSaver(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetSaverReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetSaverReq);
    swaps(&stuff->screen, n);
    return ProcXF86MiscGetSaver(client);
}

static int
SProcXF86MiscSetSaver(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetSaverReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetSaverReq);
    swaps(&stuff->screen, n);
    swapl(&stuff->suspendTime, n);
    swapl(&stuff->offTime, n);
    return ProcXF86MiscSetSaver(client);
}
#endif /* _XF86MISC_SAVER_COMPAT_ */

static int
SProcXF86MiscGetMouseSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetMouseSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetMouseSettingsReq);
    return ProcXF86MiscGetMouseSettings(client);
}

static int
SProcXF86MiscGetKbdSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetKbdSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetKbdSettingsReq);
    return ProcXF86MiscGetKbdSettings(client);
}

static int
SProcXF86MiscSetMouseSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetMouseSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetMouseSettingsReq);
    swapl(&stuff->mousetype, n);
    swapl(&stuff->baudrate, n);
    swapl(&stuff->samplerate, n);
    swapl(&stuff->resolution, n);
    swapl(&stuff->buttons, n);
    swapl(&stuff->emulate3timeout, n);
    swapl(&stuff->flags, n);
    return ProcXF86MiscSetMouseSettings(client);
}

static int
SProcXF86MiscSetKbdSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetKbdSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetKbdSettingsReq);
    swapl(&stuff->kbdtype, n);
    swapl(&stuff->rate, n);
    swapl(&stuff->delay, n);
    return ProcXF86MiscSetKbdSettings(client);
}

static int
SProcXF86MiscSetGrabKeysState(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetGrabKeysStateReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetGrabKeysStateReq);
    swaps(&stuff->enable, n);
    return ProcXF86MiscSetGrabKeysState(client);
}

static int
SProcXF86MiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86MiscQueryVersion:
	return SProcXF86MiscQueryVersion(client);
#ifdef _XF86MISC_SAVER_COMPAT_
    case X_XF86MiscGetSaver:
	return SProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return SProcXF86MiscSetSaver(client);
#endif
    case X_XF86MiscGetMouseSettings:
	return SProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return SProcXF86MiscGetKbdSettings(client);
    default:
	if (!xf86GetModInDevEnabled())
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86GetModInDevAllowNonLocal() || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return SProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return SProcXF86MiscSetKbdSettings(client);
	        case X_XF86MiscSetGrabKeysState:
		    return SProcXF86MiscSetGrabKeysState(client);
	        default:
		    return BadRequest;
	    }
	} else
	    return miscErrorBase + XF86MiscModInDevClientNotLocal;
    }
}

