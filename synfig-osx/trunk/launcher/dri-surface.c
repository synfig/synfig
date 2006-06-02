/* $XFree86: xc/programs/Xserver/GL/dri/dri.c,v 1.34 2001/12/10 19:07:19 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
Copyright (c) 2002 Apple Computer, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Jens Owen <jens@valinux.com>
 *   Rickard E. (Rik) Faith <faith@valinux.com>
 *
 */

#ifdef XFree86LOADER
#include "xf86.h"
#include "xf86_ansic.h"
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"
#define _APPLEDRI_SERVER_
#include "appledristr.h"
#include "swaprep.h"
#include "dri-surface.h"
#include "dristruct.h"
#include "mi.h"
#include "mipointer.h"
#include "rootless-common.h"
#include "x-hash.h"
#include "x-hook.h"

#include <AvailabilityMacros.h>

static int DRIScreenPrivIndex = -1;
static int DRIWindowPrivIndex = -1;
static int DRIPixmapPrivIndex = -1;
static unsigned long DRIGeneration = 0;

static RESTYPE DRIDrawablePrivResType;

static x_hash_table *surface_hash;	/* maps surface ids -> drawablePrivs */

Bool
DRIScreenInit(ScreenPtr pScreen)
{
    DRIScreenPrivPtr    pDRIPriv;
    int                 i;

    if (DRIGeneration != serverGeneration) {
	if ((DRIScreenPrivIndex = AllocateScreenPrivateIndex()) < 0)
	    return FALSE;
	DRIGeneration = serverGeneration;
    }

    pDRIPriv = (DRIScreenPrivPtr) xcalloc(1, sizeof(DRIScreenPrivRec));
    if (!pDRIPriv) {
        pScreen->devPrivates[DRIScreenPrivIndex].ptr = NULL;
        return FALSE;
    }

    pScreen->devPrivates[DRIScreenPrivIndex].ptr = (pointer) pDRIPriv;
    pDRIPriv->directRenderingSupport = TRUE;
    pDRIPriv->nrWindows = 0;

    /* Initialize drawable tables */
    for( i=0; i < DRI_MAX_DRAWABLES; i++) {
	pDRIPriv->DRIDrawables[i] = NULL;
    }

    return TRUE;
}

Bool
DRIFinishScreenInit(ScreenPtr pScreen)
{
    DRIScreenPrivPtr  pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    /* Wrap DRI support */
    pDRIPriv->wrap.ValidateTree = pScreen->ValidateTree;
    pScreen->ValidateTree = DRIValidateTree;

    pDRIPriv->wrap.PostValidateTree = pScreen->PostValidateTree;
    pScreen->PostValidateTree = DRIPostValidateTree;

    pDRIPriv->wrap.WindowExposures = pScreen->WindowExposures;
    pScreen->WindowExposures = DRIWindowExposures;

    pDRIPriv->wrap.CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = DRICopyWindow;

    pDRIPriv->wrap.ClipNotify = pScreen->ClipNotify;
    pScreen->ClipNotify = DRIClipNotify;

    ErrorF("[DRI] screen %d installation complete\n", pScreen->myNum);

    return TRUE;
}

void
DRICloseScreen(ScreenPtr pScreen)
{
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv && pDRIPriv->directRenderingSupport) {
	xfree(pDRIPriv);
	pScreen->devPrivates[DRIScreenPrivIndex].ptr = NULL;
    }
}

Bool
DRIExtensionInit(void)
{
    int		    	i;
    ScreenPtr		pScreen;

    if (DRIScreenPrivIndex < 0) {
	return FALSE;
    }

    /* Allocate a window private index with a zero sized private area for
     * each window, then should a window become a DRI window, we'll hang
     * a DRIWindowPrivateRec off of this private index.
     */
    if ((DRIWindowPrivIndex = AllocateWindowPrivateIndex()) < 0)
	return FALSE;
    if ((DRIPixmapPrivIndex = AllocatePixmapPrivateIndex()) < 0)
	return FALSE;

    DRIDrawablePrivResType = CreateNewResourceType(DRIDrawablePrivDelete);

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (!AllocateWindowPrivate(pScreen, DRIWindowPrivIndex, 0))
	    return FALSE;
	if (!AllocatePixmapPrivate(pScreen, DRIPixmapPrivIndex, 0))
	    return FALSE;
    }

    return TRUE;
}

