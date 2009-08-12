/*
 * Rootless window management
 */
/*
 * Copyright (c) 2001 Greg Parker. All Rights Reserved.
 * Copyright (c) 2002 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
/* Portions of this file are based on fbwindow.c, which contains the
 * following copyright:
 *
 * Copyright © 1998 Keith Packard
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootlessWindow.c,v 1.11 2002/09/28 00:43:39 torrey Exp $ */

#include "rootless-common.h"
#include "rootless-window.h"
#include "darwin.h"
#include "Xplugin.h"
#include "x-hash.h"
#include "x-list.h"
#define _APPLEWM_SERVER_
#include "applewmstr.h"

#include "fb.h"
#include "propertyst.h"

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

#include <X11/Xatom.h>
#include <pthread.h>

#define DEFINE_ATOM_HELPER(func,atom_name)			\
static Atom func (void) {					\
    static unsigned int generation;				\
    static Atom atom;						\
    if (generation != serverGeneration) {			\
	generation = serverGeneration;				\
	atom = MakeAtom (atom_name, strlen (atom_name), TRUE);	\
    }								\
    return atom;						\
}

DEFINE_ATOM_HELPER (xa_native_screen_origin, "_NATIVE_SCREEN_ORIGIN")
DEFINE_ATOM_HELPER (xa_native_window_id, "_NATIVE_WINDOW_ID")
DEFINE_ATOM_HELPER (xa_apple_no_order_in, "_APPLE_NO_ORDER_IN")

/* Maps xp_window_id -> AquaWindowRec */
static x_hash_table *window_hash;
static pthread_mutex_t window_hash_mutex;

static Bool no_configure_window;
static Bool windows_hidden;

static const int normal_window_levels[AppleWMNumWindowLevels+1] = {
    0, 3, 4, 5, LONG_MIN + 30, LONG_MIN + 29,
};
static const int rooted_window_levels[AppleWMNumWindowLevels+1] = {
    202, 203, 204, 205, 201, 200
};

static inline xp_error
configure_window (xp_window_id id, unsigned int mask,
		  const xp_window_changes *values)
{
    if (!no_configure_window)
	return xp_configure_window (id, mask, values);
    else
	return XP_Success;
}

static inline unsigned long
current_time_in_seconds (void)
{
    unsigned long t = 0;

    t += currentTime.milliseconds / 1000;
    t += currentTime.months * 4294967;

    return t;
}

static inline Bool
rootlessHasRoot (ScreenPtr pScreen)
{
    return WINREC (WindowTable[pScreen->myNum]) != NULL;
}

void
RootlessNativeWindowStateChanged (xp_window_id id, unsigned int state)
{
    WindowPtr pWin;
    RootlessWindowRec *winRec;

    pWin = RootlessGetXWindow (id);
    if (pWin == NULL)
	return;

    winRec = WINREC (pWin);
    if (winRec == NULL)
	return;

    winRec->is_offscreen = (state & XP_WINDOW_STATE_OFFSCREEN) != 0;
    winRec->is_obscured = (state & XP_WINDOW_STATE_OBSCURED) != 0;

#ifdef ROOTLESS
    pWin->rootlessUnhittable = winRec->is_offscreen;
#endif
}

void
RootlessNativeWindowMoved (xp_window_id id)
{
    WindowPtr pWin;
    xp_box bounds;
    int sx, sy;
    XID vlist[2];
    Mask mask;
    ClientPtr client;

    pWin = RootlessGetXWindow (id);
    if (pWin == NULL)
	return;

    if (xp_get_window_bounds (id, &bounds) != Success)
	return;

    sx = dixScreenOrigins[pWin->drawable.pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pWin->drawable.pScreen->myNum].y + darwinMainScreenY;

    /* Fake up a ConfigureWindow packet to resize the window to the
       current bounds. */

    vlist[0] = (INT16) bounds.x1 - sx;
    vlist[1] = (INT16) bounds.y1 - sy;
    mask = CWX | CWY;

    /* pretend we're the owner of the window! */
    client = LookupClient (pWin->drawable.id, NullClient);

    /* Don't want to do anything to the physical window (avoids
       notification-response feedback loops) */

    no_configure_window = TRUE;
    ConfigureWindow (pWin, mask, vlist, client);
    no_configure_window = FALSE;
}

/* Updates the _NATIVE_SCREEN_ORIGIN property on the given root window. */
static void
set_screen_origin (WindowPtr pWin)
{
    long data[2];

    if (!IsRoot (pWin))
	return;

    /* FIXME: move this to an extension? */

    data[0] = (dixScreenOrigins[pWin->drawable.pScreen->myNum].x
	       + darwinMainScreenX);
    data[1] = (dixScreenOrigins[pWin->drawable.pScreen->myNum].y
	       + darwinMainScreenY);

    ChangeWindowProperty (pWin, xa_native_screen_origin (), XA_INTEGER,
			  32, PropModeReplace, 2, data, TRUE);
}

/* For now, don't create a physical window until either the window is
   realized, or we really need it (e.g. to attach VRAM surfaces to).
   Do reset the window size so it's not clipped by the root window. */
Bool
RootlessCreateWindow (WindowPtr pWin)
{
    Bool result;
    RegionRec saveRoot;

    SCREEN_UNWRAP (pWin->drawable.pScreen, CreateWindow);

    if (!IsRoot (pWin))
    {
        /* win/border size set by DIX, not by wrapped CreateWindow, so
           correct it here. Don't HUGE_ROOT when pWin is the root! */

        HUGE_ROOT (pWin);
        SetWinSize (pWin);
        SetBorderSize (pWin);
    }
    else
    {
	set_screen_origin (pWin);
    }

    result = pWin->drawable.pScreen->CreateWindow (pWin);

    if (pWin->parent)
        NORMAL_ROOT (pWin);

    SCREEN_WRAP (pWin->drawable.pScreen, CreateWindow);

    return result;
}

/* Destroy the physical window associated with the given window */
static void
rootlessDestroyFrame (WindowPtr pWin, RootlessWindowRec *winRec)
{
    RootlessStopDrawing (pWin, FALSE);

    pthread_mutex_lock (&window_hash_mutex);
    x_hash_table_remove (window_hash, (void *) winRec->wid);
    pthread_mutex_unlock (&window_hash_mutex);

    xp_destroy_window (winRec->wid);

    xfree (winRec);
    WINREC (pWin) = NULL;
}

Bool
RootlessDestroyWindow (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC(pWin);
    Bool result;

    if (winRec != NULL)
	rootlessDestroyFrame (pWin, winRec);

    /* winRec is gone now */

    SCREEN_UNWRAP(pWin->drawable.pScreen, DestroyWindow);

    result = pWin->drawable.pScreen->DestroyWindow (pWin);

    SCREEN_WRAP(pWin->drawable.pScreen, DestroyWindow);

    return result;
}

#ifdef SHAPE
static Bool
RootlessGetShape (WindowPtr pWin, RegionPtr pShape)
{
    if (wBoundingShape (pWin) == NULL)
	return FALSE;

    /* wBoundingShape is relative to *inner* origin of window.
       Translate by borderWidth to get the outside-relative position. */

    REGION_INIT (pScreen, pShape, NullBox, 0);
    REGION_COPY (pScreen, pShape, wBoundingShape (pWin));
    REGION_TRANSLATE (pScreen, pShape, pWin->borderWidth, pWin->borderWidth);

    return TRUE;
}

