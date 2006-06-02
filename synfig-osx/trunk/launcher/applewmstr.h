/* $XFree86: xc/lib/GL/dri/xf86dristr.h,v 1.9 2001/03/21 16:01:08 dawes Exp $ */
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

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Jens Owen <jens@valinux.com>
 *   Rickard E. (Rik) Fiath <faith@valinux.com>
 *
 */

#ifndef _APPLEWMSTR_H_
#define _APPLEWMSTR_H_

#include "applewm.h"

#define APPLEWMNAME "Apple-WM"

#define APPLE_WM_MAJOR_VERSION	1	/* current version numbers */
#define APPLE_WM_MINOR_VERSION	0
#define APPLE_WM_PATCH_VERSION	0

typedef struct _AppleWMQueryVersion {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMQueryVersion */
    CARD16	length B16;
} xAppleWMQueryVersionReq;
#define sz_xAppleWMQueryVersionReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;	/* major version of WM protocol */
    CARD16	minorVersion B16;	/* minor version of WM protocol */
    CARD32	patchVersion B32;       /* patch version of WM protocol */
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xAppleWMQueryVersionReply;
#define sz_xAppleWMQueryVersionReply	32

typedef struct _AppleWMQueryDirectRenderingCapable {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* X_WMQueryDirectRenderingCapable */
    CARD16	length B16;
    CARD32	screen B32;
} xAppleWMQueryDirectRenderingCapableReq;
#define sz_xAppleWMQueryDirectRenderingCapableReq	8

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    BOOL	isCapable;
    BOOL	pad2;
    BOOL	pad3;
    BOOL	pad4;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
    CARD32	pad7 B32;
    CARD32	pad8 B32;
    CARD32	pad9 B32;
} xAppleDRIQueryDirectRenderingCapableReply;
#define sz_xAppleWMQueryDirectRenderingCapableReply	32

typedef struct _AppleWMAuthConnection {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMCloseConnection */
    CARD16	length B16;
    CARD32	screen B32;
    CARD32      magic B32;
} xAppleWMAuthConnectionReq;
#define sz_xAppleWMAuthConnectionReq	12

typedef struct {
    BYTE        type;
    BOOL        pad1;
    CARD16      sequenceNumber B16;
    CARD32      length B32;
    CARD32      authenticated B32;
    CARD32      pad2 B32;
    CARD32      pad3 B32;
    CARD32      pad4 B32;
    CARD32      pad5 B32;
    CARD32      pad6 B32;
} xAppleWMAuthConnectionReply;
#define zx_xAppleWMAuthConnectionReply  32

typedef struct _AppleWMCreateSurface {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMCreateSurface */
    CARD16	length B16;
    CARD32	screen B32;
    CARD32	drawable B32;
    CARD32	client_id B32;
} xAppleWMCreateSurfaceReq;
#define sz_xAppleWMCreateSurfaceReq	16

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	key_0 B32;
    CARD32	key_1 B32;
    CARD32	uid B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xAppleWMCreateSurfaceReply;
#define sz_xAppleWMCreateSurfaceReply	32

typedef struct _AppleWMDestroySurface {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMDestroySurface */
    CARD16	length B16;
    CARD32	screen B32;
    CARD32	drawable B32;
} xAppleWMDestroySurfaceReq;
#define sz_xAppleWMDestroySurfaceReq	12

typedef struct _AppleWMDisableUpdate {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMDisableUpdate */
    CARD16	length B16;
    CARD32	screen B32;
} xAppleWMDisableUpdateReq;
#define sz_xAppleWMDisableUpdateReq	8

typedef struct _AppleWMReenableUpdate {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMReenableUpdate */
    CARD16	length B16;
    CARD32	screen B32;
} xAppleWMReenableUpdateReq;
#define sz_xAppleWMReenableUpdateReq	8

typedef struct _AppleWMSelectInput {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSelectInput */
    CARD16	length B16;
    CARD32	mask B32;
} xAppleWMSelectInputReq;
#define sz_xAppleWMSelectInputReq	8

