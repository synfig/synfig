/**************************************************************
 *
 * Quartz-specific support for the Darwin X Server
 *
 **************************************************************/
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
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/quartz.c,v 1.1 2002/03/28 02:21:18 torrey Exp $ */

#include "quartz.h"
#include "darwin.h"
#include "quartz-audio.h"
#include "quartz-cursor.h"
#include "rootless.h"
#include "rootless-window.h"
#include "pseudoramiX.h"
#include "globals.h"
#include "dri.h"
#define _APPLEWM_SERVER_
#include "applewmstr.h"
#include "X11Application.h"

#include "scrnintstr.h"
#include "colormapst.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include <AvailabilityMacros.h>
#include <CoreGraphics/CoreGraphics.h>

/* Shared global variables for Quartz modes */
int quartzEventWriteFD = -1;
int quartzUseSysBeep = 1;
int quartzServerVisible = FALSE;
int quartzDesiredDepth = -1;
int quartzHasRoot = FALSE, quartzEnableRootless = TRUE;
int quartzFullscreenDisableHotkeys = TRUE;
int noPseudoramiXExtension = FALSE;
int quartzXpluginOptions = 0;

extern char *display;

static CGDirectDisplayID
display_at_index (int index)
{
    CGError err;
    CGDisplayCount cnt;
    CGDirectDisplayID dpy[index+1];

    err = CGGetActiveDisplayList (index + 1, dpy, &cnt);
    if (err == kCGErrorSuccess && (int) cnt == index + 1)
	return dpy[index];
    else
	return kCGNullDirectDisplay;
}

static CGRect
display_screen_bounds (CGDirectDisplayID id, Bool remove_menubar)
{
    CGRect frame;

    frame = CGDisplayBounds (id);

    if (remove_menubar && !quartzHasRoot
	&& frame.origin.x == 0 && frame.origin.y == 0)
    {
	/* Remove Aqua menubar from display bounds. */

	frame.origin.y += 22;
	frame.size.height -= 22;
    }

    return frame;
}

static void
addPseudoramiXScreens (int *x, int *y, int *width, int *height)
{
    CGDisplayCount i, total = 16;	/* FIXME: hardcoded maximum */
    CGRect unionRect = CGRectNull, frame;
    CGDirectDisplayID screens[total];
	
    CGGetActiveDisplayList (total, screens, &total);

    /* Get the union of all screens */
    for (i = 0; i < total; i++)
    {
	CGDirectDisplayID dpy = screens[i];

	/* we can't remove the menubar from the screen - doing so
	   would constrain the pointer to the screen, not allowing it
	   to reach the menubar.. */

	frame = display_screen_bounds (dpy, FALSE);
	unionRect = CGRectUnion (unionRect, frame);
    }

    /* Use unionRect as the screen size for the X server. */
    *x = unionRect.origin.x;
    *y = unionRect.origin.y;
    *width = unionRect.size.width;
    *height = unionRect.size.height;

    /* Tell PseudoramiX about the real screens. */
    for (i = 0; i < total; i++)
    {
	CGDirectDisplayID dpy = screens[i];

	frame = display_screen_bounds (dpy, TRUE);

#ifdef DEBUG
	ErrorF("PseudoramiX screen %d added: %dx%d @ (%d,%d).\n", i,
	       (int)frame.size.width, (int)frame.size.height,
	       (int)frame.origin.x, (int)frame.origin.y);
#endif

	frame.origin.x -= unionRect.origin.x;
	frame.origin.y -= unionRect.origin.y;

#ifdef DEBUG
	ErrorF("PseudoramiX screen %d placed at X11 coordinate (%d,%d).\n",
	       i, (int)frame.origin.x, (int)frame.origin.y);
#endif

	PseudoramiXAddScreen(frame.origin.x, frame.origin.y,
			     frame.size.width, frame.size.height);
    }
}