/* boundingShape = outside border (like borderClip)
   clipShape = inside border (like clipList)
   Both are in window-local coordinates
   We only care about boundingShape (FIXME true?)

   RootlessReallySetShape is used in several places other than SetShape.
   Most importantly, SetShape is often called on unmapped windows, so we
   have to wait until the window is mapped to reshape the frame. */
static void
rootlessSetShape (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);

    RegionRec newShape;
    RegionPtr pShape;
    xp_window_changes wc;

    if (winRec == NULL)
	return;

    RootlessStopDrawing (pWin, FALSE);

    pShape = RootlessGetShape (pWin, &newShape) ? &newShape : NULL;

    RL_DEBUG_MSG("reshaping...");
    RL_DEBUG_MSG("numrects %d, extents %d %d %d %d\n",
                 REGION_NUM_RECTS(&newShape),
                 newShape.extents.x1, newShape.extents.y1,
                 newShape.extents.x2, newShape.extents.y2);

    RootlessDisableUpdate (pWin);

    if (pShape != NULL)
    {
	wc.shape_nrects = REGION_NUM_RECTS (pShape);
	wc.shape_rects = REGION_RECTS (pShape);
    }
    else
    {
	wc.shape_nrects = -1;
	wc.shape_rects = NULL;
    }

    wc.shape_tx = wc.shape_ty = 0;

    configure_window (winRec->wid, XP_SHAPE, &wc);
    
    if (pShape != NULL)
	REGION_UNINIT(pScreen, &newShape);
}

void
RootlessSetShape (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    SCREEN_UNWRAP (pScreen, SetShape);

    pScreen->SetShape (pWin);

    SCREEN_WRAP (pScreen, SetShape);

    rootlessSetShape (pWin);
}
#endif

/* Disallow ParentRelative background on top-level windows
   because the root window doesn't really have the right background
   and fb will try to draw on the root instead of on the window.
   ParentRelative prevention is also in PaintWindowBackground/Border()
   so it is no longer really needed here. */
Bool
RootlessChangeWindowAttributes (WindowPtr pWin, unsigned long vmask)
{
    Bool result;
    ScreenPtr pScreen = pWin->drawable.pScreen;

    RL_DEBUG_MSG("change window attributes start\n");

    SCREEN_UNWRAP (pScreen, ChangeWindowAttributes);

    result = pScreen->ChangeWindowAttributes (pWin, vmask);

    SCREEN_WRAP (pScreen, ChangeWindowAttributes);

    if (WINREC (pWin) != NULL)
    {
        /* disallow ParentRelative background state */

        if (pWin->backgroundState == ParentRelative)
	{
            XID pixel = 0;
            ChangeWindowAttributes (pWin, CWBackPixel, &pixel, serverClient);
        }
    }

    RL_DEBUG_MSG("change window attributes end\n");
    return result;
}

/* This is a hook for when DIX moves or resizes a window.
   Update the frame position now. (x, y) are *inside* position.
   After this, mi and fb are expecting the pixmap to be at the new location. */
Bool
RootlessPositionWindow (WindowPtr pWin, int x, int y)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec = WINREC (pWin);
    Bool result;

    RL_DEBUG_MSG("positionwindow start (win 0x%x)\n", pWin);

    if (winRec != NULL)
    {
        if (winRec->is_drawing)
	{
            /* Reset frame's pixmap and move it to the new position. */
            int bw = wBorderWidth (pWin);

            winRec->pixmap->devPrivate.ptr = winRec->data;
            TranslatePixmapBase (winRec->pixmap, - (x - bw), - (y - bw));
        }
    }

    SCREEN_UNWRAP (pScreen, PositionWindow);

    result = pScreen->PositionWindow (pWin, x, y);

    SCREEN_WRAP(pScreen, PositionWindow);

    RL_DEBUG_MSG("positionwindow end\n");
    return result;
}

/* Initialize some basic attributes of the frame. Note that winRec
   may already have valid data in it, so don't overwrite anything
   valuable. */
static void
rootlessInitializeFrame (WindowPtr pWin, RootlessWindowRec *winRec)
{
    DrawablePtr d = &pWin->drawable;
    int bw = wBorderWidth (pWin);

    winRec->win = pWin;

    winRec->x = d->x - bw;
    winRec->y = d->y - bw;
    winRec->width = d->width + 2*bw;
    winRec->height = d->height + 2*bw;
    winRec->borderWidth = bw;
}

static void
rootlessSetNativeProperty (RootlessWindowRec *winRec)
{
    xp_error err;
    unsigned int native_id;
    long data;

    err = xp_get_native_window (winRec->wid, &native_id);
    if (err == Success)
    {
	/* FIXME: move this to an extension? */

	data = native_id;
	ChangeWindowProperty (winRec->win, xa_native_window_id (),
			      XA_INTEGER, 32, PropModeReplace, 1, &data, TRUE);
    }
}

static xp_error
rootlessColormapCallback (void *data, int first_color,
			  int n_colors, uint32_t *colors)
{
    return (RootlessResolveColormap (data, first_color, n_colors, colors)
	    ? XP_Success : XP_BadMatch);
}

/* If the given window doesn't have a physical window associated with it,
   attempt to create one. If that's unsuccessful, return null. */
static RootlessWindowRec *
rootlessEnsureFrame (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec;
    RegionRec shape;
    RegionPtr pShape = NULL;

    xp_window_changes wc;
    unsigned int mask;
    xp_error err;
    int sx, sy;

    if (WINREC (pWin) != NULL)
	return WINREC (pWin);

    if (pWin->drawable.class != InputOutput)
	return NULL;

    winRec = xalloc (sizeof(RootlessWindowRec));

    if (!winRec)
	return NULL;

    rootlessInitializeFrame (pWin, winRec);

    winRec->is_drawing = FALSE;
    winRec->pixmap = NULL;
    winRec->wid = 0;
    winRec->is_update_disabled = FALSE;
    winRec->is_reorder_pending = FALSE;
    winRec->level = !IsRoot (pWin) ? 0 : AppleWMNumWindowLevels;
    WINREC(pWin) = winRec;

#ifdef SHAPE
    if (RootlessGetShape (pWin, &shape))
	pShape = &shape;
#endif

    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

    mask = 0;

    wc.x = sx + winRec->x;
    wc.y = sy + winRec->y;
    wc.width = winRec->width;
    wc.height = winRec->height;
    wc.bit_gravity = XP_GRAVITY_NONE;
    mask |= XP_BOUNDS;

    if (pWin->drawable.depth == 8)
    {
	wc.depth = XP_DEPTH_INDEX8;
	wc.colormap = rootlessColormapCallback;
	wc.colormap_data = pScreen;
	mask |= XP_COLORMAP;
    }
    else if (pWin->drawable.depth == 15)
	wc.depth = XP_DEPTH_RGB555;
    else if (pWin->drawable.depth == 24)
	wc.depth = XP_DEPTH_ARGB8888;
    else
	wc.depth = XP_DEPTH_NIL;
    mask |= XP_DEPTH;

    if (pShape != NULL)
    {
	wc.shape_nrects = REGION_NUM_RECTS (pShape);
	wc.shape_rects = REGION_RECTS (pShape);
	wc.shape_tx = wc.shape_ty = 0;
	mask |= XP_SHAPE;
    }

    if (!rootlessHasRoot (pScreen))
	wc.window_level = normal_window_levels[winRec->level];
    else
	wc.window_level = rooted_window_levels[winRec->level];
    mask |= XP_WINDOW_LEVEL;

    err = xp_create_window (mask, &wc, &winRec->wid);

    if (err != Success)
    {
	xfree (winRec);
	return NULL;
    }
	
    if (window_hash == NULL)
    {
	window_hash = x_hash_table_new (NULL, NULL, NULL, NULL);
	pthread_mutex_init (&window_hash_mutex, NULL);
    }

    pthread_mutex_lock (&window_hash_mutex);
    x_hash_table_insert (window_hash, (void *) winRec->wid, winRec);
    pthread_mutex_unlock (&window_hash_mutex);

    rootlessSetNativeProperty (winRec);

    if (pShape != NULL)
	REGION_UNINIT (pScreen, &shape);

    return winRec;
}

