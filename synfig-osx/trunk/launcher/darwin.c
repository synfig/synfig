/**************************************************************
 *
 * Shared code for the Darwin X Server
 * running with Quartz or the IOKit
 *
 **************************************************************/
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
/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.c,v 1.45 2002/03/28 02:21:08 torrey Exp $ */

#include <CoreGraphics/CoreGraphics.h>

#include "X.h"
#include "Xproto.h"
#include "os.h"
#include "servermd.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "mibstore.h"		// mi backing store implementation
#include "mipointer.h"		// mi software cursor
#include "micmap.h"		// mi colormap code
#include "fb.h"			// fb framebuffer code
#include "site.h"
#include "globals.h"
#include "xf86Version.h"
#include "dix.h"
#include "dri-surface.h"
#define _APPLEDRI_SERVER_
#include "appledristr.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/syslimits.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#define NO_CFPLUGIN
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>

#include "darwin.h"
#include "quartz.h"
#include "rootless-common.h"
#include "pseudoramiX.h"
#include "X11Application.h"

/* Fake button press/release for scroll wheel move. */
#define SCROLLWHEELUPFAKE	4
#define SCROLLWHEELDOWNFAKE	5

/* X server shared global variables */
int darwinScreensFound = 0;
int darwinScreenIndex = 0;
int darwinFakeButtons = 1;
Bool darwinSwapAltMeta = FALSE;

/* location of X11's (0,0) point in global screen coordinates */
int darwinMainScreenX = 0;
int darwinMainScreenY = 0;

/* parameters read from the command line or user preferences */
char *darwinKeymapFile;
Bool darwinSyncKeymap = TRUE;

/* modifier masks for faking mouse buttons */
static int darwinFakeMouse2Mask = Mod1Mask;	/* option */
static int darwinFakeMouse3Mask = Mod2Mask;	/* command */

static DeviceIntPtr darwinPointer;
static DeviceIntPtr darwinKeyboard;

/* Track our view of the keyboard state. Everything we sent to dix will
   be represented here until released. */
static CARD8 keysDown[DOWN_LENGTH];
static int lockMods;

#define SetBit(ptr,bit) \
    do {((BYTE *) ptr)[(bit) >> 3] |= (1 << ((bit) & 7));} while (0)

#define ClearBit(ptr,bit) \
    do {((BYTE *) ptr)[(bit) >> 3] &= ~(1 << ((bit) & 7));} while (0)

/* Common pixmap formats */
static PixmapFormatRec formats[] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   32,     BITMAP_SCANLINE_PAD },
        { 32,   32,     BITMAP_SCANLINE_PAD }
};
const int NUMFORMATS = sizeof(formats)/sizeof(formats[0]);

#ifndef OSNAME
#define OSNAME " Mac OS X"
#endif
#ifndef OSVENDOR
#define OSVENDOR " Apple"
#endif
#ifndef PRE_RELEASE
#define PRE_RELEASE XF86_VERSION_SNAP
#endif

extern void AppleDRIExtensionInit(void);
extern void AppleWMExtensionInit(void);

static void
DarwinPrintBanner (void)
{
  ErrorF("\nXFree86 Version %d.%d.%d", XF86_VERSION_MAJOR, XF86_VERSION_MINOR,
                                    XF86_VERSION_PATCH);
#if XF86_VERSION_SNAP > 0
  ErrorF(".%d", XF86_VERSION_SNAP);
#endif

#if XF86_VERSION_SNAP >= 900
  ErrorF(" (%d.%d.0 RC %d)", XF86_VERSION_MAJOR, XF86_VERSION_MINOR + 1,
				XF86_VERSION_SNAP - 900);
#endif

#ifdef XF86_CUSTOM_VERSION
  ErrorF(" (%s)", XF86_CUSTOM_VERSION);
#endif
  ErrorF(" / X Window System\n");
  ErrorF("(protocol Version %d, revision %d, vendor release %d)\n",
         X_PROTOCOL, X_PROTOCOL_REVISION, VENDOR_RELEASE );
}

/* X screensaver support. Not implemented. */
static Bool
DarwinSaveScreen (ScreenPtr pScreen, int on)
{
    return TRUE;
}

