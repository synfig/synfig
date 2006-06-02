/*
 * Common rootless definitions and code
 *
 * Greg Parker     gparker@cs.stanford.edu
 */

/* Copyright (c) 2002 Apple Computer, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
   HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name(s) of the above
   copyright holders shall not be used in advertising or otherwise to
   promote the sale, use or other dealings in this Software without
   prior written authorization. */

/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootlessCommon.c,v 1.6 2002/07/15 19:58:31 torrey Exp $ */

#include "rootless-common.h"
#include "dri-surface.h"
#include "fb.h"
#include "colormapst.h"

RegionRec rootlessHugeRoot = {{-32767, -32767, 32767, 32767}, NULL};

/* Following two macros from miregion.c */

/*  true iff two Boxes overlap */
#define EXTENTCHECK(r1,r2) \
      (!( ((r1)->x2 <= (r2)->x1)  || \
          ((r1)->x1 >= (r2)->x2)  || \
          ((r1)->y2 <= (r2)->y1)  || \
          ((r1)->y1 >= (r2)->y2) ) )

/* true iff Box r1 contains Box r2 */
#define SUBSUMES(r1,r2) \
      ( ((r1)->x1 <= (r2)->x1) && \
        ((r1)->x2 >= (r2)->x2) && \
        ((r1)->y1 <= (r2)->y1) && \
        ((r1)->y2 >= (r2)->y2) )

int rootlessNoDRIDrawing = 0;

/* Returns the top-level parent of pWindow. The root is the top-level
   parent of itself, even though the root is not otherwise considered
   to be a top-level window. */
WindowPtr
TopLevelParent (WindowPtr pWindow)
{
    WindowPtr top;

    if (IsRoot (pWindow))
	return pWindow;

    top = pWindow;
    while (top != NULL && !IsTopLevel (top))
	top = top->parent;

    return top;
}

/* Returns TRUE if this window is visible inside a frame (e.g. it is
   visible and has a top-level parent) */
Bool
IsFramedWindow (WindowPtr pWin)
{
    WindowPtr top;

    if (!pWin->realized)
	return FALSE;

    top = TopLevelParent (pWin);

    return top != NULL && WINREC (top) != NULL;
}

void
TranslatePixmapBase (PixmapPtr pPix, int dx, int dy)
{
    unsigned diff;

    pPix->devPrivate.ptr = ((char *) pPix->devPrivate.ptr +
			    ((dx + pPix->drawable.x)
			     * pPix->drawable.bitsPerPixel / 8 +
			     dy * pPix->devKind));

    if (pPix->drawable.bitsPerPixel != FB_UNIT)
    {
	diff = ((unsigned) pPix->devPrivate.ptr) & (FB_UNIT / CHAR_BIT - 1);
	pPix->devPrivate.ptr = ((char *) pPix->devPrivate.ptr) - diff;

	if (pPix->drawable.bitsPerPixel == 16)
	    pPix->drawable.x = diff / (16 / CHAR_BIT);
	else if (pPix->drawable.bitsPerPixel == 8)
	    pPix->drawable.x = diff / (8 / CHAR_BIT);
	else
	    pPix->drawable.x = diff / (pPix->drawable.bitsPerPixel / CHAR_BIT);
    }
}

void
RootlessDisableUpdate (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);

    if (winRec != NULL
	&& !winRec->is_offscreen
	&& !winRec->is_reorder_pending
	&& !winRec->is_update_disabled)
    {
	xp_disable_update ();
	winRec->is_update_disabled = TRUE;
    }
}

void
RootlessReenableUpdate (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);

    if (winRec != NULL && winRec->is_update_disabled)
    {
	xp_reenable_update ();
	winRec->is_update_disabled = FALSE;
    }
}

Bool
RootlessResolveColormap (ScreenPtr pScreen, int first_color,
			 int n_colors, uint32_t *colors)
{
    int last, i;
    ColormapPtr map;

    map = RootlessGetColormap (pScreen);
    if (map == NULL || map->class != PseudoColor)
	return FALSE;

    last = MIN (map->pVisual->ColormapEntries, first_color + n_colors);
    for (i = MAX (0, first_color); i < last; i++)
    {
	Entry *ent = map->red + i;
	uint16_t red, green, blue;

	if (!ent->refcnt)
	    continue;
	if (ent->fShared)
	{
	    red = ent->co.shco.red->color;
	    green = ent->co.shco.green->color;
	    blue = ent->co.shco.blue->color;
	}
	else
	{
	    red = ent->co.local.red;
	    green = ent->co.local.green;
	    blue = ent->co.local.blue;
	}

	colors[i - first_color] = (0xFF000000UL
				   | ((uint32_t) red & 0xff00) << 8
				   | (green & 0xff00)
				   | (blue >> 8));
    }

    return TRUE;
}

