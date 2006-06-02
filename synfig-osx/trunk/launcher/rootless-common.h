/*
 * Common internal rootless definitions and code
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

/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootlessCommon.h,v 1.6 2002/07/24 05:58:33 torrey Exp $ */

#ifndef _ROOTLESSCOMMON_H
#define _ROOTLESSCOMMON_H

#include "rootless.h"

#include "pixmapstr.h"
#include "windowstr.h"

#ifdef RENDER
#include "picturestr.h"
#endif

/* Debug output, or not. */
#ifdef ROOTLESSDEBUG
#define RL_DEBUG_MSG ErrorF
#else
#define RL_DEBUG_MSG(a, ...)
#endif

#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))

/* Global variables */
extern int rootlessGCPrivateIndex;
extern int rootlessScreenPrivateIndex;
extern int rootlessWindowPrivateIndex;
extern int rootlessNoDRIDrawing;

/* RootlessGCRec: private per-gc data */
typedef struct {
    GCFuncs *originalFuncs;
    GCOps *originalOps;
} RootlessGCRec;

/* RootlessWindowRec: private per-window data */
typedef struct RootlessWindowRec {
    int x, y;
    unsigned int width, height;
    unsigned int borderWidth;
    int level;

    xp_window_id wid;
    WindowPtr win;

    /* Valid when locked (i.e. is_drawing is set) */
    void *data;
    unsigned int rowbytes;

    PixmapPtr pixmap;
    PixmapPtr oldPixmap;

    unsigned long unrealize_time;	/* in seconds */

    unsigned int is_drawing :1;
    unsigned int is_update_disabled :1;
    unsigned int is_reorder_pending :1;
    unsigned int is_offscreen :1;
    unsigned int is_obscured :1;
} RootlessWindowRec;

/* RootlessScreenRec: per-screen private data */
typedef struct {
    ScreenPtr pScreen;

    CreateScreenResourcesProcPtr CreateScreenResources;
    CloseScreenProcPtr CloseScreen;

    CreateWindowProcPtr CreateWindow;
    DestroyWindowProcPtr DestroyWindow;
    RealizeWindowProcPtr RealizeWindow;
    UnrealizeWindowProcPtr UnrealizeWindow;
    ReparentWindowProcPtr ReparentWindow;
    MoveWindowProcPtr MoveWindow;
    ResizeWindowProcPtr ResizeWindow;
    RestackWindowProcPtr RestackWindow;
    ChangeBorderWidthProcPtr ChangeBorderWidth;
    PositionWindowProcPtr PositionWindow;
    ChangeWindowAttributesProcPtr ChangeWindowAttributes;

    CreateGCProcPtr CreateGC;
    PaintWindowBackgroundProcPtr PaintWindowBackground;
    PaintWindowBorderProcPtr PaintWindowBorder;
    CopyWindowProcPtr CopyWindow;
    GetImageProcPtr GetImage;
    SourceValidateProcPtr SourceValidate;

    MarkOverlappedWindowsProcPtr MarkOverlappedWindows;
    ValidateTreeProcPtr ValidateTree;

#ifdef SHAPE
    SetShapeProcPtr SetShape;
#endif

#ifdef RENDER
    CompositeProcPtr Composite;
    GlyphsProcPtr Glyphs;
#endif

    InstallColormapProcPtr InstallColormap;
    UninstallColormapProcPtr UninstallColormap;
    StoreColorsProcPtr StoreColors;

    void *pixmap_data;
    unsigned int pixmap_data_size;

    ColormapPtr colormap;

    void *redisplay_timer;
    CARD32 last_redisplay;

    unsigned int redisplay_timer_set :1;
    unsigned int redisplay_queued :1;
    unsigned int redisplay_expired :1;
    unsigned int colormap_changed :1;
} RootlessScreenRec;


/* "Definition of the Porting Layer for the X11 Sample Server" says
   unwrap and rewrap of screen functions is unnecessary, but
   screen->CreateGC changes after a call to cfbCreateGC. */

#define SCREEN_UNWRAP(screen, fn) \
    screen->fn = SCREENREC(screen)->fn;

#define SCREEN_WRAP(screen, fn) \
    SCREENREC(screen)->fn = screen->fn; \
    screen->fn = Rootless##fn

/* Accessors for screen and window privates */

#define SCREENREC(pScreen) \
   ((RootlessScreenRec*)(pScreen)->devPrivates[rootlessScreenPrivateIndex].ptr)

#define WINREC(pWin) \
    ((RootlessWindowRec *)(pWin)->devPrivates[rootlessWindowPrivateIndex].ptr)

/* BoxRec manipulators (Copied from shadowfb) */

#define TRIM_BOX(box, pGC) { \
    BoxPtr extents = &pGC->pCompositeClip->extents;\
    if(box.x1 < extents->x1) box.x1 = extents->x1; \
    if(box.x2 > extents->x2) box.x2 = extents->x2; \
    if(box.y1 < extents->y1) box.y1 = extents->y1; \
    if(box.y2 > extents->y2) box.y2 = extents->y2; \
}