void
DRIReset(void)
{
    /*
     * This stub routine is called when the X Server recycles, resources
     * allocated by DRIExtensionInit need to be managed here.
     *
     * Currently this routine is a stub because all the interesting resources
     * are managed via the screen init process.
     */
}

Bool
DRIQueryDirectRenderingCapable(ScreenPtr pScreen, Bool* isCapable)
{
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv)
	*isCapable = pDRIPriv->directRenderingSupport;
    else
	*isCapable = FALSE;

    return TRUE;
}

Bool
DRIAuthConnection(ScreenPtr pScreen, unsigned int magic)
{
#if 0
    /* FIXME: something? */

    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (drmAuthMagic(pDRIPriv->drmFD, magic)) return FALSE;
#endif
    return TRUE;
}

static void
DRIUpdateSurface (DRIDrawablePrivPtr pDRIDrawablePriv, DrawablePtr pDraw)
{
    WindowPtr pTopWin;
    xp_window_changes wc;
    unsigned int flags = 0;

    if (pDRIDrawablePriv->sid == 0)
	return;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    wc.depth = (pDraw->bitsPerPixel == 32 ? XP_DEPTH_ARGB8888
		: pDraw->bitsPerPixel == 16 ? XP_DEPTH_RGB555 : XP_DEPTH_NIL);
    if (wc.depth != XP_DEPTH_NIL)
	flags |= XP_DEPTH;
#endif

    if (pDraw->type == DRAWABLE_WINDOW) {
	WindowPtr pWin = (WindowPtr) pDraw;

	DRIStopDrawing (&pWin->drawable, FALSE);

	pTopWin = TopLevelParent (pWin);

	wc.x = pWin->drawable.x - (pTopWin->drawable.x - pTopWin->borderWidth);
	wc.y = pWin->drawable.y - (pTopWin->drawable.y - pTopWin->borderWidth);
	wc.width = pWin->drawable.width;
	wc.height = pWin->drawable.height;
	wc.bit_gravity = XP_GRAVITY_NONE;

	wc.shape_nrects = REGION_NUM_RECTS (&pWin->clipList);
	wc.shape_rects = REGION_RECTS (&pWin->clipList);
	wc.shape_tx = - (pTopWin->drawable.x - pTopWin->borderWidth);
	wc.shape_ty = - (pTopWin->drawable.y - pTopWin->borderWidth);

	flags |= XP_BOUNDS | XP_SHAPE;

	pDRIDrawablePriv->x = pWin->drawable.x - pTopWin->borderWidth;
	pDRIDrawablePriv->y = pWin->drawable.y - pTopWin->borderWidth;

    } else if (pDraw->type == DRAWABLE_PIXMAP) {
	wc.x = 0;
	wc.y = 0;
	wc.width = pDraw->width;
	wc.height = pDraw->height;
	wc.bit_gravity = XP_GRAVITY_NONE;
	flags |= XP_BOUNDS;
    }

    xp_configure_surface (pDRIDrawablePriv->sid, flags, &wc);
}

