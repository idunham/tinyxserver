/*
 * $XFree86: xc/programs/Xserver/randr/randr.c,v 1.12 2001/07/20 19:30:11 keithp Exp $
 *
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Compaq makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * COMPAQ DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL COMPAQ
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, Compaq Computer Corporation, Inc.
 */

#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "servermd.h"
#include "randr.h"
#include "randrproto.h"
#include "randrstr.h"
#include <X11/Xfuncproto.h>
#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

#define RR_VALIDATE
int	RRGeneration;
int	RRNScreens;

static int ProcRRQueryVersion (ClientPtr pClient);
static int ProcRRDispatch (ClientPtr pClient);
static int SProcRRDispatch (ClientPtr pClient);
static int SProcRRQueryVersion (ClientPtr pClient);

#define wrap(priv,real,mem,func) {\
    priv->mem = real->mem; \
    real->mem = func; \
}

#define unwrap(priv,real,mem) {\
    real->mem = priv->mem; \
}

static CARD8	RRReqCode;
static int	RRErrBase;
static int	RREventBase;
static RESTYPE ClientType, EventType; /* resource types for event masks */

/*
 * each window has a list of clients requesting
 * RRNotify events.  Each client has a resource
 * for each window it selects RRNotify input for,
 * this resource is used to delete the RRNotifyRec
 * entry from the per-window queue.
 */

typedef struct _RREvent *RREventPtr;

typedef struct _RREvent {
    RREventPtr  next;
    ClientPtr	client;
    WindowPtr	window;
    XID		clientResource;
} RREventRec;

int	rrPrivIndex = -1;

static void
RRResetProc (ExtensionEntry *extEntry)
{
}

    
static Bool
RRCloseScreen (int i, ScreenPtr pScreen)
{
    rrScrPriv(pScreen);

    unwrap (pScrPriv, pScreen, CloseScreen);
    if (pScrPriv->pSizes)
	xfree (pScrPriv->pSizes);
    if (pScrPriv->pGroupsOfVisualGroups)
	xfree (pScrPriv->pGroupsOfVisualGroups);
    if (pScrPriv->pVisualGroups)
	xfree (pScrPriv->pVisualGroups);
    xfree (pScrPriv);
    RRNScreens -= 1;	/* ok, one fewer screen with RandR running */
    return (*pScreen->CloseScreen) (i, pScreen);    
}

static void
SRRScreenChangeNotifyEvent(from, to)
    xRRScreenChangeNotifyEvent *from, *to;
{
    to->type = from->type;
    to->rotation = from->rotation;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->timestamp, to->timestamp);
    cpswapl(from->configTimestamp, to->configTimestamp);
    cpswapl(from->root, to->root);
    cpswapl(from->window, to->window);
    cpswaps(from->sizeID, to->sizeID);
    cpswaps(from->visualGroupID, to->visualGroupID);
    cpswaps(from->widthInPixels, to->widthInPixels);
    cpswaps(from->heightInPixels, to->heightInPixels);
    cpswaps(from->widthInMillimeters, to->widthInMillimeters);
    cpswaps(from->heightInMillimeters, to->heightInMillimeters);
}

Bool RRScreenInit(ScreenPtr pScreen)
{
    rrScrPrivPtr   pScrPriv;

    if (RRGeneration != serverGeneration)
    {
	if ((rrPrivIndex = AllocateScreenPrivateIndex()) < 0)
	    return FALSE;
	RRGeneration = serverGeneration;
    }

    pScrPriv = (rrScrPrivPtr) xalloc (sizeof (rrScrPrivRec));
    if (!pScrPriv)
	return FALSE;

    SetRRScreen(pScreen, pScrPriv);

    /*
     * Calling function best set these function vectors
     */
    pScrPriv->rrSetConfig = 0;
    pScrPriv->rrGetInfo = 0;
    /*
     * This value doesn't really matter -- any client must call
     * GetScreenInfo before reading it which will automatically update
     * the time
     */
    pScrPriv->lastSetTime = currentTime;
    pScrPriv->lastConfigTime = currentTime;
    
    wrap (pScrPriv, pScreen, CloseScreen, RRCloseScreen);

    pScrPriv->rotations = RR_Rotate_0;
    pScrPriv->swaps = 0;
    pScrPriv->nVisualGroups = 0;
    pScrPriv->nVisualGroupsInUse = 0;
    pScrPriv->pVisualGroups = 0;
    
    pScrPriv->nGroupsOfVisualGroups = 0;
    pScrPriv->nGroupsOfVisualGroupsInUse = 0;
    pScrPriv->pGroupsOfVisualGroups = 0;
    
    pScrPriv->nSizes = 0;
    pScrPriv->nSizesInUse = 0;
    pScrPriv->pSizes = 0;
    
    pScrPriv->rotation = RR_Rotate_0;
    pScrPriv->pSize = 0;
    pScrPriv->pVisualGroup = 0;
    
    RRNScreens += 1;	/* keep count of screens that implement randr */
    return TRUE;
}

/*ARGSUSED*/
static int
RRFreeClient (pointer data, XID id)
{
    RREventPtr   pRREvent;
    WindowPtr	    pWin;
    RREventPtr   *pHead, pCur, pPrev;

    pRREvent = (RREventPtr) data;
    pWin = pRREvent->window;
    pHead = (RREventPtr *) LookupIDByType(pWin->drawable.id, EventType);
    if (pHead) {
	pPrev = 0;
	for (pCur = *pHead; pCur && pCur != pRREvent; pCur=pCur->next)
	    pPrev = pCur;
	if (pCur)
	{
	    if (pPrev)
	    	pPrev->next = pRREvent->next;
	    else
	    	*pHead = pRREvent->next;
	}
    }
    xfree ((pointer) pRREvent);
    return 1;
}

