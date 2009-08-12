/* darwin-new-keymap.c -- code to build a keymap from the system
   $Id: darwin-new-keymap.c,v 1.7 2003/02/21 22:33:19 jharper Exp $

   Copyright (c) 2003 Apple Computer, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
   HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name(s) of the above
   copyright holders shall not be used in advertising or otherwise to
   promote the sale, use or other dealings in this Software without
   prior written authorization. */


#define Cursor X_Cursor
# include "darwin-keyboard.h"
# include "keysym.h"
#undef Cursor

#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>

#include "keysym2ucs.h"

#define HACK_MISSING 1
#define HACK_KEYPAD 1

enum {
    MOD_COMMAND = 256,
    MOD_SHIFT = 512,
    MOD_OPTION = 2048,
    MOD_CONTROL = 4096,
};

#define UKEYSYM(u) ((u) | 0x01000000)

/* Table of keycode->keysym mappings we use to fallback on for important
   keys that are often not in the Unicode mapping. */

const static struct {
    unsigned short keycode;
    KeySym keysym;
} known_keys[] = {
    {55,  XK_Meta_L},
    {56,  XK_Shift_L},
    {57,  XK_Caps_Lock},
    {58,  XK_Alt_L},
    {59,  XK_Control_L},
    {60,  XK_Shift_R},
    {61,  XK_Alt_R},
    {62,  XK_Control_R},

    {122, XK_F1},
    {120, XK_F2},
    {99,  XK_F3},
    {118, XK_F4},
    {96,  XK_F5},
    {97,  XK_F6},
    {98,  XK_F7},
    {100, XK_F8},
    {101, XK_F9},
    {109, XK_F10},
    {103, XK_F11},
    {111, XK_F12},
    {105, XK_F13},
    {107, XK_F14},
    {113, XK_F15},
};

/* Table of keycode->old,new-keysym mappings we use to fixup the numeric
   keypad entries. */

const static struct {
    unsigned short keycode;
    KeySym normal, keypad;
} known_numeric_keys[] = {
    {65, XK_period, XK_KP_Decimal},
    {67, XK_asterisk, XK_KP_Multiply},
    {69, XK_plus, XK_KP_Add},
    {75, XK_slash, XK_KP_Divide},
    {76, 0x01000003, XK_KP_Enter},
    {78, XK_minus, XK_KP_Subtract},
    {81, XK_equal, XK_KP_Equal},
    {82, XK_0, XK_KP_0},
    {83, XK_1, XK_KP_1},
    {84, XK_2, XK_KP_2},
    {85, XK_3, XK_KP_3},
    {86, XK_4, XK_KP_4},
    {87, XK_5, XK_KP_5},
    {88, XK_6, XK_KP_6},
    {89, XK_7, XK_KP_7},
    {91, XK_8, XK_KP_8},
    {92, XK_9, XK_KP_9},
};

/* Table mapping normal keysyms to their dead equivalents.
   FIXME: all the unicode keysyms (apart from circumflex) were guessed. */

const static struct {
    KeySym normal, dead;
} dead_keys[] = {
    {XK_grave, XK_dead_grave},
    {XK_acute, XK_dead_acute},
    {XK_asciicircum, XK_dead_circumflex},
    {UKEYSYM (0x2c6), XK_dead_circumflex},	/* MODIFIER LETTER CIRCUMFLEX ACCENT */
    {XK_asciitilde, XK_dead_tilde},
    {UKEYSYM (0x2dc), XK_dead_tilde},		/* SMALL TILDE */
    {XK_macron, XK_dead_macron},
    {XK_breve, XK_dead_breve},
    {XK_abovedot, XK_dead_abovedot},
    {XK_diaeresis, XK_dead_diaeresis},
    {UKEYSYM (0x2da), XK_dead_abovering},	/* DOT ABOVE */
    {XK_doubleacute, XK_dead_doubleacute},
    {XK_caron, XK_dead_caron},
    {XK_cedilla, XK_dead_cedilla},
    {XK_ogonek, XK_dead_ogonek},
    {UKEYSYM (0x269), XK_dead_iota},		/* LATIN SMALL LETTER IOTA */
    {UKEYSYM (0x2ec), XK_dead_voiced_sound},	/* MODIFIER LETTER VOICING */
/*  {XK_semivoiced_sound, XK_dead_semivoiced_sound}, */
    {UKEYSYM (0x323), XK_dead_belowdot},	/* COMBINING DOT BELOW */
    {UKEYSYM (0x309), XK_dead_hook}, 		/* COMBINING HOOK ABOVE */
    {UKEYSYM (0x31b), XK_dead_horn},		/* COMBINING HORN */
};

