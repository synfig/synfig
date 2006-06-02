/* $XFree86: xc/programs/Xserver/GL/dri/xf86dri.c,v 1.10 2000/12/07 20:26:14 dawes Exp $ */
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
#include "servermd.h"
#define _APPLEWM_SERVER_
#include "applewmstr.h"
#include "swaprep.h"
#include "rootless-common.h"
#include "X11Application.h"

static int WMErrorBase;

static DISPATCH_PROC(ProcAppleWMDispatch);
static DISPATCH_PROC(SProcAppleWMDispatch);

static void AppleWMResetProc(ExtensionEntry* extEntry);

static unsigned char WMReqCode = 0;
static int WMEventBase = 0;

static RESTYPE ClientType, EventType; /* resource types for event masks */
static XID eventResource;

/* Currently selected events */
static unsigned int eventMask = 0;

extern void AppleWMExtensionInit(void);

static int WMFreeClient (pointer data, XID id);
static int WMFreeEvents (pointer data, XID id);
static void SNotifyEvent(xAppleWMNotifyEvent *from, xAppleWMNotifyEvent *to);

typedef struct _WMEvent *WMEventPtr;
typedef struct _WMEvent {
    WMEventPtr     next;
    ClientPtr	    client;
    XID		    clientResource;
    unsigned int    mask;
} WMEventRec;

static inline BoxRec
make_box (int x, int y, int w, int h)
{
    BoxRec r;
    r.x1 = x;
    r.y1 = y;
    r.x2 = x + w;
    r.y2 = y + h;
    return r;
}

void
AppleWMExtensionInit(void)
{
    ExtensionEntry* extEntry;

    ClientType = CreateNewResourceType(WMFreeClient);
    EventType = CreateNewResourceType(WMFreeEvents);
    eventResource = FakeClientID(0);

    if (ClientType && EventType &&
	(extEntry = AddExtension(APPLEWMNAME,
				 AppleWMNumberEvents,
				 AppleWMNumberErrors,
				 ProcAppleWMDispatch,
				 SProcAppleWMDispatch,
				 AppleWMResetProc,
				 StandardMinorOpcode))) {
	WMReqCode = (unsigned char)extEntry->base;
	WMErrorBase = extEntry->errorBase;
	WMEventBase = extEntry->eventBase;
	EventSwapVector[WMEventBase] = (EventSwapPtr) SNotifyEvent;
    }
}

/*ARGSUSED*/
static void
AppleWMResetProc (
    ExtensionEntry* extEntry
)
{
}

static int
ProcAppleWMQueryVersion(
    register ClientPtr client
)
{
    xAppleWMQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xAppleWMQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = APPLE_WM_MAJOR_VERSION;
    rep.minorVersion = APPLE_WM_MINOR_VERSION;
    rep.patchVersion = APPLE_WM_PATCH_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xAppleWMQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}


/* events */

static inline void
updateEventMask (WMEventPtr *pHead)
{
    WMEventPtr pCur;

    eventMask = 0;
    for (pCur = *pHead; pCur != NULL; pCur = pCur->next)
	eventMask |= pCur->mask;
}

/*ARGSUSED*/
static int
WMFreeClient (data, id)
    pointer	    data;
    XID		    id;
{
    WMEventPtr   pEvent;
    WMEventPtr   *pHead, pCur, pPrev;

    pEvent = (WMEventPtr) data;
    pHead = (WMEventPtr *) LookupIDByType(eventResource, EventType);
    if (pHead) {
	pPrev = 0;
	for (pCur = *pHead; pCur && pCur != pEvent; pCur=pCur->next)
	    pPrev = pCur;
	if (pCur)
	{
	    if (pPrev)
	    	pPrev->next = pEvent->next;
	    else
	    	*pHead = pEvent->next;
	}
	updateEventMask (pHead);
    }
    xfree ((pointer) pEvent);
    return 1;
}

/*ARGSUSED*/
static int
WMFreeEvents (data, id)
    pointer	    data;
    XID		    id;
{
    WMEventPtr   *pHead, pCur, pNext;

    pHead = (WMEventPtr *) data;
    for (pCur = *pHead; pCur; pCur = pNext) {
	pNext = pCur->next;
	FreeResource (pCur->clientResource, ClientType);
	xfree ((pointer) pCur);
    }
    xfree ((pointer) pHead);
    eventMask = 0;
    return 1;
}