/* The frame is usually created here and not in CreateWindow so that
   windows do not eat memory until they are realized. */
Bool
RootlessRealizeWindow(WindowPtr pWin)
{
    Bool result = FALSE;
    RegionRec saveRoot;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    XID pixel;

    RL_DEBUG_MSG("realizewindow start (win 0x%x)\n", pWin);

    if (IsTopLevel (pWin) && pWin->drawable.class == InputOutput)
    {
        RootlessWindowRec *winRec;

	winRec = rootlessEnsureFrame (pWin);
	if (winRec == NULL)
	    return NULL;

	winRec->is_reorder_pending = TRUE;

        /* Disallow ParentRelative background state on top-level windows.
           This might have been set before the window was mapped. */

        if (pWin->backgroundState == ParentRelative)
	{
            pixel = 0;
            ChangeWindowAttributes (pWin, CWBackPixel, &pixel, serverClient);
        }
    }

    if (!IsRoot(pWin)) HUGE_ROOT(pWin);
    SCREEN_UNWRAP (pScreen, RealizeWindow);

    result = pScreen->RealizeWindow (pWin);

    SCREEN_WRAP (pScreen, RealizeWindow);
    if (!IsRoot(pWin)) NORMAL_ROOT(pWin);

    RL_DEBUG_MSG("realizewindow end\n");
    return result;
}

void
RootlessEnableRoot (ScreenPtr pScreen)
{
    WindowPtr pRoot;
    pRoot = WindowTable[pScreen->myNum];

    rootlessEnsureFrame (pRoot);
    (*pScreen->ClearToBackground) (pRoot, 0, 0, 0, 0, TRUE);
    RootlessReorderWindow (pRoot);
}

void
RootlessDisableRoot (ScreenPtr pScreen)
{
    WindowPtr pRoot;
    RootlessWindowRec *winRec;

    pRoot = WindowTable[pScreen->myNum];
    winRec = WINREC (pRoot);

    if (winRec != NULL)
    {
	rootlessDestroyFrame (pRoot, winRec);
	DeleteProperty (pRoot, xa_native_window_id ());
    }
}

void
RootlessHideAllWindows (void)
{
    int i;
    ScreenPtr pScreen;
    WindowPtr pWin;
    RootlessWindowRec *winRec;
    xp_window_changes wc;

    if (windows_hidden)
	return;

    windows_hidden = TRUE;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	pWin = WindowTable[i];
	if (pScreen == NULL || pWin == NULL)
	    continue;

	for (pWin = pWin->firstChild; pWin != NULL; pWin = pWin->nextSib)
	{
	    if (!pWin->realized)
		continue;

	    RootlessStopDrawing (pWin, FALSE);

	    winRec = WINREC (pWin);
	    if (winRec != NULL)
	    {
		wc.stack_mode = XP_UNMAPPED;
		wc.sibling = 0;
		configure_window (winRec->wid, XP_STACKING, &wc);
	    }
	}
    }
}

void
RootlessShowAllWindows (void)
{
    int i;
    ScreenPtr pScreen;
    WindowPtr pWin;
    RootlessWindowRec *winRec;

    if (!windows_hidden)
	return;

    windows_hidden = FALSE;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	pWin = WindowTable[i];
	if (pScreen == NULL || pWin == NULL)
	    continue;

	for (pWin = pWin->firstChild; pWin != NULL; pWin = pWin->nextSib)
	{
	    if (!pWin->realized)
		continue;

	    winRec = rootlessEnsureFrame (pWin);
	    if (winRec == NULL)
		continue;

	    RootlessReorderWindow (pWin);
	}

	RootlessScreenExpose (pScreen);
    }
}

void
RootlessSetWindowLevel (WindowPtr pWin, int level)
{
    RootlessWindowRec *winRec;
    xp_window_changes wc;

    winRec = WINREC (pWin);
    if (!IsTopLevel (pWin) || winRec == NULL || winRec->level == level)
	return;

    RootlessStopDrawing (pWin, FALSE);

    winRec->level = level;

    if (!rootlessHasRoot (pWin->drawable.pScreen))
	wc.window_level = normal_window_levels[level];
    else
	wc.window_level = rooted_window_levels[level];

    configure_window (winRec->wid, XP_WINDOW_LEVEL, &wc);
}

/* Return the id of the physical window displaying the given window. If
   CREATE is true and the window has no frame, attempt to create one. */
xp_window_id
RootlessGetPhysicalWindow (WindowPtr pWin, Bool create)
{
    RootlessWindowRec *winRec;

    if (TopLevelParent (pWin) == NULL)
	return 0;

    winRec = WINREC (pWin);

    if (winRec == NULL && create && !IsRoot (pWin)
	&& pWin->drawable.class == InputOutput)
    {
	rootlessEnsureFrame (pWin);
	winRec = WINREC (pWin);
    }

    if (winRec == NULL)
	return 0;

    return winRec->wid;
}

/* Given the id of a physical window, try to find the top-level (or root)
   X window that it represents. */
WindowPtr
RootlessGetXWindow (xp_window_id wid)
{
    RootlessWindowRec *winRec;

    if (window_hash == NULL)
	return NULL;

    winRec = x_hash_table_lookup (window_hash, (void *) wid, NULL);

    return winRec != NULL ? winRec->win : NULL;
}

/* Number is an appkit window number. Returns true if X is displaying
   a window with that number. */
int
RootlessKnowsWindowNumber (int number)
{
    Bool ret;
    xp_window_id wid;

    /* need to lock, since this function can be called by any thread */

    if (window_hash == NULL)
	return FALSE;

    pthread_mutex_lock (&window_hash_mutex);

    if (xp_lookup_native_window (number, &wid))
	ret = RootlessGetXWindow (wid) != NULL;
    else
	ret = FALSE;

    pthread_mutex_unlock (&window_hash_mutex);

    return ret;
}

Bool
RootlessUnrealizeWindow (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec = WINREC(pWin);
    xp_window_changes wc;
    Bool result;

    RL_DEBUG_MSG("unrealizewindow start\n");

    if (winRec != NULL)
    {
	RootlessStopDrawing (pWin, FALSE);

	wc.stack_mode = XP_UNMAPPED;
	wc.sibling = 0;

	configure_window (winRec->wid, XP_STACKING, &wc);

	winRec->unrealize_time = current_time_in_seconds ();
	winRec->is_reorder_pending = FALSE;

	RootlessReenableUpdate (pWin);
    }

    SCREEN_UNWRAP (pScreen, UnrealizeWindow);

    result = pScreen->UnrealizeWindow (pWin);

    SCREEN_WRAP (pScreen, UnrealizeWindow);

    RL_DEBUG_MSG ("unrealizewindow end\n");
    return result;
}