/*ARGSUSED*/
static int
RRFreeEvents (pointer data, XID id)
{
    RREventPtr   *pHead, pCur, pNext;

    pHead = (RREventPtr *) data;
    for (pCur = *pHead; pCur; pCur = pNext) {
	pNext = pCur->next;
	FreeResource (pCur->clientResource, ClientType);
	xfree ((pointer) pCur);
    }
    xfree ((pointer) pHead);
    return 1;
}

void
RRExtensionInit (void)
{
    ExtensionEntry *extEntry;

    if (RRNScreens == 0) return;

    ClientType = CreateNewResourceType(RRFreeClient);
    if (!ClientType)
	return;
    EventType = CreateNewResourceType(RRFreeEvents);
    if (!EventType)
	return;
    extEntry = AddExtension (RANDR_NAME, RRNumberEvents, RRNumberErrors,
			     ProcRRDispatch, SProcRRDispatch,
			     RRResetProc, StandardMinorOpcode);
    if (!extEntry)
	return;
    RRReqCode = (CARD8) extEntry->base;
    RRErrBase = extEntry->errorBase;
    RREventBase = extEntry->eventBase;
    EventSwapVector[RREventBase + RRScreenChangeNotify] = (EventSwapPtr) 
      SRRScreenChangeNotifyEvent;

    return;
}
		
static int
TellChanged (WindowPtr pWin, pointer value)
{
    RREventPtr			*pHead, pRREvent;
    ClientPtr			client;
    xRRScreenChangeNotifyEvent	se;
    ScreenPtr			pScreen = pWin->drawable.pScreen;
    rrScrPriv(pScreen);
    RRScreenSizePtr		pSize = pScrPriv->pSize;
    WindowPtr			pRoot = WindowTable[pScreen->myNum];

    pHead = (RREventPtr *) LookupIDByType (pWin->drawable.id, EventType);
    if (!pHead)
	return WT_WALKCHILDREN;

    se.type = RRScreenChangeNotify + RREventBase;
    se.rotation = (CARD8) pScrPriv->rotation;
    se.timestamp = pScrPriv->lastSetTime.milliseconds;
    se.configTimestamp = pScrPriv->lastConfigTime.milliseconds;
    se.root = pRoot->drawable.id;
    se.window = pWin->drawable.id;
    if (pSize)
    {
	se.sizeID = pSize->id;
	se.visualGroupID = pScrPriv->pVisualGroup->id;
	se.widthInPixels = pSize->width;
	se.heightInPixels = pSize->height;
	se.widthInMillimeters = pSize->mmWidth;
	se.heightInMillimeters = pSize->mmHeight;
    }
    else
    {
	/*
	 * This "shouldn't happen", but a broken DDX can
	 * forget to set the current configuration on GetInfo
	 */
	se.sizeID = 0xffff;
	se.visualGroupID = 0xffff;
	se.widthInPixels = 0;
	se.heightInPixels = 0;
	se.widthInMillimeters = 0;
	se.heightInMillimeters = 0;
    }    

    for (pRREvent = *pHead; pRREvent; pRREvent = pRREvent->next) 
    {
	client = pRREvent->client;
	if (client == serverClient || client->clientGone)
	    continue;
	se.sequenceNumber = client->sequence;
	WriteEventsToClient (client, 1, (xEvent *) &se);
    }
    return WT_WALKCHILDREN;
}

static Bool
RRGetInfo (ScreenPtr pScreen)
{
    rrScrPriv (pScreen);
    int		    i, j;
    Bool	    changed;
    Rotation	    rotations;

    for (i = 0; i < pScrPriv->nVisualGroups; i++)
    {
	pScrPriv->pVisualGroups[i].oldReferenced = pScrPriv->pVisualGroups[i].referenced;
	pScrPriv->pVisualGroups[i].referenced = FALSE;
    }
    for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
    {
	pScrPriv->pGroupsOfVisualGroups[i].oldReferenced = pScrPriv->pGroupsOfVisualGroups[i].referenced;
	pScrPriv->pGroupsOfVisualGroups[i].referenced = FALSE;
    }
    for (i = 0; i < pScrPriv->nSizes; i++)
    {
	pScrPriv->pSizes[i].oldReferenced = pScrPriv->pSizes[i].referenced;
	pScrPriv->pSizes[i].referenced = FALSE;
    }
    if (!(*pScrPriv->rrGetInfo) (pScreen, &rotations))
	return FALSE;

    changed = FALSE;

    /*
     * Check whether anything changed and simultaneously generate
     * the protocol id values for the objects
     */
    if (rotations != pScrPriv->rotations)
    {
	pScrPriv->rotations = rotations;
	changed = TRUE;
    }

    j = 0;
    for (i = 0; i < pScrPriv->nVisualGroups; i++)
    {
	if (pScrPriv->pVisualGroups[i].oldReferenced != pScrPriv->pVisualGroups[i].referenced)
	    changed = TRUE;
	if (pScrPriv->pVisualGroups[i].referenced)
	    pScrPriv->pVisualGroups[i].id = j++;
    }
    pScrPriv->nVisualGroupsInUse = j;
    j = 0;
    for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
    {
	if (pScrPriv->pGroupsOfVisualGroups[i].oldReferenced != pScrPriv->pGroupsOfVisualGroups[i].referenced)
	    changed = TRUE;
	if (pScrPriv->pGroupsOfVisualGroups[i].referenced)
	    pScrPriv->pGroupsOfVisualGroups[i].id = j++;
    }
    pScrPriv->nGroupsOfVisualGroupsInUse = j;
    j = 0;
    for (i = 0; i < pScrPriv->nSizes; i++)
    {
	if (pScrPriv->pSizes[i].oldReferenced != pScrPriv->pSizes[i].referenced)
	    changed = TRUE;
	if (pScrPriv->pSizes[i].referenced)
	    pScrPriv->pSizes[i].id = j++;
    }
    pScrPriv->nSizesInUse = j;
    if (changed)
    {
	UpdateCurrentTime ();
	pScrPriv->lastConfigTime = currentTime;
	WalkTree (pScreen, TellChanged, (pointer) pScreen);
    }
    return TRUE;
}