static int
ProcAppleWMSelectInput (client)
    register ClientPtr	client;
{
    REQUEST(xAppleWMSelectInputReq);
    WMEventPtr	pEvent, pNewEvent, *pHead;
    XID			clientResource;

    REQUEST_SIZE_MATCH (xAppleWMSelectInputReq);
    pHead = (WMEventPtr *)SecurityLookupIDByType(client,
			eventResource, EventType, SecurityWriteAccess);
    if (stuff->mask != 0) {
	if (pHead) {
	    /* check for existing entry. */
	    for (pEvent = *pHead; pEvent; pEvent = pEvent->next)
	    {
		if (pEvent->client == client)
		{
		    pEvent->mask = stuff->mask;
		    updateEventMask (pHead);
		    return Success;
		}
	    }
	}

	/* build the entry */
    	pNewEvent = (WMEventPtr) xalloc (sizeof (WMEventRec));
    	if (!pNewEvent)
	    return BadAlloc;
    	pNewEvent->next = 0;
    	pNewEvent->client = client;
	pNewEvent->mask = stuff->mask;
    	/*
 	 * add a resource that will be deleted when
     	 * the client goes away
     	 */
   	clientResource = FakeClientID (client->index);
    	pNewEvent->clientResource = clientResource;
    	if (!AddResource (clientResource, ClientType, (pointer)pNewEvent))
	    return BadAlloc;
    	/*
     	 * create a resource to contain a pointer to the list
     	 * of clients selecting input.  This must be indirect as
     	 * the list may be arbitrarily rearranged which cannot be
     	 * done through the resource database.
     	 */
    	if (!pHead)
    	{
	    pHead = (WMEventPtr *) xalloc (sizeof (WMEventPtr));
	    if (!pHead ||
	    	!AddResource (eventResource, EventType, (pointer)pHead))
	    {
	    	FreeResource (clientResource, RT_NONE);
	    	return BadAlloc;
	    }
	    *pHead = 0;
    	}
    	pNewEvent->next = *pHead;
    	*pHead = pNewEvent;
	updateEventMask (pHead);
    } else if (stuff->mask == 0) {
	/* delete the interest */
	if (pHead) {
	    pNewEvent = 0;
	    for (pEvent = *pHead; pEvent; pEvent = pEvent->next) {
		if (pEvent->client == client)
		    break;
		pNewEvent = pEvent;
	    }
	    if (pEvent) {
		FreeResource (pEvent->clientResource, ClientType);
		if (pNewEvent)
		    pNewEvent->next = pEvent->next;
		else
		    *pHead = pEvent->next;
		xfree (pEvent);
		updateEventMask (pHead);
	    }
	}
    } else {
	client->errorValue = stuff->mask;
	return BadValue;
    }
    return Success;
}

/*
 * deliver the event
 */

void
AppleWMSendEvent (type, mask, which, arg)
    int type, which, arg;
    unsigned int mask;
{
    WMEventPtr	*pHead, pEvent;
    ClientPtr		client;
    xAppleWMNotifyEvent se;

    pHead = (WMEventPtr *) LookupIDByType(eventResource, EventType);
    if (!pHead)
	return;
    for (pEvent = *pHead; pEvent; pEvent = pEvent->next) {
	client = pEvent->client;
	if ((pEvent->mask & mask) == 0
	    || client == serverClient || client->clientGone)
	{
	    continue;
	}
	se.type = type + WMEventBase;
	se.kind = which;
	se.arg = arg;
	se.sequenceNumber = client->sequence;
	se.time = currentTime.milliseconds;
	WriteEventsToClient (client, 1, (xEvent *) &se);
    }
}

/* Safe to call from any thread. */
unsigned int
AppleWMSelectedEvents (void)
{
    return eventMask;
}


/* general utility functions */