void
RootlessReparentWindow (WindowPtr pWin, WindowPtr pPriorParent)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec = WINREC (pWin);

    WindowPtr pTopWin;

    if (IsRoot (pWin) || IsRoot (pWin->parent)
	|| IsTopLevel (pWin) || winRec == NULL)
    {
	return;
    }

    /* If the window is moving upwards towards the root has a frame,
       we want to try to move it onto its new toplevel parent. If we
       can't do that, we'll just have to jettison it.. */

    pTopWin = TopLevelParent (pWin);
    assert (pTopWin != pWin);

    pWin->rootlessUnhittable = FALSE;

    DeleteProperty (pWin, xa_native_window_id ());

    if (WINREC (pTopWin) != NULL)
    {
	/* We're screwed. */
	rootlessDestroyFrame (pWin, winRec);
    }
    else
    {
	xp_window_changes wc;
	int sx, sy;

	if (!pTopWin->realized && pWin->realized)
	{
	    wc.stack_mode = XP_UNMAPPED;
	    wc.sibling = 0;

	    RootlessStopDrawing (pWin, FALSE);
	    configure_window (winRec->wid, XP_STACKING, &wc);
	}

	/* Switch the frame record from one to the other. */

	WINREC (pWin) = NULL;
	WINREC (pTopWin) = winRec;

	rootlessInitializeFrame (pTopWin, winRec);
	rootlessSetShape (pTopWin);

	sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
	sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

	wc.x = sx + winRec->x;
	wc.y = sy + winRec->y;
	wc.width = winRec->width;
	wc.height = winRec->height;
	wc.bit_gravity = XP_GRAVITY_NONE;

	RootlessStopDrawing (pWin, FALSE);
	configure_window (winRec->wid, XP_BOUNDS, &wc);

	rootlessSetNativeProperty (winRec);

	if (pTopWin->realized && !pWin->realized)
	    winRec->is_reorder_pending = TRUE;
    }
}

/* Reorder the window associated with the given frame so that it's
   physically above the window below it in the X stacking order. */
void
RootlessReorderWindow (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);

    if (pWin->realized && winRec != NULL
	&& !winRec->is_reorder_pending && !windows_hidden)
    {
	WindowPtr newPrevW;
	RootlessWindowRec *newPrev;
	xp_window_changes wc;
	Atom atom;
	PropertyPtr prop;

	/* quartz-wm sets the _APPLE_NO_ORDER_IN property on windows
	   that are being genie-restored from the Dock. We want them to
	   be mapped but remain ordered-out until the animation
	   completes (when the Dock will order them in) */

	atom = xa_apple_no_order_in ();
	for (prop = wUserProps (pWin); prop != NULL; prop = prop->next)
	{
	    if (prop->propertyName == atom && prop->type == atom)
		return;
	}

	RootlessStopDrawing (pWin, FALSE);

	if (IsRoot (pWin))
	{
	    wc.stack_mode = XP_MAPPED_BELOW;
	    wc.sibling = 0;
	}
	else
	{
	    /* Find the next window above this one that has a mapped frame. */

	    newPrevW = pWin->prevSib;
	    while (newPrevW
		   && (WINREC (newPrevW) == NULL || !newPrevW->realized))
	    {
		newPrevW = newPrevW->prevSib;
	    }

	    newPrev = newPrevW != NULL ? WINREC (newPrevW) : NULL;

	    /* Then either stack ourselves below it if it exists, or raise
	       ourselves above everything otherwise. */

	    if (newPrev == NULL)
	    {
		wc.stack_mode = XP_MAPPED_ABOVE;
		wc.sibling = 0;
	    }
	    else
	    {
		if (newPrev->is_reorder_pending)
		{
		    newPrev->is_reorder_pending = FALSE;
		    RootlessReorderWindow (newPrevW);
		}

		wc.stack_mode = XP_MAPPED_BELOW;
		wc.sibling = newPrev->wid;
	    }
	}

	configure_window (winRec->wid, XP_STACKING, &wc);
    }
}

void
RootlessRestackWindow (WindowPtr pWin, WindowPtr pOldNextSib)
{
    RegionRec saveRoot;
    RootlessWindowRec *winRec = WINREC (pWin);
    ScreenPtr pScreen = pWin->drawable.pScreen;

    RL_DEBUG_MSG("restackwindow start\n");
    if (winRec != NULL)
	RL_DEBUG_MSG("restack top level \n");

    HUGE_ROOT(pWin);
    SCREEN_UNWRAP(pScreen, RestackWindow);

    if (pScreen->RestackWindow != NULL)
	pScreen->RestackWindow (pWin, pOldNextSib);

    SCREEN_WRAP(pScreen, RestackWindow);
    NORMAL_ROOT(pWin);

    if (winRec != NULL && pWin->viewable)
	RootlessReorderWindow (pWin);

    RL_DEBUG_MSG("restackwindow end\n");
}

/*
 * Specialized window copy procedures
 */

/* Globals needed during window resize and move. */
static pointer gResizeDeathBits = NULL;
static int gResizeDeathCount;
static PixmapPtr gResizeDeathPix[2];
static BoxRec gResizeDeathBounds[2];
static CopyWindowProcPtr gResizeOldCopyWindowProc = NULL;

/* CopyWindow () that doesn't do anything. For MoveWindow() of
   top-level windows. */
static void
RootlessNoCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    /* Some code expects the region to be translated */

    int dx = ptOldOrg.x - pWin->drawable.x;
    int dy = ptOldOrg.y - pWin->drawable.y;

    REGION_TRANSLATE (pWin->drawable.pScreen, prgnSrc, -dx, -dy);
}

/* CopyWindow used during ResizeWindow for gravity moves. (from fbCopyWindow)
   The original always draws on the root pixmap (which we don't have).
   Instead, draw on the parent window's pixmap.
   Resize version: the old location's pixels are in gResizeCopyWindowSource */
static void
RootlessResizeCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg,
			  RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RegionRec rgnDst;
    int dx, dy;

    RL_DEBUG_MSG("resizecopywindowFB start (win 0x%x)\n", pWin);

    /* Don't unwrap pScreen->CopyWindow.
       The bogus rewrap with RootlessCopyWindow causes a crash if
       CopyWindow is called again during the same resize. */

    if (gResizeDeathCount == 0)
	return;

    RootlessStartDrawing (pWin);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE (pScreen, prgnSrc, -dx, -dy);
    REGION_INIT (pScreen, &rgnDst, NullBox, 0);
    REGION_INTERSECT (pScreen, &rgnDst, &pWin->borderClip, prgnSrc);

    if (gResizeDeathCount == 1)
    {
	/* Simple case, we only have a single source pixmap. */

	fbCopyRegion (&gResizeDeathPix[0]->drawable,
		      &pScreen->GetWindowPixmap(pWin)->drawable, 0,
		      &rgnDst, dx, dy, fbCopyWindowProc, 0, 0);
    }
    else
    {
	int i;
	RegionRec clip, clipped;

	/* More complex case, N source pixmaps (usually two). So we
	   intersect the destination with each source and copy those bits. */

	for (i = 0; i < gResizeDeathCount; i++)
	{
	    REGION_INIT (pScreen, &clip, gResizeDeathBounds + 0, 1);
	    REGION_INIT (pScreen, &clipped, NullBox, 0);
	    REGION_INTERSECT (pScreen, &rgnDst, &clip, &clipped);

	    fbCopyRegion (&gResizeDeathPix[i]->drawable,
			   &pScreen->GetWindowPixmap(pWin)->drawable, 0,
			   &clipped, dx, dy, fbCopyWindowProc, 0, 0);

	    REGION_UNINIT (pScreen, &clipped);
	    REGION_UNINIT (pScreen, &clip);
	}
    }
	
    /* Don't update - resize will update everything */
    REGION_UNINIT (pScreen, &rgnDst);

    fbValidateDrawable (&pWin->drawable);

    RL_DEBUG_MSG("resizecopywindowFB end\n");
}

