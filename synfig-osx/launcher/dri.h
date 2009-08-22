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

#ifndef _DRI_H_

#include "Xdefs.h"
#include "screenint.h"
#define _APPLEDRI_SERVER_
#include "appledri.h"

extern Bool DRIAuthConnection(ScreenPtr pScreen, unsigned int magic);

extern Bool DRIExtensionInit(void);

extern Bool DRIFinishScreenInit(ScreenPtr pScreen);

extern Bool DRIQueryDirectRenderingCapable(ScreenPtr pScreen,
                                           Bool *isCapable);

extern void DRIQueryVersion(int *majorVersion,
                            int *minorVersion,
                            int *patchVersion);

extern void DRIReset(void);

extern Bool DRIScreenInit(ScreenPtr pScreen);

#define _DRI_H_

#endif