/* Prepare a window for direct access to its backing buffer. Each
   top-level parent has a Pixmap representing its backing store, which
   all of its children inherit. */
void
RootlessStartDrawing (WindowPtr pWindow)
{
    ScreenPtr pScreen = pWindow->drawable.pScreen;
    WindowPtr top = TopLevelParent (pWindow);
    RootlessWindowRec *winRec;

    if (!rootlessNoDRIDrawing && DRIStartDrawing (&pWindow->drawable))
    {
	return;
    }

    /* At the top of the stack now. */

    if (top == NULL || WINREC (top) == NULL)
	return;

    winRec = WINREC(top);

    /* Make sure the window's top-level parent is prepared for drawing. */

    if (!winRec->is_drawing)
    {
	void *data[2];
	unsigned int rowbytes[2];
	xp_error err;

        int bw = wBorderWidth (top);

	err = xp_lock_window (winRec->wid, NULL, NULL, data, rowbytes, NULL);
	if (err != Success)
	    abort ();			/* FIXME: */

	winRec->data = data[0];
	winRec->rowbytes = rowbytes[0];

        winRec->pixmap =
            GetScratchPixmapHeader(pScreen, winRec->width, winRec->height,
                                   winRec->win->drawable.depth,
                                   winRec->win->drawable.bitsPerPixel,
                                   winRec->rowbytes, winRec->data);
        TranslatePixmapBase (winRec->pixmap,
			     - (top->drawable.x - bw),
			     - (top->drawable.y - bw));

        winRec->is_drawing = TRUE;
    }

    winRec->oldPixmap = pScreen->GetWindowPixmap (pWindow);
    pScreen->SetWindowPixmap (pWindow, winRec->pixmap);
}

void
RootlessFinishedDrawing (WindowPtr pWindow)
{
    /* Called after each drawing primitive, lets us unlock surfaces
       as often as possible (which is a Good Thing to do.) */

    DRIStopDrawing (&pWindow->drawable, FALSE);

    /* Also, see if we're due a flush. */

    RootlessMayRedisplay (pWindow->drawable.pScreen);
}

void
RootlessStopDrawing (WindowPtr pWindow, Bool flush)
{
    WindowPtr top = TopLevelParent (pWindow);
    RootlessWindowRec *winRec;

    if (top == NULL || WINREC (top) == NULL)
	return;

    winRec = WINREC(top);

    DRIStopDrawing (&pWindow->drawable, flush);

    if (winRec->is_drawing)
    {
        ScreenPtr pScreen = pWindow->drawable.pScreen;

	xp_unlock_window (winRec->wid, flush);

        FreeScratchPixmapHeader (winRec->pixmap);
        pScreen->SetWindowPixmap (pWindow, winRec->oldPixmap);
        winRec->pixmap = NULL;

        winRec->is_drawing = FALSE;
    }
    else if (flush)
    {
	xp_flush_window (winRec->wid);
    }

    /* FIXME: instead of just checking if we tried to flush (which
       happens everytime we block for I/O), I used to check if
       anything was actually marked in the window. But that often
       caused problems with some window managers, and it didn't really
       make any noticeable difference, so... */

    if (flush && winRec->is_reorder_pending)
    {
	winRec->is_reorder_pending = FALSE;
	RootlessReorderWindow (pWindow);
    }

    if (flush && winRec->is_update_disabled)
    {
	RootlessReenableUpdate (pWindow);
    }
}

