/* darwin-input.c -- code to manage the input event queue
   $Id: darwin-input.c,v 1.4 2002/12/13 00:22:51 jharper Exp $ */

/*
 * Copyright (c) 2001-2002 Torrey T. Lyons. All Rights Reserved.
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

#include "darwin.h"

#include "mipointer.h"		// mi software cursor

#include <pthread.h>

#define QUEUE_SIZE 256

static struct {
    pthread_mutex_t mutex;

    /* DIX looks at these two integer values, when they're equal
       it won't call ProcessInputEvents (). */
    HWEventQueueType head, tail;

    xEvent events[QUEUE_SIZE];
} event_queue;

/* fd[0] = reading, fd[1] = writing */
static int event_fd[2];

void
DarwinEnqueueEvent (const xEvent *e)
{
    int newtail, oldtail;
    int need_write = FALSE;

    pthread_mutex_lock (&event_queue.mutex);

    oldtail = (event_queue.tail - 1) % QUEUE_SIZE;

    if (e->u.u.type == MotionNotify
	&& event_queue.tail != event_queue.head
	&& event_queue.events[oldtail].u.u.type == MotionNotify)
    {
	/* Two adjacent motion notify events. Coalesce them. */

	memcpy (&event_queue.events[oldtail], e, sizeof (xEvent));
    }
    else
    {
	newtail = (event_queue.tail + 1) % QUEUE_SIZE;

	if (newtail != event_queue.head)
	{	
	    memcpy (&event_queue.events[event_queue.tail], e, sizeof (xEvent));
	    event_queue.tail = newtail;
	    need_write = TRUE;
	}
    }

    pthread_mutex_unlock (&event_queue.mutex);

    if (need_write)
	write (event_fd[1], &need_write, sizeof (need_write));
}

Bool
DarwinDequeueEvent (xEvent *e)
{
    Bool ret = FALSE;
    int unused;

    pthread_mutex_lock (&event_queue.mutex);

    if (event_queue.head != event_queue.tail)
    {
	memcpy (e, &event_queue.events[event_queue.head], sizeof (xEvent));
	event_queue.head = (event_queue.head + 1) % QUEUE_SIZE;
	ret = TRUE;
    }
	
    pthread_mutex_unlock (&event_queue.mutex);

    if (ret)
	read (event_fd[0], &unused, sizeof (unused));

    return ret;
}

void
DarwinInputPreInit (void)
{
    if (pipe (event_fd) != 0)
    {
	perror ("pipe");
	exit (1);
    }

    event_queue.head = event_queue.tail = 0;
    pthread_mutex_init (&event_queue.mutex, NULL);
}

void
DarwinInputInit (void)
{
    SetInputCheck (&event_queue.head, &event_queue.tail);
}


/*
 =============================================================================

 mouse and keyboard callbacks

 =============================================================================
*/

/*
 * DarwinChangePointerControl
 *  Set mouse acceleration and thresholding
 *  FIXME: We currently ignore the threshold in ctrl->threshold.
 */
static void DarwinChangePointerControl(DeviceIntPtr device, PtrCtrl *ctrl)
{
    /* do nothing here */
}


/*
 * DarwinMouseProc
 *  Handle the initialization, etc. of a mouse
 */

int DarwinMouseProc(DeviceIntPtr pPointer, int what)
{
    char map[6];

    switch (what) {

        case DEVICE_INIT:
            pPointer->public.on = FALSE;

            // Set button map.
            map[1] = 1;
            map[2] = 2;
            map[3] = 3;
            map[4] = 4;
            map[5] = 5;
            InitPointerDeviceStruct( (DevicePtr)pPointer,
                        map,
                        5,   // numbuttons (4 & 5 are scroll wheel)
                        miPointerGetMotionEvents,
                        DarwinChangePointerControl,
                        0 );
            break;

        case DEVICE_ON:
            pPointer->public.on = TRUE;
            AddEnabledDevice(event_fd[0]);
            return Success;

        case DEVICE_CLOSE:
        case DEVICE_OFF:
            pPointer->public.on = FALSE;
            RemoveEnabledDevice(event_fd[0]);
            return Success;
    }

    return Success;
}

/*
 * DarwinKeybdProc
 *  Callback from X
 */
int DarwinKeybdProc(DeviceIntPtr pDev, int onoff)
{
    switch ( onoff ) {
        case DEVICE_INIT:
            DarwinKeyboardInit( pDev );
            break;
        case DEVICE_ON:
            pDev->public.on = TRUE;
            AddEnabledDevice(event_fd[0]);
            break;
        case DEVICE_OFF:
            pDev->public.on = FALSE;
            RemoveEnabledDevice(event_fd[0]);
            break;
        case DEVICE_CLOSE:
            break;
    }

    return Success;
}