/* Do mode dependent initialization of each screen for Quartz. */
Bool
QuartzAddScreen (int index, ScreenPtr pScreen)
{
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);

    /* If no specific depth chosen, look for the depth of the main display.
       Else if 16bpp specified, use that. Else use 32bpp. */

    dfb->componentCount = 3;
    dfb->bitsPerComponent = 8;
    dfb->bitsPerPixel = 32;

    if (quartzDesiredDepth == -1)
    {
	dfb->bitsPerComponent = CGDisplayBitsPerSample (kCGDirectMainDisplay);
	dfb->bitsPerPixel = CGDisplayBitsPerPixel (kCGDirectMainDisplay);
    }
    else if (quartzDesiredDepth == 15)
    {
	dfb->bitsPerComponent = 5;
	dfb->bitsPerPixel = 16;
    }
    else if (quartzDesiredDepth == 8)
    {
	dfb->bitsPerComponent = 8;
	dfb->bitsPerPixel = 8;
	dfb->componentCount = 1;
    }

    if (noPseudoramiXExtension)
    {
        CGDirectDisplayID dpy;
        CGRect frame;

	dpy = display_at_index (index);

	frame = display_screen_bounds (dpy, TRUE);

        dfb->x = frame.origin.x;
        dfb->y = frame.origin.y;
        dfb->width =  frame.size.width;
        dfb->height = frame.size.height;
    }
    else
    {
	addPseudoramiXScreens (&dfb->x, &dfb->y, &dfb->width, &dfb->height);
    }

    dfb->colorBitsPerPixel = dfb->bitsPerComponent * dfb->componentCount;

    /* Passing zero width (pitch) makes miCreateScreenResources set the
       screen pixmap to the framebuffer pointer, i.e. null. We'll take
       it from there.. */
    dfb->pitch = 0;
    dfb->framebuffer = NULL;

    DRIScreenInit (pScreen);

    return TRUE;
}

/* Finalize mode specific setup of each screen. */
Bool
QuartzSetupScreen (int index, ScreenPtr pScreen)
{
    // do full screen or rootless specific setup
    if (! RootlessSetupScreen(index, pScreen))
	return FALSE;

    // setup cursor support
    if (! QuartzInitCursor(pScreen))
        return FALSE;

    DRIFinishScreenInit (pScreen);

    return TRUE;
}


/* Quartz display initialization. */
void
QuartzInitOutput (int argc, char **argv)
{
    static int orig_noPanoramiXExtension;
    int total;

    if (serverGeneration == 1) {
	orig_noPanoramiXExtension = noPanoramiXExtension;
        QuartzAudioInit();
    }

    /* +xinerama option sets noPanoramiXExtension variable */
    noPseudoramiXExtension = orig_noPanoramiXExtension;

    total = 16;				/* FIXME: hardcoded maximum */
    if (total > 0)
    {
	CGDirectDisplayID screens[total];
	CGGetActiveDisplayList (total, screens, &total);
    }

    if (noPseudoramiXExtension)
	darwinScreensFound = total;
    else
        darwinScreensFound =  1; // only PseudoramiX knows about the rest

    if (!quartzEnableRootless)
	RootlessHideAllWindows ();
}

/* This function from randr.c */
extern char	*ConnectionInfo;
static int padlength[4] = {0, 3, 2, 1};
static void
RREditConnectionInfo (ScreenPtr pScreen)
{
    xConnSetup	    *connSetup;
    char	    *vendor;
    xPixmapFormat   *formats;
    xWindowRoot	    *root;
    xDepth	    *depth;
    xVisualType	    *visual;
    int		    screen = 0;
    int		    d;

    connSetup = (xConnSetup *) ConnectionInfo;
    vendor = (char *) connSetup + sizeof (xConnSetup);
    formats = (xPixmapFormat *) ((char *) vendor +
				 connSetup->nbytesVendor +
				 padlength[connSetup->nbytesVendor & 3]);
    root = (xWindowRoot *) ((char *) formats +
			    sizeof (xPixmapFormat) * screenInfo.numPixmapFormats);
    while (screen != pScreen->myNum)
    {
	depth = (xDepth *) ((char *) root + 
			    sizeof (xWindowRoot));
	for (d = 0; d < root->nDepths; d++)
	{
	    visual = (xVisualType *) ((char *) depth +
				      sizeof (xDepth));
	    depth = (xDepth *) ((char *) visual +
				depth->nVisuals * sizeof (xVisualType));
	}
	root = (xWindowRoot *) ((char *) depth);
	screen++;
    }
    root->pixWidth = pScreen->width;
    root->pixHeight = pScreen->height;
    root->mmWidth = pScreen->mmWidth;
    root->mmHeight = pScreen->mmHeight;
}