/* This is a callback from dix during AddScreen() from InitOutput().
   Initialize the screen and communicate information about it back to dix. */
static Bool
DarwinAddScreen (int index, ScreenPtr pScreen, int argc, char **argv)
{
    int         i, dpi;
    static int  foundIndex = 0;
    Bool        ret;
    VisualPtr   visual;
    DarwinFramebufferPtr dfb;

    /* reset index of found screens for each server generation */
    if (index == 0)
	foundIndex = 0;

    /* allocate space for private per screen storage */
    dfb = xalloc (sizeof (DarwinFramebufferRec));
    SCREEN_PRIV(pScreen) = dfb;

    /* setup hardware/mode specific details */
    ret = QuartzAddScreen (foundIndex, pScreen);
    foundIndex++;
    if (!ret)
        return FALSE;

    /* reset the visual list */
    miClearVisualTypes();

    /* setup a single visual appropriate for our pixel type. Note: we use
       TrueColor, not DirectColor */
    if (dfb->componentCount != 1)
    {
	if (!miSetVisualTypes (dfb->colorBitsPerPixel, TrueColorMask,
			       dfb->bitsPerComponent, TrueColor))
	    return FALSE;
#ifdef ENABLE_PSEUDOCOLOR
	/* FIXME: currently we can't handle pseudocolor windows
	   inside truecolor top-level windows, so disable this. */
	if (!miSetVisualTypes (8, PseudoColorMask, 8, PseudoColor))
	    return FALSE;
#endif
    }
    else
    {
	if (!miSetVisualTypes (8, PseudoColorMask, 8, PseudoColor))
	    return FALSE;
    }

    /* create the common 8 bit PseudoColor visual; do this last to prevent
       it becoming the root visual. */

    miSetPixmapDepths();

    /* Machine independent screen init */
    if (monitorResolution)
        dpi = monitorResolution;
    else
        dpi = 75;

    /* initialize fb */
    if (!fbScreenInit (pScreen, dfb->framebuffer, dfb->width,
		       dfb->height, dpi, dpi,
		       dfb->pitch/(dfb->bitsPerPixel/8), dfb->bitsPerPixel))
    {
        return FALSE;
    }

    /* set the RGB order correctly for TrueColor */
    if (dfb->bitsPerPixel > 8)
    {
	int bitsPerRGB = dfb->bitsPerComponent;

        for (i = 0, visual = pScreen->visuals;
            i < pScreen->numVisuals; i++, visual++)
	{
            if (visual->class == TrueColor) {
                visual->offsetRed = bitsPerRGB * 2;
                visual->offsetGreen = bitsPerRGB;
                visual->offsetBlue = 0;
                visual->redMask = ((1<<bitsPerRGB)-1) << visual->offsetRed;
                visual->greenMask = ((1<<bitsPerRGB)-1) << visual->offsetGreen;
                visual->blueMask = ((1<<bitsPerRGB)-1) << visual->offsetBlue;
            }
        }
    }

#ifdef RENDER
    if (! fbPictureInit (pScreen, 0, 0))
        return FALSE;
#endif

#ifdef MITSHM
    ShmRegisterFbFuncs (pScreen);
#endif

    /* this must be initialized (why doesn't X have a default?) */
    pScreen->SaveScreen = DarwinSaveScreen;

    /* finish mode dependent screen setup including cursor support */
    if (!QuartzSetupScreen (index, pScreen))
	return FALSE;

    /* create and install the default colormap and set black / white pixels */
    if (!miCreateDefColormap (pScreen))
        return FALSE;

    dixScreenOrigins[index].x = dfb->x;
    dixScreenOrigins[index].y = dfb->y;

    ErrorF("Screen %d added: %dx%d @ (%d,%d)\n",
            index, dfb->width, dfb->height, dfb->x, dfb->y);

    return TRUE;
}

/* Search for a file in the standard Library paths, which are (in order):

      ~/Library/              user specific
      /Library/               host specific
      /Network/Library/       LAN specific
      /System/Library/        OS specific

   A sub-path can be specified to search in below the various Library
   directories. Returns a new character string (owned by the caller)
   containing the full path to the first file found. */