static int
ProcAppleWMDisableUpdate(
    register ClientPtr client
)
{
    REQUEST_SIZE_MATCH(xAppleWMDisableUpdateReq);

    xp_disable_update ();

    return (client->noClientException);
}

static int
ProcAppleWMReenableUpdate(
    register ClientPtr client
)
{
    REQUEST_SIZE_MATCH(xAppleWMReenableUpdateReq);

    xp_reenable_update ();

    return (client->noClientException);
}


/* window functions */

static int
ProcAppleWMSetWindowMenu(
    register ClientPtr client
)
{
    const char *bytes, **items;
    char *shortcuts;
    int max_len, nitems, i, j;
    REQUEST(xAppleWMSetWindowMenuReq);

    REQUEST_AT_LEAST_SIZE(xAppleWMSetWindowMenuReq);

    nitems = stuff->nitems;
    items = alloca (sizeof (char *) * nitems);
    shortcuts = alloca (sizeof (char) * nitems);

    max_len = (stuff->length << 2) - sizeof(xAppleWMSetWindowMenuReq);
    bytes = (char *) &stuff[1];

    for (i = j = 0; i < max_len && j < nitems;)
    {
	shortcuts[j] = bytes[i++];
	items[j++] = bytes + i;

	while (i < max_len)
	{
	    if (bytes[i++] == 0)
		break;
	}
    }

    X11ApplicationSetWindowMenu (nitems, items, shortcuts);

    return (client->noClientException);
}

static int
ProcAppleWMSetWindowMenuCheck(
    register ClientPtr client
)
{
    REQUEST(xAppleWMSetWindowMenuCheckReq);

    REQUEST_SIZE_MATCH(xAppleWMSetWindowMenuCheckReq);

    X11ApplicationSetWindowMenuCheck (stuff->index);

    return (client->noClientException);
}

static int
ProcAppleWMSetFrontProcess(
    register ClientPtr client
)
{
    REQUEST_SIZE_MATCH(xAppleWMSetFrontProcessReq);

    X11ApplicationSetFrontProcess ();

    return (client->noClientException);
}

static int
ProcAppleWMSetWindowLevel(
    register ClientPtr client
)
{
    REQUEST(xAppleWMSetWindowLevelReq);
    WindowPtr pWin;

    REQUEST_SIZE_MATCH(xAppleWMSetWindowLevelReq);

    if (!(pWin = SecurityLookupWindow((Drawable)stuff->window,
				      client, SecurityReadAccess))) {
	return BadValue;
    }

    if (stuff->level < 0 || stuff->level >= AppleWMNumWindowLevels) {
	return BadValue;
    }

    RootlessSetWindowLevel (pWin, stuff->level);

    return (client->noClientException);
}

static int
ProcAppleWMSetCanQuit(
    register ClientPtr client
)
{
    REQUEST(xAppleWMSetCanQuitReq);

    REQUEST_SIZE_MATCH(xAppleWMSetCanQuitReq);

    X11ApplicationSetCanQuit (stuff->state);

    return (client->noClientException);
}


/* frame functions */

