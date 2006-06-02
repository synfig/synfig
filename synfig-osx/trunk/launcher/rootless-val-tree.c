/*
 * Calculate window clip lists for rootless mode
 *
 * This file is very closely based on mivaltree.c.
 */
 /* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootlessValTree.c,v 1.2 2002/06/18 19:43:04 torrey Exp $ */

/*
 * mivaltree.c --
 *	Functions for recalculating window clip lists. Main function
 *	is miValidateTree.
 *

Copyright 1987, 1988, 1989, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

 *
 * Copyright 1987, 1988, 1989 by 
 * Digital Equipment Corporation, Maynard, Massachusetts,
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 ******************************************************************/

/* The panoramix components contained the following notice */
/****************************************************************
*                                                               *
*    Copyright (c) Digital Equipment Corporation, 1991, 1997    *
*                                                               *
*   All Rights Reserved.  Unpublished rights  reserved  under   *
*   the copyright laws of the United States.                    *
*                                                               *
*   The software contained on this media  is  proprietary  to   *
*   and  embodies  the  confidential  technology  of  Digital   *
*   Equipment Corporation.  Possession, use,  duplication  or   *
*   dissemination of the software and media is authorized only  *
*   pursuant to a valid written license from Digital Equipment  *
*   Corporation.                                                *
*                                                               *
*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
*   by the U.S. Government is subject to restrictions  as  set  *
*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
*   or  in  FAR 52.227-19, as applicable.                       *
*                                                               *
*****************************************************************/

 /* 
  * Aug '86: Susan Angebranndt -- original code
  * July '87: Adam de Boor -- substantially modified and commented
  * Summer '89: Joel McCormack -- so fast you wouldn't believe it possible.
  *             In particular, much improved code for window mapping and
  *             circulating.
  *		Bob Scheifler -- avoid miComputeClips for unmapped windows,
  *				 valdata changes
  */
#include    "X.h"
#include    "scrnintstr.h"
#include    "validate.h"
#include    "windowstr.h"
#include    "mi.h"
#include    "regionstr.h"
#include    "mivalidate.h"

#include    "globals.h"

#ifdef SHAPE
/*
 * Compute the visibility of a shaped window
 */
int
RootlessShapedWindowIn (pScreen, universe, bounding, rect, x, y)
    ScreenPtr	pScreen;
    RegionPtr	universe, bounding;
    BoxPtr	rect;
    register int x, y;
{
    BoxRec  box;
    register BoxPtr  boundBox;
    int	    nbox;
    Bool    someIn, someOut;
    register int t, x1, y1, x2, y2;

    nbox = REGION_NUM_RECTS (bounding);
    boundBox = REGION_RECTS (bounding);
    someIn = someOut = FALSE;
    x1 = rect->x1;
    y1 = rect->y1;
    x2 = rect->x2;
    y2 = rect->y2;
    while (nbox--)
    {
	if ((t = boundBox->x1 + x) < x1)
	    t = x1;
	box.x1 = t;
	if ((t = boundBox->y1 + y) < y1)
	    t = y1;
	box.y1 = t;
	if ((t = boundBox->x2 + x) > x2)
	    t = x2;
	box.x2 = t;
	if ((t = boundBox->y2 + y) > y2)
	    t = y2;
	box.y2 = t;
	if (box.x1 > box.x2)
	    box.x2 = box.x1;
	if (box.y1 > box.y2)
	    box.y2 = box.y1;
	switch (RECT_IN_REGION(pScreen, universe, &box))
	{
	case rgnIN:
	    if (someOut)
		return rgnPART;
	    someIn = TRUE;
	    break;
	case rgnOUT:
	    if (someIn)
		return rgnPART;
	    someOut = TRUE;
	    break;
	default:
	    return rgnPART;
	}
	boundBox++;
    }
    if (someIn)
	return rgnIN;
    return rgnOUT;
}
#endif

