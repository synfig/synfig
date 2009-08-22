/**************************************************************
 *
 * Support for using the Quartz Window Manager cursor
 *
 **************************************************************/
/*
 * Copyright (c) 2001 Torrey T. Lyons and Greg Parker.
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
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzCursor.c,v 1.15 2001/12/22 05:28:35 torrey Exp $ */

#include "quartz.h"
#include "quartz-cursor.h"
#include "Xplugin.h"

#include "mi.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "mipointrst.h"
#include "windowstr.h"
#include "globals.h"
#include "servermd.h"
#include "dixevents.h"

#include <CoreGraphics/CoreGraphics.h>

typedef struct {
    int                     CursorVisible;
    QueryBestSizeProcPtr    QueryBestSize;
    miPointerSpriteFuncPtr  spriteFuncs;
} QuartzCursorScreenRec, *QuartzCursorScreenPtr;

static int darwinCursorScreenIndex = -1;
static unsigned long darwinCursorGeneration = 0;

static Bool movedCursor = FALSE;

#define CURSOR_PRIV(pScreen) \
    ((QuartzCursorScreenPtr)pScreen->devPrivates[darwinCursorScreenIndex].ptr)

static Bool
load_cursor (CursorPtr src, int screen)
{
    uint32_t *data;
    uint32_t rowbytes;
    int width, height;
    int hot_x, hot_y;

    uint32_t fg_color, bg_color;
    uint8_t *srow, *sptr;
    uint8_t *mrow, *mptr;
    uint32_t *drow, *dptr;
    unsigned xcount, ycount;

    xp_error err;

    width = src->bits->width;
    height = src->bits->height;
    hot_x = src->bits->xhot;
    hot_y = src->bits->yhot;

#ifdef ARGB_CURSOR
    if (src->bits->argb != NULL)
    {
	rowbytes = src->bits->width * sizeof (CARD32);
	data = (uint32_t *) src->bits->argb;
    }
    else
#endif
    {
	fg_color = 0xFF00 | (src->foreRed >> 8);
	fg_color <<= 16;
	fg_color |= src->foreGreen & 0xFF00;
	fg_color |= src->foreBlue >> 8;

	bg_color = 0xFF00 | (src->backRed >> 8);
	bg_color <<= 16;
	bg_color |= src->backGreen & 0xFF00;
	bg_color |= src->backBlue >> 8;

	fg_color = htonl (fg_color);
	bg_color = htonl (bg_color);

	/* round up to 8 pixel boundary so we can convert whole bytes */
	rowbytes = ((src->bits->width * 4) + 31) & ~31;
	data = alloca (rowbytes * src->bits->height);

	if (!src->bits->emptyMask)
	{
	    ycount = src->bits->height;
	    srow = src->bits->source; mrow = src->bits->mask;
	    drow = data;

	    while (ycount-- > 0)
	    {
		xcount = (src->bits->width + 7) / 8;
		sptr = srow; mptr = mrow;
		dptr = drow;

		while (xcount-- > 0)
		{
		    uint8_t s, m;
		    int i;

		    s = *sptr++; m = *mptr++;
		    for (i = 0; i < 8; i++)
		    {
#if BITMAP_BIT_ORDER == MSBFirst
			if (m & 128)
			    *dptr++ = (s & 128) ? fg_color : bg_color;
			else
			    *dptr++ = 0;
			s <<= 1; m <<= 1;
#else
			if (m & 1)
			    *dptr++ = (s & 1) ? fg_color : bg_color;
			else
			    *dptr++ = 0;
			s >>= 1; m >>= 1;
#endif
		    }
		}

		srow += BitmapBytePad (src->bits->width);
		mrow += BitmapBytePad (src->bits->width);
		drow = (uint32_t *) ((char *) drow + rowbytes);
	    }
	}
	else
	{
	    memset (data, 0, src->bits->height * rowbytes);
	}
    }

    err = xp_set_cursor (width, height, hot_x, hot_y, data, rowbytes);
    return err == Success;
}

/* Convert the X cursor representation to native format if possible. */
static Bool
QuartzRealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    if(pCursor == NULL || pCursor->bits == NULL)
        return FALSE;

    /* FIXME: cache ARGB8888 representation? */

    return TRUE;
}

/* Free the storage space associated with a realized cursor. */
static Bool
QuartzUnrealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    return TRUE;
}

