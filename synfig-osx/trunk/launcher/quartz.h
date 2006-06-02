/*
 * quartz.h
 *
 * External interface of the Quartz modes seen by the generic, mode
 * independent parts of the Darwin X server.
 */
/*
 * Copyright (c) 2001 Greg Parker and Torrey T. Lyons.
 * Copyright (c) 2002 Apple Computer, Inc.
 *                 All Rights Reserved.
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
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/quartz.h,v 1.1 2002/03/28 02:21:18 torrey Exp $ */

#ifndef _QUARTZ_H
#define _QUARTZ_H

#include "darwin.h"

/* Local client message types */
enum {
  kXquartzControllerNotify = kXquartzFirstEvent,
  kXquartzPasteboardNotify,
  kXquartzActivate,
  kXquartzDeactivate,
  kXquartzDisplayChanged,
  kXquartzWindowState,
  kXquartzWindowMoved,
  kXquartzToggleFullscreen,
  kXquartzSetRootless,
};

extern int quartzUseSysBeep;
extern int quartzServerVisible;
extern int quartzDesiredDepth;
extern int quartzHasRoot, quartzEnableRootless;
extern int quartzFullscreenDisableHotkeys;
extern int quartzXpluginOptions;

int QuartzProcessArgument(int argc, char *argv[], int i);
void QuartzInitOutput(int argc, char **argv);
void QuartzGiveUp(void);

void QuartzMessageMainThread (int type, int argc, ...);
void QuartzClientMessage (const struct _xEvent *xe);

void QuartzRunClient (const char *command);

Bool QuartzAddScreen(int index, ScreenPtr pScreen);
Bool QuartzSetupScreen(int index, ScreenPtr pScreen);

#endif