typedef struct _AppleWMNotify {
	BYTE	type;		/* always eventBase + event type */
	BYTE	kind;
	CARD16	sequenceNumber B16;
	Time	time B32;	/* time of change */
	CARD16	pad1 B16;
	CARD32	arg B32;
	CARD32	pad3 B32;
} xAppleWMNotifyEvent;
#define sz_xAppleWMNotifyEvent	20

typedef struct _AppleWMSetWindowMenu {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSetWindowMenu */
    CARD16	length B16;
    CARD16	nitems B16;
    CARD16	pad1 B16;
} xAppleWMSetWindowMenuReq;
#define sz_xAppleWMSetWindowMenuReq	8

typedef struct _AppleWMSetWindowMenuCheck {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSetWindowMenuCheck */
    CARD16	length B16;
    CARD32	index;
} xAppleWMSetWindowMenuCheckReq;
#define sz_xAppleWMSetWindowMenuCheckReq 8

typedef struct _AppleWMSetFrontProcess {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSetFrontProcess */
    CARD16	length B16;
} xAppleWMSetFrontProcessReq;
#define sz_xAppleWMSetFrontProcessReq 4

typedef struct _AppleWMSetWindowLevel {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSetWindowLevel */
    CARD16	length B16;
    CARD32	window;
    CARD32	level;
} xAppleWMSetWindowLevelReq;
#define sz_xAppleWMSetWindowLevelReq 12

typedef struct _AppleWMSetCanQuit {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMSetCanQuit */
    CARD16	length B16;
    CARD32	state;
} xAppleWMSetCanQuitReq;
#define sz_xAppleWMSetCanQuitReq 8

typedef struct _AppleWMFrameGetRect {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMFrameGetRect */
    CARD16	length B16;
    CARD16	frame_class B16;
    CARD16	frame_rect B16;
    CARD16	ix B16;
    CARD16	iy B16;
    CARD16	iw B16;
    CARD16	ih B16;
    CARD16	ox B16;
    CARD16	oy B16;
    CARD16	ow B16;
    CARD16	oh B16;
} xAppleWMFrameGetRectReq;
#define sz_xAppleWMFrameGetRectReq	24

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	x B16;
    CARD16	y B16;
    CARD16	w B16;
    CARD16	h B16;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xAppleWMFrameGetRectReply;
#define sz_xAppleWMFrameGetRectReply	32

typedef struct _AppleWMFrameHitTest {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMFrameHitTest */
    CARD16	length B16;
    CARD16	frame_class B16;
    CARD16	pad1 B16;
    CARD16	px B16;
    CARD16	py B16;
    CARD16	ix B16;
    CARD16	iy B16;
    CARD16	iw B16;
    CARD16	ih B16;
    CARD16	ox B16;
    CARD16	oy B16;
    CARD16	ow B16;
    CARD16	oh B16;
} xAppleWMFrameHitTestReq;
#define sz_xAppleWMFrameHitTestReq	28

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	ret B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xAppleWMFrameHitTestReply;
#define sz_xAppleWMFrameHitTestReply	32

typedef struct _AppleWMFrameDraw {
    CARD8	reqType;		/* always WMReqCode */
    CARD8	wmReqType;		/* always X_WMFrameDraw */
    CARD16	length B16;
    CARD32	screen B32;
    CARD32	window B32;
    CARD16	frame_class B16;
    CARD16	frame_attr B16;
    CARD16	ix B16;
    CARD16	iy B16;
    CARD16	iw B16;
    CARD16	ih B16;
    CARD16	ox B16;
    CARD16	oy B16;
    CARD16	ow B16;
    CARD16	oh B16;
    CARD32	title_length B32;
} xAppleWMFrameDrawReq;
#define sz_xAppleWMFrameDrawReq	36

#ifdef _APPLEWM_SERVER_

void AppleWMSendEvent (
#if NeedFunctionPrototypes
    int			/* type */,
    unsigned int	/* mask */,
    int			/* which */,
    int			/* arg */
#endif
);

unsigned int AppleWMSelectedEvents (
#if NeedFunctionPrototypes
    void
#endif
);

#endif /* _APPLEWM_SERVER_ */
#endif /* _APPLEWMSTR_H_ */