#define HasParentRelativeBorder(w) (!(w)->borderIsPixel && \
				    HasBorder(w) && \
				    (w)->backgroundState == ParentRelative)


/*
 *-----------------------------------------------------------------------
 * RootlessComputeClips --
 *	Recompute the clipList, borderClip, exposed and borderExposed
 *	regions for pParent and its children. Only viewable windows are
 *	taken into account.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	clipList, borderClip, exposed and borderExposed are altered.
 *	A VisibilityNotify event may be generated on the parent window.
 *
 *-----------------------------------------------------------------------
 */
static void
RootlessComputeClips (pParent, pScreen, universe, kind, exposed)
    register WindowPtr	pParent;
    register ScreenPtr	pScreen;
    register RegionPtr	universe;
    VTKind		kind;
    RegionPtr		exposed; /* for intermediate calculations */
{
    int			dx,
			dy;
    RegionRec		childUniverse;
    register WindowPtr	pChild;
    int     	  	oldVis, newVis;
    BoxRec		borderSize;
    RegionRec		childUnion;
    Bool		overlap;
    RegionPtr		borderVisible;
    Bool		resized;
    /*
     * Figure out the new visibility of this window.
     * The extent of the universe should be the same as the extent of
     * the borderSize region. If the window is unobscured, this rectangle
     * will be completely inside the universe (the universe will cover it
     * completely). If the window is completely obscured, none of the
     * universe will cover the rectangle.
     */
    borderSize.x1 = pParent->drawable.x - wBorderWidth(pParent);
    borderSize.y1 = pParent->drawable.y - wBorderWidth(pParent);
    dx = (int) pParent->drawable.x + (int) pParent->drawable.width + wBorderWidth(pParent);
    if (dx > 32767)
	dx = 32767;
    borderSize.x2 = dx;
    dy = (int) pParent->drawable.y + (int) pParent->drawable.height + wBorderWidth(pParent);
    if (dy > 32767)
	dy = 32767;
    borderSize.y2 = dy;

    oldVis = pParent->visibility;
    switch (RECT_IN_REGION( pScreen, universe, &borderSize)) 
    {
    case rgnIN:
	    newVis = VisibilityUnobscured;
	    break;
	case rgnPART:
	    newVis = VisibilityPartiallyObscured;
#ifdef SHAPE
	    {
		RegionPtr   pBounding;

		if ((pBounding = wBoundingShape (pParent)))
		{
		    switch (RootlessShapedWindowIn (pScreen, universe, 
						    pBounding, &borderSize,
						    pParent->drawable.x,
						    pParent->drawable.y))
		    {
		    case rgnIN:
			newVis = VisibilityUnobscured;
			break;
		    case rgnOUT:
			newVis = VisibilityFullyObscured;
			break;
		    }
		}
	    }
#endif
	    break;
	default:
	    newVis = VisibilityFullyObscured;
	    break;
    }

    pParent->visibility = newVis;
    if (oldVis != newVis &&
	((pParent->eventMask | wOtherEventMasks(pParent)) & VisibilityChangeMask))
	SendVisibilityNotify(pParent);

    dx = pParent->drawable.x - pParent->valdata->before.oldAbsCorner.x;
    dy = pParent->drawable.y - pParent->valdata->before.oldAbsCorner.y;

    /*
     * avoid computations when dealing with simple operations
     */

    switch (kind) {
    case VTMap:
    case VTStack:
    case VTUnmap:
	break;
    case VTMove:
	if ((oldVis == newVis) &&
	    ((oldVis == VisibilityFullyObscured) ||
	     (oldVis == VisibilityUnobscured)))
	{
	    pChild = pParent;
	    while (1)
	    {
		if (pChild->viewable)
		{
		    if (pChild->visibility != VisibilityFullyObscured)
		    {
			REGION_TRANSLATE( pScreen, &pChild->borderClip,
						      dx, dy);
			REGION_TRANSLATE( pScreen, &pChild->clipList,
						      dx, dy);
			pChild->drawable.serialNumber = NEXT_SERIAL_NUMBER;
			if (pScreen->ClipNotify)
			    (* pScreen->ClipNotify) (pChild, dx, dy);

		    }
		    if (pChild->valdata)
		    {
			REGION_INIT(pScreen, 
				    &pChild->valdata->after.borderExposed,
				    NullBox, 0);
			if (HasParentRelativeBorder(pChild))
			  {
			    REGION_SUBTRACT(pScreen,
					 &pChild->valdata->after.borderExposed,
					 &pChild->borderClip,
					 &pChild->winSize);
			}
			REGION_INIT( pScreen, &pChild->valdata->after.exposed,
						 NullBox, 0);
		    }
		    if (pChild->firstChild)
		    {
			pChild = pChild->firstChild;
			continue;
		    }
		}
		while (!pChild->nextSib && (pChild != pParent))
		    pChild = pChild->parent;
		if (pChild == pParent)
		    break;
		pChild = pChild->nextSib;
	    }
	    return;
	}
	/* fall through */
    default:
    	/*
     	 * To calculate exposures correctly, we have to translate the old
     	 * borderClip and clipList regions to the window's new location so there
     	 * is a correspondence between pieces of the new and old clipping regions.
     	 */
    	if (dx || dy) 
    	{
	    /*
	     * We translate the old clipList because that will be exposed or copied
	     * if gravity is right.
	     */
	    REGION_TRANSLATE( pScreen, &pParent->borderClip, dx, dy);
	    REGION_TRANSLATE( pScreen, &pParent->clipList, dx, dy);
    	} 
	break;
    case VTBroken:
	REGION_EMPTY (pScreen, &pParent->borderClip);
	REGION_EMPTY (pScreen, &pParent->clipList);
	break;
    }

    borderVisible = pParent->valdata->before.borderVisible;
    resized = pParent->valdata->before.resized;
    REGION_INIT( pScreen, &pParent->valdata->after.borderExposed, NullBox, 0);
    REGION_INIT( pScreen, &pParent->valdata->after.exposed, NullBox, 0);

    /*
     * Since the borderClip must not be clipped by the children, we do
     * the border exposure first...
     *
     * 'universe' is the window's borderClip. To figure the exposures, remove
     * the area that used to be exposed from the new.
     * This leaves a region of pieces that weren't exposed before.
     */

    if (HasBorder (pParent))
    {
    	if (borderVisible)
    	{
	    /*
	     * when the border changes shape, the old visible portions
	     * of the border will be saved by DIX in borderVisible --
	     * use that region and destroy it
	     */
	    REGION_SUBTRACT( pScreen, exposed, universe, borderVisible);
	    REGION_DESTROY( pScreen, borderVisible);
    	}
    	else
    	{
	    REGION_SUBTRACT( pScreen, exposed, universe, &pParent->borderClip);
    	}
	if (HasParentRelativeBorder(pParent) && (dx || dy)) {
	    REGION_SUBTRACT( pScreen, &pParent->valdata->after.borderExposed,
				  universe,
				  &pParent->winSize);
	} else {
	    REGION_SUBTRACT( pScreen, &pParent->valdata->after.borderExposed,
			       exposed, &pParent->winSize);
	}

    	REGION_COPY( pScreen, &pParent->borderClip, universe);
    
    	/*
     	 * To get the right clipList for the parent, and to make doubly sure
     	 * that no child overlaps the parent's border, we remove the parent's
     	 * border from the universe before proceeding.
     	 */
    
    	REGION_INTERSECT( pScreen, universe, universe, &pParent->winSize);
    }
    else
    	REGION_COPY( pScreen, &pParent->borderClip, universe);
    
    if ((pChild = pParent->firstChild) && pParent->mapped)
    {
	REGION_INIT(pScreen, &childUniverse, NullBox, 0);
	REGION_INIT(pScreen, &childUnion, NullBox, 0);
	if ((pChild->drawable.y < pParent->lastChild->drawable.y) ||
	    ((pChild->drawable.y == pParent->lastChild->drawable.y) &&
	     (pChild->drawable.x < pParent->lastChild->drawable.x)))
	{
	    for (; pChild; pChild = pChild->nextSib)
	    {
		if (pChild->viewable)
		    REGION_APPEND( pScreen, &childUnion, &pChild->borderSize);
	    }
	}
	else
	{
	    for (pChild = pParent->lastChild; pChild; pChild = pChild->prevSib)
	    {
		if (pChild->viewable)
		    REGION_APPEND( pScreen, &childUnion, &pChild->borderSize);
	    }
	}
	REGION_VALIDATE( pScreen, &childUnion, &overlap);

	for (pChild = pParent->firstChild;
	     pChild;
	     pChild = pChild->nextSib)
 	{
	    if (pChild->viewable) {
		/*
		 * If the child is viewable, we want to remove its extents
		 * from the current universe, but we only re-clip it if
		 * it's been marked.
		 */
		if (pChild->valdata) {
		    /*
		     * Figure out the new universe from the child's
		     * perspective and recurse.
		     */
		    REGION_INTERSECT( pScreen, &childUniverse,
					    universe,
					    &pChild->borderSize);
		    RootlessComputeClips (pChild, pScreen, &childUniverse, 
					  kind, exposed);
		}
		/*
		 * Once the child has been processed, we remove its extents
		 * from the current universe, thus denying its space to any
		 * other sibling.
		 */
		if (overlap)
		    REGION_SUBTRACT( pScreen, universe, universe,
					  &pChild->borderSize);
	    }
	}
	if (!overlap)
	    REGION_SUBTRACT( pScreen, universe, universe, &childUnion);
	REGION_UNINIT( pScreen, &childUnion);
	REGION_UNINIT( pScreen, &childUniverse);
    } /* if any children */

    /*
     * 'universe' now contains the new clipList for the parent window.
     *
     * To figure the exposure of the window we subtract the old clip from the
     * new, just as for the border.
     */

    if (oldVis == VisibilityFullyObscured ||
	oldVis == VisibilityNotViewable)
    {
	REGION_COPY( pScreen, &pParent->valdata->after.exposed, universe);
    }
    else if (newVis != VisibilityFullyObscured &&
	     newVis != VisibilityNotViewable)
    {
    	REGION_SUBTRACT( pScreen, &pParent->valdata->after.exposed,
			       universe, &pParent->clipList);
    }

    /*
     * One last thing: backing storage. We have to try to save what parts of
     * the window are about to be obscured. We can just subtract the universe
     * from the old clipList and get the areas that were in the old but aren't
     * in the new and, hence, are about to be obscured.
     */
    if (pParent->backStorage && !resized)
    {
	REGION_SUBTRACT( pScreen, exposed, &pParent->clipList, universe);
	(* pScreen->SaveDoomedAreas)(pParent, exposed, dx, dy);
    }
    
    /* HACK ALERT - copying contents of regions, instead of regions */
    {
	RegionRec   tmp;

	tmp = pParent->clipList;
	pParent->clipList = *universe;
	*universe = tmp;
    }

#ifdef NOTDEF
    REGION_COPY( pScreen, &pParent->clipList, universe);
#endif

    pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    if (pScreen->ClipNotify)
	(* pScreen->ClipNotify) (pParent, dx, dy);
}