static void
QuartzUpdateScreens (void)
{
    ScreenPtr pScreen;
    WindowPtr pRoot;
    int x, y, width, height, sx, sy;
    xEvent e;

    if (noPseudoramiXExtension || screenInfo.numScreens != 1)
    {
	/* FIXME: if not using Xinerama, we have multiple screens, and
	   to do this properly may need to add or remove screens. Which
	   isn't possible. So don't do anything. Another reason why
	   we default to running with Xinerama. */

	return;
    }

    pScreen = screenInfo.screens[0];

    PseudoramiXResetScreens ();
    addPseudoramiXScreens (&x, &y, &width, &height);

    dixScreenOrigins[pScreen->myNum].x = x;
    dixScreenOrigins[pScreen->myNum].y = y;
    pScreen->mmWidth = pScreen->mmWidth * ((double) width / pScreen->width);
    pScreen->mmHeight = pScreen->mmHeight * ((double) height / pScreen->height);
    pScreen->width = width;
    pScreen->height = height;

    /* FIXME: should probably do something with RandR here. */

    DarwinAdjustScreenOrigins (&screenInfo);
    RootlessRepositionWindows (screenInfo.screens[0]);
    RootlessUpdateScreenPixmap (screenInfo.screens[0]);

    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

    /* Adjust the root window. */

    pRoot = WindowTable[pScreen->myNum];
    pScreen->ResizeWindow (pRoot, x - sx, y - sy, width, height, NULL);
    pScreen->PaintWindowBackground (pRoot, &pRoot->borderClip,  PW_BACKGROUND);
    QuartzIgnoreNextWarpCursor ();
    DefineInitialRootWindow (pRoot);

    /* Send an event for the root reconfigure */

    e.u.u.type = ConfigureNotify;
    e.u.configureNotify.window = pRoot->drawable.id;
    e.u.configureNotify.aboveSibling = None;
    e.u.configureNotify.x = x - sx;
    e.u.configureNotify.y = y - sy;
    e.u.configureNotify.width = width;
    e.u.configureNotify.height = height;
    e.u.configureNotify.borderWidth = wBorderWidth (pRoot);
    e.u.configureNotify.override = pRoot->overrideRedirect;
    DeliverEvents (pRoot, &e, 1, NullWindow);

    /* FIXME: what does this do? */
    RREditConnectionInfo (pScreen);
}

static void
do_exec (void (*callback) (void *data), void *data)
{
    /* Do the fork-twice trick to avoid needing to reap zombies */

    int child1, child2 = 0;
    int status;

    /* we should really try to report errors here.. */

    child1 = fork ();

    switch (child1)
    {
    case -1:				/* error */
	break;

    case 0:				/* child1 */
	child2 = fork ();

	switch (child2)
	{
	    int max_files, i;
	    char buf[1024], *tem;

	case -1:			/* error */
	    _exit (1);

	case 0:				/* child2 */
	    /* close all open files except for standard streams */
	    max_files = sysconf (_SC_OPEN_MAX);
	    for (i = 3; i < max_files; i++)
		close (i);

	    /* ensure stdin is on /dev/null */
	    close (0);
	    open ("/dev/null", O_RDONLY);

	    /* cd $HOME */
	    tem = getenv ("HOME");
	    if (tem != NULL)
		chdir (tem);

	    /* Setup environment */
	    snprintf (buf, sizeof (buf), ":%s", display);
	    setenv ("DISPLAY", buf, TRUE);
	    tem = getenv ("PATH");
	    if (tem != NULL && tem[0] != NULL)
		snprintf (buf, sizeof (buf), "%s:/usr/X11R6/bin", tem);
	    else
		snprintf (buf, sizeof (buf), "/bin:/usr/bin:/usr/X11R6/bin");
	    setenv ("PATH", buf, TRUE);

	    (*callback) (data);

	    _exit (2);

	default:			/* parent (child1) */
	    _exit (0);
	}
	break;

    default:				/* parent */
	waitpid (child1, &status, 0);
    }
}

static void
run_client_callback (void *data)
{
    char **argv = data;
    execvp (argv[0], argv);
}

/* Note that this function is called from both X server and appkit threads */
void
QuartzRunClient (const char *command)
{
    const char *shell;
    const char *argv[5];

    shell = getenv ("SHELL");
    if (shell == NULL)
	shell = "/bin/bash";

    /* At least [ba]sh, [t]csh and zsh all work with this syntax. We
       need to use an interactive shell to force it to load the user's
       environment. Otherwise things like fink don't work at all well.. */

    argv[0] = shell;
    argv[1] = "-i";
    argv[2] = "-c";
    argv[3] = command;
    argv[4] = NULL;

    do_exec (run_client_callback, argv);
}

static void
QuartzSetFullscreen (Bool state)
{
    if (quartzHasRoot == state)
	return;

    quartzHasRoot = state;

    xp_disable_update ();

    if (!quartzHasRoot && !quartzEnableRootless)
	RootlessHideAllWindows ();

    RootlessUpdateRooted (quartzHasRoot);

    if (quartzHasRoot && !quartzEnableRootless)
	RootlessShowAllWindows ();

    /* Only update screen info when something is visible. Avoids the wm
       moving the windows out from under the menubar when it shouldn't */

    if (quartzHasRoot || quartzEnableRootless)
	QuartzUpdateScreens ();

    /* Somehow the menubar manages to interfere with our event stream
       in fullscreen mode, even though it's not visible. */

    X11ApplicationShowHideMenubar (!quartzHasRoot);

    xp_reenable_update ();

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    if (quartzFullscreenDisableHotkeys)
	xp_disable_hot_keys (quartzHasRoot);
#endif
}