unsigned int
DarwinSystemKeymapSeed (void)
{
    static unsigned int seed;

    static KeyboardLayoutRef last_key_layout;
    KeyboardLayoutRef key_layout;

    KLGetCurrentKeyboardLayout (&key_layout);

    if (key_layout != last_key_layout)
	seed++;

    last_key_layout = key_layout;

    return seed;
}

static inline UniChar
macroman2ucs (unsigned char c)
{
    /* Precalculated table mapping MacRoman-128 to Unicode. Generated
       by creating single element CFStringRefs then extracting the
       first character. */

    static const unsigned short table[128] = {
	0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1,
	0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
	0xea, 0xeb, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3,
	0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
	0x2020, 0xb0, 0xa2, 0xa3, 0xa7, 0x2022, 0xb6, 0xdf,
	0xae, 0xa9, 0x2122, 0xb4, 0xa8, 0x2260, 0xc6, 0xd8,
	0x221e, 0xb1, 0x2264, 0x2265, 0xa5, 0xb5, 0x2202, 0x2211,
	0x220f, 0x3c0, 0x222b, 0xaa, 0xba, 0x3a9, 0xe6, 0xf8,
	0xbf, 0xa1, 0xac, 0x221a, 0x192, 0x2248, 0x2206, 0xab,
	0xbb, 0x2026, 0xa0, 0xc0, 0xc3, 0xd5, 0x152, 0x153,
	0x2013, 0x2014, 0x201c, 0x201d, 0x2018, 0x2019, 0xf7, 0x25ca,
	0xff, 0x178, 0x2044, 0x20ac, 0x2039, 0x203a, 0xfb01, 0xfb02,
	0x2021, 0xb7, 0x201a, 0x201e, 0x2030, 0xc2, 0xca, 0xc1,
	0xcb, 0xc8, 0xcd, 0xce, 0xcf, 0xcc, 0xd3, 0xd4,
	0xf8ff, 0xd2, 0xda, 0xdb, 0xd9, 0x131, 0x2c6, 0x2dc,
	0xaf, 0x2d8, 0x2d9, 0x2da, 0xb8, 0x2dd, 0x2db, 0x2c7,
    };

    if (c < 128)
	return c;
    else
	return table[c - 128];
}

static KeySym
make_dead_key (KeySym in)
{
    int i;

    for (i = 0; i < sizeof (dead_keys) / sizeof (dead_keys[0]); i++)
    {
	if (dead_keys[i].normal == in)
	    return dead_keys[i].dead;
    }

    return in;
}

