/*
 * Minimal implementation of PanoramiX/Xinerama
 *
 * This is used in rootless mode where the underlying window server
 * already provides an abstracted view of multiple screens as one
 * large screen area.
 *
 * This code is largely based on panoramiX.c, which contains the
 * following copyright notice:
 */
/*****************************************************************
Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.
******************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/pseudoramiX.c,v 1.1 2002/03/28 02:21:18 torrey Exp $ */

#include "pseudoramiX.h"

#include "extnsionst.h"
#include "dixstruct.h"
#include "window.h"
#include "panoramiXproto.h"
#include "globals.h"

extern int ProcPanoramiXQueryVersion (ClientPtr client);

static void PseudoramiXResetProc(ExtensionEntry *extEntry);

static int ProcPseudoramiXQueryVersion(ClientPtr client);
static int ProcPseudoramiXGetState(ClientPtr client);
static int ProcPseudoramiXGetScreenCount(ClientPtr client);
static int ProcPseudoramiXGetScreenSize(ClientPtr client);
static int ProcPseudoramiXIsActive(ClientPtr client);
static int ProcPseudoramiXQueryScreens(ClientPtr client);
static int ProcPseudoramiXDispatch(ClientPtr client);

static int SProcPseudoramiXQueryVersion(ClientPtr client);
static int SProcPseudoramiXGetState(ClientPtr client);
static int SProcPseudoramiXGetScreenCount(ClientPtr client);
static int SProcPseudoramiXGetScreenSize(ClientPtr client);
static int SProcPseudoramiXIsActive(ClientPtr client);
static int SProcPseudoramiXQueryScreens(ClientPtr client);
static int SProcPseudoramiXDispatch(ClientPtr client);


typedef struct {
    int x;
    int y;
    int w;
    int h;
} PseudoramiXScreenRec;

static PseudoramiXScreenRec *pseudoramiXScreens = NULL;
static int pseudoramiXScreensAllocated = 0;
static int pseudoramiXNumScreens = 0;
static int pseudoramiXGeneration = -1;


// Add a PseudoramiX screen.
// The rest of the X server will know nothing about this screen.
// Can be called before or after extension init.
// Screens must be re-added once per generation.
void
PseudoramiXAddScreen(int x, int y, int w, int h)
{
    PseudoramiXScreenRec *s;

    if (noPseudoramiXExtension) return;

    if (pseudoramiXNumScreens == pseudoramiXScreensAllocated) {
        pseudoramiXScreensAllocated += pseudoramiXScreensAllocated + 1;
        pseudoramiXScreens = xrealloc(pseudoramiXScreens,
                                      pseudoramiXScreensAllocated *
                                      sizeof(PseudoramiXScreenRec));
    }

    s = &pseudoramiXScreens[pseudoramiXNumScreens++];
    s->x = x;
    s->y = y;
    s->w = w;
    s->h = h;
}


// Initialize PseudoramiX.
// Copied from PanoramiXExtensionInit
void PseudoramiXExtensionInit(int argc, char *argv[])
{
    Bool	     	success = FALSE;
    ExtensionEntry 	*extEntry;

    if (noPseudoramiXExtension) return;

#if 0
    if (pseudoramiXNumScreens == 1) {
        // Only one screen - disable Xinerama extension.
        noPseudoramiXExtension = TRUE;
        return;
    }
#endif

    // The server must not run the PanoramiX operations.
    noPanoramiXExtension = TRUE;

    if (pseudoramiXGeneration != serverGeneration) {
        extEntry = AddExtension(PANORAMIX_PROTOCOL_NAME, 0, 0,
                                ProcPseudoramiXDispatch,
                                SProcPseudoramiXDispatch,
                                PseudoramiXResetProc,
                                StandardMinorOpcode);
        if (!extEntry) {
            ErrorF("PseudoramiXExtensionInit(): AddExtension failed\n");
        } else {
            pseudoramiXGeneration = serverGeneration;
            success = TRUE;
        }
    }

    if (!success) {
        ErrorF("%s Extension (PseudoramiX) failed to initialize\n",
               PANORAMIX_PROTOCOL_NAME);
        return;
    }
}


static void PseudoramiXResetProc(ExtensionEntry *extEntry)
{
    pseudoramiXNumScreens = 0;
}

void PseudoramiXResetScreens (void)
{
    pseudoramiXNumScreens = 0;
}

// was PanoramiX
static int ProcPseudoramiXQueryVersion(ClientPtr client)
{
    return ProcPanoramiXQueryVersion(client);
}


// was PanoramiX
static int ProcPseudoramiXGetState(ClientPtr client)
{
    REQUEST(xPanoramiXGetStateReq);
    WindowPtr pWin;
    xPanoramiXGetStateReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
        return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.state = !noPseudoramiXExtension;
    if (client->swapped) {
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swaps (&rep.state, n);
    }
    WriteToClient (client, sizeof (xPanoramiXGetStateReply), (char *) &rep);
    return client->noClientException;
}


// was PanoramiX
static int ProcPseudoramiXGetScreenCount(ClientPtr client)
{
    REQUEST(xPanoramiXGetScreenCountReq);
    WindowPtr pWin;
    xPanoramiXGetScreenCountReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xPanoramiXGetScreenCountReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
        return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.ScreenCount = pseudoramiXNumScreens;
    if (client->swapped) {
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swaps (&rep.ScreenCount, n);
    }
    WriteToClient (client, sizeof(xPanoramiXGetScreenCountReply), (char *)&rep);
    return client->noClientException;
}