static void
RRSendConfigNotify (ScreenPtr pScreen)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    xEvent	event;

    event.u.u.type = ConfigureNotify;
    event.u.configureNotify.window = pWin->drawable.id;
    event.u.configureNotify.aboveSibling = None;
    event.u.configureNotify.x = 0;
    event.u.configureNotify.y = 0;

    /* XXX xinerama stuff ? */
    
    event.u.configureNotify.width = pWin->drawable.width;
    event.u.configureNotify.height = pWin->drawable.height;
    event.u.configureNotify.borderWidth = wBorderWidth (pWin);
    event.u.configureNotify.override = pWin->overrideRedirect;
    DeliverEvents(pWin, &event, 1, NullWindow);
}

static int
ProcRRQueryVersion (ClientPtr client)
{
    xRRQueryVersionReply rep;
    register int n;
    REQUEST(xRRQueryVersionReq);

    REQUEST_SIZE_MATCH(xRRQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = RANDR_MAJOR;
    rep.minorVersion = RANDR_MINOR;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xRRQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static Bool
RRVisualGroupContains (RRVisualGroupPtr pVisualGroup,
		     VisualID	    visual)
{
    int	    i;

    for (i = 0; i < pVisualGroup->nvisuals; i++)
	if (pVisualGroup->visuals[i]->vid == visual)
	    return TRUE;
    return FALSE;
}

static CARD16
RRNumMatchingVisualGroups (ScreenPtr  pScreen,
			 VisualID   visual)
{
    rrScrPriv(pScreen);
    int		    i;
    CARD16	    n = 0;
    RRVisualGroupPtr  pVisualGroup;

    for (i = 0; i < pScrPriv->nVisualGroups; i++)
    {
	pVisualGroup = &pScrPriv->pVisualGroups[i];
	if (pVisualGroup->referenced && RRVisualGroupContains (pVisualGroup, visual))
	    n++;
    }
    return n;
}

static void
RRGetMatchingVisualGroups (ScreenPtr	pScreen,
			 VisualID	visual,
			 VisualGroupID	*pVisualGroupIDs)
{
    rrScrPriv(pScreen);
    int		    i;
    CARD16	    n = 0;
    RRVisualGroupPtr  pVisualGroup;

    for (i = 0; i < pScrPriv->nVisualGroups; i++)
    {
	pVisualGroup = &pScrPriv->pVisualGroups[i];
	if (pVisualGroup->referenced && RRVisualGroupContains (pVisualGroup, visual))
	    *pVisualGroupIDs++ = pVisualGroup->id;
    }
}

extern char	*ConnectionInfo;

static int padlength[4] = {0, 3, 2, 1};

static void
RREditConnectionInfo (ScreenPtr pScreen)
{
    xConnSetup	    *connSetup;
    char	    *vendor;
    xPixmapFormat   *formats;
    xWindowRoot	    *root;
    xDepth	    *depth;
    xVisualType	    *visual;
    int		    screen = 0;
    int		    d;

    connSetup = (xConnSetup *) ConnectionInfo;
    vendor = (char *) connSetup + sizeof (xConnSetup);
    formats = (xPixmapFormat *) ((char *) vendor +
				 connSetup->nbytesVendor +
				 padlength[connSetup->nbytesVendor & 3]);
    root = (xWindowRoot *) ((char *) formats +
			    sizeof (xPixmapFormat) * screenInfo.numPixmapFormats);
    while (screen != pScreen->myNum)
    {
	depth = (xDepth *) ((char *) root + 
			    sizeof (xWindowRoot));
	for (d = 0; d < root->nDepths; d++)
	{
	    visual = (xVisualType *) ((char *) depth +
				      sizeof (xDepth));
	    depth = (xDepth *) ((char *) visual +
				depth->nVisuals * sizeof (xVisualType));
	}
	root = (xWindowRoot *) ((char *) depth);
	screen++;
    }
    root->pixWidth = pScreen->width;
    root->pixHeight = pScreen->height;
    root->mmWidth = pScreen->mmWidth;
    root->mmHeight = pScreen->mmHeight;
}

static int
ProcRRGetScreenInfo (ClientPtr client)
{
    REQUEST(xRRGetScreenInfoReq);
    xRRGetScreenInfoReply   rep;
    WindowPtr	    	    pWin;
    int			    n;
    ScreenPtr		    pScreen;
    rrScrPrivPtr	    pScrPriv;
    CARD8		    *extra;
    int			    extraLen;

    REQUEST_SIZE_MATCH(xRRGetScreenInfoReq);
    pWin = (WindowPtr)SecurityLookupWindow(stuff->window, client,
					   SecurityReadAccess);

    if (!pWin)
	return BadWindow;

    pScreen = pWin->drawable.pScreen;
    pScrPriv = rrGetScrPriv(pScreen);
    if (!pScrPriv)
    {
	rep.type = X_Reply;
	rep.setOfRotations = RR_Rotate_0;;
	rep.sequenceNumber = client->sequence;
	rep.length = 0;
	rep.root = WindowTable[pWin->drawable.pScreen->myNum]->drawable.id;
	rep.timestamp = currentTime.milliseconds;
	rep.configTimestamp = currentTime.milliseconds;
	rep.nVisualGroups = 0;
	rep.nGroupsOfVisualGroups = 0;
	rep.nSizes = 0;
	rep.sizeID = 0;
	rep.visualGroupID = 0;
	rep.rotation = RR_Rotate_0;
	extra = 0;
	extraLen = 0;
    }
    else
    {
	int			i, j;
	int			nGroupsOfVisualGroupsElements;
	RRGroupOfVisualGroupPtr pGroupsOfVisualGroups;
	int			nVisualGroupElements;
	RRVisualGroupPtr	pVisualGroup;
	xScreenSizes		*size;
	CARD16			*data16;
	CARD32			*data32;
    
	RRGetInfo (pScreen);

	rep.type = X_Reply;
	rep.setOfRotations = pScrPriv->rotations;
	rep.sequenceNumber = client->sequence;
	rep.length = 0;
	rep.root = WindowTable[pWin->drawable.pScreen->myNum]->drawable.id;
	rep.timestamp = pScrPriv->lastSetTime.milliseconds;
	rep.configTimestamp = pScrPriv->lastConfigTime.milliseconds;
	
	rep.nVisualGroups = pScrPriv->nVisualGroupsInUse;
	rep.rotation = pScrPriv->rotation;
	rep.nSizes = pScrPriv->nSizesInUse;
	rep.nGroupsOfVisualGroups = pScrPriv->nGroupsOfVisualGroupsInUse;
	if (pScrPriv->pSize)
	    rep.sizeID = pScrPriv->pSize->id;
	else
	    return BadImplementation;
	if (pScrPriv->pVisualGroup)
	    rep.visualGroupID = pScrPriv->pVisualGroup->id;
	else
	    return BadImplementation;
	/*
	 * Count up the total number of spaces needed to transmit
	 * the groups of visual groups
	 */
	nGroupsOfVisualGroupsElements = 0;
	for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
	{
	    pGroupsOfVisualGroups = &pScrPriv->pGroupsOfVisualGroups[i];
	    if (pGroupsOfVisualGroups->referenced)
		nGroupsOfVisualGroupsElements += pGroupsOfVisualGroups->ngroups + 1;
	}
	/*
	 * Count up the total number of spaces needed to transmit
	 * the visual groups
	 */
	nVisualGroupElements = 0;
	for (i = 0; i < pScrPriv->nVisualGroups; i++)
	{
	    pVisualGroup = &pScrPriv->pVisualGroups[i];
	    if (pVisualGroup->referenced)
		nVisualGroupElements += pVisualGroup->nvisuals + 1;
	}
	/*
	 * Allocate space for the extra information
	 */
	extraLen = (rep.nSizes * sizeof (xScreenSizes) +
		    nVisualGroupElements * sizeof (CARD32) +
		    nGroupsOfVisualGroupsElements * sizeof (CARD16));
	extra = (CARD8 *) xalloc (extraLen);
	if (!extra)
	    return BadAlloc;
	/*
	 * First comes the size information
	 */
	size = (xScreenSizes *) extra;
	for (i = 0; i < pScrPriv->nSizes; i++)
	{
	    if (pScrPriv->pSizes[i].referenced)
	    {
		size->widthInPixels = pScrPriv->pSizes[i].width;
		size->heightInPixels = pScrPriv->pSizes[i].height;
		size->widthInMillimeters = pScrPriv->pSizes[i].mmWidth;
		size->heightInMillimeters = pScrPriv->pSizes[i].mmHeight;
		size->visualGroup = pScrPriv->pGroupsOfVisualGroups[pScrPriv->pSizes[i].groupOfVisualGroups].id;
		if (client->swapped)
		{
		    swaps (&size->widthInPixels, n);
		    swaps (&size->heightInPixels, n);
		    swaps (&size->widthInMillimeters, n);
		    swaps (&size->heightInMillimeters, n);
		    swaps (&size->visualGroup, n);
		}
		size++;
	    }
	}
	data32 = (CARD32 *) size;
	/*
	 * Next comes the visual groups
	 */
	for (i = 0; i < pScrPriv->nVisualGroups; i++)
	{
	    pVisualGroup = &pScrPriv->pVisualGroups[i];
	    if (pVisualGroup->referenced)
	    {
		*data32++ = pVisualGroup->nvisuals;
		for (j = 0; j < pVisualGroup->nvisuals; j++)
		    *data32++ = pVisualGroup->visuals[j]->vid;
	    }
	}
	if (client->swapped)
	    SwapLongs (data32 - nVisualGroupElements, nVisualGroupElements);
	/*
	 * Next comes the groups of visual groups
	 */
	data16 = (CARD16 *) data32;
	for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
	{
	    pGroupsOfVisualGroups = &pScrPriv->pGroupsOfVisualGroups[i];
	    if (pGroupsOfVisualGroups->referenced)
	    {
		*data16++ = (CARD16) pGroupsOfVisualGroups->ngroups;
		for (j = 0; j < pGroupsOfVisualGroups->ngroups; j++)
		{
		    pVisualGroup = &pScrPriv->pVisualGroups[pGroupsOfVisualGroups->groups[j]];
		    *data16++ = (CARD16) pVisualGroup->id;
		}
	    }
	}
	
	if (client->swapped)
	    SwapShorts ((CARD16 *) data32, data16 - (CARD16 *) data32);
	
	if ((CARD8 *) data16 - (CARD8 *) extra != extraLen)
	    FatalError ("RRGetScreenInfo bad extra len %d != %d\n",
			(CARD8 *) data16 - (CARD8 *) extra, extraLen);
	rep.length = extraLen >> 2;
    }
    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.timestamp, n);
	swaps(&rep.rotation, n);
	swaps(&rep.nSizes, n);
	swaps(&rep.nVisualGroups, n);
	swaps(&rep.sizeID, n);
	swaps(&rep.visualGroupID, n);
    }
    WriteToClient(client, sizeof(xRRGetScreenInfoReply), (char *)&rep);
    if (extraLen)
    {
	WriteToClient (client, extraLen, extra);
	xfree (extra);
    }
    return (client->noClientException);
}