Bool
DRICreateSurface (ScreenPtr pScreen, Drawable id,
		  DrawablePtr pDrawable, xp_client_id client_id,
		  xp_surface_id *surface_id, unsigned int ret_key[2],
		  void (*notify) (void *arg, void *data), void *notify_data)
{
    DRIScreenPrivPtr	pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr	pDRIDrawablePriv;
    WindowPtr		pWin;
    PixmapPtr		pPix;
    xp_window_id	wid = 0;

    if (pDrawable->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr)pDrawable;
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);
	if (pDRIDrawablePriv == NULL) {
	    WindowPtr pTopWin;
            xp_error err;
	    xp_window_changes wc;

	    pTopWin = TopLevelParent (pWin);

	    /* allocate a DRI Window Private record */
	    if (!(pDRIDrawablePriv = xcalloc(1, sizeof(DRIDrawablePrivRec)))) {
		return FALSE;
	    }

	    pDRIDrawablePriv->pDraw = pDrawable;
	    pDRIDrawablePriv->pScreen = pScreen;
	    pDRIDrawablePriv->refCount = 0;
	    pDRIDrawablePriv->drawableIndex = -1;
	    pDRIDrawablePriv->notifiers = NULL;

	    /* find the physical window */
	    wid = RootlessGetPhysicalWindow (pTopWin, TRUE);
	    if (wid == 0) {
		xfree (pDRIDrawablePriv);
		return FALSE;
	    }

	    /* allocate the physical surface */
	    err = xp_create_surface (wid, &pDRIDrawablePriv->sid);
	    if (err != Success) {
		xfree (pDRIDrawablePriv);
		return FALSE;
	    }

	    /* Make it visible */
	    wc.stack_mode = XP_MAPPED_ABOVE;
	    wc.sibling = 0;
	    err = xp_configure_surface (pDRIDrawablePriv->sid,
					XP_STACKING, &wc);
	    if (err != Success)
	    {
		xp_destroy_surface (pDRIDrawablePriv->sid);
		xfree (pDRIDrawablePriv);
		return FALSE;
	    }

	    /* save private off of preallocated index */
	    pWin->devPrivates[DRIWindowPrivIndex].ptr = (pointer)pDRIDrawablePriv;
	}
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    else if (pDrawable->type == DRAWABLE_PIXMAP) {
	pPix = (PixmapPtr)pDrawable;
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP(pPix);
	if (pDRIDrawablePriv == NULL) {
	    xp_error err;

	    /* allocate a DRI Window Private record */
	    if (!(pDRIDrawablePriv = xcalloc(1, sizeof(DRIDrawablePrivRec)))) {
		return FALSE;
	    }

	    pDRIDrawablePriv->pDraw = pDrawable;
	    pDRIDrawablePriv->pScreen = pScreen;
	    pDRIDrawablePriv->refCount = 0;
	    pDRIDrawablePriv->drawableIndex = -1;
	    pDRIDrawablePriv->notifiers = NULL;

	    /* Passing a null window id to Xplugin in 10.3+ asks for
	       an accelerated offscreen surface. */

	    err = xp_create_surface (0, &pDRIDrawablePriv->sid);
	    if (err != Success) {
		xfree (pDRIDrawablePriv);
		return FALSE;
	    }

	    /* save private off of preallocated index */
	    pPix->devPrivates[DRIPixmapPrivIndex].ptr = (pointer)pDRIDrawablePriv;
	}
    }
#endif
    else { /* for GLX 1.3, a PBuffer */
	/* NOT_DONE */
	return FALSE;
    }

    /* Finish initialization of new surfaces */
    if (pDRIDrawablePriv->refCount == 0) {
	unsigned int key[2] = {0};
	xp_error err;

	/* try to give the client access to the surface
	   FIXME: how to export pixmaps? */
	if (client_id != 0 && wid != 0)
	{
	    err = xp_export_surface (wid, pDRIDrawablePriv->sid,
				     client_id, key);
	    if (err != Success) {
		xp_destroy_surface (pDRIDrawablePriv->sid);
		xfree (pDRIDrawablePriv);
		return FALSE;
	    }
	}

	pDRIDrawablePriv->key[0] = key[0];
	pDRIDrawablePriv->key[1] = key[1];

	++pDRIPriv->nrWindows;

	/* and stash it by surface id */
	if (surface_hash == NULL)
	    surface_hash = x_hash_table_new (NULL, NULL, NULL, NULL);
	x_hash_table_insert (surface_hash,
			     (void *) pDRIDrawablePriv->sid, pDRIDrawablePriv);

	/* track this in case this window is destroyed */
	AddResource(id, DRIDrawablePrivResType, (pointer)pDrawable);

	/* Initialize shape */
	DRIUpdateSurface (pDRIDrawablePriv, pDrawable);
    }

    pDRIDrawablePriv->refCount++;

    *surface_id = pDRIDrawablePriv->sid;

    if (ret_key != NULL) {
	ret_key[0] = pDRIDrawablePriv->key[0];
	ret_key[1] = pDRIDrawablePriv->key[1];
    }

    if (notify != NULL) {
	pDRIDrawablePriv->notifiers = x_hook_add (pDRIDrawablePriv->notifiers,
						  notify, notify_data);
    }

    return TRUE;
}