static int
ProcAppleWMFrameGetRect(
    register ClientPtr client
)
{
    xAppleWMFrameGetRectReply rep;
    BoxRec ir, or, rr;
    REQUEST(xAppleWMFrameGetRectReq);

    REQUEST_SIZE_MATCH(xAppleWMFrameGetRectReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    ir = make_box (stuff->ix, stuff->iy, stuff->iw, stuff->ih);
    or = make_box (stuff->ox, stuff->oy, stuff->ow, stuff->oh);

    if (xp_frame_get_rect (stuff->frame_rect,
			   stuff->frame_class,
			   &or, &ir, &rr) != Success) {
	return BadValue;
    }

    rep.x = rr.x1;
    rep.y = rr.y1;
    rep.w = rr.x2 - rr.x1;
    rep.h = rr.y2 - rr.y1;

    WriteToClient(client, sizeof(xAppleWMFrameGetRectReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcAppleWMFrameHitTest(
    register ClientPtr client
)
{
    xAppleWMFrameHitTestReply rep;
    BoxRec ir, or;
    int ret;
    REQUEST(xAppleWMFrameHitTestReq);

    REQUEST_SIZE_MATCH(xAppleWMFrameHitTestReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    ir = make_box (stuff->ix, stuff->iy, stuff->iw, stuff->ih);
    or = make_box (stuff->ox, stuff->oy, stuff->ow, stuff->oh);

    if (xp_frame_hit_test (stuff->frame_class, stuff->px,
			   stuff->py, &or, &ir, &ret) != Success)
    {
	return BadValue;
    }

    rep.ret = ret;

    WriteToClient(client, sizeof(xAppleWMFrameHitTestReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcAppleWMFrameDraw(
    register ClientPtr client
)
{
    BoxRec ir, or;
    unsigned int title_length, title_max;
    unsigned char *title_bytes;
    REQUEST(xAppleWMFrameDrawReq);
    WindowPtr pWin;
    xp_window_id wid;

    REQUEST_AT_LEAST_SIZE(xAppleWMFrameDrawReq);

    if (!(pWin = SecurityLookupWindow((Drawable)stuff->window,
				      client, SecurityReadAccess))) {
	return BadValue;
    }

    ir = make_box (stuff->ix, stuff->iy, stuff->iw, stuff->ih);
    or = make_box (stuff->ox, stuff->oy, stuff->ow, stuff->oh);

    title_length = stuff->title_length;
    title_max = (stuff->length << 2) - sizeof(xAppleWMFrameDrawReq);

    if (title_max < title_length)
	return BadValue;

    title_bytes = (unsigned char *) &stuff[1];

    wid = RootlessGetPhysicalWindow (pWin, FALSE);
    if (wid == 0)
	return BadWindow;

    if (xp_frame_draw (wid, stuff->frame_class,
		       stuff->frame_attr, &or, &ir,
		       title_length, title_bytes) != Success) {
	return BadValue;
    }

    return (client->noClientException);
}


/* dispatch */

static int
ProcAppleWMDispatch (
    register ClientPtr	client
)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
    case X_AppleWMQueryVersion:
	return ProcAppleWMQueryVersion(client);
    }

    if (!LocalClient(client))
	return WMErrorBase + AppleWMClientNotLocal;

    switch (stuff->data)
    {
    case X_AppleWMSelectInput:
	return ProcAppleWMSelectInput(client);
    case X_AppleWMDisableUpdate:
	return ProcAppleWMDisableUpdate(client);
    case X_AppleWMReenableUpdate:
	return ProcAppleWMReenableUpdate(client);
    case X_AppleWMSetWindowMenu:
	return ProcAppleWMSetWindowMenu(client);
    case X_AppleWMSetWindowMenuCheck:
	return ProcAppleWMSetWindowMenuCheck(client);
    case X_AppleWMSetFrontProcess:
	return ProcAppleWMSetFrontProcess(client);
    case X_AppleWMSetWindowLevel:
	return ProcAppleWMSetWindowLevel(client);
    case X_AppleWMSetCanQuit:
	return ProcAppleWMSetCanQuit(client);
    case X_AppleWMFrameGetRect:
	return ProcAppleWMFrameGetRect(client);
    case X_AppleWMFrameHitTest:
	return ProcAppleWMFrameHitTest(client);
    case X_AppleWMFrameDraw:
	return ProcAppleWMFrameDraw(client);
    default:
	return BadRequest;
    }
}

static void
SNotifyEvent(from, to)
    xAppleWMNotifyEvent *from, *to;
{
    to->type = from->type;
    to->kind = from->kind;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->time, to->time);
    cpswapl (from->arg, to->arg);
}

static int
SProcAppleWMQueryVersion(
    register ClientPtr	client
)
{
    register int n;
    REQUEST(xAppleWMQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcAppleWMQueryVersion(client);
}

static int
SProcAppleWMDispatch (
    register ClientPtr	client
)
{
    REQUEST(xReq);

    /* It is bound to be non-local when there is byte swapping */
    if (!LocalClient(client))
	return WMErrorBase + AppleWMClientNotLocal;

    /* only local clients are allowed WM access */
    switch (stuff->data)
    {
    case X_AppleWMQueryVersion:
	return SProcAppleWMQueryVersion(client);
    default:
	return BadRequest;
    }
}