/* Library search paths */
static const char *libraryPathList[] = {
    "",
    "/Network",
    "/System",
    NULL
};

char *
DarwinFindLibraryFile (const char *file, const char *pathext)
{
    char *home;
    char *fullPath;
    int i = 0;

    // Return the file name as is if it is already a fully qualified path.
    if (!access(file, F_OK)) {
        fullPath = xalloc(strlen(file)+1);
        strcpy(fullPath, file);
        return fullPath;
    }

    fullPath = xalloc(PATH_MAX);

    home = getenv("HOME");
    if (home) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", home, pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    while (libraryPathList[i]) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", libraryPathList[i++],
                 pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    xfree(fullPath);
    return NULL;
}

/* Press or release the given key, specified by NX keycode. xe must already
   have event time and mouse location filled in. pressed is KeyPress or
   KeyRelease. keycode is NX keycode without MIN_KEYCODE adjustment. */
static inline void
DarwinPressKeycode (xEvent *xe, int pressed, int keycode)
{
    if (pressed == KeyRelease && !BitIsOn (keysDown, keycode + MIN_KEYCODE))
    {
	/* Don't release keys that aren't pressed. It generates extra
	   KeyPress events instead of just discarding them. */

	return;
    }

    if (pressed == KeyPress)
	SetBit (keysDown, keycode + MIN_KEYCODE);
    else
	ClearBit (keysDown, keycode + MIN_KEYCODE);

    xe->u.u.type = pressed;
    xe->u.u.detail = keycode + MIN_KEYCODE;
    (darwinKeyboard->public.processInputProc) (xe, darwinKeyboard, 1);
}

/* Ensure that X's idea of what modifiers are down matches the real
   window server's. Do this by looking at what keys we previously sent
   X and deciding if they need to be released/toggled yet to make FLAGS
   become X's current modifier state. */
static void
DarwinUpdateModifiers (xEvent xe, unsigned int flags)
{
    static const struct {int mask; int nxkey;} pairs[] = {
	{ShiftMask, NX_MODIFIERKEY_SHIFT},
	{ControlMask, NX_MODIFIERKEY_CONTROL},
	{Mod1Mask, NX_MODIFIERKEY_ALTERNATE},
	{Mod2Mask, NX_MODIFIERKEY_COMMAND},
	{Mod3Mask, NX_MODIFIERKEY_SECONDARYFN}
    };

    int i, keycode;

    for (i = 0; i < (int) (sizeof (pairs) / sizeof (pairs[0])); i++)
    {
	keycode = DarwinModifierNXKeyToNXKeycode (pairs[i].nxkey, 0);

	if (keycode == 0)
	    continue;

	/* For each known modifier, sync up the state of the key X thinks
	   it's bound to and the real value of the flag. */

	if ((flags & pairs[i].mask)
	    && !BitIsOn (keysDown, keycode + MIN_KEYCODE))
	{
	    DarwinPressKeycode (&xe, KeyPress, keycode);
	}
	else if (!(flags & pairs[i].mask)
		 && BitIsOn (keysDown, keycode + MIN_KEYCODE))
	{
	    DarwinPressKeycode (&xe, KeyRelease, keycode);
	}
    }

    /* Do the same for Lock, but need both press and release to toggle it. */

    if ((flags ^ lockMods) & LockMask)
    {
	keycode = DarwinModifierNXKeyToNXKeycode (NX_MODIFIERKEY_ALPHALOCK, 0);

	if (keycode != 0)
	{
	    DarwinPressKeycode (&xe, KeyPress, keycode);
	    DarwinPressKeycode (&xe, KeyRelease, keycode);

	    lockMods ^= LockMask;
	}
    }
}

/* Release all non-modifier keys that we think are currently pressed.
   Usually this is done when X becomes inactive to avoid leaving keys
   stuck down when we become active again. Modifiers are handled separately
   in the function above. */
