/*
 * External interface to generic rootless mode
 *
 * Greg Parker     gparker@cs.stanford.edu     March 3, 2001
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

/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/rootless.h,v 1.2 2002/08/28 06:41:26 torrey Exp $ */

#ifndef _ROOTLESS_H
#define _ROOTLESS_H

#include "mi.h"
#include "gcstruct.h"

#include "Xplugin.h"

/* Initialize rootless mode on the given screen. */
extern Bool RootlessSetupScreen(int index, ScreenPtr pScreen);

extern void RootlessUpdateScreenPixmap (ScreenPtr pScreen);

extern xp_window_id RootlessGetPhysicalWindow (WindowPtr pWin, Bool create);
extern WindowPtr RootlessGetXWindow (xp_window_id wid);

extern int RootlessKnowsWindowNumber (int number);

extern void RootlessDisableUpdate (WindowPtr pWin);
extern void RootlessReenableUpdate (WindowPtr pWin);

extern void RootlessUpdateRooted (Bool state);
extern void RootlessHideAllWindows (void);
extern void RootlessShowAllWindows (void);

#endif /* _ROOTLESS_H */
