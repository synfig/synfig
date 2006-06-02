/* darwin-keyboard.c -- Keyboard support for the Darwin X Server

   Copyright (c) 2001-2002 Torrey T. Lyons. All Rights Reserved.
   Copyright (c) 2003 Apple Computer, Inc. All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
     3. The name of the author may not be used to endorse or promote products
        derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* $XFree86: xc/programs/Xserver/hw/darwin/darwinKeyboard.c,v 1.16 2002/03/28 02:21:08 torrey Exp $ */


/* An X keyCode must be in the range XkbMinLegalKeyCode (8) to
   XkbMaxLegalKeyCode(255).

   The keyCodes we get from the kernel range from 0 to 127, so we need to
   offset the range before passing the keyCode to X.

   An X KeySym is an extended ascii code that is device independent.

   The modifier map is accessed by the keyCode, but the normal map is
   accessed by keyCode - MIN_KEYCODE.  Sigh. */


/* Define this to get a diagnostic output to stderr which is helpful
   in determining how the X server is interpreting the Darwin keymap. */
#undef DUMP_DARWIN_KEYMAP

/* Define this to use Alt for Mode_switch. */
#define ALT_IS_MODE_SWITCH 1

#include "darwin.h"
#include "darwin-keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include "darwin.h"
#include "quartz-audio.h"

/* For NX_ constants */
#include <IOKit/hidsystem/IOLLEvent.h>
#include <IOKit/hidsystem/ev_keymap.h>

#include "keysym.h"

static darwin_keyboard_info info;

static void
DarwinChangeKeyboardControl (DeviceIntPtr device, KeybdCtrl *ctrl)
{
    /* keyclick, bell volume / pitch, autorepeat, LED's */
}

/* Use the key_map field of INFO to populate the mod_map and
   modifier_keycodes fields */
static void
build_modifier_maps (darwin_keyboard_info *info)
{
    int i;
    KeySym *k;

    memset (info->mod_map, NoSymbol, sizeof (info->mod_map));
    memset (info->modifier_keycodes, 0, sizeof (info->modifier_keycodes));

    for (i = 0; i < NUM_KEYCODES; i++)
    {
	k = info->key_map + i * GLYPHS_PER_KEY;

	switch (k[0])
	{
	case XK_Shift_L:
	    info->modifier_keycodes[NX_MODIFIERKEY_SHIFT][0] = i;
	    info->mod_map[MIN_KEYCODE + i] = ShiftMask;
	    break;

	case XK_Shift_R:
	    info->modifier_keycodes[NX_MODIFIERKEY_SHIFT][1] = i;
	    info->mod_map[MIN_KEYCODE + i] = ShiftMask;
	    break;

	case XK_Control_L:
	    info->modifier_keycodes[NX_MODIFIERKEY_CONTROL][0] = i;
	    info->mod_map[MIN_KEYCODE + i] = ControlMask;
	    break;

	case XK_Control_R:
	    info->modifier_keycodes[NX_MODIFIERKEY_CONTROL][1] = i;
	    info->mod_map[MIN_KEYCODE + i] = ControlMask;
	    break;

	case XK_Caps_Lock:
	    info->modifier_keycodes[NX_MODIFIERKEY_ALPHALOCK][0] = i;
	    info->mod_map[MIN_KEYCODE + i] = LockMask;
	    break;

	case XK_Alt_L:
	    info->modifier_keycodes[NX_MODIFIERKEY_ALTERNATE][0] = i;
	    info->mod_map[MIN_KEYCODE + i] = Mod1Mask;
	    break;

	case XK_Alt_R:
	    info->modifier_keycodes[NX_MODIFIERKEY_ALTERNATE][1] = i;
	    info->mod_map[MIN_KEYCODE + i] = Mod1Mask;
	    break;

	case XK_Mode_switch:
	    info->mod_map[MIN_KEYCODE + i] = Mod1Mask;
	    break;

	case XK_Meta_L:
	    info->modifier_keycodes[NX_MODIFIERKEY_COMMAND][0] = i;
	    info->mod_map[MIN_KEYCODE + i] = Mod2Mask;
	    break;

	case XK_Meta_R:
	    info->modifier_keycodes[NX_MODIFIERKEY_COMMAND][1] = i;
	    info->mod_map[MIN_KEYCODE + i] = Mod2Mask;
	    break;

	case XK_Num_Lock:
	    info->mod_map[MIN_KEYCODE + i] = Mod3Mask;
	    break;    
	}

	if (darwinSwapAltMeta)
	{
	    switch (k[0])
	    {
	    case XK_Alt_L:
		k[0] = XK_Meta_L; break;
	    case XK_Alt_R:
		k[0] = XK_Meta_R; break;
	    case XK_Meta_L:
		k[0] = XK_Alt_L; break;
	    case XK_Meta_R:
		k[0] = XK_Alt_R; break;
	    }
	}

#if ALT_IS_MODE_SWITCH
	if (k[0] == XK_Alt_L || k[0] == XK_Alt_R)
	    k[0] = XK_Mode_switch;
#endif
    }
}

