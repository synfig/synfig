/*
 * Screen routines for Mac OS X rootless X server
 *
 * Greg Parker     gparker@cs.stanford.edu
 *
 * February 2001  Created
 * March 3, 2001  Restructured as generic rootless mode
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

/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootlessScreen.c,v 1.2 2002/04/03 00:06:32 torrey Exp $ */


#include "mi.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "propertyst.h"
#include "mivalidate.h"
#include "picturestr.h"
#include "os.h"
#include "servermd.h"
#include "colormapst.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rootless-common.h"
#include "rootless-window.h"

/* In milcroseconds */
#define REDISPLAY_DELAY     10
#define REDISPLAY_MAX_DELAY 60

extern int RootlessMiValidateTree(WindowPtr pRoot,
				  WindowPtr pChild, VTKind kind);
extern Bool RootlessCreateGC(GCPtr pGC);

int rootlessGCPrivateIndex = -1;
int rootlessScreenPrivateIndex = -1;
int rootlessWindowPrivateIndex = -1;

void
RootlessUpdateScreenPixmap (ScreenPtr pScreen)
{
    RootlessScreenRec *s = SCREENREC(pScreen);
    PixmapPtr pPix;
    unsigned int rowbytes;

    pPix = (*pScreen->GetScreenPixmap) (pScreen);
    if (pPix == NULL)
    {
	pPix = (*pScreen->CreatePixmap) (pScreen, 0, 0, pScreen->rootDepth);
	(*pScreen->SetScreenPixmap) (pPix);
    }

    rowbytes = PixmapBytePad (pScreen->width, pScreen->rootDepth);

    if (s->pixmap_data_size < rowbytes)
    {
	if (s->pixmap_data != NULL)
	    xfree (s->pixmap_data);

	s->pixmap_data_size = rowbytes;
	s->pixmap_data = xalloc (s->pixmap_data_size);
	if (s->pixmap_data == NULL)
	    return;

	xp_fill_bytes (s->pixmap_data_size, 1, ~0,
		       s->pixmap_data, s->pixmap_data_size);

	pScreen->ModifyPixmapHeader (pPix, pScreen->width, pScreen->height,
				     pScreen->rootDepth,
				     BitsPerPixel (pScreen->rootDepth),
				     0, s->pixmap_data);
	/* ModifyPixmapHeader ignores zero arguments, so install rowbytes
	   by hand. */
	pPix->devKind = 0;
    }
}

static Bool
RootlessCreateScreenResources (ScreenPtr pScreen)
{
    Bool ret = TRUE;

    SCREEN_UNWRAP (pScreen, CreateScreenResources);

    if (pScreen->CreateScreenResources != NULL)
	ret = (*pScreen->CreateScreenResources) (pScreen);
   
    SCREEN_WRAP(pScreen, CreateScreenResources);

    if (!ret)
	return ret;

    /* miCreateScreenResources doesn't like our null framebuffer pointer,
       it leaves the screen pixmap with an uninitialized data pointer. So
       we gave it depth=0,bits=0, which says, leave it the fsck alone.
       So we have some work to do since we need the screen pixmap to be
       valid (e.g. CopyArea from the root window) */

    RootlessUpdateScreenPixmap (pScreen);

    return ret;
}

static Bool
RootlessCloseScreen(int i, ScreenPtr pScreen)
{
    RootlessScreenRec *s;

    s = SCREENREC(pScreen);

    /* FIXME: unwrap everything that was wrapped? */
    pScreen->CloseScreen = s->CloseScreen;

    if (s->pixmap_data != NULL)
    {
	xfree (s->pixmap_data);
	s->pixmap_data = NULL;
	s->pixmap_data_size = 0;
    }

    xfree(s);
    return pScreen->CloseScreen(i, pScreen);
}