static int
ProcRRSetScreenConfig (ClientPtr client)
{
    REQUEST(xRRSetScreenConfigReq);
    xRRSetScreenConfigReply rep;
    DrawablePtr		    pDraw;
    int			    n;
    ScreenPtr		    pScreen;
    rrScrPrivPtr	    pScrPriv;
    TimeStamp		    configTime;
    TimeStamp		    time;
    RRScreenSizePtr	    pSize;
    RRVisualGroupPtr	    pVisualGroup;
    RRGroupOfVisualGroupPtr pGroupsOfVisualGroups;
    int			    i;
    Rotation		    rotation;
    short		    oldWidth, oldHeight;

    UpdateCurrentTime ();

    REQUEST_SIZE_MATCH(xRRSetScreenConfigReq);
    SECURITY_VERIFY_DRAWABLE(pDraw, stuff->drawable, client,
			     SecurityWriteAccess);

    pScreen = pDraw->pScreen;

    pScrPriv= rrGetScrPriv(pScreen);
    
    time = ClientTimeToServerTime(stuff->timestamp);
    configTime = ClientTimeToServerTime(stuff->configTimestamp);
    
    oldWidth = pScreen->width;
    oldHeight = pScreen->height;
    
    if (!pScrPriv)
    {
	time = currentTime;
	rep.status = RRSetConfigFailed;
	goto sendReply;
    }
    if (!RRGetInfo (pScreen))
	return BadAlloc;
    
    /*
     * if the client's config timestamp is not the same as the last config
     * timestamp, then the config information isn't up-to-date and
     * can't even be validated
     */
    if (CompareTimeStamps (configTime, pScrPriv->lastConfigTime) != 0)
    {
	rep.status = RRSetConfigInvalidConfigTime;
	goto sendReply;
    }
    
    /*
     * Search for the requested size
     */
    for (i = 0; i < pScrPriv->nSizes; i++)
    {
	pSize = &pScrPriv->pSizes[i];
	if (pSize->referenced && pSize->id == stuff->sizeID)
	    break;
    }
    if (i == pScrPriv->nSizes)
    {
	/*
	 * Invalid size ID
	 */
	client->errorValue = stuff->sizeID;
	return BadValue;
    }
    
    /*
     * Search for the requested visual group
     */
    for (i = 0; i < pScrPriv->nVisualGroups; i++)
    {
	pVisualGroup = &pScrPriv->pVisualGroups[i];
	if (pVisualGroup->referenced && pVisualGroup->id == stuff->visualGroupID)
	    break;
    }
    if (i == pScrPriv->nVisualGroups)
    {
	/*
	 * Invalid group ID
	 */
	client->errorValue = stuff->visualGroupID;
	return BadValue;
    }
    
    /*
     * Make sure visualgroup is supported by size
     */
    pGroupsOfVisualGroups = &pScrPriv->pGroupsOfVisualGroups[pSize->groupOfVisualGroups];
    for (i = 0; i < pGroupsOfVisualGroups->ngroups; i++)
    {
	if (pGroupsOfVisualGroups->groups[i] == pVisualGroup - pScrPriv->pVisualGroups)
	    break;
    }
    if (i == pGroupsOfVisualGroups->ngroups)
    {
	/*
	 * requested group not supported by requested size
	 */
	return BadMatch;
    }

    /*
     * Validate requested rotation
     */
    rotation = (Rotation) stuff->rotation;
    switch (rotation) {
    case RR_Rotate_0:
    case RR_Rotate_90:
    case RR_Rotate_180:
    case RR_Rotate_270:
	break;
    default:
	/*
	 * Invalid rotation
	 */
	client->errorValue = stuff->rotation;
	return BadValue;
    }
    if (!(pScrPriv->rotations & rotation))
    {
	/*
	 * requested rotation not supported by screen
	 */
	return BadMatch;
    }
    
    /*
     * Make sure the requested set-time is not older than
     * the last set-time
     */
    if (CompareTimeStamps (time, pScrPriv->lastSetTime) < 0)
    {
	rep.status = RRSetConfigInvalidTime;
	goto sendReply;
    }

    /*
     * call out to ddx routine to effect the change
     */
    if (!(*pScrPriv->rrSetConfig) (pScreen, rotation, 
					pSize, pVisualGroup))
    {
	/*
	 * unknown DDX failure, report to client
	 */
	rep.status = RRSetConfigFailed;
	goto sendReply;
    }
    
    /*
     * set current extension configuration pointers
     */
    RRSetCurrentConfig (pScreen, rotation, pSize, pVisualGroup);
    
    /*
     * Deliver ScreenChangeNotify events whenever
     * the configuration is updated
     */
    WalkTree (pScreen, TellChanged, (pointer) pScreen);
    
    /*
     * Deliver ConfigureNotify events when root changes
     * pixel size
     */
    if (oldWidth != pScreen->width || oldHeight != pScreen->height)
	RRSendConfigNotify (pScreen);
    RREditConnectionInfo (pScreen);
    
    /*
     * Fix pointer bounds and location
     */
    ScreenRestructured (pScreen);
    pScrPriv->lastSetTime = time;
    
    /*
     * Report Success
     */
    rep.status = RRSetConfigSuccess;
    
sendReply:
    
    rep.type = X_Reply;
    /* rep.status has already been filled in */
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    rep.newTimestamp = pScrPriv->lastSetTime.milliseconds;
    rep.newConfigTimestamp = pScrPriv->lastConfigTime.milliseconds;
    rep.root = WindowTable[pDraw->pScreen->myNum]->drawable.id;
    
    if (client->swapped) 
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.newTimestamp, n);
	swapl(&rep.newConfigTimestamp, n);
	swapl(&rep.root, n);
    }
    WriteToClient(client, sizeof(xRRSetScreenConfigReply), (char *)&rep);

    return (client->noClientException);
}