Bool
DRIDestroySurface(ScreenPtr pScreen, Drawable id, DrawablePtr pDrawable,
		  void (*notify) (void *, void *), void *notify_data)
{
    DRIDrawablePrivPtr	pDRIDrawablePriv;

    if (pDrawable->type == DRAWABLE_WINDOW) {
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW((WindowPtr)pDrawable);
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP((PixmapPtr)pDrawable);
    } else {
	return FALSE;
    }

    if (pDRIDrawablePriv != NULL) {
	if (notify != NULL) {
	    pDRIDrawablePriv->notifiers = x_hook_remove (pDRIDrawablePriv->notifiers,
							 notify, notify_data);
	}
	if (--pDRIDrawablePriv->refCount <= 0) {
	    /* This calls back to DRIDrawablePrivDelete
	       which frees the private area */
	    FreeResourceByType(id, DRIDrawablePrivResType, FALSE);
	}
    }

    return TRUE;
}

Bool
DRIDrawablePrivDelete(pointer pResource, XID id)
{
    DrawablePtr		pDrawable = (DrawablePtr)pResource;
    DRIScreenPrivPtr	pDRIPriv = DRI_SCREEN_PRIV(pDrawable->pScreen);
    DRIDrawablePrivPtr	pDRIDrawablePriv = NULL;
    WindowPtr		pWin = NULL;
    PixmapPtr		pPix = NULL;

    if (pDrawable->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr)pDrawable;
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);
	DRIStopDrawing (pDrawable, FALSE);
	pDRIPriv->drawing = x_list_remove (pDRIPriv->drawing,
					   pDRIDrawablePriv);
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
	pPix = (PixmapPtr)pDrawable;
	pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP(pPix);
    }

    if (pDRIDrawablePriv == NULL)
	return FALSE;

    if (pDRIDrawablePriv->drawableIndex != -1) {
	/* release drawable table entry */
	pDRIPriv->DRIDrawables[pDRIDrawablePriv->drawableIndex] = NULL;
    }

    if (pDRIDrawablePriv->sid != 0) {
	xp_destroy_surface (pDRIDrawablePriv->sid);
	x_hash_table_remove (surface_hash, (void *) pDRIDrawablePriv->sid);
    }

    if (pDRIDrawablePriv->notifiers != NULL)
	x_hook_free (pDRIDrawablePriv->notifiers);

    xfree(pDRIDrawablePriv);

    if (pDrawable->type == DRAWABLE_WINDOW) {
	pWin->devPrivates[DRIWindowPrivIndex].ptr = NULL;
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
	pPix->devPrivates[DRIPixmapPrivIndex].ptr = NULL;
    }

    --pDRIPriv->nrWindows;

    return TRUE;
}

void
DRIWindowExposures(WindowPtr pWin, RegionPtr prgn, RegionPtr bsreg)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);

    if(pDRIDrawablePriv) {
	/* FIXME: something? */
    }

    pScreen->WindowExposures = pDRIPriv->wrap.WindowExposures;

    (*pScreen->WindowExposures)(pWin, prgn, bsreg);

    pDRIPriv->wrap.WindowExposures = pScreen->WindowExposures;
    pScreen->WindowExposures = DRIWindowExposures;

}