static void
RootlessGetImage(DrawablePtr pDrawable, int sx, int sy, int w, int h,
                 unsigned int format, unsigned long planeMask, char *pdstLine)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    SCREEN_UNWRAP(pScreen, GetImage);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	int x0, y0, x1, y1;

	RootlessWindowRec *winRec;

        /* Many apps use GetImage to sync with the visible frame buffer
           FIXME: entire screen or just window or all screens? */
        RootlessRedisplayScreen (pScreen);

        /* RedisplayScreen stops drawing, so we need to start it again */
        RootlessStartDrawing ((WindowPtr) pDrawable);

	/* Check that we have some place to read from. */
	winRec = WINREC (TopLevelParent ((WindowPtr) pDrawable));
	if (winRec == NULL)
	    goto out;

	/* Clip to top-level window bounds. */

	x0 = pDrawable->x + sx;
	y0 = pDrawable->y + sy;
	x1 = x0 + w;
	y1 = y0 + h;

	x0 = MAX (x0, winRec->x);
	y0 = MAX (y0, winRec->y);
	x1 = MIN (x1, winRec->x + (int) winRec->width);
	y1 = MIN (y1, winRec->y + (int) winRec->height);

	/* FIXME: if clipped we need to adjust the data returned from
	   fbGetImage (), since it calculates the destination stride
	   from the passed in width.. */

	sx = x0 - pDrawable->x;
	sy = y0 - pDrawable->y;
	w = x1 - x0;
	h = y1 - y0;

	if (w <= 0 || h <= 0)
	    goto out;
    }

    pScreen->GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);

out:
    SCREEN_WRAP(pScreen, GetImage);
}

/*
 * RootlessSourceValidate
 *  CopyArea and CopyPlane use a GC tied to the destination drawable.
 *  StartDrawing/StopDrawing wrappers won't be called if source is
 *  a visible window but the destination isn't. So, we call StartDrawing
 *  here and leave StopDrawing for the block handler.
 */
static void
RootlessSourceValidate(DrawablePtr pDrawable, int x, int y, int w, int h)
{
    SCREEN_UNWRAP(pDrawable->pScreen, SourceValidate);
    if (pDrawable->type == DRAWABLE_WINDOW) {
        WindowPtr pWin = (WindowPtr)pDrawable;
        RootlessStartDrawing(pWin);
    }
    if (pDrawable->pScreen->SourceValidate) {
        pDrawable->pScreen->SourceValidate(pDrawable, x, y, w, h);
    }
    SCREEN_WRAP(pDrawable->pScreen, SourceValidate);
}

#ifdef RENDER

static void
rootlessComposite(CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
                  INT16 xSrc, INT16 ySrc, INT16  xMask, INT16  yMask,
                  INT16 xDst, INT16 yDst, CARD16 width, CARD16 height)
{
    ScreenPtr pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr ps = GetPictureScreen(pScreen);
    WindowPtr srcWin, dstWin, maskWin = NULL;

    if (pMask) {
        maskWin = (pMask->pDrawable->type == DRAWABLE_WINDOW) ?
                  (WindowPtr)pMask->pDrawable :  NULL;
    }
    srcWin  = (pSrc->pDrawable->type  == DRAWABLE_WINDOW) ?
              (WindowPtr)pSrc->pDrawable  :  NULL;
    dstWin  = (pDst->pDrawable->type == DRAWABLE_WINDOW) ?
              (WindowPtr)pDst->pDrawable  :  NULL;

    ps->Composite = SCREENREC(pScreen)->Composite;

    if (srcWin  && IsFramedWindow(srcWin))  RootlessStartDrawing(srcWin);
    if (maskWin && IsFramedWindow(maskWin)) RootlessStartDrawing(maskWin);
    if (dstWin  && IsFramedWindow(dstWin))  RootlessStartDrawing(dstWin);

    ps->Composite(op, pSrc, pMask, pDst,
                  xSrc, ySrc, xMask, yMask,
                  xDst, yDst, width, height);

    if (dstWin  && IsFramedWindow(dstWin)) {
        RootlessDamageRect(dstWin, xDst, yDst, width, height);
    }

    ps->Composite = RootlessComposite;
}