#define TRANSLATE_BOX(box, pDraw) { \
    box.x1 += pDraw->x; \
    box.x2 += pDraw->x; \
    box.y1 += pDraw->y; \
    box.y2 += pDraw->y; \
}

#define TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC) { \
    TRANSLATE_BOX(box, pDraw); \
    TRIM_BOX(box, pGC); \
}

#define BOX_NOT_EMPTY(box) \
    (((box.x2 - box.x1) > 0) && ((box.y2 - box.y1) > 0))

/* We don't want to clip windows to the edge of the screen. HUGE_ROOT
   temporarily makes the root window really big. This is needed as a
   wrapper around any function that calls SetWinSize or SetBorderSize
   which clip a window against its parents, including the root. */

extern RegionRec rootlessHugeRoot;

#define HUGE_ROOT(pWin) 			\
    do { 					\
	WindowPtr w = pWin; 			\
	while (w->parent != NULL)		\
	    w = w->parent; 			\
        saveRoot = w->winSize; 			\
        w->winSize = rootlessHugeRoot; 		\
    } while (0)

#define NORMAL_ROOT(pWin) 		\
    do { 				\
	WindowPtr w = pWin; 		\
	while (w->parent != NULL)	\
	    w = w->parent;		\
	w->winSize = saveRoot; 		\
    } while (0)

/* Returns TRUE if this window is a top-level window (i.e. child of the root)
   The root is not a top-level window. */
#define IsTopLevel(pWin) \
   ((pWin) != NULL && (pWin)->parent != NULL && (pWin)->parent->parent == NULL)

/* Returns TRUE if this window is a root window */
#define IsRoot(pWin) \
   ((pWin) == WindowTable[(pWin)->drawable.pScreen->myNum])

/* Returns the top-level parent of pWindow.
   The root is the top-level parent of itself, even though the root is
   not otherwise considered to be a top-level window. */
extern WindowPtr TopLevelParent (WindowPtr pWindow);

/* Returns TRUE if this window is visible inside a frame (e.g. it is
   visible and has a top-level or root parent) */
extern Bool IsFramedWindow (WindowPtr pWin);

/* Adjust base address of pixmap by DX,DY */
extern void TranslatePixmapBase (PixmapPtr pPix, int dx, int dy);

/* Prepare a window for direct access to its backing buffer. */
extern void RootlessStartDrawing (WindowPtr pWindow);

/* Mark that no more drawing operations will hit the window until ``start
   drawing'' is issued again. */
extern void RootlessFinishedDrawing (WindowPtr pWindow);

/* Finish drawing to a window's backing buffer. */
extern void RootlessStopDrawing (WindowPtr pWindow, Bool flush);

/* Routines that cause regions to get redrawn.
   DamageRegion and DamageRect are in global coordinates.
   DamageBox is in window-local coordinates. */
extern void RootlessDamageRegion (WindowPtr pWindow, RegionPtr pRegion);
extern void RootlessDamageRect (WindowPtr pWindow, int x, int y, int w, int h);
extern void RootlessDamageBox (WindowPtr pWindow, BoxPtr pBox);
extern void RootlessRedisplay (WindowPtr pWindow);
extern void RootlessRedisplayScreen (ScreenPtr pScreen);

extern void RootlessQueueRedisplay (ScreenPtr pScreen);
extern Bool RootlessMayRedisplay (ScreenPtr pScreen);
extern void RootlessScreenExpose (ScreenPtr pScreen);

/* Return the colormap currently installed on the given screen. */
extern ColormapPtr RootlessGetColormap (ScreenPtr pScreen);

/* Convert colormap to ARGB. */
extern Bool RootlessResolveColormap (ScreenPtr pScreen, int first_color,
				     int n_colors, uint32_t *colors);

extern void RootlessFlushWindowColormap (WindowPtr pWin);
extern void RootlessFlushScreenColormaps (ScreenPtr pScreen);

/* Move windows back to their position relative to the screen origin. */
extern void RootlessRepositionWindow (WindowPtr pWin);
extern void RootlessRepositionWindows (ScreenPtr pScreen);

/* Move the window to it's correct place in the physical stacking order */
extern void RootlessReorderWindow (WindowPtr pWin);

/* Bit mask for alpha channel with a particular number of bits per
   pixel. Note that we only care for 32bpp data. OS X uses planar alpha
   for 16bpp. */
#define RootlessAlphaMask(bpp) ((bpp) == 32 ? 0xFF000000 : 0)

#ifdef RENDER
extern void RootlessComposite(CARD8 op, PicturePtr pSrc,
			      PicturePtr pMask, PicturePtr pDst,
			      INT16 xSrc, INT16 ySrc,
			      INT16 xMask, INT16 yMask,
			      INT16 xDst, INT16 yDst,
			      CARD16 width, CARD16 height);
#endif

extern void RootlessNativeWindowStateChanged (xp_window_id id,
					      unsigned int state);

extern void RootlessEnableRoot (ScreenPtr pScreen);
extern void RootlessDisableRoot (ScreenPtr pScreen);
extern void RootlessSetWindowLevel (WindowPtr pWin, int level);

#endif /* _ROOTLESSCOMMON_H */