void
DRICopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr pDRIDrawablePriv;

    if(pDRIPriv->nrWindows > 0) {
       pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW (pWin);
       if (pDRIDrawablePriv != NULL) {
	   DRIUpdateSurface (pDRIDrawablePriv, &pWin->drawable);
       }
    }

    /* unwrap */
    pScreen->CopyWindow = pDRIPriv->wrap.CopyWindow;

    /* call lower layers */
    (*pScreen->CopyWindow)(pWin, ptOldOrg, prgnSrc);

    /* rewrap */
    pDRIPriv->wrap.CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = DRICopyWindow;
}

int
DRIValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    ScreenPtr pScreen = pParent->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    int returnValue;

    /* unwrap */
    pScreen->ValidateTree = pDRIPriv->wrap.ValidateTree;

    /* call lower layers */
    returnValue = (*pScreen->ValidateTree)(pParent, pChild, kind);

    /* rewrap */
    pDRIPriv->wrap.ValidateTree = pScreen->ValidateTree;
    pScreen->ValidateTree = DRIValidateTree;

    return returnValue;
}

void
DRIPostValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    ScreenPtr pScreen;
    DRIScreenPrivPtr pDRIPriv;

    if (pParent) {
	pScreen = pParent->drawable.pScreen;
    } else {
	pScreen = pChild->drawable.pScreen;
    }
    pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv->wrap.PostValidateTree) {
	/* unwrap */
	pScreen->PostValidateTree = pDRIPriv->wrap.PostValidateTree;

	/* call lower layers */
	(*pScreen->PostValidateTree)(pParent, pChild, kind);

	/* rewrap */
	pDRIPriv->wrap.PostValidateTree = pScreen->PostValidateTree;
	pScreen->PostValidateTree = DRIPostValidateTree;
    }
}

void
DRIClipNotify(WindowPtr pWin, int dx, int dy)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr	pDRIDrawablePriv;

    if ((pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin))) {
	DRIUpdateSurface (pDRIDrawablePriv, &pWin->drawable);
    }

    if(pDRIPriv->wrap.ClipNotify) {
        pScreen->ClipNotify = pDRIPriv->wrap.ClipNotify;

        (*pScreen->ClipNotify)(pWin, dx, dy);

        pDRIPriv->wrap.ClipNotify = pScreen->ClipNotify;
        pScreen->ClipNotify = DRIClipNotify;
    }
}

/* This lets get at the unwrapped functions so that they can correctly
 * call the lowerlevel functions, and choose whether they will be
 * called at every level of recursion (eg in validatetree).
 */
DRIWrappedFuncsRec *
DRIGetWrappedFuncs(ScreenPtr pScreen)
{
    return &(DRI_SCREEN_PRIV(pScreen)->wrap);
}

void
DRIQueryVersion(int *majorVersion,
                int *minorVersion,
                int *patchVersion)
{
    *majorVersion = APPLE_DRI_MAJOR_VERSION;
    *minorVersion = APPLE_DRI_MINOR_VERSION;
    *patchVersion = APPLE_DRI_PATCH_VERSION;
}

void
DRISurfaceNotify (xp_surface_id id, int kind)
{
    DRIDrawablePrivPtr pDRIDrawablePriv = NULL;
    DRISurfaceNotifyArg arg;

    arg.id = id;
    arg.kind = kind;

    if (surface_hash != NULL)
    {
	pDRIDrawablePriv = x_hash_table_lookup (surface_hash,
						(void *) id, NULL);
    }

    if (pDRIDrawablePriv == NULL)
	return;

    if (kind == AppleDRISurfaceNotifyDestroyed)
    {
	pDRIDrawablePriv->sid = 0;
	x_hash_table_remove (surface_hash, (void *) id);
    }

    x_hook_run (pDRIDrawablePriv->notifiers, &arg);

    if (kind == AppleDRISurfaceNotifyDestroyed)
    {
	/* Kill off the handle. */

	FreeResourceByType (pDRIDrawablePriv->pDraw->id,
			    DRIDrawablePrivResType, FALSE);
    }
}


/* Experimental support for X drawing directly into VRAM surfaces. */