static void
RootlessGlyphs(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
               PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
               int nlist, GlyphListPtr list, GlyphPtr *glyphs)
{
    ScreenPtr pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr ps = GetPictureScreen(pScreen);
    int x, y;
    int n;
    GlyphPtr glyph;
    WindowPtr srcWin, dstWin;

    srcWin = (pSrc->pDrawable->type == DRAWABLE_WINDOW) ?
             (WindowPtr)pSrc->pDrawable  :  NULL;
    dstWin = (pDst->pDrawable->type == DRAWABLE_WINDOW) ?
             (WindowPtr)pDst->pDrawable  :  NULL;

    if (srcWin && IsFramedWindow(srcWin)) RootlessStartDrawing(srcWin);
    if (dstWin && IsFramedWindow(dstWin)) RootlessStartDrawing(dstWin);

    ps->Glyphs = SCREENREC(pScreen)->Glyphs;
    ps->Glyphs(op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, list, glyphs);
    ps->Glyphs = RootlessGlyphs;

    if (dstWin && IsFramedWindow(dstWin))
    {
        x = xSrc;
        y = ySrc;

	while (nlist--)
	{
	    /* Originally this code called DamageRect for the bounding
	       box of each glyph. But that was causing way too much
	       time to be spent in CGSBoundingRegionAddRect. So compute
	       the union of all glyphs in a list and damage that. It
	       shouldn't be very different. */

            x += list->xOff;
            y += list->yOff;
            n = list->len;

	    if (n > 0)
	    {
		BoxRec box;

                glyph = *glyphs++;

		box.x1 = x - glyph->info.x;
		box.y1 = y - glyph->info.y;
		box.x2 = box.x1 + glyph->info.width;
		box.y2 = box.y2 + glyph->info.height;

		x += glyph->info.xOff;
		y += glyph->info.yOff;

		while (--n > 0)
		{
		    short x1, y1, x2, y2;

		    glyph = *glyphs++;

		    x1 = x - glyph->info.x;
		    y1 = y - glyph->info.y;
		    x2 = x1 + glyph->info.width;
		    y2 = y1 + glyph->info.height;

		    box.x1 = MAX (box.x1, x1);
		    box.y1 = MAX (box.y1, y1);
		    box.x2 = MAX (box.x2, x2);
		    box.y2 = MAX (box.y2, y2);

		    x += glyph->info.xOff;
		    y += glyph->info.yOff;
		}

		RootlessDamageBox (dstWin, &box);
	    }

            list++;
        }
    }
}

#endif /* RENDER */

/* ValidateTree is modified in two ways:
    - top-level windows don't clip each other
    - windows aren't clipped against root.
   These only matter when validating from the root. */
static int
RootlessValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    int result;
    RegionRec saveRoot;
    ScreenPtr pScreen = pParent->drawable.pScreen;

    SCREEN_UNWRAP(pScreen, ValidateTree);
    RL_DEBUG_MSG("VALIDATETREE start ");

    /* Use our custom version to validate from root */
    if (IsRoot(pParent)) {
        RL_DEBUG_MSG("custom ");
        result = RootlessMiValidateTree(pParent, pChild, kind);
    } else {
        HUGE_ROOT(pParent);
        result = pScreen->ValidateTree(pParent, pChild, kind);
        NORMAL_ROOT(pParent);
    }

    SCREEN_WRAP(pScreen, ValidateTree);
    RL_DEBUG_MSG("VALIDATETREE end\n");

    return result;
}

/* MarkOverlappedWindows is modified to ignore overlapping top-level
   windows. */