static int
ProcRRScreenChangeSelectInput (ClientPtr client)
{
    REQUEST(xRRScreenChangeSelectInputReq);
    WindowPtr	pWin;
    RREventPtr	pRREvent, pNewRREvent, *pHead;
    XID		clientResource;

    REQUEST_SIZE_MATCH(xRRScreenChangeSelectInputReq);
    pWin = SecurityLookupWindow (stuff->window, client, SecurityWriteAccess);
    if (!pWin)
	return BadWindow;
    pHead = (RREventPtr *)SecurityLookupIDByType(client,
			pWin->drawable.id, EventType, SecurityWriteAccess);
    switch (stuff->enable) {
    case xTrue:
	if (pHead) {

	    /* check for existing entry. */
	    for (pRREvent = *pHead;
		 pRREvent;
 		 pRREvent = pRREvent->next)
	    {
		if (pRREvent->client == client)
		    return Success;
	    }
	}

	/* build the entry */
    	pNewRREvent = (RREventPtr)
			    xalloc (sizeof (RREventRec));
    	if (!pNewRREvent)
	    return BadAlloc;
    	pNewRREvent->next = 0;
    	pNewRREvent->client = client;
    	pNewRREvent->window = pWin;
    	/*
 	 * add a resource that will be deleted when
     	 * the client goes away
     	 */
   	clientResource = FakeClientID (client->index);
    	pNewRREvent->clientResource = clientResource;
    	if (!AddResource (clientResource, ClientType, (pointer)pNewRREvent))
	    return BadAlloc;
    	/*
     	 * create a resource to contain a pointer to the list
     	 * of clients selecting input.  This must be indirect as
     	 * the list may be arbitrarily rearranged which cannot be
     	 * done through the resource database.
     	 */
    	if (!pHead)
    	{
	    pHead = (RREventPtr *) xalloc (sizeof (RREventPtr));
	    if (!pHead ||
	    	!AddResource (pWin->drawable.id, EventType, (pointer)pHead))
	    {
	    	FreeResource (clientResource, RT_NONE);
	    	return BadAlloc;
	    }
	    *pHead = 0;
    	}
    	pNewRREvent->next = *pHead;
    	*pHead = pNewRREvent;
	break;
    case xFalse:
	/* delete the interest */
	if (pHead) {
	    pNewRREvent = 0;
	    for (pRREvent = *pHead; pRREvent; pRREvent = pRREvent->next) {
		if (pRREvent->client == client)
		    break;
		pNewRREvent = pRREvent;
	    }
	    if (pRREvent) {
		FreeResource (pRREvent->clientResource, ClientType);
		if (pNewRREvent)
		    pNewRREvent->next = pRREvent->next;
		else
		    *pHead = pRREvent->next;
		xfree (pRREvent);
	    }
	}
	break;
    default:
	client->errorValue = stuff->enable;
	return BadValue;
    }
    return Success;
}

