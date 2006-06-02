/* $XFree86: xc/programs/Xserver/GL/dri/dri.h,v 1.18 2001/03/21 16:21:40 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
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
 *   Jens Owen <jens@precisioninsight.com>
 *
 */

/* Prototypes for DRI functions */

#ifndef _DRI_SURFACE_H_
#define _DRI_SURFACE_H_

#include "dri.h"
#include "Xplugin.h"

typedef void (*ClipNotifyPtr)( WindowPtr, int, int );


/*
 * These functions can be wrapped by the DRI.  Each of these have
 * generic default funcs (initialized in DRICreateInfoRec) and can be
 * overridden by the driver in its [driver]DRIScreenInit function.
 */
typedef struct {
    WindowExposuresProcPtr       WindowExposures;
    CopyWindowProcPtr            CopyWindow;
    ValidateTreeProcPtr          ValidateTree;
    PostValidateTreeProcPtr      PostValidateTree;
    ClipNotifyProcPtr            ClipNotify;
} DRIWrappedFuncsRec, *DRIWrappedFuncsPtr;

typedef struct {
    xp_surface_id id;
    int kind;
} DRISurfaceNotifyArg;

extern void DRIClipNotify(WindowPtr pWin,
                          int dx,
                          int dy);

extern void DRICloseScreen(ScreenPtr pScreen);

extern void DRICopyWindow(WindowPtr pWin,
                          DDXPointRec ptOldOrg,
                          RegionPtr prgnSrc);

extern Bool DRICreateSurface(ScreenPtr pScreen,
			     Drawable id,
			     DrawablePtr pDrawable,
			     xp_client_id client_id,
			     xp_surface_id *surface_id,
			     unsigned int key[2],
			     void (*notify) (void *arg, void *data),
			     void *notify_data);

extern Bool DRIDestroySurface(ScreenPtr pScreen, 
			      Drawable id,
			      DrawablePtr pDrawable,
			      void (*notify) (void *arg, void *data),
			      void *notify_data);

extern Bool DRIDrawablePrivDelete(pointer pResource,
                                  XID id);

extern DRIWrappedFuncsRec *DRIGetWrappedFuncs(ScreenPtr pScreen);

extern void DRIPostValidateTree(WindowPtr pParent,
                                WindowPtr pChild,
                                VTKind    kind);

extern int DRIValidateTree(WindowPtr pParent,
                           WindowPtr pChild,
                           VTKind    kind);

extern void DRIWindowExposures(WindowPtr pWin,
                              RegionPtr prgn,
                              RegionPtr bsreg);

extern void DRISurfaceNotify (xp_surface_id id, int kind);

extern Bool DRIStartDrawing (DrawablePtr pDraw);
extern Bool DRIStopDrawing (DrawablePtr pDraw, Bool flush);
extern Bool DRIDamageRegion (DrawablePtr pDraw, RegionPtr pRegion);
extern void DRISynchronizeDrawable (DrawablePtr pDraw, Bool flush);
extern void DRISynchronize (Bool flush);

#endif