static Bool
RootlessMarkOverlappedWindows(WindowPtr pWin, WindowPtr pFirst,
                              WindowPtr *ppLayerWin)
{
    RegionRec saveRoot;
    Bool result;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    SCREEN_UNWRAP(pScreen, MarkOverlappedWindows);
    RL_DEBUG_MSG("MARKOVERLAPPEDWINDOWS start ");

    HUGE_ROOT(pWin);
    if (IsRoot(pWin)) {
        /* root - mark nothing */
        RL_DEBUG_MSG("is root not marking ");
        result = FALSE;
    }
    else if (! IsTopLevel(pWin)) {
        /* not top-level window - mark normally */
        result = pScreen->MarkOverlappedWindows(pWin, pFirst, ppLayerWin);
    }
    else {
        /* top-level window - mark children ONLY - NO overlaps with sibs (?)
	   This code copied from miMarkOverlappedWindows() */

        register WindowPtr pChild;
        Bool anyMarked = FALSE;
        void (* MarkWindow)() = pScreen->MarkWindow;

        RL_DEBUG_MSG("is top level! ");
        /* single layered systems are easy */
        if (ppLayerWin) *ppLayerWin = pWin;

        if (pWin == pFirst) {
            /* Blindly mark pWin and all of its inferiors.   This is a slight
            * overkill if there are mapped windows that outside pWin's border,
            * but it's better than wasting time on RectIn checks.
            */
            pChild = pWin;
            while (1) {
                if (pChild->viewable) {
                    if (REGION_BROKEN (pScreen, &pChild->winSize))
                        SetWinSize (pChild);
                    if (REGION_BROKEN (pScreen, &pChild->borderSize))
                        SetBorderSize (pChild);
                    (* MarkWindow)(pChild);
                    if (pChild->firstChild) {
                        pChild = pChild->firstChild;
                        continue;
                    }
                }
                while (!pChild->nextSib && (pChild != pWin))
                    pChild = pChild->parent;
                if (pChild == pWin)
                    break;
                pChild = pChild->nextSib;
            }
            anyMarked = TRUE;
            pFirst = pFirst->nextSib;
        }
        if (anyMarked)
            (* MarkWindow)(pWin->parent);
        result = anyMarked;
    }
    NORMAL_ROOT(pWin);
    SCREEN_WRAP(pScreen, MarkOverlappedWindows);
    RL_DEBUG_MSG("MARKOVERLAPPEDWINDOWS end\n");
    return result;
}

static void
expose_1 (WindowPtr pWin)
{
    WindowPtr pChild;

    if (!pWin->realized)
	return;

    (*pWin->drawable.pScreen->PaintWindowBackground) (pWin, &pWin->borderClip,
						      PW_BACKGROUND);

    /* FIXME: comments in windowstr.h indicate that borderClip doesn't
       include subwindow visibility. But I'm not so sure.. so we may
       be exposing too much.. */

    miSendExposures (pWin, &pWin->borderClip,
		     pWin->drawable.x, pWin->drawable.y);

    for (pChild = pWin->firstChild; pChild != NULL; pChild = pChild->nextSib)
	expose_1 (pChild);
}

void
RootlessScreenExpose (ScreenPtr pScreen)
{
    expose_1 (WindowTable[pScreen->myNum]);
}

ColormapPtr
RootlessGetColormap (ScreenPtr pScreen)
{
    RootlessScreenRec *s = SCREENREC (pScreen);

    return s->colormap;
}

static void
RootlessInstallColormap (ColormapPtr pMap)
{
    ScreenPtr pScreen = pMap->pScreen;
    RootlessScreenRec *s = SCREENREC (pScreen);

    SCREEN_UNWRAP(pScreen, InstallColormap);

    if (s->colormap != pMap)
    {
	s->colormap = pMap;
	s->colormap_changed = TRUE;
	RootlessQueueRedisplay (pScreen);
    }

    pScreen->InstallColormap (pMap);

    SCREEN_WRAP (pScreen, InstallColormap);
}

static void
RootlessUninstallColormap (ColormapPtr pMap)
{
    ScreenPtr pScreen = pMap->pScreen;
    RootlessScreenRec *s = SCREENREC (pScreen);

    SCREEN_UNWRAP(pScreen, UninstallColormap);

    if (s->colormap == pMap)
	s->colormap = NULL;

    pScreen->UninstallColormap (pMap);

    SCREEN_WRAP(pScreen, UninstallColormap);
}

