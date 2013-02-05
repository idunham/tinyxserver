/*
 * $XFree86: xc/programs/Xserver/randr/randrstr.h,v 1.4 2001/06/03 21:52:44 keithp Exp $
 *
 * Copyright © 2000 Compaq Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Compaq makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * COMPAQ DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL COMPAQ BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RANDRSTR_H_
#define _RANDRSTR_H_

#include <X11/extensions/randr.h>

typedef struct _rrVisualGroup {
    int		id;
    int		nvisuals;
    VisualPtr	*visuals;
    Bool	referenced;
    Bool	oldReferenced;
} RRVisualGroup, *RRVisualGroupPtr;

typedef struct _rrGroupOfVisualGroup {
    int		id;
    int		ngroups;
    int		*groups;
    Bool	referenced;
    Bool	oldReferenced;
} RRGroupOfVisualGroup, *RRGroupOfVisualGroupPtr;

typedef struct _rrScreenSize {
    int		id;
    short	width, height;
    short	mmWidth, mmHeight;
    int		groupOfVisualGroups;
    Bool	referenced;
    Bool	oldReferenced;
} RRScreenSize, *RRScreenSizePtr;

typedef Bool (*RRSetConfigProcPtr) (ScreenPtr		pScreen,
				    Rotation		rotation,
				    RRScreenSizePtr	pSize,
				    RRVisualGroupPtr	pVisualGroup);

typedef Bool (*RRGetInfoProcPtr) (ScreenPtr pScreen, Rotation *rotations);
typedef Bool (*RRCloseScreenProcPtr) ( int i, ScreenPtr pscreen);
	
typedef struct _rrScrPriv {
    RRSetConfigProcPtr	    rrSetConfig;
    RRGetInfoProcPtr	    rrGetInfo;
    
    TimeStamp		    lastSetTime;	/* last changed by client */
    TimeStamp		    lastConfigTime;	/* possible configs changed */
    RRCloseScreenProcPtr    CloseScreen;

    /*
     * Configuration information
     */
    Rotation		    rotations;
    int			    swaps;
    
    int			    nVisualGroups;
    int			    nVisualGroupsInUse;
    RRVisualGroupPtr	    pVisualGroups;
    int			    nGroupsOfVisualGroups;
    int			    nGroupsOfVisualGroupsInUse;
    RRGroupOfVisualGroupPtr pGroupsOfVisualGroups;
    int			    nSizes;
    int			    nSizesInUse;
    RRScreenSizePtr	    pSizes;

    /*
     * Current state
     */
    Rotation		    rotation;
    RRScreenSizePtr	    pSize;
    RRVisualGroupPtr	    pVisualGroup;

} rrScrPrivRec, *rrScrPrivPtr;

extern int rrPrivIndex;

#define rrGetScrPriv(pScr)  ((rrScrPrivPtr) (pScr)->devPrivates[rrPrivIndex].ptr)
#define rrScrPriv(pScr)	rrScrPrivPtr    pScrPriv = rrGetScrPriv(pScr)
#define SetRRScreen(s,p) ((s)->devPrivates[rrPrivIndex].ptr = (pointer) (p))

/*
 * First, create the visual groups and register them with the screen
 */
RRVisualGroupPtr
RRCreateVisualGroup (ScreenPtr pScreen);

void
RRDestroyVisualGroup (ScreenPtr		pScreen,
		      RRVisualGroupPtr  pVisualGroup);

Bool
RRAddVisualToVisualGroup (ScreenPtr	    pScreen,
			  RRVisualGroupPtr  pVisualGroup,
			  VisualPtr	    pVisual);

Bool
RRAddDepthToVisualGroup (ScreenPtr	    pScreen,
			 RRVisualGroupPtr   pVisualGroup,
			 DepthPtr	    pDepth);

RRVisualGroupPtr
RRRegisterVisualGroup (ScreenPtr	pScreen,
		       RRVisualGroupPtr	pVisualGroup);

/*
 * Next, create the group of visual groups and register that with the screen
 */
RRGroupOfVisualGroupPtr
RRCreateGroupOfVisualGroup (ScreenPtr   pScreen);

void
RRDestroyGroupOfVisualGroup (ScreenPtr			pScreen,
			     RRGroupOfVisualGroupPtr	pGroupOfVisualGroup);

Bool
RRAddVisualGroupToGroupOfVisualGroup (ScreenPtr			pScreen,
				      RRGroupOfVisualGroupPtr	pGroupOfVisualGroup,
				      RRVisualGroupPtr		pVisualGroup);
				
				
RRGroupOfVisualGroupPtr
RRRegisterGroupOfVisualGroup (ScreenPtr			pScreen,
			      RRGroupOfVisualGroupPtr	pGroupOfVisualGroup);


/*
 * Then, register the specific size with the screen
 */

RRScreenSizePtr
RRRegisterSize (ScreenPtr		pScreen,
		short			width, 
		short			height,
		short			mmWidth,
		short			mmHeight,
		RRGroupOfVisualGroup    *visualgroups);

/*
 * Finally, set the current configuration of the screen
 */

void
RRSetCurrentConfig (ScreenPtr		pScreen,
		    Rotation		rotation,
		    RRScreenSizePtr	pSize,
		    RRVisualGroupPtr	pVisualGroup);

Bool
miRandRInit (ScreenPtr pScreen);

Bool
miRRSetConfig (ScreenPtr	pScreen,
	       Rotation		rotation,
	       RRScreenSizePtr	size,
	       RRVisualGroupPtr	pVisualGroup);

Bool
miRRGetScreenInfo (ScreenPtr pScreen);

#endif /* _RANDRSTR_H_ */