/* Update *new* location of window. Old location is redrawn with
   PaintWindowBackground/Border. Cloned from fbCopyWindow
   The original always draws on the root pixmap (which we don't have).
   Instead, draw on the parent window's pixmap. */
void
RootlessCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec;
    WindowPtr top;

    RegionRec rgnDst;
    int dx, dy;
    BoxPtr extents;
    unsigned int area;

    RL_DEBUG_MSG("copywindowFB start (win 0x%x)\n", pWin);

    top = TopLevelParent (pWin);
    if (top == NULL)
    {
	RL_DEBUG_MSG("no parent\n");
	return;
    }

    winRec = WINREC(top);
    if (winRec == NULL)
    {
	RL_DEBUG_MSG("not framed\n");
	return;
    }

    SCREEN_UNWRAP (pScreen, CopyWindow);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE (pScreen, prgnSrc, -dx, -dy);

    REGION_INIT (pScreen, &rgnDst, NullBox, 0);
    REGION_INTERSECT (pScreen, &rgnDst, &pWin->borderClip, prgnSrc);

    extents = REGION_EXTENTS (pScreen, &rgnDst);
    area = (extents->x2 - extents->x1) * (extents->y2 - extents->y1);

    if (area > xp_scroll_area_threshold)
    {
	/* Move region to window local coords */
	REGION_TRANSLATE (pScreen, &rgnDst, -winRec->x, -winRec->y);

	RootlessStopDrawing (pWin, FALSE);

	xp_copy_window (winRec->wid, winRec->wid,
			REGION_NUM_RECTS (&rgnDst),
			REGION_RECTS (&rgnDst), dx, dy);
    }
    else
    {
	RootlessStartDrawing (pWin);

        fbCopyRegion ((DrawablePtr) pWin, (DrawablePtr) pWin,
		      0, &rgnDst, dx, dy, fbCopyWindowProc, 0, 0);

	/* prgnSrc has been translated to dst position */
	RootlessDamageRegion(pWin, prgnSrc);
    }

    REGION_UNINIT (pScreen, &rgnDst);
    fbValidateDrawable (&pWin->drawable);
	
    SCREEN_WRAP (pScreen, CopyWindow);

    RL_DEBUG_MSG("copywindowFB end\n");
}

/*
 * Window resize procedures
 */

enum {
    WIDTH_SMALLER = 1,
    HEIGHT_SMALLER = 2,
};

/* Compute which directions the window is resizing in. */
static inline unsigned int
resize_code (int oldX, int oldY, int oldW, int oldH,
	     int newX, int newY, int newW, int newH)
{
    unsigned int code = 0;

    /* These comparisons were chosen to avoid setting bits when the sizes
       are the same. (So the fastest case automatically gets taken when
       dimensions are unchanging.) */

    if (newW < oldW)
	code |= WIDTH_SMALLER;

    if (newH < oldH)
	code |= HEIGHT_SMALLER;

    return code;
}

static inline unsigned int
resize_weighting (int oldX1, int oldY1, int oldX2, int oldY2, int oldBW,
		  int newX1, int newY1, int newX2, int newY2, int newBW)
{
    /* Choose gravity to avoid local copies. Do that by looking for
       a corner that doesn't move _relative to the screen_  */

    if (newBW != oldBW)
	return XP_GRAVITY_NONE;

    if (newX1 == oldX1 && newY1 == oldY1)
	return XP_GRAVITY_NORTH_WEST;
    else if (newX1 == oldX1 && newY2 == oldY2)
	return XP_GRAVITY_SOUTH_WEST;
    else if (newX2 == oldX2 && newY2 == oldY2)
	return XP_GRAVITY_SOUTH_EAST;
    else if (newX2 == oldX2 && newY1 == oldY1)
	return XP_GRAVITY_NORTH_EAST;
    else
	return XP_GRAVITY_NONE;
}

/* Resize the given window to its new position and size. */
static void
resize_frame (ScreenPtr pScreen, WindowPtr pWin,
	      RootlessWindowRec *winRec, int gravity)
{
    int sx, sy;
    xp_window_changes wc;

    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

    wc.x = sx + winRec->x;
    wc.y = sy + winRec->y;
    wc.width = winRec->width;
    wc.height = winRec->height;
    wc.bit_gravity = gravity;

    /* It's unlikely that being async will save us anything here.
       But it can't hurt. */

    configure_window (winRec->wid, XP_BOUNDS, &wc);
}

/* Prepare to resize a top-level window. The old window's pixels are
   saved and the implementation is told to change the window size.
   (x,y,w,h) is outer frame of window (outside border) */