static void
DarwinReleaseKeys (void)
{
    KeyClassPtr keyc = darwinKeyboard->key;
    xEvent xe;
    int i, x, y;

    memset (&xe, 0, sizeof (xe));
    xe.u.keyButtonPointer.time = GetTimeInMillis ();
    xe.u.keyButtonPointer.state = darwinKeyboard->key->state;
    GetSpritePosition (&x, &y);
    xe.u.keyButtonPointer.rootX = x;
    xe.u.keyButtonPointer.rootY = y;
    
    for (i = 0; i < DOWN_LENGTH * 8; i++)
    {
	if (!keyc->modifierMap[i] && BitIsOn (keysDown, i))
	    DarwinPressKeycode (&xe, KeyRelease, i - MIN_KEYCODE);
    }
}

static int
parseModifierString (const char *str)
{
    if (strcasecmp (str, "shift") == 0)
	return ShiftMask;
    else if (strcasecmp (str, "control") == 0)
	return ControlMask;
    else if (strcasecmp (str, "option") == 0)
	return Mod1Mask;
    else if (strcasecmp (str, "command") == 0)
	return Mod2Mask;
    else if (strcasecmp (str, "fn") == 0)
	return Mod3Mask;
    else
	return 0;
}

/* Parse a list of modifier names and return a corresponding modifier mask */
static int
DarwinParseModifierList (const char *constmodifiers)
{
    int result, mask;
    char *modifiers, *modifier, *p;

    if (constmodifiers == NULL
	|| strlen (constmodifiers) == 0
	|| strcasecmp (constmodifiers, "none") == 0)
    {
	return 0;
    }

    modifiers = strdup (constmodifiers);
    p = modifiers;
    result = 0;

    while (p != NULL)
    {
	modifier = strsep (&p, " ,+&|/"); /* allow lots of separators */
	mask = parseModifierString (modifier);
	if (mask != 0)
	    result |= mask;
	else
	    ErrorF ("fakebuttons: Unknown modifier \"%s\"\n", modifier);
    }

    free (modifiers);
    return result;
}

void
DarwinSetFakeButtons (const char *mod2, const char *mod3)
{
    if (mod2 != NULL)
	darwinFakeMouse2Mask = DarwinParseModifierList (mod2);
    if (mod3 != NULL)
	darwinFakeMouse3Mask = DarwinParseModifierList (mod3);
}