static void
RootlessTreeObscured(pParent)
    register WindowPtr pParent;
{
    register WindowPtr pChild;
    register int    oldVis;

    pChild = pParent;
    while (1)
    {
	if (pChild->viewable)
	{
	    oldVis = pChild->visibility;
	    if (oldVis != (pChild->visibility = VisibilityFullyObscured) &&
		((pChild->eventMask | wOtherEventMasks(pChild)) & VisibilityChangeMask))
		SendVisibilityNotify(pChild);
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pParent))
	    pChild = pChild->parent;
	if (pChild == pParent)
	    break;
	pChild = pChild->nextSib;
    }
}

/*
 *-----------------------------------------------------------------------
 * RootlessMiValidateTree --
 *	Recomputes the clip list for pParent and all its inferiors.
 *
 * Results:
 *	Always returns 1.
 *
 * Side Effects:
 *	The clipList, borderClip, exposed, and borderExposed regions for
 *	each marked window are altered.
 *
 * Notes:
 *	This routine assumes that all affected windows have been marked
 *	(valdata created) and their winSize and borderSize regions
 *	adjusted to correspond to their new positions. The borderClip and
 *	clipList regions should not have been touched.
 *
 *	The top-most level is treated differently from all lower levels
 *	because pParent is unchanged. For the top level, we merge the
 *	regions taken up by the marked children back into the clipList
 *	for pParent, thus forming a region from which the marked children
 *	can claim their areas. For lower levels, where the old clipList
 *	and borderClip are invalid, we can't do this and have to do the
 *	extra operations done in miComputeClips, but this is much faster
 *	e.g. when only one child has moved...
 *
 *-----------------------------------------------------------------------
 */