static Bool
StartFrameResize (WindowPtr pWin, Bool gravity,
		  int oldX, int oldY, int oldW, int oldH, int oldBW,
		  int newX, int newY, int newW, int newH, int newBW)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec = WINREC(pWin);
    Bool need_window_source = FALSE, resize_after = FALSE;

    BoxRec rect, copy_rect;
    int oldX2, newX2;
    int oldY2, newY2;
    unsigned int weight;

    oldX2 = oldX + oldW, newX2 = newX + newW;
    oldY2 = oldY + oldH, newY2 = newY + newH;

    /* Decide which resize weighting to use */
    weight = resize_weighting (oldX, oldY, oldW, oldH, oldBW,
			       newX, newY, newW, newH, newBW);

    /* Compute intersection between old and new rects */
    rect.x1 = max(oldX, newX);
    rect.y1 = max(oldY, newY);
    rect.x2 = min(oldX2, newX2);
    rect.y2 = min(oldY2, newY2);

    RL_DEBUG_MSG("RESIZE TOPLEVEL WINDOW with gravity %i ", gravity);
    RL_DEBUG_MSG("%d %d %d %d %d   %d %d %d %d %d\n",
                 oldX, oldY, oldW, oldH, oldBW,
                 newX, newY, newW, newH, newBW);

    RootlessDisableUpdate (pWin);
    RootlessRedisplay (pWin);

    /* If gravity is true, then we need to have a way of recovering all
       the original bits in the window for when X rearranges the contents
       based on the various gravity settings. The obvious way is to just
       snapshot the entire backing store before resizing it, but that
       it slow on large windows.
       
       So the optimization here is to use CG's resize weighting options
       to allow us to reason about what is left in the backing store
       after the resize. We can then only copy what won't be there after
       the resize, and do a two-stage copy operation.

       Most of these optimizations are only applied when the top-left
       corner of the window is fixed, since that's the common case. They
       could probably be extended with some thought. */

    gResizeDeathCount = 0;

    if (gravity && weight == XP_GRAVITY_NORTH_WEST)
    {
	unsigned int code;

	/* Top left corner is anchored. We never need to copy the
	   entire window. */

	need_window_source = TRUE;

	code = resize_code (oldX, oldY, oldW, oldH,
			    newX, newY, newW, newH);

	if (((code ^ (code >> 1)) & 1) == 0)
	{
	    /* Both dimensions are either getting larger, or both
	       are getting smaller. No need to copy anything. */

	    if (code == (WIDTH_SMALLER | HEIGHT_SMALLER))
	    {
		/* Since the window is getting smaller, we can do gravity
		   repair on it with it's current size, then resize it
		   afterwards. */

		resize_after = TRUE;
	    }

	    gResizeDeathCount = 1;
	}
	else
	{
	    unsigned int copy_rowbytes, Bpp;

	    /* We can get away with a partial copy. 'rect' is the
	       intersection between old and new bounds, so copy
	       everything to the right of or below the intersection. */

	    RootlessStartDrawing (pWin);

	    if (code == WIDTH_SMALLER)
	    {
		copy_rect.x1 = rect.x2;
		copy_rect.y1 = rect.y1;
		copy_rect.x2 = oldX2;
		copy_rect.y2 = oldY2;
	    }
	    else if (code == HEIGHT_SMALLER)
	    {
		copy_rect.x1 = rect.x1;
		copy_rect.y1 = rect.y2;
		copy_rect.x2 = oldX2;
		copy_rect.y2 = oldY2;
	    }
	    else
		abort ();

	    Bpp = winRec->win->drawable.bitsPerPixel / 8;
	    copy_rowbytes = (((copy_rect.x2 - copy_rect.x1) * Bpp) + 31) & ~31;
	    gResizeDeathBits = xalloc (copy_rowbytes
				       * (copy_rect.y2 - copy_rect.y1));

	    xp_copy_bytes ((copy_rect.x2 - copy_rect.x1) * Bpp,
			   copy_rect.y2 - copy_rect.y1, ((char *) winRec->data)
			   + ((copy_rect.y1 - oldY) * winRec->rowbytes)
			   + (copy_rect.x1 - oldX) * Bpp, winRec->rowbytes,
			   gResizeDeathBits, copy_rowbytes);
	    
	    gResizeDeathBounds[1] = copy_rect;
	    gResizeDeathPix[1]
		= GetScratchPixmapHeader(pScreen, copy_rect.x2 - copy_rect.x1,
					 copy_rect.y2 - copy_rect.y1,
					 winRec->win->drawable.depth,
					 winRec->win->drawable.bitsPerPixel,
					 winRec->rowbytes,
					 (void *) gResizeDeathBits);

	    TranslatePixmapBase (gResizeDeathPix[1],
				 -copy_rect.x1, -copy_rect.y1);

	    gResizeDeathCount = 2;
	}
    }
    else if (gravity)
    {
	/* The general case. Just copy everything. */

	RootlessStartDrawing (pWin);

        gResizeDeathBits = xalloc(winRec->rowbytes * winRec->height);

	memcpy(gResizeDeathBits, winRec->data,
	       winRec->rowbytes * winRec->height);

	gResizeDeathBounds[0] = (BoxRec) {oldX, oldY, oldX2, oldY2};
	gResizeDeathPix[0]
	    = GetScratchPixmapHeader(pScreen, winRec->width,
				     winRec->height,
				     winRec->win->drawable.depth,
				     winRec->win->drawable.bitsPerPixel,
				     winRec->rowbytes,
				     (void *) gResizeDeathBits);

	TranslatePixmapBase (gResizeDeathPix[0], -oldX, -oldY);
	gResizeDeathCount = 1;
    }

    RootlessStopDrawing (pWin, FALSE);

    winRec->x = newX;
    winRec->y = newY;
    winRec->width = newW;
    winRec->height = newH;
    winRec->borderWidth = newBW;

    /* Unless both dimensions are getting smaller, Resize the frame
       before doing gravity repair */

    if (!resize_after)
	resize_frame (pScreen, pWin, winRec, weight);

    RootlessStartDrawing(pWin);

    /* If necessary, create a source pixmap pointing at the current
       window bits. */

    if (need_window_source)
    {
	gResizeDeathBounds[0] = (BoxRec) {oldX, oldY, oldX2, oldY2};
	gResizeDeathPix[0]
	    = GetScratchPixmapHeader (pScreen, oldW, oldH,
				      winRec->win->drawable.depth,
				      winRec->win->drawable.bitsPerPixel,
				      winRec->rowbytes, winRec->data);

	TranslatePixmapBase (gResizeDeathPix[0], -oldX, -oldY);
    }

    /* Use custom CopyWindow when moving gravity bits around
       ResizeWindow assumes the old window contents are in the same
       pixmap, but here they're in deathPix instead. */

    if (gravity)
    {
	gResizeOldCopyWindowProc = pScreen->CopyWindow;
	pScreen->CopyWindow = RootlessResizeCopyWindow;
    }

    /* If we can't rely on the window server preserving the bits we
       need in the position we need, copy the pixels in the
       intersection from src to dst. ResizeWindow assumes these pixels
       are already present when making gravity adjustments. pWin
       currently has new-sized pixmap but is in old position.

       FIXME: border width change! (?) */

    if (gravity && weight == XP_GRAVITY_NONE)
    {
        PixmapPtr src, dst;

	assert (gResizeDeathCount == 1);

	src = gResizeDeathPix[0];
        dst = pScreen->GetWindowPixmap(pWin);

        RL_DEBUG_MSG("Resize copy rect %d %d %d %d\n",
                     rect.x1, rect.y1, rect.x2, rect.y2);

        /* rect is the intersection of the old location and new location */
        if (BOX_NOT_EMPTY(rect) && src != NULL && dst != NULL)
	{
	    int dx, dy;

	    /* The window drawable still has the old frame position, which
	       means that DST doesn't actually point at the origin of our
	       physical backing store when adjusted by the drawable.x,y
	       position. So sneakily adjust it temporarily while copying.. */

	    dx = newX - oldX;
	    dy = newY - oldY;
	    TranslatePixmapBase (dst, -dx, -dy);

            fbCopyWindowProc(&src->drawable, &dst->drawable, NULL,
			     &rect, 1, 0, 0, FALSE, FALSE, 0, 0);

	    TranslatePixmapBase (dst, dx, dy);
        }
    }

    return resize_after;
}

static void
FinishFrameResize (WindowPtr pWin, Bool gravity, int oldX, int oldY,
                  unsigned int oldW, unsigned int oldH, unsigned int oldBW,
                  int newX, int newY, unsigned int newW, unsigned int newH,
		  unsigned int newBW, Bool resize_now)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RootlessWindowRec *winRec = WINREC(pWin);
    BoxRec box;
    int i;

    RootlessStopDrawing (pWin, FALSE);

    if (resize_now)
    {
	unsigned int weight;

	/* We didn't resize anything earlier, so do it now, now that
	   we've finished gravitating the bits. */

	weight = resize_weighting (oldX, oldY, oldW, oldH, oldBW,
				   newX, newY, newW, newH, newBW);

	resize_frame (pScreen, pWin, winRec, weight);
    }

    /* Redraw everything. FIXME: there must be times when we don't need
       to do this. Perhaps when top-left weighting and no gravity? */

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = winRec->width;
    box.y2 = winRec->height;

    xp_mark_window (winRec->wid, 1, &box, 0, 0);

    for (i = 0; i < 2; i++)
    {
	if (gResizeDeathPix[i] != NULL)
	{
	    FreeScratchPixmapHeader (gResizeDeathPix[i]);
	    gResizeDeathPix[i] = NULL;
	}
    }

    if (gResizeDeathBits != NULL)
    {
	xfree (gResizeDeathBits);
	gResizeDeathBits = NULL;
    }

    if (gravity)
        pScreen->CopyWindow = gResizeOldCopyWindowProc;
}

/* If kind==VTOther, window border is resizing (and borderWidth is
   already changed!!@#$)  This case works like window resize, not move. */