/* Read and process events from the event pipe until it is empty. */
void
ProcessInputEvents (void)
{
    static int here_before = 0;

    /* last known modifier state */
    static unsigned int current_flags = 0;

    /* button number and modifier mask of currently pressed fake button */
    static int fake_button;
    static unsigned int fake_button_mask, fake_button_modifier;

    xEvent xe;

    if (!here_before)
    {
	X11ApplicationServerReady ();
	here_before = TRUE;
    }

    while (DarwinDequeueEvent (&xe))
    {
	unsigned int real_state;

	real_state = xe.u.keyButtonPointer.state;
	xe.u.keyButtonPointer.state |= fake_button_modifier;

	/* Filter event for faked mouse button presses. */
	if (darwinFakeButtons)
	{
	    switch (xe.u.u.type)
	    {
	    case ButtonPress:
		if (xe.u.u.detail != 1)
		    break;
		if ((xe.u.keyButtonPointer.state & darwinFakeMouse2Mask)
		    == darwinFakeMouse2Mask)
		{
		    fake_button = 2;
		    fake_button_modifier = Button2Mask;
		    fake_button_mask = darwinFakeMouse2Mask;
		    xe.u.u.detail = 2;
		}
		else if ((xe.u.keyButtonPointer.state & darwinFakeMouse3Mask)
			 == darwinFakeMouse3Mask)
		{
		    fake_button = 3;
		    fake_button_modifier = Button3Mask;
		    fake_button_mask = darwinFakeMouse3Mask;
		    xe.u.u.detail = 3;
		}
		break;

	    case ButtonRelease:
		if (fake_button != 0 && xe.u.u.detail == 1)
		    xe.u.u.detail = fake_button;
		break;
	    }
	}

	xe.u.keyButtonPointer.state &= ~fake_button_mask;

	switch (xe.u.u.type)
	{
	case 0: /* flags-changed */
	case KeyPress:
	    if (current_flags == 0
		&& darwinSyncKeymap && darwinKeymapFile == NULL)
	    {
		/* See if keymap has changed. */

		static unsigned int last_seed;
		unsigned int this_seed;

		this_seed = DarwinSystemKeymapSeed ();
		if (this_seed != last_seed)
		{
		    last_seed = this_seed;
		    DarwinKeyboardReload (darwinKeyboard);
		}
	    }
	    /* fall through */

	case KeyRelease:
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:

	    /* Initialize time field. */

	    xe.u.keyButtonPointer.time = GetTimeInMillis ();

	    /* Update X's idea of what modifiers are set. */

	    if (xe.u.keyButtonPointer.state != 0xffff
		&& current_flags != xe.u.keyButtonPointer.state)
	    {
		current_flags = xe.u.keyButtonPointer.state;
		DarwinUpdateModifiers (xe, current_flags);
	    }
	}

        switch (xe.u.u.type)
	{
	case 0:
	    break;

	case MotionNotify:
	    if (!quartzServerVisible)
	    {
		xp_window_id wid;

		/* Sigh. Need to check that we're really over one of
		   our windows. (We need to receive pointer events while
		   not in the foreground, and the only way to do that
		   right now is to ask for _all_ pointer events..) */

		wid = 0;
		xp_find_window (xe.u.keyButtonPointer.rootX,
				xe.u.keyButtonPointer.rootY, 0, &wid);
		if (wid == 0)
		    break;
	    }

	    /* Shift from global screen coordinates to coordinates
	       relative to the origin of the current screen. */

	    xe.u.keyButtonPointer.rootX -= darwinMainScreenX -
		    dixScreenOrigins[miPointerCurrentScreen()->myNum].x;
	    xe.u.keyButtonPointer.rootY -= darwinMainScreenY -
		    dixScreenOrigins[miPointerCurrentScreen()->myNum].y;

	    miPointerAbsoluteCursor (xe.u.keyButtonPointer.rootX,
				     xe.u.keyButtonPointer.rootY,
				     xe.u.keyButtonPointer.time);
	    break;

	case ButtonPress:
	case ButtonRelease:
	    darwinPointer->public.processInputProc (&xe, darwinPointer, 1);
	    break;

	case KeyPress:
	case KeyRelease:
	    DarwinPressKeycode (&xe, xe.u.u.type, xe.u.u.detail);
	    break;

	case ClientMessage:
	    /* Update server's current time, since we may generate
	       events, and it's nice if the timestamps are correct. */
	    currentTime.milliseconds = GetTimeInMillis ();

	    switch (xe.u.clientMessage.u.l.type)
	    {
	    case kXdarwinQuit:
		GiveUp (0);
		break;

	    case kXquartzDeactivate:
		DarwinReleaseKeys ();
		/* fall through */

	    default:
		if (xe.u.clientMessage.u.l.type >= kXquartzFirstEvent
		    && xe.u.clientMessage.u.l.type <= kXquartzLastEvent)
		{
		    QuartzClientMessage (&xe);
		}
		else
		{
		    ErrorF ("Unknown application defined event: %d.\n",
			    xe.u.clientMessage.u.l.longs0);
		}
		break;
	    }
	    break;

	default:
	    ErrorF("Unknown event caught: %d\n", xe.u.u.type);
	    break;
        }

	/* Filter event for faked mouse button releases. */
	if (fake_button != 0 && xe.u.u.type == ButtonRelease)
	{
	    current_flags |= (real_state & fake_button_mask);
	    DarwinUpdateModifiers (xe, current_flags);

	    fake_button = 0;
	    fake_button_modifier = 0;
	    fake_button_mask = 0;
	}
    }

    miPointerUpdate ();
}

void
DarwinEnqueuePointerEvent (xEvent *xe)
{
    darwinPointer->public.processInputProc (xe, darwinPointer, 1);
}

/* Register the keyboard and mouse devices */
void
InitInput (int argc, char **argv)
{
    DarwinInputInit ();

    darwinPointer = AddInputDevice(DarwinMouseProc, TRUE);
    RegisterPointerDevice( darwinPointer );

    darwinKeyboard = AddInputDevice(DarwinKeybdProc, TRUE);
    RegisterKeyboardDevice( darwinKeyboard );
}