static int
ProcRRDispatch (ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_RRQueryVersion:
	return ProcRRQueryVersion(client);
    case X_RRGetScreenInfo:
        return ProcRRGetScreenInfo(client);
    case X_RRSetScreenConfig:
        return ProcRRSetScreenConfig(client);
    case X_RRScreenChangeSelectInput:
        return ProcRRScreenChangeSelectInput(client);
    default:
	return BadRequest;
    }
}

static int
SProcRRQueryVersion (ClientPtr client)
{
    register int n;
    REQUEST(xRRQueryVersionReq);

    swaps(&stuff->length, n);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return ProcRRQueryVersion(client);
}

static int
SProcRRGetScreenInfo (ClientPtr client)
{
    register int n;
    REQUEST(xRRGetScreenInfoReq);

    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return ProcRRGetScreenInfo(client);
}

static int
SProcRRSetScreenConfig (ClientPtr client)
{
    register int n;
    REQUEST(xRRSetScreenConfigReq);

    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->timestamp, n);
    swaps(&stuff->sizeID, n);
    swaps(&stuff->visualGroupID, n);
    swaps(&stuff->rotation, n);
    return ProcRRSetScreenConfig(client);
}

static int
SProcRRScreenChangeSelectInput (ClientPtr client)
{
    register int n;
    REQUEST(xRRScreenChangeSelectInputReq);

    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return ProcRRScreenChangeSelectInput(client);
}