// was PanoramiX
static int ProcPseudoramiXGetScreenSize(ClientPtr client)
{
    REQUEST(xPanoramiXGetScreenSizeReq);
    WindowPtr			pWin;
    xPanoramiXGetScreenSizeReply	rep;
    register int			n;

    REQUEST_SIZE_MATCH(xPanoramiXGetScreenSizeReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
        return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    /* screen dimensions */
    rep.width  = pseudoramiXScreens[stuff->screen].w;
    // was panoramiXdataPtr[stuff->screen].width;
    rep.height = pseudoramiXScreens[stuff->screen].h;
    // was panoramiXdataPtr[stuff->screen].height;
    if (client->swapped) {
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swaps (&rep.width, n);
        swaps (&rep.height, n);
    }
    WriteToClient (client, sizeof(xPanoramiXGetScreenSizeReply), (char *)&rep);
    return client->noClientException;
}


// was Xinerama
static int ProcPseudoramiXIsActive(ClientPtr client)
{
    /* REQUEST(xXineramaIsActiveReq); */
    xXineramaIsActiveReply	rep;

    REQUEST_SIZE_MATCH(xXineramaIsActiveReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.state = !noPseudoramiXExtension;
    if (client->swapped) {
	register int n;
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.state, n);
    }
    WriteToClient (client, sizeof (xXineramaIsActiveReply), (char *) &rep);
    return client->noClientException;
}


// was Xinerama
static int ProcPseudoramiXQueryScreens(ClientPtr client)
{
    /* REQUEST(xXineramaQueryScreensReq); */
    xXineramaQueryScreensReply	rep;

    REQUEST_SIZE_MATCH(xXineramaQueryScreensReq);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.number = noPseudoramiXExtension ? 0 : pseudoramiXNumScreens;
    rep.length = rep.number * sz_XineramaScreenInfo >> 2;
    if (client->swapped) {
	register int n;
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.number, n);
    }
    WriteToClient (client, sizeof (xXineramaQueryScreensReply), (char *) &rep);

    if (!noPseudoramiXExtension) {
	xXineramaScreenInfo scratch;
	int i;

	for(i = 0; i < pseudoramiXNumScreens; i++) {
	    scratch.x_org  = pseudoramiXScreens[i].x;
	    scratch.y_org  = pseudoramiXScreens[i].y;
	    scratch.width  = pseudoramiXScreens[i].w;
	    scratch.height = pseudoramiXScreens[i].h;

	    if(client->swapped) {
		register int n;
		swaps (&scratch.x_org, n);
		swaps (&scratch.y_org, n);
		swaps (&scratch.width, n);
		swaps (&scratch.height, n);
	    }
	    WriteToClient (client, sz_XineramaScreenInfo, (char *) &scratch);
	}
    }

    return client->noClientException;
}


// was PanoramiX
static int ProcPseudoramiXDispatch (ClientPtr client)
{   REQUEST(xReq);
    switch (stuff->data)
    {
	case X_PanoramiXQueryVersion:
	     return ProcPseudoramiXQueryVersion(client);
	case X_PanoramiXGetState:
	     return ProcPseudoramiXGetState(client);
	case X_PanoramiXGetScreenCount:
	     return ProcPseudoramiXGetScreenCount(client);
	case X_PanoramiXGetScreenSize:
	     return ProcPseudoramiXGetScreenSize(client);
	case X_XineramaIsActive:
	     return ProcPseudoramiXIsActive(client);
	case X_XineramaQueryScreens:
	     return ProcPseudoramiXQueryScreens(client);
    }
    return BadRequest;
}



static int
SProcPseudoramiXQueryVersion (ClientPtr client)
{
	REQUEST(xPanoramiXQueryVersionReq);
	register int n;

	swaps(&stuff->length,n);
	REQUEST_SIZE_MATCH (xPanoramiXQueryVersionReq);
	return ProcPseudoramiXQueryVersion(client);
}

static int
SProcPseudoramiXGetState(ClientPtr client)
{
	REQUEST(xPanoramiXGetStateReq);
	register int n;

 	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);
	return ProcPseudoramiXGetState(client);
}

static int
SProcPseudoramiXGetScreenCount(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenCountReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xPanoramiXGetScreenCountReq);
	return ProcPseudoramiXGetScreenCount(client);
}

static int
SProcPseudoramiXGetScreenSize(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenSizeReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xPanoramiXGetScreenSizeReq);
	return ProcPseudoramiXGetScreenSize(client);
}


static int
SProcPseudoramiXIsActive(ClientPtr client)
{
	REQUEST(xXineramaIsActiveReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xXineramaIsActiveReq);
	return ProcPseudoramiXIsActive(client);
}


static int
SProcPseudoramiXQueryScreens(ClientPtr client)
{
	REQUEST(xXineramaQueryScreensReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xXineramaQueryScreensReq);
	return ProcPseudoramiXQueryScreens(client);
}


static int
SProcPseudoramiXDispatch (ClientPtr client)
{   REQUEST(xReq);
    switch (stuff->data)
    {
	case X_PanoramiXQueryVersion:
	     return SProcPseudoramiXQueryVersion(client);
	case X_PanoramiXGetState:
	     return SProcPseudoramiXGetState(client);
	case X_PanoramiXGetScreenCount:
	     return SProcPseudoramiXGetScreenCount(client);
	case X_PanoramiXGetScreenSize:
	     return SProcPseudoramiXGetScreenSize(client);
	case X_XineramaIsActive:
	     return SProcPseudoramiXIsActive(client);
	case X_XineramaQueryScreens:
	     return SProcPseudoramiXQueryScreens(client);
    }
    return BadRequest;
}
