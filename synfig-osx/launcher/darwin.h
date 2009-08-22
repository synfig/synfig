/*
 * Copyright (c) 2001 Torrey T. Lyons. All Rights Reserved.
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
/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.h,v 1.11 2002/03/28 02:21:08 torrey Exp $ */

#ifndef _DARWIN_H
#define _DARWIN_H

#include "inputstr.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "extensions/XKB.h"

typedef struct {
    void                *framebuffer;
    int                 x;
    int                 y;
    int                 width;
    int                 height;
    int                 pitch;
    int                 bitsPerPixel;
    int			componentCount;
    int			bitsPerComponent;
    int                 colorBitsPerPixel;
    unsigned int	redMask, greenMask, blueMask;
} DarwinFramebufferRec, *DarwinFramebufferPtr;

/* From darwinKeyboard.c */
int DarwinModifierNXKeyToNXKeycode(int key, int side);
void DarwinKeyboardInit(DeviceIntPtr pDev);
void DarwinKeyboardReload (DeviceIntPtr pDev);

/* from darwin-new-keymap.c */
unsigned int DarwinSystemKeymapSeed (void);

/* from darwin-input.c */
void DarwinEnqueueEvent (const xEvent *e);
Bool DarwinDequeueEvent (xEvent *e);
void DarwinInputInit (void);
void DarwinInputPreInit (void);
int DarwinMouseProc(DeviceIntPtr pPointer, int what);
int DarwinKeybdProc(DeviceIntPtr pDev, int onoff);

#undef assert
#define assert(x) { if ((x) == 0) \
    FatalError("assert failed on line %d of %s!\n", __LINE__, __FILE__); }
#define kern_assert(x) { if ((x) != KERN_SUCCESS) \
    FatalError("assert failed on line %d of %s with kernel return 0x%x!\n", \
                __LINE__, __FILE__, x); }
#define SCREEN_PRIV(pScreen) \
    ((DarwinFramebufferPtr)pScreen->devPrivates[darwinScreenIndex].ptr)

#define MIN_KEYCODE XkbMinLegalKeyCode     // unfortunately, this isn't 0...

extern void DarwinEnqueuePointerEvent (xEvent *xe);
extern void DarwinAdjustScreenOrigins (ScreenInfo *pScreenInfo);

/* Global variables from darwin.c */
extern int darwinScreenIndex; // index into pScreen.devPrivates
extern int darwinScreensFound;

/* User preferences used by generic Darwin X server code */
extern Bool darwinSwapAltMeta;
extern int darwinFakeButtons;
extern char *darwinKeymapFile;
extern Bool darwinSyncKeymap;

/* location of X11's (0,0) point in global screen coordinates */
extern int darwinMainScreenX;
extern int darwinMainScreenY;

/* Client message event type ranges. */
enum {
    kXdarwinFirstEvent = 1,
    kXdarwinLastEvent = kXdarwinFirstEvent + 99,
    kXquartzFirstEvent = kXdarwinLastEvent + 1,
    kXquartzLastEvent = kXquartzFirstEvent + 99,
};

/* "Darwin" client message types */
enum {
    kXdarwinQuit = kXdarwinFirstEvent,
};

extern char *DarwinFindLibraryFile (const char *file, const char *pathext);
extern void DarwinForeachLibraryFile (const char *dir,
				      void (*callback) (const char *dir,
							const char *file,
							void *data),
				      void *data);

extern void DarwinSetFakeButtons (const char *mod2, const char *mod3);

#endif	/* _DARWIN_H */
