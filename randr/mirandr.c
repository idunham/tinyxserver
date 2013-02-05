/*
 * $XFree86: xc/programs/Xserver/randr/mirandr.c,v 1.5 2001/06/04 09:45:40 keithp Exp $
 *
 * Copyright © 2001 Compaq Computer Corporation
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
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jim Gettys, Compaq Computer Corporation
 */

#include "scrnintstr.h"
#include "mi.h"
#include <X11/extensions/randr.h>
#include "randrstr.h"
#include <stdio.h>

/*
 * This function assumes that only a single depth can be
 * displayed at a time, but that all visuals of that depth
 * can be displayed simultaneously.  It further assumes that
 * only a single size is available.  Hardware providing
 * additional capabilties should use different code.
 */

Bool
miRRGetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    int	i;
    Bool setConfig = FALSE;
    
    *rotations = RR_Rotate_0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	if (pScreen->allowedDepths[i].numVids)
	{
	    RRVisualGroupPtr  pVisualGroup;

	    pVisualGroup = RRCreateVisualGroup (pScreen);
	    if (pVisualGroup)
	    {
		RRGroupOfVisualGroupPtr pGroupOfVisualGroup;
		RRScreenSizePtr		pSize;

		if (!RRAddDepthToVisualGroup (pScreen,
					      pVisualGroup,
					      &pScreen->allowedDepths[i]))
		{
		    RRDestroyVisualGroup (pScreen, pVisualGroup);
		    return FALSE;
		}
		pVisualGroup = RRRegisterVisualGroup (pScreen, pVisualGroup);
		if (!pVisualGroup)
		    return FALSE;

		pGroupOfVisualGroup = RRCreateGroupOfVisualGroup (pScreen);
		
		if (!RRAddVisualGroupToGroupOfVisualGroup (pScreen,
						     pGroupOfVisualGroup,
						     pVisualGroup))
		{
		    RRDestroyGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
		    /* pVisualGroup left until screen closed */
		    return FALSE;
		}

		pSize = RRRegisterSize (pScreen,
					pScreen->width,
					pScreen->height,
					pScreen->mmWidth,
					pScreen->mmHeight,
					pGroupOfVisualGroup);
		if (!pSize)
		    return FALSE;
		if (!setConfig)
		{
		    RRSetCurrentConfig (pScreen, RR_Rotate_0, pSize, pVisualGroup);
		    setConfig = TRUE;
		}
	    }
	}
    }
}

/*
 * Any hardware that can actually change anything will need something
 * different here
 */
Bool
miRRSetConfig (ScreenPtr	pScreen,
	       Rotation		rotation,
	       RRScreenSizePtr	pSize,
	       RRVisualGroupPtr	pVisualGroup)
{
    return TRUE;
}


Bool
miRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr    rp;
    
    if (!RRScreenInit (pScreen))
	return FALSE;
    rp = rrGetScrPriv(pScreen);
    rp->rrGetInfo = miRRGetInfo;
    rp->rrSetConfig = miRRSetConfig;
    return TRUE;
}