void
DarwinAdjustScreenOrigins (ScreenInfo *pScreenInfo)
{
    int i, left, top;

    /* Shift all screens so the X11 (0, 0) coordinate is at the top
       left of the global screen coordinates.

       Screens can be arranged so the top left isn't on any screen, so
       instead use the top left of the leftmost screen as (0,0). This
       may mean some screen space is in -y, but it's better that (0,0)
       be onscreen, or else default xterms disappear. It's better that
       -y be used than -x, because when popup menus are forced
       "onscreen" by dumb window managers like twm, they'll shift the
       menus down instead of left, which still looks funny but is an
       easier target to hit. */

    left = dixScreenOrigins[0].x;
    top  = dixScreenOrigins[0].y;

    /* Find leftmost screen. If there's a tie, take the topmost of the two. */
    for (i = 1; i < pScreenInfo->numScreens; i++) {
        if (dixScreenOrigins[i].x < left  ||
            (dixScreenOrigins[i].x == left &&
             dixScreenOrigins[i].y < top))
        {
            left = dixScreenOrigins[i].x;
            top = dixScreenOrigins[i].y;
        }
    }

    darwinMainScreenX = left;
    darwinMainScreenY = top;

    /* Shift all screens so that there is a screen whose top left
       is at X11 (0,0) and at global screen coordinate
       (darwinMainScreenX, darwinMainScreenY). */

    if (darwinMainScreenX != 0 || darwinMainScreenY != 0) {
        for (i = 0; i < pScreenInfo->numScreens; i++) {
            dixScreenOrigins[i].x -= darwinMainScreenX;
            dixScreenOrigins[i].y -= darwinMainScreenY;
            ErrorF("Screen %d placed at X11 coordinate (%d,%d).\n",
                   i, dixScreenOrigins[i].x, dixScreenOrigins[i].y);
        }
    }
}

/* Initialize screenInfo for all actually accessible framebuffers.

   The display mode dependent code gets called three times. The mode
   specific InitOutput routines are expected to discover the number of
   potentially useful screens and cache routes to them internally.
   Inside DarwinAddScreen are two other mode specific calls. A mode
   specific AddScreen routine is called for each screen to actually
   initialize the screen with the ScreenPtr structure. After other
   screen setup has been done, a mode specific SetupScreen function can
   be called to finalize screen setup. */

void
InitOutput (ScreenInfo *pScreenInfo, int argc, char **argv)
{
    int i;
    static unsigned long generation = 0;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    /* List how we want common pixmap formats to be padded */
    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i = 0; i < NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

    /* Allocate private storage for each screen's Darwin specific info */
    if (generation != serverGeneration) {
        darwinScreenIndex = AllocateScreenPrivateIndex();
        generation = serverGeneration;
    }

    /* Discover screens and do mode specific initialization */
    QuartzInitOutput(argc, argv);

    for (i = 0; i < darwinScreensFound; i++)
        AddScreen( DarwinAddScreen, argc, argv );

    DarwinAdjustScreenOrigins (pScreenInfo);

    PseudoramiXExtensionInit (argc, argv);
    AppleDRIExtensionInit ();
    AppleWMExtensionInit ();

    DRIExtensionInit ();
}

void
OsVendorFatalError (void)
{
    ErrorF( "   OsVendorFatalError\n" );
}

void
OsVendorInit (void)
{
    if (serverGeneration == 1)
        DarwinPrintBanner();
}

/* Process device-dependent command line args. Returns 0 if argument is
   not device dependent, otherwise Count of number of elements of argv
   that are part of a device dependent commandline option. */