int
DarwinReadSystemKeymap (darwin_keyboard_info *info)
{
    KeyboardLayoutRef key_layout;
    const void *chr_data;
    int num_keycodes = NUM_KEYCODES;
    UInt32 keyboard_type = 0;
    int is_uchr, i, j;
    OSStatus err;
    KeySym *k;

    KLGetCurrentKeyboardLayout (&key_layout);
    KLGetKeyboardLayoutProperty (key_layout, kKLuchrData, &chr_data);

    if (chr_data != NULL)
    {
	is_uchr = 1;
	keyboard_type = LMGetKbdType ();
    }
    else
    {
	KLGetKeyboardLayoutProperty (key_layout, kKLKCHRData, &chr_data);

	if (chr_data == NULL)
	{
	    fprintf (stderr, "couldn't get uchr or kchr resource\n");
	    return FALSE;
	}

	is_uchr = 0;
	num_keycodes = 128;
    }    


    /* Scan the keycode range for the Unicode character that each
       key produces in the four shift states. Then convert that to
       an X11 keysym (which may just the bit that says "this is
       Unicode" if it can't find the real symbol.) */

    for (i = 0; i < num_keycodes; i++)
    {
	static const int mods[4] = {0, MOD_SHIFT, MOD_OPTION,
				    MOD_OPTION | MOD_SHIFT};

	k = info->key_map + i * GLYPHS_PER_KEY;

	for (j = 0; j < 4; j++)
	{
	    if (is_uchr)
	    {
		UniChar s[8];
		UniCharCount len;
		UInt32 dead_key_state, extra_dead;

		dead_key_state = 0;
		err = UCKeyTranslate (chr_data, i, kUCKeyActionDown,
				      mods[j] >> 8, keyboard_type, 0,
				      &dead_key_state, 8, &len, s);
		if (err != noErr)
		    continue;

		if (len == 0 && dead_key_state != 0)
		{
		    /* Found a dead key. Work out which one it is, but
		       remembering that it's dead. */

		    extra_dead = 0;
		    err = UCKeyTranslate (chr_data, i, kUCKeyActionDown,
					  mods[j] >> 8, keyboard_type,
					  kUCKeyTranslateNoDeadKeysMask,
					  &extra_dead, 8, &len, s);
		    if (err != noErr)
			continue;
		}

		if (len > 0 && s[0] != 0x0010)
		{
		    k[j] = ucs2keysym (s[0]);

		    if (dead_key_state != 0)
			k[j] = make_dead_key (k[j]);
		}
	    }
	    else
	    {
		UInt32 c, state = 0;
		UInt16 code;

		code = i | mods[j];
		c = KeyTranslate (chr_data, code, &state);

		/* Dead keys are only processed on key-down, so ask
		   to translate those events. When we find a dead key,
		   translating the matching key up event will give
		   us the actual dead character. */

		if (state != 0)
		{
		    UInt32 state2 = 0;
		    c = KeyTranslate (chr_data, code | 128, &state2);
		}

		/* Characters seem to be in MacRoman encoding. */

		if (c != 0 && c != 0x0010)
		{
		    k[j] = ucs2keysym (macroman2ucs (c & 255));

		    if (state != 0)
			k[j] = make_dead_key (k[j]);
		}
	    }
	}

	if (k[3] == k[2])
	    k[3] = NoSymbol;
	if (k[2] == k[1])
	    k[2] = NoSymbol;
	if (k[1] == k[0])
	    k[1] = NoSymbol;
	if (k[0] == k[2] && k[1] == k[3])
	    k[2] = k[3] = NoSymbol;
    }

    /* Fix up some things that are normally missing.. */

    if (HACK_MISSING)
    {
	for (i = 0; i < sizeof (known_keys) / sizeof (known_keys[0]); i++)
	{
	    k = info->key_map + known_keys[i].keycode * GLYPHS_PER_KEY;

	    if (k[0] == NoSymbol && k[1] == NoSymbol
		&& k[2] == NoSymbol && k[3] == NoSymbol)
	    {
		k[0] = known_keys[i].keysym;
	    }
	}
    }

    /* And some more things. We find the right symbols for the numeric
       keypad, but not the KP_ keysyms. So try to convert known keycodes. */

    if (HACK_KEYPAD)
    {
	for (i = 0; i < sizeof (known_numeric_keys)
	     / sizeof (known_numeric_keys[0]); i++)
	{
	    k = info->key_map + known_numeric_keys[i].keycode * GLYPHS_PER_KEY;

	    if (k[0] == known_numeric_keys[i].normal)
	    {
		k[0] = known_numeric_keys[i].keypad;
	    }
	}
    }

    return TRUE;
}