/* Set the cursor sprite and position. */
static void
QuartzSetCursor (ScreenPtr pScreen, CursorPtr pCursor, int x, int y)
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    if (!quartzServerVisible)
        return;

    if (pCursor == NULL)
    {
	if (ScreenPriv->CursorVisible)
	{
	    xp_hide_cursor ();
	    ScreenPriv->CursorVisible = FALSE;
	}
    }
    else
    {
	load_cursor (pCursor, pScreen->myNum);

	if (!ScreenPriv->CursorVisible)
	{
	    xp_show_cursor ();
	    ScreenPriv->CursorVisible = TRUE;
	}
    }
}

/* Move the cursor. This is a noop for us. */
static void
QuartzMoveCursor (ScreenPtr pScreen, int x, int y)
{
}

static miPointerSpriteFuncRec quartzSpriteFuncsRec = {
    QuartzRealizeCursor,
    QuartzUnrealizeCursor,
    QuartzSetCursor,
    QuartzMoveCursor
};

static Bool
QuartzCursorOffScreen (ScreenPtr *pScreen, int *x, int *y)
{
    return FALSE;
}

static void
QuartzCrossScreen (ScreenPtr pScreen, Bool entering)
{
    return;
}

/* Change the cursor position without generating an event or motion history.
   The input coordinates (x,y) are in pScreen-local X11 coordinates. */
static void
QuartzWarpCursor (ScreenPtr pScreen, int x, int y)
{
    if (!movedCursor)
    {
        /* Don't move the cursor the first time. This is the
	   jump-to-center initialization, and it's annoying. */

        movedCursor = TRUE;
        return;
    }

    if (quartzServerVisible)
    {
	int sx, sy;

	sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
	sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;

	CGWarpMouseCursorPosition (CGPointMake (sx + x, sy + y));
    }

    miPointerWarpCursor (pScreen, x, y);
}

void
QuartzIgnoreNextWarpCursor (void)
{
    movedCursor = FALSE;
}

static miPointerScreenFuncRec quartzScreenFuncsRec = {
    QuartzCursorOffScreen,
    QuartzCrossScreen,
    QuartzWarpCursor,
    DarwinEnqueuePointerEvent,
};

/* Handle queries for best cursor size */
static void
QuartzCursorQueryBestSize (int class, unsigned short *width,
			   unsigned short *height, ScreenPtr pScreen)
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    if (class == CursorShape)
    {
	/* FIXME: query window server? */
        *width = 32;
        *height = 32;
    }
    else
    {
        (*ScreenPriv->QueryBestSize) (class, width, height, pScreen);
    }
}

/* Initialize cursor support */
Bool
QuartzInitCursor (ScreenPtr pScreen)
{
    QuartzCursorScreenPtr ScreenPriv;
    miPointerScreenPtr PointPriv;

    /* initialize software cursor handling (always needed as backup) */
    if (!miDCInitialize (pScreen, &quartzScreenFuncsRec))
        return FALSE;

    /* allocate private storage for this screen's QuickDraw cursor info */
    if (darwinCursorGeneration != serverGeneration)
    {
        if ((darwinCursorScreenIndex = AllocateScreenPrivateIndex ()) < 0)
            return FALSE;

        darwinCursorGeneration = serverGeneration;
    }

    ScreenPriv = xcalloc (1, sizeof(QuartzCursorScreenRec));
    if (ScreenPriv == NULL)
	return FALSE;

    CURSOR_PRIV (pScreen) = ScreenPriv;

    /* override some screen procedures */
    ScreenPriv->QueryBestSize = pScreen->QueryBestSize;
    pScreen->QueryBestSize = QuartzCursorQueryBestSize;

    PointPriv = (miPointerScreenPtr) pScreen->devPrivates[miPointerScreenIndex].ptr;

    ScreenPriv->spriteFuncs = PointPriv->spriteFuncs;
    PointPriv->spriteFuncs = &quartzSpriteFuncsRec;

    ScreenPriv->CursorVisible = TRUE;
    return TRUE;
}

/* X server is hiding. Restore the Aqua cursor. */
void
QuartzSuspendXCursor (ScreenPtr pScreen)
{
}

/* X server is showing. Restore the X cursor. */
void
QuartzResumeXCursor (ScreenPtr pScreen)
{
    WindowPtr pWin;
    CursorPtr pCursor;
    int x, y;

    pWin = GetSpriteWindow ();
    if (pWin->drawable.pScreen != pScreen)
	return;

    pCursor = GetSpriteCursor ();
    if (pCursor == NULL)
	return;

    GetSpritePosition (&x, &y);
    QuartzSetCursor (pScreen, pCursor, x, y);
}
