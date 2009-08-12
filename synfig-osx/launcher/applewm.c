/* $XFree86: xc/lib/GL/dri/XF86dri.c,v 1.12 2001/08/27 17:40:57 dawes Exp $ */
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

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include "applewmstr.h"
#include <X11/extensions/Xext.h>
#include "extutil.h"
#include <stdio.h>

static XExtensionInfo _applewm_info_data;
static XExtensionInfo *applewm_info = &_applewm_info_data;
static char *applewm_extension_name = APPLEWMNAME;

#define AppleWMCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, applewm_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display(Display *dpy, XExtCodes *extCodes);
static Bool wire_to_event();
static Status event_to_wire();

static /* const */ XExtensionHooks applewm_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    wire_to_event,			/* wire_to_event */
    event_to_wire,			/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, applewm_info, 
				   applewm_extension_name, 
				   &applewm_extension_hooks, 
				   AppleWMNumberEvents, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, applewm_info)

static Bool wire_to_event (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtDisplayInfo *info = find_display (dpy);
    XAppleWMNotifyEvent *se;
    xAppleWMNotifyEvent *sevent;

    AppleWMCheckExtension (dpy, info, False);

    switch ((event->u.u.type & 0x7f) - info->codes->first_event) {
    case AppleWMControllerNotify:
    case AppleWMActivationNotify:
    case AppleWMPasteboardNotify:
    	se = (XAppleWMNotifyEvent *) re;
	sevent = (xAppleWMNotifyEvent *) event;
    	se->type = sevent->type & 0x7f;
    	se->serial = _XSetLastRequestRead(dpy,(xGenericReply *) event);
    	se->send_event = (sevent->type & 0x80) != 0;
    	se->display = dpy;
	se->window = 0;
	se->time = sevent->time;
    	se->kind = sevent->kind;
    	se->arg = sevent->arg;
    	return True;
    }
    return False;
}

static Status event_to_wire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtDisplayInfo *info = find_display (dpy);
    XAppleWMNotifyEvent *se;
    xAppleWMNotifyEvent *sevent;

    AppleWMCheckExtension (dpy, info, False);

    switch ((re->type & 0x7f) - info->codes->first_event) {
    case AppleWMControllerNotify:
    case AppleWMActivationNotify:
    case AppleWMPasteboardNotify:
    	se = (XAppleWMNotifyEvent *) re;
	sevent = (xAppleWMNotifyEvent *) event;
    	sevent->type = se->type | (se->send_event ? 0x80 : 0);
    	sevent->sequenceNumber = se->serial & 0xffff;
    	sevent->kind = se->kind;
    	sevent->arg = se->arg;
	sevent->time = se->time;
    	return 1;
    }
    return 0;
}

/*****************************************************************************
 *                                                                           *
 *		    public Apple-WM Extension routines                      *
 *                                                                           *
 *****************************************************************************/

#if 0
#include <stdio.h>
#define TRACE(msg)  fprintf(stderr, "AppleWM%s\n", msg);
#else
#define TRACE(msg)
#endif


Bool XAppleWMQueryExtension (dpy, event_basep, error_basep)
    Display *dpy;
    int *event_basep, *error_basep;
{
    XExtDisplayInfo *info = find_display (dpy);

    TRACE("QueryExtension...");
    if (XextHasExtension(info)) {
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
        TRACE("QueryExtension... return True");
	return True;
    } else {
        TRACE("QueryExtension... return False");
	return False;
    }
}

Bool XAppleWMQueryVersion(dpy, majorVersion, minorVersion, patchVersion)
    Display* dpy;
    int* majorVersion; 
    int* minorVersion;
    int* patchVersion;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMQueryVersionReply rep;
    xAppleWMQueryVersionReq *req;

    TRACE("QueryVersion...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
        TRACE("QueryVersion... return False");
	return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    *patchVersion = rep.patchVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("QueryVersion... return True");
    return True;
}

Bool XAppleWMDisableUpdate(dpy, screen)
    Display* dpy;
    int screen;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMDisableUpdateReq *req;

    TRACE("DisableUpdate...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMDisableUpdate, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMDisableUpdate;
    req->screen = screen;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("DisableUpdate... return True");
    return True;
}

Bool XAppleWMReenableUpdate(dpy, screen)
    Display* dpy;
    int screen;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMReenableUpdateReq *req;

    TRACE("ReenableUpdate...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMReenableUpdate, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMReenableUpdate;
    req->screen = screen;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("ReenableUpdate... return True");
    return True;
}

Bool XAppleWMSelectInput(dpy, mask)
    Display* dpy;
    unsigned long mask;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSelectInputReq *req;

    TRACE("SelectInput...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSelectInput, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSelectInput;
    req->mask = mask;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetlectInput... return True");
    return True;
}