int
ddxProcessArgument (int argc, char *argv[], int i)
{
    int numDone;

    if ((numDone = QuartzProcessArgument( argc, argv, i )))
        return numDone;

    if ( !strcmp( argv[i], "-fakebuttons" ) ) {
        darwinFakeButtons = TRUE;
        ErrorF( "Faking a three button mouse\n" );
        return 1;
    }

    if ( !strcmp( argv[i], "-nofakebuttons" ) ) {
        darwinFakeButtons = FALSE;
        ErrorF( "Not faking a three button mouse\n" );
        return 1;
    }

    if (!strcmp( argv[i], "-fakemouse2" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse2 must be followed by a modifer list\n" );
        }
	darwinFakeMouse2Mask = DarwinParseModifierList(argv[i+1]);
        return 2;
    }

    if (!strcmp( argv[i], "-fakemouse3" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse3 must be followed by a modifer list\n" );
        }
	darwinFakeMouse3Mask = DarwinParseModifierList(argv[i+1]);
        return 2;
    }

    if ( !strcmp( argv[i], "-keymap" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-keymap must be followed by a filename\n" );
        }
        darwinKeymapFile = argv[i+1];
        return 2;
    }

    if ( !strcmp( argv[i], "-nokeymap" ) ) {
        darwinKeymapFile = NULL;
        return 1;
    }

    if ( !strcmp( argv[i], "+synckeymap" ) ) {
        darwinSyncKeymap = TRUE;
        return 1;
    }
    if ( !strcmp( argv[i], "-synckeymap" ) ) {
        darwinSyncKeymap = FALSE;
        return 1;
    }

    if (strcmp (argv[i], "-swapAltMeta") == 0) {
	darwinSwapAltMeta = TRUE;
	return 1;
    }

    if (!strcmp( argv[i], "-showconfig" ) || !strcmp( argv[i], "-version" )) {
        DarwinPrintBanner();
        exit(0);
    }

    /* XDarwinStartup uses this argument to indicate the IOKit X server
       should be started. Ignore it here. */
    if ( !strcmp( argv[i], "-iokit" ) ) {
        return 1;
    }

    return 0;
}

/* Print out correct use of device dependent commandline options.
   Maybe the user now knows what really to do ... */
void
ddxUseMsg (void)
{
    ErrorF("\n");
    ErrorF("\n");
    ErrorF("Device Dependent Usage:\n");
    ErrorF("\n");
    ErrorF("-depth <depth>         use <depth> bits per pixel. Options: 8, 15, 24\b\n");
    ErrorF("-fakebuttons           fake a 3 button mouse with Command and Option\n");
    ErrorF("-nofakebuttons\n");
    ErrorF("-fakemouse2 <keys>     fake middle mouse button with modifier keys\n");
    ErrorF("-fakemouse3 <keys>     fake right mouse button with modifier keys\n");
    ErrorF("                         e.g.: -fakemouse2 \"option,shift\"\n");
    ErrorF("-keymap <file>         read the keymap from <file>\n");
    ErrorF("-nokeymap\n");
    ErrorF("+synckeymap            synchronize X keymap with system keymap\n");
    ErrorF("-synckeymap            only set X keymap on server startup\n");
    ErrorF("-swapAltMeta           swap meaning of Alt and Meta modifiers\n");
    ErrorF("-version               show server version.\n");
    ErrorF("\n");
}

/* Device dependent cleanup. Called by dix before normal server death. */
void
ddxGiveUp (void)
{
    ErrorF( "Quitting XDarwin...\n" );
    QuartzGiveUp();
}

/* DDX - specific abort routine.  Called by AbortServer(). The attempt is
   made to restore all original setting of the displays. Also all devices
   are closed. */
void
AbortDDX (void)
{
    ErrorF( "   AbortDDX\n" );

    /* This is needed for a abnormal server exit, since the normal exit stuff
       MUST also be performed (i.e. the vt must be left in a defined state) */
    ddxGiveUp();
}

extern void GlxExtensionInit();
extern void GlxWrapInitVisuals(void *procPtr);
void DarwinGlxExtensionInit (void) { GlxExtensionInit (); }
void DarwinGlxWrapInitVisuals (void *ptr) { GlxWrapInitVisuals (ptr); }

#ifdef DPMSExtension
Bool
DPMSSupported (void)
{
    return FALSE;
}

void
DPMSSet (int level)
{
}

int
DPMSGet (int *level)
{
    return -1;
}
#endif

#ifdef DDXTIME
CARD32
GetTimeInMillis (void)
{
    extern void Microseconds ();
    UnsignedWide usec;

    /* This doesn't involve trapping into the kernel, unlike gettimeofday. */
    Microseconds (&usec);

    /* Should be good enough? (-2% error) */
    return (usec.hi << 22) | (usec.lo >> 10);
}
#endif