void
RootlessMoveWindow (WindowPtr pWin, int x, int y, WindowPtr pSib, VTKind kind)
{
    RootlessWindowRec *winRec = WINREC(pWin);
    ScreenPtr pScreen = pWin->drawable.pScreen;
    CopyWindowProcPtr oldCopyWindowProc = NULL;

    int oldX = 0, oldY = 0;
    unsigned int oldW = 0, oldH = 0, oldBW = 0;
    int newX = 0, newY = 0;
    unsigned int newW = 0, newH = 0, newBW = 0;

    Bool resize_after = FALSE;
    RegionRec saveRoot;

    RL_DEBUG_MSG("movewindow start \n");

    if (winRec != NULL)
    {
        if (kind == VTMove)
	{
	    oldX = winRec->x;
            oldY = winRec->y;
            RootlessRedisplay (pWin);
            RootlessStartDrawing (pWin);
        }
	else
	{
            RL_DEBUG_MSG("movewindow border resizing ");

            oldBW = winRec->borderWidth;
            oldX = winRec->x;
            oldY = winRec->y;
            oldW = winRec->width;
            oldH = winRec->height;

            newBW = wBorderWidth (pWin);
            newX = x;
            newY = y;
            newW = pWin->drawable.width  + 2*newBW;
            newH = pWin->drawable.height + 2*newBW;

            resize_after = StartFrameResize (pWin, FALSE,
					     oldX, oldY, oldW, oldH, oldBW,
					     newX, newY, newW, newH, newBW);
        }
    }

    HUGE_ROOT (pWin);
    SCREEN_UNWRAP (pScreen, MoveWindow);

    oldCopyWindowProc = pScreen->CopyWindow;

    if (winRec != NULL)
        pScreen->CopyWindow = RootlessNoCopyWindow;

    pScreen->MoveWindow (pWin, x, y, pSib, kind);

    if (winRec != NULL)
        pScreen->CopyWindow = oldCopyWindowProc;

    NORMAL_ROOT (pWin);
    SCREEN_WRAP (pScreen, MoveWindow);

    if (winRec != NULL)
    {
        if (kind == VTMove)
	{
	    xp_window_changes wc;
	    int sx, sy;

            RootlessStopDrawing (pWin, FALSE);

	    winRec->x = x;
	    winRec->y = y;

	    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
	    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

	    wc.x = sx + winRec->x;
	    wc.y = sy + winRec->y;

	    configure_window (winRec->wid, XP_ORIGIN, &wc);
        }
	else
	{
            FinishFrameResize (pWin, FALSE, oldX, oldY, oldW, oldH, oldBW,
			       newX, newY, newW, newH, newBW, resize_after);
        }
    }

    RL_DEBUG_MSG("movewindow end\n");
}


/* Note: (x, y, w, h) as passed to this procedure don't match the frame
   definition. (x,y) is corner of very outer edge, *outside* border
   w,h is width and height *inside* border, *ignoring* border width
   The rect (x, y, w, h) doesn't mean anything. (x, y, w+2*bw, h+2*bw)
   is total rect (x+bw, y+bw, w, h) is inner rect */
void
RootlessResizeWindow (WindowPtr pWin, int x, int y,
		      unsigned int w, unsigned int h, WindowPtr pSib)
{
    RootlessWindowRec *winRec = WINREC(pWin);
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RegionRec saveRoot;

    int oldX = 0, oldY = 0;
    unsigned int oldW = 0, oldH = 0, oldBW = 0;
    int newX = 0, newY = 0;
    unsigned int newW = 0, newH = 0, newBW = 0;

    Bool resize_after = FALSE;

    RL_DEBUG_MSG("resizewindow start (win 0x%x)\n", pWin);

    if (winRec != NULL)
    {
        oldBW = winRec->borderWidth;
        oldX = winRec->x;
        oldY = winRec->y;
        oldW = winRec->width;
        oldH = winRec->height;

        newBW = oldBW;
        newX = x;
        newY = y;
        newW = w + 2*newBW;
        newH = h + 2*newBW;

        resize_after = StartFrameResize (pWin, TRUE,
					 oldX, oldY, oldW, oldH, oldBW,
					 newX, newY, newW, newH, newBW);
    }

    HUGE_ROOT (pWin);
    SCREEN_UNWRAP (pScreen, ResizeWindow);

    if (!IsRoot (pWin))
    {
	pScreen->ResizeWindow (pWin, x, y, w, h, pSib);
    }
    else
    {
	BoxRec box;

	/* mi won't resize the root. So do it ourselves... */

	pWin->drawable.x = x;
	pWin->drawable.y = y;
	pWin->drawable.width = w;
	pWin->drawable.height = h;

	box.x1 = x; box.y1 = y;
	box.x2 = x + w; box.y2 = y + h;
	REGION_UNINIT (pScreen, &pWin->winSize);
	REGION_INIT (pScreen, &pWin->winSize, &box, 1);
	REGION_COPY (pScreen, &pWin->borderSize, &pWin->winSize);
	REGION_COPY (pScreen, &pWin->clipList, &pWin->winSize);
	REGION_COPY (pScreen, &pWin->borderClip, &pWin->winSize);

	miSendExposures (pWin, &pWin->borderClip,
			 pWin->drawable.x, pWin->drawable.y);
    }

    SCREEN_WRAP (pScreen, ResizeWindow);
    NORMAL_ROOT (pWin);

    if (winRec != NULL)
    {
        FinishFrameResize (pWin, TRUE, oldX, oldY, oldW, oldH, oldBW,
			   newX, newY, newW, newH, newBW, resize_after);
    }

    RL_DEBUG_MSG("resizewindow end\n");
}

void
RootlessRepositionWindow (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);
    ScreenPtr pScreen = pWin->drawable.pScreen;
    xp_window_changes wc;
    int sx, sy;

    if (IsRoot (pWin))
	set_screen_origin (pWin);

    if (winRec == NULL)
	return;

    RootlessStopDrawing (pWin, FALSE);

    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

    wc.x = sx + winRec->x;
    wc.y = sy + winRec->y;

    if (!rootlessHasRoot (pScreen))
	wc.window_level = normal_window_levels[winRec->level];
    else
	wc.window_level = rooted_window_levels[winRec->level];

    configure_window (winRec->wid, XP_ORIGIN | XP_WINDOW_LEVEL, &wc);

    RootlessReorderWindow (pWin);
}

void
RootlessFlushWindowColormap (WindowPtr pWin)
{
    RootlessWindowRec *winRec = WINREC (pWin);
    xp_window_changes wc;

    if (winRec == NULL)
	return;

    RootlessStopDrawing (pWin, FALSE);

    /* This is how we tell xp that the colormap may have changed. */

    wc.colormap = rootlessColormapCallback;
    wc.colormap_data = pWin->drawable.pScreen;

    configure_window (winRec->wid, XP_COLORMAP, &wc);
}

/* Set the Pixmaps on all ParentRelative windows up the ancestor chain. */
static void
SetPixmapOfAncestors (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    WindowPtr topWin = TopLevelParent (pWin);
    RootlessWindowRec *topWinRec = WINREC (topWin);
    XID pixel;

    while (pWin->backgroundState == ParentRelative)
    {
        if (pWin == topWin)
	{
            /* disallow ParentRelative background state on top level */

            pixel = 0;
            ChangeWindowAttributes(pWin, CWBackPixel, &pixel, serverClient);

            RL_DEBUG_MSG("Cleared ParentRelative on 0x%x.\n", pWin);
            break;
        }

        pWin = pWin->parent;
        pScreen->SetWindowPixmap (pWin, topWinRec->pixmap);
    }
}

/* RootlessPaintWindowBackground
   Paint the window background while filling in the alpha channel
   with all on. */