static void
RootlessStoreColors (ColormapPtr pMap, int ndef, xColorItem *pdef)
{
    ScreenPtr pScreen = pMap->pScreen;
    RootlessScreenRec *s = SCREENREC (pScreen);

    SCREEN_UNWRAP(pScreen, StoreColors);

    if (s->colormap == pMap && ndef > 0)
    {
	s->colormap_changed = TRUE;
	RootlessQueueRedisplay (pScreen);
    }

    pScreen->StoreColors (pMap, ndef, pdef);

    SCREEN_WRAP(pScreen, StoreColors);
}

static CARD32
redisplay_callback (OsTimerPtr timer, CARD32 time, void *arg)
{
    RootlessScreenRec *screenRec = arg;

    if (!screenRec->redisplay_queued)
    {
	/* No update needed. Stop the timer. */

	screenRec->redisplay_timer_set = FALSE;
	return 0;
    }

    screenRec->redisplay_queued = FALSE;

    /* Mark that we should redisplay before waiting for I/O next time */
    screenRec->redisplay_expired = TRUE;

    /* Reinstall the timer immediately, so we get as close to our
       redisplay interval as possible. */

    return REDISPLAY_DELAY;
}

void
RootlessQueueRedisplay (ScreenPtr pScreen)
{
    RootlessScreenRec *screenRec = SCREENREC (pScreen);

    screenRec->redisplay_queued = TRUE;

    if (screenRec->redisplay_timer_set)
	return;

    screenRec->redisplay_timer = TimerSet (screenRec->redisplay_timer,
					   0, REDISPLAY_DELAY,
					   redisplay_callback, screenRec);
    screenRec->redisplay_timer_set = TRUE;
}

/* Call this function when it might be a good idea to flush updates.
   Note that it will unlock window buffers! */
Bool
RootlessMayRedisplay (ScreenPtr pScreen)
{
    RootlessScreenRec *screenRec = SCREENREC (pScreen);

    if (!screenRec->redisplay_queued)
	return FALSE;

    /* If the timer has fired, or it's been long enough since the last
       update, redisplay everything now. */

    if (!screenRec->redisplay_expired)
    {
	CARD32 now = GetTimeInMillis ();

	if (screenRec->last_redisplay + REDISPLAY_MAX_DELAY >= now)
	    return FALSE;
    }

    if (screenRec->redisplay_timer_set)
    {
	TimerCancel (screenRec->redisplay_timer);
	screenRec->redisplay_timer_set = FALSE;
    }

    RootlessRedisplayScreen (screenRec->pScreen);
    screenRec->redisplay_expired = FALSE;

    return TRUE;
}

/* Flush drawing before blocking on select(). */
static void
RootlessBlockHandler(pointer pbdata, OSTimePtr pTimeout, pointer pReadmask)
{
    ScreenPtr pScreen = pbdata;
    RootlessScreenRec *screenRec = SCREENREC (pScreen);

    if (screenRec->redisplay_expired)
    {
	screenRec->redisplay_expired = FALSE;

	if (screenRec->colormap_changed)
	{
	    RootlessFlushScreenColormaps (screenRec->pScreen);
	    screenRec->colormap_changed = FALSE;
	}

	RootlessRedisplayScreen (screenRec->pScreen);
    }
}

static void
RootlessWakeupHandler(pointer data, int i, pointer LastSelectMask)
{
}

static Bool
RootlessAllocatePrivates(ScreenPtr pScreen)
{
    RootlessScreenRec *s;
    static unsigned int rootlessGeneration = -1;

    if (rootlessGeneration != serverGeneration) {
        rootlessScreenPrivateIndex = AllocateScreenPrivateIndex();
        if (rootlessScreenPrivateIndex == -1) return FALSE;
        rootlessGCPrivateIndex = AllocateGCPrivateIndex();
        if (rootlessGCPrivateIndex == -1) return FALSE;
        rootlessWindowPrivateIndex = AllocateWindowPrivateIndex();
        if (rootlessWindowPrivateIndex == -1) return FALSE;
        rootlessGeneration = serverGeneration;
    }

    /* no allocation needed for screen privates */
    if (!AllocateGCPrivate(pScreen, rootlessGCPrivateIndex,
                           sizeof(RootlessGCRec)))
        return FALSE;
    if (!AllocateWindowPrivate(pScreen, rootlessWindowPrivateIndex, 0))
        return FALSE;

    s = xalloc(sizeof(RootlessScreenRec));
    if (! s) return FALSE;
    SCREENREC(pScreen) = s;

    s->pixmap_data = NULL;
    s->pixmap_data_size = 0;

    s->redisplay_timer = NULL;
    s->redisplay_timer_set = FALSE;

    return TRUE;
}