static int
SProcRRDispatch (ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_RRQueryVersion:
	return SProcRRQueryVersion(client);
    case X_RRGetScreenInfo:
        return SProcRRGetScreenInfo(client);
    case X_RRSetScreenConfig:
        return SProcRRSetScreenConfig(client);
    case X_RRScreenChangeSelectInput:
        return SProcRRScreenChangeSelectInput(client);
    default:
	return BadRequest;
    }
}

/*
 * Utility functions for creating the group of possible
 * configurations
 */

RRVisualGroupPtr
RRCreateVisualGroup (ScreenPtr pScreen)
{
    RRVisualGroupPtr  pVisualGroup;
    
    pVisualGroup = (RRVisualGroupPtr) xalloc (sizeof (RRVisualGroup));
    pVisualGroup->nvisuals = 0;
    pVisualGroup->visuals = 0;
    pVisualGroup->referenced = TRUE;
    pVisualGroup->oldReferenced = FALSE;
    return pVisualGroup;
}

void
RRDestroyVisualGroup (ScreenPtr	    pScreen,
		    RRVisualGroupPtr  pVisualGroup)
{
#ifdef RR_VALIDATE
    int	i;
    rrScrPriv(pScreen);

    for (i = 0; i < pScrPriv->nVisualGroups; i++)
	if (pVisualGroup == &pScrPriv->pVisualGroups[i])
	    FatalError ("Freeing registered visual group");
#endif
    xfree (pVisualGroup->visuals);
    xfree (pVisualGroup);
}

Bool
RRAddVisualToVisualGroup (ScreenPtr	pScreen,
			RRVisualGroupPtr	pVisualGroup,
			VisualPtr	pVisual)
{
    VisualPtr	*new;

    new = xrealloc (pVisualGroup->visuals, 
		    (pVisualGroup->nvisuals + 1) * sizeof (VisualPtr));
    if (!new)
	return FALSE;
    (pVisualGroup->visuals = new)[pVisualGroup->nvisuals++] = pVisual;
    return TRUE;
}

Bool
RRAddDepthToVisualGroup (ScreenPtr	pScreen,
		       RRVisualGroupPtr	pVisualGroup,
		       DepthPtr		pDepth)
{
    int		i;
    int		v;

    for (i = 0; i < pDepth->numVids; i++)
	for (v = 0; v < pScreen->numVisuals; v++)
	    if (pScreen->visuals[v].vid == pDepth->vids[i])
		if (!RRAddVisualToVisualGroup (pScreen, pVisualGroup,
					     &pScreen->visuals[v]))
		    return FALSE;
    return TRUE;
}

/*
 * Return true if a and b reference the same group of visuals
 */

static Bool
RRVisualGroupMatches (RRVisualGroupPtr  a,
		    RRVisualGroupPtr  b)
{
    int	ai, bi;
    
    if (a->nvisuals != b->nvisuals)
	return FALSE;
    for (ai = 0; ai < a->nvisuals; ai++)
    {
	for (bi = 0; bi < b->nvisuals; bi++)
	    if (a->visuals[ai] == b->visuals[bi])
		break;
	if (bi == b->nvisuals)
	    return FALSE;
    }
    return TRUE;
}

RRVisualGroupPtr
RRRegisterVisualGroup (ScreenPtr	    pScreen,
		     RRVisualGroupPtr pVisualGroup)
{
    rrScrPriv (pScreen);
    int	    i;
    RRVisualGroupPtr  pNew;

    if (!pScrPriv)
    {
	RRDestroyVisualGroup (pScreen, pVisualGroup);
	return 0;
    }
    for (i = 0; i < pScrPriv->nVisualGroups; i++)
	if (RRVisualGroupMatches (pVisualGroup,
				&pScrPriv->pVisualGroups[i]))
	{
	    RRDestroyVisualGroup (pScreen, pVisualGroup);
	    pScrPriv->pVisualGroups[i].referenced = TRUE;
	    return &pScrPriv->pVisualGroups[i];
	}
    pNew = xrealloc (pScrPriv->pVisualGroups,
		     (pScrPriv->nVisualGroups + 1) * sizeof (RRVisualGroup));
    if (!pNew)
    {
	RRDestroyVisualGroup (pScreen, pVisualGroup);
	return 0;
    }
    pNew[pScrPriv->nVisualGroups++] = *pVisualGroup;
    xfree (pVisualGroup);
    pScrPriv->pVisualGroups = pNew;
    return &pNew[pScrPriv->nVisualGroups-1];
}

RRGroupOfVisualGroupPtr
RRCreateGroupOfVisualGroup (ScreenPtr pScreen)
{
    RRGroupOfVisualGroupPtr  pGroupOfVisualGroup;
    
    pGroupOfVisualGroup = (RRGroupOfVisualGroupPtr) xalloc (sizeof (RRGroupOfVisualGroup));
    pGroupOfVisualGroup->ngroups = 0;
    pGroupOfVisualGroup->groups = 0;
    pGroupOfVisualGroup->referenced = TRUE;
    pGroupOfVisualGroup->oldReferenced = FALSE;
    return pGroupOfVisualGroup;
}

void
RRDestroyGroupOfVisualGroup (ScreenPtr		pScreen,
			 RRGroupOfVisualGroupPtr	pGroupOfVisualGroup)
{
#ifdef RR_VALIDATE
    int	i;
    rrScrPriv(pScreen);

    for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
	if (pGroupOfVisualGroup == &pScrPriv->pGroupsOfVisualGroups[i])
	    FatalError ("Freeing registered visual group");
#endif
    xfree (pGroupOfVisualGroup->groups);
    xfree (pGroupOfVisualGroup);
}