static void
QuartzSetRootless (Bool state)
{
    if (quartzEnableRootless == state)
	return;

    quartzEnableRootless = state;

    if (!quartzEnableRootless && !quartzHasRoot)
    {
	xp_disable_update ();
	RootlessHideAllWindows ();
	xp_reenable_update ();
    }
    else if (quartzEnableRootless && !quartzHasRoot)
    {
	xp_disable_update ();
	RootlessShowAllWindows ();
	QuartzUpdateScreens ();
	xp_reenable_update ();
    }
}

/* Show the X server on screen. Does nothing if already shown.  Restore the
   X clip regions the X server cursor state. */
static void
QuartzShow (void)
{
    int i;

    if (quartzServerVisible)
	return;

    quartzServerVisible = TRUE;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	if (screenInfo.screens[i])
	    QuartzResumeXCursor(screenInfo.screens[i]);
    }

    /* FIXME: not sure about this, it may need to have a preference like
       in XDarwin..? */

    if (!quartzEnableRootless)
	QuartzSetFullscreen (TRUE);
}

/* Remove the X server display from the screen. Does nothing if already
   hidden. Set X clip regions to prevent drawing, and restore the Aqua
   cursor. */
static void
QuartzHide (void)
{
    int i;

    if (!quartzServerVisible)
	return;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	if (screenInfo.screens[i])
	    QuartzSuspendXCursor(screenInfo.screens[i]);
    }

    QuartzSetFullscreen (FALSE);

    quartzServerVisible = FALSE;
}

/* Cleanup before X server shutdown. Release the screen and restore the
   Aqua cursor. */
void
QuartzGiveUp (void)
{
    int i;

    for (i = 0; i < screenInfo.numScreens; i++) {
        if (screenInfo.screens[i]) {
            QuartzSuspendXCursor(screenInfo.screens[i]);
        }
    }
}

int
QuartzProcessArgument( int argc, char *argv[], int i )
{
    /* This arg is passed when launched from the Aqua GUI. */
    if (strncmp (argv[i], "-psn_", 5) == 0)
    {
        return 1;
    }

    if (strcmp (argv[i], "-depth") == 0)
    {
	int arg;

	if (i == argc - 1)
	    FatalError ("-depth requires an argument\n");

	arg = atoi (argv[i + 1]);
	if (arg == 8 || arg == 15 || arg == 24)
	    quartzDesiredDepth = arg;
	else
	    FatalError ("Only 8, 15 and 24 bit color depths are supported.\n");

	return 2;
    }

    return 0;
}

void
QuartzClientMessage (const xEvent *xe)
{
    switch (xe->u.clientMessage.u.l.type)
    {
    case kXquartzControllerNotify:
	AppleWMSendEvent (AppleWMControllerNotify,
			  AppleWMControllerNotifyMask,
			  xe->u.clientMessage.u.l.longs0,
			  xe->u.clientMessage.u.l.longs1);
	break;

    case kXquartzPasteboardNotify:
	AppleWMSendEvent (AppleWMPasteboardNotify,
			  AppleWMPasteboardNotifyMask,
			  xe->u.clientMessage.u.l.longs0,
			  xe->u.clientMessage.u.l.longs1);
	break;

    case kXquartzActivate:
	QuartzShow ();
	AppleWMSendEvent (AppleWMActivationNotify,
			  AppleWMActivationNotifyMask,
			  AppleWMIsActive, 0);
	break;

    case kXquartzDeactivate:
	AppleWMSendEvent (AppleWMActivationNotify,
			  AppleWMActivationNotifyMask,
			  AppleWMIsInactive, 0);
	QuartzHide ();
	break;

    case kXquartzDisplayChanged:
	QuartzUpdateScreens ();
	break;

    case kXquartzWindowState:
	RootlessNativeWindowStateChanged (xe->u.clientMessage.u.l.longs0,
					  xe->u.clientMessage.u.l.longs1);
	break;

    case kXquartzWindowMoved:
	RootlessNativeWindowMoved (xe->u.clientMessage.u.l.longs0);
	break;

    case kXquartzToggleFullscreen:
	if (quartzEnableRootless)
	    QuartzSetFullscreen (!quartzHasRoot);
	else if (quartzHasRoot)
	    QuartzHide ();
	else
	    QuartzShow ();
	break;

    case kXquartzSetRootless:
	QuartzSetRootless (xe->u.clientMessage.u.l.longs0);
	if (!quartzEnableRootless && !quartzHasRoot)
	    QuartzHide ();
    }
}