/* 
   Quartz version: used for validate from root in rootless mode.
   We need to make sure top-level windows don't clip each other, 
   and that top-level windows aren't clipped to the root window.
*/
/*ARGSUSED*/
// fixme this is ugly
// Xprint/ValTree.c doesn't work, but maybe that method can?
int
RootlessMiValidateTree (pRoot, pChild, kind)
    WindowPtr	  	pRoot;      /* Parent to validate */
    WindowPtr	  	pChild;     /* First child of pRoot that was
				     * affected */
    VTKind    	  	kind;       /* What kind of configuration caused call */
{
    RegionRec	  	childClip;  /* The new borderClip for the current
				     * child */
    RegionRec		exposed;    /* For intermediate calculations */
    register ScreenPtr	pScreen;
    register WindowPtr	pWin;

    pScreen = pRoot->drawable.pScreen;
    if (pChild == NullWindow)
	pChild = pRoot->firstChild;

    REGION_INIT(pScreen, &childClip, NullBox, 0);
    REGION_INIT(pScreen, &exposed, NullBox, 0);

    if (REGION_BROKEN (pScreen, &pRoot->clipList) &&
	!REGION_BROKEN (pScreen, &pRoot->borderClip))
    {
        // fixme this might not work, but hopefully doesn't happen anyway.
        kind = VTBroken;
        REGION_EMPTY (pScreen, &pRoot->clipList);
        ErrorF("ValidateTree: BUSTED!\n");
    }

    /* 
     * Recursively compute the clips for all children of the root. 
     * They don't clip against each other or the root itself, so 
     * childClip is always reset to that child's size.
     */

    for (pWin = pChild;
	 pWin != NullWindow;
	 pWin = pWin->nextSib)
    {
        if (pWin->viewable) {
            if (pWin->valdata) {
                REGION_COPY( pScreen, &childClip, &pWin->borderSize);
                RootlessComputeClips (pWin, pScreen, &childClip, kind, &exposed);
            } else if (pWin->visibility == VisibilityNotViewable) {
                RootlessTreeObscured(pWin);
            }
        } else {
            if (pWin->valdata) {
                REGION_EMPTY( pScreen, &pWin->clipList);
                if (pScreen->ClipNotify)
                    (* pScreen->ClipNotify) (pWin, 0, 0);
                REGION_EMPTY( pScreen, &pWin->borderClip);
                pWin->valdata = (ValidatePtr)NULL;
            }
        }
    }

    REGION_UNINIT( pScreen, &childClip);

    /* The root is never clipped by its children, so nothing on the root 
       is ever exposed by moving or mapping its children. */
    REGION_INIT( pScreen, &pRoot->valdata->after.exposed, NullBox, 0);
    REGION_INIT( pScreen, &pRoot->valdata->after.borderExposed, NullBox, 0);

    return 1;
}