Bool
RRAddVisualGroupToGroupOfVisualGroup (ScreenPtr	    pScreen,
				RRGroupOfVisualGroupPtr pGroupOfVisualGroup,
				RRVisualGroupPtr	    pVisualGroup)
{
    rrScrPriv(pScreen);
    int		*new;

#ifdef RR_VALIDATE
    int	i;
    for (i = 0; i < pScrPriv->nVisualGroups; i++)
	if (pVisualGroup == &pScrPriv->pVisualGroups[i])
	    break;

    if (i == pScrPriv->nVisualGroups)
	FatalError ("Adding unregistered visual group");
#endif
    new = (int*) xrealloc (pGroupOfVisualGroup->groups, 
			   (pGroupOfVisualGroup->ngroups + 1) * sizeof (int *));
    if (!new)
	return FALSE;
    (pGroupOfVisualGroup->groups = new)[pGroupOfVisualGroup->ngroups++] = pVisualGroup - pScrPriv->pVisualGroups;
    return TRUE;
}

/*
 * Return true if a and b reference the same group of groups
 */

static Bool
RRGroupOfVisualGroupMatches (RRGroupOfVisualGroupPtr  a,
			 RRGroupOfVisualGroupPtr  b)
{
    int	ai, bi;
    
    if (a->ngroups != b->ngroups)
	return FALSE;
    for (ai = 0; ai < a->ngroups; ai++)
    {
	for (bi = 0; bi < b->ngroups; bi++)
	    if (a->groups[ai] == b->groups[bi])
		break;
	if (bi == b->ngroups)
	    return FALSE;
    }
    return TRUE;
}

RRGroupOfVisualGroupPtr
RRRegisterGroupOfVisualGroup (ScreenPtr		pScreen,
			  RRGroupOfVisualGroupPtr	pGroupOfVisualGroup)
{
    rrScrPriv (pScreen);
    int			i;
    RRGroupOfVisualGroupPtr pNew;

    if (!pScrPriv)
    {
	RRDestroyGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
	return 0;
    }
    for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
	if (RRGroupOfVisualGroupMatches (pGroupOfVisualGroup,
				     &pScrPriv->pGroupsOfVisualGroups[i]))
	{
	    RRDestroyGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
	    pScrPriv->pGroupsOfVisualGroups[i].referenced = TRUE;
	    return &pScrPriv->pGroupsOfVisualGroups[i];
	}
    pNew = xrealloc (pScrPriv->pGroupsOfVisualGroups,
		     (pScrPriv->nGroupsOfVisualGroups + 1) * sizeof (RRGroupOfVisualGroup));
    if (!pNew)
    {
	RRDestroyGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
	return 0;
    }
    pNew[pScrPriv->nGroupsOfVisualGroups++] = *pGroupOfVisualGroup;
    xfree (pGroupOfVisualGroup);
    pScrPriv->pGroupsOfVisualGroups = pNew;
    return &pNew[pScrPriv->nGroupsOfVisualGroups-1];
}

static Bool
RRScreenSizeMatches (RRScreenSizePtr  a,
		   RRScreenSizePtr  b)
{
    if (a->width != b->width)
	return FALSE;
    if (a->height != b->height)
	return FALSE;
    if (a->mmWidth != b->mmWidth)
	return FALSE;
    if (a->mmHeight != b->mmHeight)
	return FALSE;
    if (a->groupOfVisualGroups != b->groupOfVisualGroups)
	return FALSE;
    return TRUE;
}

RRScreenSizePtr
RRRegisterSize (ScreenPtr	    pScreen,
		short		    width, 
		short		    height,
		short		    mmWidth,
		short		    mmHeight,
		RRGroupOfVisualGroup    *pGroupsOfVisualGroups)
{
    rrScrPriv (pScreen);
    int		    i;
    RRScreenSize	    tmp;
    RRScreenSizePtr   pNew;

    if (!pScrPriv)
	return 0;
    
#ifdef RR_VALIDATE
    for (i = 0; i < pScrPriv->nGroupsOfVisualGroups; i++)
	if (pGroupsOfVisualGroups == &pScrPriv->pGroupsOfVisualGroups[i])
	    break;

    if (i == pScrPriv->nGroupsOfVisualGroups)
	FatalError ("Adding unregistered group of visual groups");
#endif
    
    tmp.width = width;
    tmp.height= height;
    tmp.mmWidth = mmWidth;
    tmp.mmHeight = mmHeight;
    tmp.groupOfVisualGroups = pGroupsOfVisualGroups - pScrPriv->pGroupsOfVisualGroups;
    tmp.referenced = TRUE;
    tmp.oldReferenced = FALSE;
    for (i = 0; i < pScrPriv->nSizes; i++)
	if (RRScreenSizeMatches (&tmp, &pScrPriv->pSizes[i]))
	{
	    pScrPriv->pSizes[i].referenced = TRUE;
	    return &pScrPriv->pSizes[i];
	}
    pNew = xrealloc (pScrPriv->pSizes,
		     (pScrPriv->nSizes + 1) * sizeof (RRScreenSize));
    if (!pNew)
	return 0;
    pNew[pScrPriv->nSizes++] = tmp;
    pScrPriv->pSizes = pNew;
    return &pNew[pScrPriv->nSizes-1];
}

void
RRSetCurrentConfig (ScreenPtr		pScreen,
		    Rotation		rotation,
		    RRScreenSizePtr	pSize,
		    RRVisualGroupPtr	pVisualGroup)
{
    rrScrPriv (pScreen);

    if (!pScrPriv)
	return;

    pScrPriv->rotation = rotation;
    pScrPriv->pSize = pSize;
    pScrPriv->pVisualGroup = pVisualGroup;
}