/* pRegion is GLOBAL */
void
RootlessDamageRegion (WindowPtr pWindow, RegionPtr pRegion)
{
    RootlessWindowRec *winRec;
    RegionRec clipped;
    WindowPtr pTop;
    BoxPtr b1, b2;

    RL_DEBUG_MSG("Damaged win 0x%x ", pWindow);

    pTop = TopLevelParent (pWindow);
    if (pTop == NULL)
	return;

    winRec = WINREC (pTop);
    if (winRec == NULL)
	return;

    if (DRIDamageRegion (&pWindow->drawable, pRegion))
    {
	return;
    }

    /* We need to intersect the drawn region with the clip of the window
       to avoid marking places we didn't actually draw (which can cause
       problems when the window has an extra client-side backing store)

       But this is a costly operation and since we'll normally just be
       drawing inside the clip, go to some lengths to avoid the general
       case intersection. */

    b1 = REGION_EXTENTS (pScreen, &pWindow->borderClip);
    b2 = REGION_EXTENTS (pScreen, pRegion);

    if (EXTENTCHECK (b1, b2))
    {
	/* Regions may overlap. */

	if (REGION_NUM_RECTS (pRegion) == 1)
	{
	    int in;

	    /* Damaged region only has a single rect, so we can
	       just compare that against the region */

	    in = RECT_IN_REGION (pScreen, &pWindow->borderClip,
				 REGION_RECTS (pRegion));
	    if (in == rgnIN)
	    {
		/* clip totally contains pRegion */

		xp_mark_window (winRec->wid, REGION_NUM_RECTS (pRegion),
				REGION_RECTS (pRegion),
				-winRec->x, -winRec->y);

		RootlessQueueRedisplay (pTop->drawable.pScreen);
		goto out;
	    }
	    else if (in == rgnOUT)
	    {
		/* clip doesn't contain pRegion */

		goto out;
	    }
	}

	/* clip overlaps pRegion, need to intersect */

	REGION_INIT (pScreen, &clipped, NullBox, 0);
	REGION_INTERSECT (pScreen, &clipped, &pWindow->borderClip, pRegion);

	xp_mark_window (winRec->wid, REGION_NUM_RECTS (&clipped),
			REGION_RECTS (&clipped), -winRec->x, -winRec->y);

	REGION_UNINIT (pScreen, &clipped);

	RootlessQueueRedisplay (pTop->drawable.pScreen);
    }

out:
#ifdef ROOTLESSDEBUG
    {
        BoxRec *box = REGION_RECTS(pRegion), *end;
        int numBox = REGION_NUM_RECTS(pRegion);

        for (end = box+numBox; box < end; box++) {
            RL_DEBUG_MSG("Damage rect: %i, %i, %i, %i\n",
                         box->x1, box->x2, box->y1, box->y2);
        }
    }
#endif
    return;
}

/* pBox is GLOBAL */
void
RootlessDamageBox (WindowPtr pWindow, BoxPtr pBox)
{
    RegionRec region;

    REGION_INIT (pWindow->drawable.pScreen, &region, pBox, 1);

    RootlessDamageRegion (pWindow, &region);

    REGION_UNINIT (pWindow->drawable.pScreen, &region);	/* no-op */
}


/* (x, y, w, h) is in window-local coordinates. */
void
RootlessDamageRect(WindowPtr pWindow, int x, int y, int w, int h)
{
    BoxRec box;
    RegionRec region;

    x += pWindow->drawable.x;
    y += pWindow->drawable.y;

    box.x1 = x;
    box.x2 = x + w;
    box.y1 = y;
    box.y2 = y + h;

    REGION_INIT (pWindow->drawable.pScreen, &region, &box, 1);

    RootlessDamageRegion (pWindow, &region);

    REGION_UNINIT (pWindow->drawable.pScreen, &region);	/* no-op */
}

/* Stop drawing and redisplay the damaged region of a window. */
void
RootlessRedisplay (WindowPtr pWindow)
{
    DRISynchronizeDrawable (&pWindow->drawable, TRUE);
    RootlessStopDrawing(pWindow, TRUE);
}

/* Walk every window on a screen and redisplay the damaged regions. */
void
RootlessRedisplayScreen (ScreenPtr pScreen)
{
    RootlessScreenRec *screenRec = SCREENREC (pScreen);
    WindowPtr root = WindowTable[pScreen->myNum];
    WindowPtr win;

    if (root != NULL)
    {
        RootlessRedisplay(root);

        for (win = root->firstChild; win; win = win->nextSib)
	{
            if (WINREC (win) != NULL)
                RootlessRedisplay (win);
        }
    }

    screenRec->last_redisplay = GetTimeInMillis ();
}

void
RootlessRepositionWindows (ScreenPtr pScreen)
{
    WindowPtr root = WindowTable[pScreen->myNum];
    WindowPtr win;

    if (root != NULL)
    {
        RootlessRepositionWindow (root);

        for (win = root->firstChild; win; win = win->nextSib)
	{
            if (WINREC (win) != NULL)
                RootlessRepositionWindow (win);
        }
    }
}

void
RootlessFlushScreenColormaps (ScreenPtr pScreen)
{
    WindowPtr root = WindowTable[pScreen->myNum];
    WindowPtr win;

    if (root != NULL)
    {
        RootlessRepositionWindow (root);

        for (win = root->firstChild; win; win = win->nextSib)
	{
            if (WINREC (win) != NULL)
                RootlessFlushWindowColormap (win);
        }
    }
}