Bool
DRIStartDrawing (DrawablePtr pDraw)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1030
    return FALSE;
#else
    DRIDrawablePrivPtr priv = NULL;
    DRIScreenPrivPtr pDRIPriv;

    if (pDraw->type == DRAWABLE_WINDOW)
	priv = DRI_DRAWABLE_PRIV_FROM_WINDOW ((WindowPtr)pDraw);
#if 0
    /* FIXME: support pixmaps */
    else if (pDraw->type == DRAWABLE_PIXMAP)
	priv = DRI_DRAWABLE_PRIV_FROM_PIXMAP ((PixmapPtr)pDraw);
#endif

    if (priv == NULL)
	return FALSE;

    pDRIPriv = DRI_SCREEN_PRIV(priv->pScreen);

    if (!priv->is_drawing)
    {
	xp_error err;
	xp_box r;

	err = xp_lock_window (priv->sid, NULL, NULL,
			      &priv->data, &priv->rowbytes, &r);
	if (err != Success)
	    return FALSE;

	if (pDraw->type == DRAWABLE_WINDOW)
	{
	    WindowPtr pWin = (WindowPtr) pDraw;
	    int bw = wBorderWidth (pWin);

	    priv->pixmap =
            GetScratchPixmapHeader(pWin->drawable.pScreen, r.x2 - r.x1,
				   r.y2 - r.y1, pWin->drawable.depth,
                                   pWin->drawable.bitsPerPixel,
                                   priv->rowbytes, priv->data);
	    TranslatePixmapBase (priv->pixmap,
				 - (pWin->drawable.x - bw),
				 - (pWin->drawable.y - bw));
	}

        priv->is_drawing = TRUE;
	pDRIPriv->drawing = x_list_prepend (pDRIPriv->drawing, priv);
    }
    
    if (pDraw->type == DRAWABLE_WINDOW)
    {
	WindowPtr pWin = (WindowPtr) pDraw;

	priv->oldPixmap = pWin->drawable.pScreen->GetWindowPixmap (pWin);
	pWin->drawable.pScreen->SetWindowPixmap (pWin, priv->pixmap);
    }

    return TRUE;
#endif
}

Bool
DRIStopDrawing (DrawablePtr pDraw, Bool flush)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1030
    return FALSE;
#else
    DRIDrawablePrivPtr priv = NULL;
    ScreenPtr pScreen;
    DRIScreenPrivPtr pDRIPriv;

    if (pDraw->type == DRAWABLE_WINDOW)
	priv = DRI_DRAWABLE_PRIV_FROM_WINDOW ((WindowPtr)pDraw);
    else if (pDraw->type == DRAWABLE_PIXMAP)
	priv = DRI_DRAWABLE_PRIV_FROM_PIXMAP ((PixmapPtr)pDraw);

    if (priv == NULL)
	return FALSE;

    pScreen = priv->pScreen;
    pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (priv->is_drawing)
    {
	xp_unlock_window (priv->sid, flush);

	if (pDraw->type == DRAWABLE_WINDOW)
	{
	    FreeScratchPixmapHeader (priv->pixmap);
	    pScreen->SetWindowPixmap ((WindowPtr)pDraw, priv->oldPixmap);
	}

        priv->pixmap = NULL;

	/* If we didn't flush don't forget that we still need to.. */
	if (flush)
	    pDRIPriv->drawing = x_list_remove (pDRIPriv->drawing, priv);

        priv->is_drawing = FALSE;
    }
    else if (flush)
    {
	xp_flush_window (priv->sid);
    }

    return TRUE;
#endif /* 10.3 */
}

/*  true iff two Boxes overlap */
#define EXTENTCHECK(r1,r2) \
      (!( ((r1)->x2 <= (r2)->x1)  || \
          ((r1)->x1 >= (r2)->x2)  || \
          ((r1)->y2 <= (r2)->y1)  || \
          ((r1)->y1 >= (r2)->y2) ) )