static void
load_keyboard_mapping (KeySymsRec *keysyms)
{
    memset (info.key_map, 0, sizeof (info.key_map));

    if (darwinKeymapFile == NULL
	|| !DarwinParseKeymapFile (&info))
    {
	/* Load the system keymapping. */

	DarwinReadSystemKeymap (&info);
    }

    build_modifier_maps (&info);

#ifdef DUMP_DARWIN_KEYMAP
    ErrorF("Darwin -> X converted keyboard map\n");
    for (i = 0, k = map; i < NX_NUMKEYCODES; i++, k += GLYPHS_PER_KEY) {
        int j;
        ErrorF("0x%02x:", i);
        for (j = 0; j < GLYPHS_PER_KEY; j++) {
            if (k[j] == NoSymbol) {
                ErrorF("\tNoSym");
            } else {
                ErrorF("\t0x%x", k[j]);
            }
        }
        ErrorF("\n");
    }
#endif

    keysyms->map = info.key_map;
    keysyms->mapWidth = GLYPHS_PER_KEY;
    keysyms->minKeyCode = MIN_KEYCODE;
    keysyms->maxKeyCode = MAX_KEYCODE;
}

/* Get the Darwin keyboard map and compute an equivalent X keyboard map
   and modifier map. Set the new keyboard device structure. */
void
DarwinKeyboardInit (DeviceIntPtr pDev)
{
    KeySymsRec keysyms;
    BellProcPtr bellProc;

    load_keyboard_mapping (&keysyms);

    /* Initialize the seed, so we don't reload the keymap unnecessarily
       (and possibly overwrite xinitrc changes) */
    DarwinSystemKeymapSeed ();

    bellProc = QuartzBell;

    InitKeyboardDeviceStruct ((DevicePtr) pDev, &keysyms, info.mod_map,
			      bellProc, DarwinChangeKeyboardControl);
}

/* Borrowed from dix/devices.c */
static Bool
InitModMap(register KeyClassPtr keyc)
{
    int i, j;
    CARD8 keysPerModifier[8];
    CARD8 mask;

    if (keyc->modifierKeyMap != NULL)
	xfree (keyc->modifierKeyMap);

    keyc->maxKeysPerModifier = 0;
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		if (++keysPerModifier[j] > keyc->maxKeysPerModifier)
		    keyc->maxKeysPerModifier = keysPerModifier[j];
	    }
	}
    }
    keyc->modifierKeyMap = (KeyCode *)xalloc(8*keyc->maxKeysPerModifier);
    if (!keyc->modifierKeyMap && keyc->maxKeysPerModifier)
	return (FALSE);
    bzero((char *)keyc->modifierKeyMap, 8*(int)keyc->maxKeysPerModifier);
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		keyc->modifierKeyMap[(j*keyc->maxKeysPerModifier) +
				     keysPerModifier[j]] = i;
		keysPerModifier[j]++;
	    }
	}
    }
    return TRUE;
}

void
DarwinKeyboardReload (DeviceIntPtr pDev)
{
    KeySymsRec keysyms;

    load_keyboard_mapping (&keysyms);

    if (SetKeySymsMap (&pDev->key->curKeySyms, &keysyms))
    {
	/* now try to update modifiers. */

	memmove (pDev->key->modifierMap, info.mod_map, MAP_LENGTH);
	InitModMap (pDev->key);
    }

    SendMappingNotify (MappingKeyboard, MIN_KEYCODE, NUM_KEYCODES, 0);
    SendMappingNotify (MappingModifier, 0, 0, 0);
}

/* Return the keycode for an NX_MODIFIERKEY_* modifier. side = 0 for left
   or 1 for right. Returns 0 if key+side is not a known modifier. */
int
DarwinModifierNXKeyToNXKeycode (int key, int side)
{
    return info.modifier_keycodes[key][side];
}

/* This allows the ddx layer to prevent some keys from being remapped
   as modifier keys. */
Bool
LegalModifier (unsigned int key, DevicePtr pDev)
{
    return 1;
}