void
RootlessPaintWindowBackground (WindowPtr pWin, RegionPtr pRegion, int what)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    SCREEN_UNWRAP (pScreen, PaintWindowBackground);

    RL_DEBUG_MSG("paintwindowbackground start (win 0x%x, framed %i)\n",
                 pWin, IsFramedWindow(pWin));

    if (IsFramedWindow (pWin))
    {
	/* Don't bother trying to paint the surface background.. */
	rootlessNoDRIDrawing++;

        RootlessStartDrawing (pWin);
        RootlessDamageRegion (pWin, pRegion);

        /* For ParentRelative windows, we have to make sure the window
           pixmap is set correctly all the way up the ancestor chain. */

        if (pWin->backgroundState == ParentRelative)
	    SetPixmapOfAncestors (pWin);

	pScreen->PaintWindowBackground (pWin, pRegion, what);

	rootlessNoDRIDrawing--;
    }

    SCREEN_WRAP (pScreen, PaintWindowBackground);

    RL_DEBUG_MSG("paintwindowbackground end\n");
}


/* RootlessPaintWindowBorder
   Paint the window border while filling in the alpha channel with all on. */
void
RootlessPaintWindowBorder (WindowPtr pWin, RegionPtr pRegion, int what)
{
    SCREEN_UNWRAP (pWin->drawable.pScreen, PaintWindowBorder);

    RL_DEBUG_MSG("paintwindowborder start (win 0x%x)\n", pWin);

    if (IsFramedWindow (pWin))
    {
        RootlessStartDrawing (pWin);
        RootlessDamageRegion (pWin, pRegion);

        /* For ParentRelative windows with tiled borders, we have to make
           sure the window pixmap is set correctly all the way up the
           ancestor chain. */

        if (!pWin->borderIsPixel && pWin->backgroundState == ParentRelative)
            SetPixmapOfAncestors (pWin);
    }

    pWin->drawable.pScreen->PaintWindowBorder (pWin, pRegion, what);

    SCREEN_WRAP (pWin->drawable.pScreen, PaintWindowBorder);

    RL_DEBUG_MSG("paintwindowborder end\n");
}


/* FIXME: untested!
   pWin inside corner stays the same; pWin->drawable.[xy] stays the same
   frame moves and resizes */
void
RootlessChangeBorderWidth (WindowPtr pWin, unsigned int width)
{
    RegionRec saveRoot;
    Bool resize_after = FALSE;

    RL_DEBUG_MSG("change border width\n");

    if ((int) width != wBorderWidth (pWin))
    {
        RootlessWindowRec *winRec = WINREC(pWin);

        int oldX = 0, oldY = 0, newX = 0, newY = 0;
        unsigned int oldW = 0, oldH = 0, oldBW = 0;
        unsigned int newW = 0, newH = 0, newBW = 0;

        if (winRec != NULL)
	{
            oldBW = winRec->borderWidth;
            oldX = winRec->x;
            oldY = winRec->y;
            oldW = winRec->width;
            oldH = winRec->height;

            newBW = width;
            newX = pWin->drawable.x - newBW;
            newY = pWin->drawable.y - newBW;
            newW = pWin->drawable.width  + 2*newBW;
            newH = pWin->drawable.height + 2*newBW;

	    resize_after = StartFrameResize (pWin, FALSE,
					     oldX, oldY, oldW, oldH, oldBW,
					     newX, newY, newW, newH, newBW);
        }

        HUGE_ROOT (pWin);
        SCREEN_UNWRAP (pWin->drawable.pScreen, ChangeBorderWidth);

        pWin->drawable.pScreen->ChangeBorderWidth (pWin, width);

        SCREEN_WRAP (pWin->drawable.pScreen, ChangeBorderWidth);
        NORMAL_ROOT (pWin);

        if (winRec != NULL)
	{
            FinishFrameResize (pWin, FALSE, oldX, oldY, oldW, oldH, oldBW,
			       newX, newY, newW, newH, newBW, resize_after);
        }
    }

    RL_DEBUG_MSG("change border width end\n");
}

/*
 * RootlessFillRegionTiled
 *  Fill using a tile while leaving the alpha channel untouched.
 *  Based on fbfillRegionTiled.
 */
static void
RootlessFillRegionTiled(
    DrawablePtr pDrawable,
    RegionPtr   pRegion,
    PixmapPtr   pTile)
{
    FbBits      *dst;
    FbStride    dstStride;
    int         dstBpp;
    int         dstXoff, dstYoff;
    FbBits      *tile;
    FbStride    tileStride;
    int         tileBpp;
    int         tileXoff, tileYoff; /* XXX assumed to be zero */
    int         tileWidth, tileHeight;
    int         n = REGION_NUM_RECTS(pRegion);
    BoxPtr      pbox = REGION_RECTS(pRegion);
    int         xRot = pDrawable->x;
    int         yRot = pDrawable->y;
    FbBits      planeMask;

#ifdef PANORAMIX
    if(!noPanoramiXExtension)
    {
        int index = pDrawable->pScreen->myNum;
        if(&WindowTable[index]->drawable == pDrawable)
        {
            xRot -= panoramiXdataPtr[index].x;
            yRot -= panoramiXdataPtr[index].y;
        }
    }
#endif
    fbGetDrawable (pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    fbGetDrawable (&pTile->drawable, tile, tileStride, tileBpp,
                   tileXoff, tileYoff);
    tileWidth = pTile->drawable.width;
    tileHeight = pTile->drawable.height;
    xRot += dstXoff;
    yRot += dstYoff;
    planeMask = FB_ALLONES & ~RootlessAlphaMask(dstBpp);

    while (n--)
    {
        fbTile (dst + (pbox->y1 + dstYoff) * dstStride,
                dstStride,
                (pbox->x1 + dstXoff) * dstBpp,
                (pbox->x2 - pbox->x1) * dstBpp,
                pbox->y2 - pbox->y1,
                tile,
                tileStride,
                tileWidth * dstBpp,
                tileHeight,
                GXcopy,
                planeMask,
                dstBpp,
                xRot * dstBpp,
                yRot - pbox->y1);
        pbox++;
    }
}

/*
 * RootlessPaintWindow
 *  Paint the window while filling in the alpha channel with all on.
 *  We can't use fbPaintWindow because it zeros the alpha channel.
 */
void
RootlessPaintWindow(
    WindowPtr pWin,
    RegionPtr pRegion,
    int what)
{
    switch (what) {
      case PW_BACKGROUND:

        switch (pWin->backgroundState) {
            case None:
                break;
            case ParentRelative:
                do {
                    pWin = pWin->parent;
                } while (pWin->backgroundState == ParentRelative);
                (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
                                                                    what);
                break;
            case BackgroundPixmap:
                RootlessFillRegionTiled (&pWin->drawable,
					 pRegion,
					 pWin->background.pixmap);
                break;
            case BackgroundPixel:
            {
                Pixel pixel = pWin->background.pixel |
                              RootlessAlphaMask(pWin->drawable.bitsPerPixel);
                fbFillRegionSolid (&pWin->drawable, pRegion, 0,
                                   fbReplicatePixel (pixel,
                                        pWin->drawable.bitsPerPixel));
                break;
            }
        }
    	break;
      case PW_BORDER:
        if (pWin->borderIsPixel)
        {
            Pixel pixel = pWin->border.pixel |
                          RootlessAlphaMask(pWin->drawable.bitsPerPixel);
            fbFillRegionSolid (&pWin->drawable, pRegion, 0,
                               fbReplicatePixel (pixel,
                                    pWin->drawable.bitsPerPixel));
        }
        else
        {
            WindowPtr pBgWin;
            for (pBgWin = pWin; pBgWin->backgroundState == ParentRelative;
                 pBgWin = pBgWin->parent);
    
            RootlessFillRegionTiled (&pBgWin->drawable,
				      pRegion,
				      pWin->border.pixmap);
        }
        break;
    }
    fbValidateDrawable (&pWin->drawable);
}
