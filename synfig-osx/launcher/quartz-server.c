//
//  quartzServer.c
//
//  This class handles the interaction between CG and the X server thread
//
/*
 * Copyright (c) 2001 Andreas Monitzer. All Rights Reserved.
 * Copyright (c) 2002 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Apple Software, Inc. All Rights Reserved.
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/XServer.m,v 1.3 2002/07/15 18:57:44 torrey Exp $ */

#include "quartz.h"
#include "Xplugin.h"
#include "X11Controller.h"
#define _APPLEDRI_SERVER_
#include "appledri.h"
#include "dri-surface.h"
#include "opaque.h"
#include "globals.h"
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

char **envpGlobal;      // argcGlobal and argvGlobal
                        // are from dix/globals.c

void
QuartzMessageMainThread (int type, int argc, ...)
{
    xEvent xe;
    INT32 *argv;
    int i, max_args;
    va_list args;

    memset (&xe, 0, sizeof (xe));
    xe.u.u.type = ClientMessage;
    xe.u.clientMessage.u.l.type = type;

    argv = &xe.u.clientMessage.u.l.longs0;
    max_args = 4;

    if (argc > 0 && argc <= max_args)
    {
	va_start (args, argc);
	for (i = 0; i < argc; i++)
	    argv[i] = (int) va_arg (args, int);
	va_end (args);
    }

    DarwinEnqueueEvent (&xe);
}

static void
event_handler (unsigned int type, const void *arg,
	       unsigned int arg_size, void *data)
{
    switch (type)
    {
    case XP_EVENT_DISPLAY_CHANGED:
	QuartzMessageMainThread (kXquartzDisplayChanged, 0);
	break;

    case XP_EVENT_WINDOW_STATE_CHANGED:
	if (arg_size >= sizeof (xp_window_state_event))
	{
	    const xp_window_state_event *ws_arg = arg;
	    QuartzMessageMainThread (kXquartzWindowState, 2,
				     ws_arg->id, ws_arg->state);
	}
	break;

    case XP_EVENT_WINDOW_MOVED:
	if (arg_size == sizeof (xp_window_id))
	{
	    xp_window_id id = * (xp_window_id *) arg;

	    QuartzMessageMainThread (kXquartzWindowMoved, 1, id);
	}
	break;

    case XP_EVENT_SURFACE_DESTROYED:
    case XP_EVENT_SURFACE_CHANGED:
	if (arg_size == sizeof (xp_surface_id))
	{
	    int kind;

	    if (type == XP_EVENT_SURFACE_DESTROYED)
		kind = AppleDRISurfaceNotifyDestroyed;
	    else
		kind = AppleDRISurfaceNotifyChanged;

	    DRISurfaceNotify (*(xp_surface_id *) arg, kind);
	}
	break;
    }
}

static void
server_thread (void *arg)
{
    extern int main (int argc, char **argv, char **envp);

    /* Xinerama defaults to enabled */
    noPanoramiXExtension = FALSE;

    if (xp_init (XP_BACKGROUND_EVENTS | quartzXpluginOptions) != Success)
    {
	fprintf (stderr, "can't initialize window system\n");
	exit (1);
    }

    xp_select_events (XP_EVENT_DISPLAY_CHANGED
		      | XP_EVENT_WINDOW_STATE_CHANGED
		      | XP_EVENT_WINDOW_MOVED
		      | XP_EVENT_SURFACE_CHANGED
		      | XP_EVENT_SURFACE_DESTROYED,
		      event_handler, NULL);

    exit (main (argcGlobal, argvGlobal, envpGlobal));
}

/*
 * DarwinHandleGUI
 *  This function is called first from main(). We use it to connect to
 *  the cg window server and spawn the the thread that will listen for
 *  cg events
 */
void DarwinHandleGUI(
    int         argc,
    char        *argv[],
    char        *envp[] )
{
    static Bool here_before;

    extern void _InitHLTB (void);

    if (here_before)
	return;

    here_before = TRUE;

    DarwinInputPreInit ();

    // Store command line arguments to pass back to main()
    argcGlobal = argc;
    argvGlobal = argv;
    envpGlobal = envp;

    /* Initially I ran the X server on the main thread, and received
       events on the second thread. But now we may be using Carbon,
       that needs to run on the main thread. (Otherwise, when it's
       prebound, it will initialize itself on the wrong thread)

       grr.. but doing that means that if the X thread gets scheduled
       before the main thread when we're _not_ prebound, things fail,
       so initialize by hand. */

    _InitHLTB ();

    X11ControllerMain (argc, argv, server_thread, NULL);

    /* not reached */
    exit (1);
}