Bool
DRIDamageRegion (DrawablePtr pDraw, RegionPtr pRegion)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1030
    return FALSE;
#else
    DRIDrawablePrivPtr priv = NULL;
    RegionRec clipped;
    BoxPtr b1, b2;

    if (pDraw->type == DRAWABLE_WINDOW)
	priv = DRI_DRAWABLE_PRIV_FROM_WINDOW ((WindowPtr)pDraw);

    if (priv == NULL)
	return FALSE;

    /* adapted from RootlessDamageRegion () */

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	WindowPtr pWin = (WindowPtr) pDraw;

	b1 = REGION_EXTENTS (pWin->drawable.pScreen, &pWin->borderClip);
	b2 = REGION_EXTENTS (pWin->drawable.pScreen, pRegion);

	if (EXTENTCHECK (b1, b2))
	{
	    /* Regions may overlap. */

	    if (REGION_NUM_RECTS (pRegion) == 1)
	    {
		int in;

		/* Damaged region only has a single rect, so we can
		   just compare that against the region */

		in = RECT_IN_REGION (pWin->drawable.pScreen, &pWin->borderClip,
				     REGION_RECTS (pRegion));
		if (in == rgnIN)
		{
		    /* clip totally contains pRegion */

		    xp_mark_window (priv->sid, REGION_NUM_RECTS (pRegion),
				    REGION_RECTS (pRegion),
				    -priv->x, -priv->y);

		    RootlessQueueRedisplay (pWin->drawable.pScreen);
		    goto out;
		}
		else if (in == rgnOUT)
		{
		    /* clip doesn't contain pRegion */

		    goto out;
		}
	    }

	    /* clip overlaps pRegion, need to intersect */

	    REGION_INIT (pWin->drawable.pScreen, &clipped, NullBox, 0);
	    REGION_INTERSECT (pWin->drawable.pScreen, &clipped,
			      &pWin->borderClip, pRegion);

	    xp_mark_window (priv->sid, REGION_NUM_RECTS (&clipped),
			    REGION_RECTS (&clipped), -priv->x, -priv->y);

	    REGION_UNINIT (pWin->drawable.pScreen, &clipped);

	    RootlessQueueRedisplay (pWin->drawable.pScreen);
	}
    }

out:
    return TRUE;
#endif /* 10.3 */
}

void
DRISynchronizeDrawable (DrawablePtr pDraw, Bool flush)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    ScreenPtr pScreen;
    DRIScreenPrivPtr pDRIPriv;
    DRIDrawablePrivPtr priv;
    x_list *node, *copy;

    pScreen = pDraw->pScreen;
    pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv == NULL || pDRIPriv->drawing == NULL)
	return;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	WindowPtr pWin = TopLevelParent ((WindowPtr) pDraw);

	/* need to find _any_ window under pWin that is drawing. Scan the
	   list looking for candidates. */

	copy = x_list_copy (pDRIPriv->drawing);

	for (node = copy; node != NULL; node = node->next)
	{
	    priv = node->data;

	    if (priv->pDraw->type == DRAWABLE_WINDOW
		&& TopLevelParent ((WindowPtr) priv->pDraw) == pWin)
	    {
		DRIStopDrawing (priv->pDraw, flush);
	    }
	}

	x_list_free (copy);
    }
    else
    {
	DRIStopDrawing (pDraw, flush);
    }
#endif /* 10.3 */
}

void
DRISynchronize (Bool flush)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    int i;
    ScreenPtr pScreen;
    DRIScreenPrivPtr pDRIPriv;
    DRIDrawablePrivPtr priv;
    x_list *node, *copy;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	pDRIPriv = DRI_SCREEN_PRIV(pScreen);

	if (pDRIPriv == NULL || pDRIPriv->drawing == NULL)
	    continue;

	copy = x_list_copy (pDRIPriv->drawing);

	for (node = copy; node != NULL; node = node->next)
	{
	    priv = node->data;

	    DRIStopDrawing (priv->pDraw, flush);
	}

	x_list_free (copy);
    }
#endif /* 10.3 */
}