static void
RootlessWrap(ScreenPtr pScreen)
{
    RootlessScreenRec *s = (RootlessScreenRec*)
            pScreen->devPrivates[rootlessScreenPrivateIndex].ptr;

#define WRAP(a) \
    if (pScreen->a) { \
        s->a = pScreen->a; \
    } else { \
        RL_DEBUG_MSG("null screen fn " #a "\n"); \
        s->a = NULL; \
    } \
    pScreen->a = Rootless##a

    WRAP(CreateScreenResources);
    WRAP(CloseScreen);
    WRAP(CreateGC);
    WRAP(PaintWindowBackground);
    WRAP(PaintWindowBorder);
    WRAP(CopyWindow);
    WRAP(GetImage);
    WRAP(SourceValidate);
    WRAP(CreateWindow);
    WRAP(DestroyWindow);
    WRAP(RealizeWindow);
    WRAP(UnrealizeWindow);
    WRAP(ReparentWindow);
    WRAP(MoveWindow);
    WRAP(PositionWindow);
    WRAP(ResizeWindow);
    WRAP(RestackWindow);
    WRAP(ChangeBorderWidth);
    WRAP(MarkOverlappedWindows);
    WRAP(ValidateTree);
    WRAP(ChangeWindowAttributes);
    WRAP(InstallColormap);
    WRAP(UninstallColormap);
    WRAP(StoreColors);

#ifdef SHAPE
    WRAP(SetShape);
#endif

#ifdef RENDER
    {
        /* Composite and Glyphs don't use normal screen wrapping */
        PictureScreenPtr ps = GetPictureScreen(pScreen);
        s->Composite = ps->Composite;
        ps->Composite = rootlessComposite;
        s->Glyphs = ps->Glyphs;
        ps->Glyphs = RootlessGlyphs;
    }
#endif

    // WRAP(ClearToBackground); fixme put this back? useful for shaped wins?
    // WRAP(RestoreAreas); fixme put this back?

#undef WRAP
}

Bool
RootlessSetupScreen(int index, ScreenPtr pScreen)
{
    RootlessScreenRec *s;

    /* Add replacements for fb screen functions */
    pScreen->PaintWindowBackground = RootlessPaintWindow;
    pScreen->PaintWindowBorder = RootlessPaintWindow;

#ifdef RENDER
    {
        PictureScreenPtr ps = GetPictureScreen(pScreen);
        ps->Composite = RootlessComposite;
    }
#endif

    if (!RootlessAllocatePrivates(pScreen))
	return FALSE;

    s = ((RootlessScreenRec*)
	 pScreen->devPrivates[rootlessScreenPrivateIndex].ptr);

    s->pScreen = pScreen;
    RootlessWrap(pScreen);

    if (!RegisterBlockAndWakeupHandlers (RootlessBlockHandler,
                                         RootlessWakeupHandler,
                                         (pointer) pScreen))
    {
        return FALSE;
    }

    return TRUE;
}

void
RootlessUpdateRooted (Bool state)
{
    int i;

    if (!state)
    {
	for (i = 0; i < screenInfo.numScreens; i++)
	    RootlessDisableRoot (screenInfo.screens[i]);
    }
    else
    {
	for (i = 0; i < screenInfo.numScreens; i++)
	    RootlessEnableRoot (screenInfo.screens[i]);
    }
}