Bool XAppleWMSetWindowMenuWithShortcuts(dpy, nitems, items, shortcuts)
    Display* dpy;
    int nitems;
    const char **items;
    const char *shortcuts;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSetWindowMenuReq *req;
    int i, total_length, len;
    char *buf, *ptr;

    TRACE("SetWindowMenu...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSetWindowMenu, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSetWindowMenu;
    req->nitems = nitems;

    total_length = 0;
    for (i = 0; i < nitems; i++)
	total_length += strlen (items[i]) + 2;

    ptr = buf = alloca (total_length);
    for (i = 0; i < nitems; i++)
    {
	len = strlen (items[i]);
	*ptr++ = shortcuts ? shortcuts[i] : 0;
	memcpy (ptr, items[i], len);
	ptr[len] = 0;
	ptr += len + 1;
    }

    req->length += (total_length + 3) >> 2;
    Data (dpy, buf, total_length);

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetlectInput... return True");
    return True;
}

Bool XAppleWMSetWindowMenu(dpy, nitems, items)
    Display* dpy;
    int nitems;
    const char **items;
{
    return XAppleWMSetWindowMenuWithShortcuts (dpy, nitems, items, NULL);
}

Bool XAppleWMSetWindowMenuCheck(dpy, idx)
    Display* dpy;
    int idx;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSetWindowMenuCheckReq *req;

    TRACE("SetWindowMenuCheck...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSetWindowMenuCheck, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSetWindowMenuCheck;
    req->index = idx;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetWindowMenuCheck... return True");
    return True;
}

Bool XAppleWMSetFrontProcess(dpy)
    Display* dpy;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSetFrontProcessReq *req;

    TRACE("SetFrontProcess...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSetFrontProcess, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSetFrontProcess;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetFrontProcess... return True");
    return True;
}

Bool XAppleWMSetWindowLevel(dpy, id, level)
    Display* dpy;
    Window id;
    int level;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSetWindowLevelReq *req;

    TRACE("SetWindowLevel...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSetWindowLevel, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSetWindowLevel;
    req->window = id;
    req->level = level;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetWindowLevel... return True");
    return True;
}

Bool XAppleWMSetCanQuit(dpy, state)
    Display* dpy;
    Bool state;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMSetCanQuitReq *req;

    TRACE("SetCanQuit...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMSetCanQuit, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMSetCanQuit;
    req->state = state;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("SetCanQuit... return True");
    return True;
}

Bool XAppleWMFrameGetRect(dpy, frame_class, frame_rect,
			   ix, iy, iw, ih, ox, oy, ow, oh, rx, ry, rw, rh)
    Display* dpy;
    unsigned int frame_class, frame_rect;
    short ix, iy, iw, ih;
    short ox, oy, ow, oh;
    short *rx, *ry, *rw, *rh;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMFrameGetRectReply rep;
    xAppleWMFrameGetRectReq *req;

    TRACE("FrameGetRect...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMFrameGetRect, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMFrameGetRect;
    req->frame_class = frame_class;
    req->frame_rect = frame_rect;
    req->ix = ix;
    req->iy = iy;
    req->iw = iw;
    req->ih = ih;
    req->ox = ox;
    req->oy = oy;
    req->ow = ow;
    req->oh = oh;
    rep.x = rep.y = rep.w = rep.h = 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
        TRACE("FrameGetRect... return False");
	return False;
    }
    *rx = rep.x; *ry = rep.y;
    *rw = rep.w; *rh = rep.h;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("FrameGetRect... return True");
    return True;
}

unsigned int XAppleWMFrameHitTest(dpy, frame_class, px, py,
				   ix, iy, iw, ih, ox, oy, ow, oh)
    Display* dpy;
    unsigned int frame_class;
    short px, py;
    short ix, iy, iw, ih;
    short ox, oy, ow, oh;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMFrameHitTestReply rep;
    xAppleWMFrameHitTestReq *req;

    TRACE("FrameHitTest...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMFrameHitTest, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMFrameHitTest;
    req->frame_class = frame_class;
    req->px = px;
    req->py = py;
    req->ix = ix;
    req->iy = iy;
    req->iw = iw;
    req->ih = ih;
    req->ox = ox;
    req->oy = oy;
    req->ow = ow;
    req->oh = oh;
    rep.ret = 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
        TRACE("FrameHitTest... return False");
	return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("FrameHiTest... return True");
    return rep.ret;
}

Bool XAppleWMFrameDraw(dpy, screen, window,
			frame_class, frame_attr,
			ix, iy, iw, ih, ox, oy, ow, oh,
			title_length, title_bytes)
    Display* dpy;
    int screen;
    Window window;
    unsigned int frame_class, frame_attr;
    short ix, iy, iw, ih;
    short ox, oy, ow, oh;
    unsigned int title_length;
    const unsigned char *title_bytes;
{
    XExtDisplayInfo *info = find_display (dpy);
    xAppleWMFrameDrawReq *req;

    TRACE("FrameDraw...");
    AppleWMCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(AppleWMFrameDraw, req);
    req->reqType = info->codes->major_opcode;
    req->wmReqType = X_AppleWMFrameDraw;
    req->screen = screen;
    req->window = window;
    req->frame_class = frame_class;
    req->frame_attr = frame_attr;
    req->ix = ix;
    req->iy = iy;
    req->iw = iw;
    req->ih = ih;
    req->ox = ox;
    req->oy = oy;
    req->ow = ow;
    req->oh = oh;
    req->title_length = title_length;

    req->length += (title_length + 3)>>2;
    Data (dpy, title_bytes, title_length);

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("FrameDraw... return True");
    return True;
}
