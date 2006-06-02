/* darwin-old-keymap.c -- ugly code we need to keep around for now

   Copyright (c) 2001-2002 Torrey T. Lyons. All Rights Reserved.

   The code to parse the Darwin keymap is derived from dumpkeymap.c
   by Eric Sunshine, which includes the following copyright:

   Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
   All rights reserved.
  
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

#include "darwin.h"
#include "darwin-keyboard.h"
#include <errno.h>
#include <sys/stat.h>
#include <drivers/event_status_driver.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <architecture/byte_order.h>  // For the NXSwap*

#define XK_TECHNICAL		// needed to get XK_Escape
#define XK_PUBLISHING
#include "keysym.h"

static  FILE *fref = NULL;
static  char *inBuffer = NULL;

// FIXME: It would be nice to support some of the extra keys in XF86keysym.h,
// at least the volume controls that now ship on every Apple keyboard.

#define UK(a)           NoSymbol	// unknown symbol

static KeySym const next_to_x[256] = {
	NoSymbol,	NoSymbol,	NoSymbol,	XK_KP_Enter,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_BackSpace,	XK_Tab,		XK_Linefeed,	NoSymbol,
	NoSymbol,	XK_Return,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	XK_Escape,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_space,	XK_exclam,	XK_quotedbl,	XK_numbersign,
	XK_dollar,	XK_percent,	XK_ampersand,	XK_apostrophe,
	XK_parenleft,	XK_parenright,	XK_asterisk,	XK_plus,
	XK_comma,	XK_minus,	XK_period,	XK_slash,
	XK_0,		XK_1,		XK_2,		XK_3,
	XK_4,		XK_5,		XK_6,		XK_7,
	XK_8,		XK_9,		XK_colon,	XK_semicolon,
	XK_less,	XK_equal,	XK_greater,	XK_question,
	XK_at,		XK_A,		XK_B,		XK_C,
	XK_D,		XK_E,		XK_F,		XK_G,
	XK_H,		XK_I,		XK_J,		XK_K,
	XK_L,		XK_M,		XK_N,		XK_O,
	XK_P,		XK_Q,		XK_R,		XK_S,
	XK_T,		XK_U,		XK_V,		XK_W,
	XK_X,		XK_Y,		XK_Z,		XK_bracketleft,
	XK_backslash,	XK_bracketright,XK_asciicircum,	XK_underscore,
	XK_grave,	XK_a,		XK_b,		XK_c,
	XK_d,		XK_e,		XK_f,		XK_g,
	XK_h,		XK_i,		XK_j,		XK_k,
	XK_l,		XK_m,		XK_n,		XK_o,
	XK_p,		XK_q,		XK_r,		XK_s,
	XK_t,		XK_u,		XK_v,		XK_w,
	XK_x,		XK_y,		XK_z,		XK_braceleft,
	XK_bar,		XK_braceright,	XK_asciitilde,	XK_BackSpace,
// 128
	NoSymbol,	XK_Agrave,	XK_Aacute,	XK_Acircumflex,
	XK_Atilde,	XK_Adiaeresis,	XK_Aring,	XK_Ccedilla,
	XK_Egrave,	XK_Eacute,	XK_Ecircumflex,	XK_Ediaeresis,
	XK_Igrave,	XK_Iacute,	XK_Icircumflex,	XK_Idiaeresis,
// 144
	XK_ETH,		XK_Ntilde,	XK_Ograve,	XK_Oacute,
	XK_Ocircumflex,	XK_Otilde,	XK_Odiaeresis,	XK_Ugrave,
	XK_Uacute,	XK_Ucircumflex,	XK_Udiaeresis,	XK_Yacute,
	XK_THORN,	XK_mu,		XK_multiply,	XK_division,
// 160
	XK_copyright,	XK_exclamdown,	XK_cent,	XK_sterling,
	UK(fraction),	XK_yen,		UK(fhook),	XK_section,
	XK_currency,	XK_rightsinglequotemark,
					XK_leftdoublequotemark,
							XK_guillemotleft,
	XK_leftanglebracket,
			XK_rightanglebracket,
					UK(filigature),	UK(flligature),
// 176
	XK_registered,	XK_endash,	XK_dagger,	XK_doubledagger,
	XK_periodcentered,XK_brokenbar,	XK_paragraph,	UK(bullet),
	XK_singlelowquotemark,
			XK_doublelowquotemark,
					XK_rightdoublequotemark,
							XK_guillemotright,
	XK_ellipsis,	UK(permille),	XK_notsign,	XK_questiondown,
// 192
	XK_onesuperior,	XK_dead_grave,	XK_dead_acute,	XK_dead_circumflex,
	XK_dead_tilde,	XK_dead_macron,	XK_dead_breve,	XK_dead_abovedot,
	XK_dead_diaeresis,
			XK_twosuperior,	XK_dead_abovering,
							XK_dead_cedilla,
	XK_threesuperior,
			XK_dead_doubleacute,
					XK_dead_ogonek,	XK_dead_caron,
// 208
	XK_emdash,	XK_plusminus,	XK_onequarter,	XK_onehalf,
	XK_threequarters,
			XK_agrave,	XK_aacute,	XK_acircumflex,
	XK_atilde,	XK_adiaeresis,	XK_aring,	XK_ccedilla,
	XK_egrave,	XK_eacute,	XK_ecircumflex,	XK_ediaeresis,
// 224
	XK_igrave,	XK_AE,		XK_iacute,	XK_ordfeminine,
	XK_icircumflex,	XK_idiaeresis,	XK_eth,		XK_ntilde,
	XK_Lstroke,	XK_Ooblique,	XK_OE,		XK_masculine,
	XK_ograve,	XK_oacute,	XK_ocircumflex, XK_otilde,
// 240
	XK_odiaeresis,	XK_ae,		XK_ugrave,	XK_uacute,
	XK_ucircumflex,	XK_idotless,	XK_udiaeresis,	XK_ygrave,
	XK_lstroke,	XK_ooblique,	XK_oe,		XK_ssharp,
	XK_thorn,	XK_ydiaeresis,	NoSymbol,	NoSymbol,
  };

#define MIN_SYMBOL		0xAC
static KeySym const symbol_to_x[] = {
    XK_Left,        XK_Up,          XK_Right,      XK_Down
  };
int const NUM_SYMBOL = sizeof(symbol_to_x) / sizeof(symbol_to_x[0]);

#define MIN_FUNCKEY		0x20
static KeySym const funckey_to_x[] = {
    XK_F1,          XK_F2,          XK_F3,          XK_F4,
    XK_F5,          XK_F6,          XK_F7,          XK_F8,
    XK_F9,          XK_F10,         XK_F11,         XK_F12,
    XK_Insert,      XK_Delete,      XK_Home,        XK_End,
    XK_Page_Up,     XK_Page_Down,   XK_F13,         XK_F14,
    XK_F15
  };
int const NUM_FUNCKEY = sizeof(funckey_to_x) / sizeof(funckey_to_x[0]);

typedef struct {
    KeySym		normalSym;
    KeySym		keypadSym;
} darwinKeyPad_t;

static darwinKeyPad_t const normal_to_keypad[] = {
    { XK_0,         XK_KP_0 },
    { XK_1,         XK_KP_1 },
    { XK_2,         XK_KP_2 },
    { XK_3,         XK_KP_3 },
    { XK_4,         XK_KP_4 },
    { XK_5,         XK_KP_5 },
    { XK_6,         XK_KP_6 },
    { XK_7,         XK_KP_7 },
    { XK_8,         XK_KP_8 },
    { XK_9,         XK_KP_9 },
    { XK_equal,     XK_KP_Equal },
    { XK_asterisk,  XK_KP_Multiply },
    { XK_plus,      XK_KP_Add },
    { XK_comma,     XK_KP_Separator },
    { XK_minus,     XK_KP_Subtract },
    { XK_period,    XK_KP_Decimal },
    { XK_slash,     XK_KP_Divide }
};
int const NUM_KEYPAD = sizeof(normal_to_keypad) / sizeof(normal_to_keypad[0]);


//-----------------------------------------------------------------------------
// Data Stream Object
//      Can be configured to treat embedded "numbers" as being composed of
//      either 1, 2, or 4 bytes, apiece.
//-----------------------------------------------------------------------------
typedef struct _DataStream
{
    unsigned char const *data;
    unsigned char const *data_end;
    short number_size;  // Size in bytes of a "number" in the stream.
} DataStream;

static DataStream* new_data_stream( unsigned char const* data, int size )
{
    DataStream* s = (DataStream*)xalloc( sizeof(DataStream) );
    s->data = data;
    s->data_end = data + size;
    s->number_size = 1; // Default to byte-sized numbers.
    return s;
}

static void destroy_data_stream( DataStream* s )
{
    xfree(s);
}

static unsigned char get_byte( DataStream* s )
{
    assert(s->data + 1 <= s->data_end);
    return *s->data++;
}

static short get_word( DataStream* s )
{
    short hi, lo;
    assert(s->data + 2 <= s->data_end);
    hi = *s->data++;
    lo = *s->data++;
    return ((hi << 8) | lo);
}

static int get_dword( DataStream* s )
{
    int b1, b2, b3, b4;
    assert(s->data + 4 <= s->data_end);
    b4 = *s->data++;
    b3 = *s->data++;
    b2 = *s->data++;
    b1 = *s->data++;
    return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}

static int get_number( DataStream* s )
{
    switch (s->number_size) {
        case 4:  return get_dword(s);
        case 2:  return get_word(s);
        default: return get_byte(s);
    }
}


//-----------------------------------------------------------------------------
// Utility functions to help parse Darwin keymap
//-----------------------------------------------------------------------------

/*
 * bits_set
 *      Calculate number of bits set in the modifier mask.
 */
static short bits_set( short mask )
{
    short n = 0;

    for ( ; mask != 0; mask >>= 1)
        if ((mask & 0x01) != 0)
            n++;
    return n;
}

/*
 * parse_next_char_code
 *      Read the next character code from the Darwin keymapping
 *      and write it to the X keymap.
 */
static void parse_next_char_code(
    DataStream  *s,
    KeySym      *k )
{
    const short charSet = get_number(s);
    const short charCode = get_number(s);

    if (charSet == 0) {                 // ascii character
        if (charCode >= 0 && charCode < 256)
            *k = next_to_x[charCode];
    } else if (charSet == 0x01) {       // symbol character
        if (charCode >= MIN_SYMBOL &&
            charCode <= MIN_SYMBOL + NUM_SYMBOL)
            *k = symbol_to_x[charCode - MIN_SYMBOL];
    } else if (charSet == 0xFE) {       // function key
        if (charCode >= MIN_FUNCKEY &&
            charCode <= MIN_FUNCKEY + NUM_FUNCKEY)
            *k = funckey_to_x[charCode - MIN_FUNCKEY];
    }
}

/*
 * DarwinReadKeymapFile
 *      Read the appropriate keymapping from a keymapping file.
 */
static Bool _DarwinReadKeymapFile(
    NXKeyMapping        *keyMap)
{
    struct stat         st;
    NXEventSystemDevice info[20];
    int                 interface = 0, handler_id = 0;
    int                 map_interface, map_handler_id, map_size = 0;
    unsigned int        i, size;
    int                 *bufferEnd;
    union km_tag {
        int             *intP;
        char            *charP;
    } km;
    char *filename;

    filename = DarwinFindLibraryFile (darwinKeymapFile, "Keyboards");
    if (filename == NULL) {
	FatalError("Could not find keymapping file %s.\n", darwinKeymapFile);
	return FALSE;
    }

    fref = fopen( filename, "rb" );
    xfree (filename);

    if (fref == NULL) {
        ErrorF("Unable to open keymapping file '%s' (errno %d).\n",
               darwinKeymapFile, errno);
        return FALSE;
    }
    if (fstat(fileno(fref), &st) == -1) {
        ErrorF("Could not stat keymapping file '%s' (errno %d).\n",
               darwinKeymapFile, errno);
        return FALSE;
    }

    // check to make sure we don't crash later
    if (st.st_size <= 16*sizeof(int)) {
        ErrorF("Keymapping file '%s' is invalid (too small).\n",
               darwinKeymapFile);
        return FALSE;
    }

    inBuffer = (char*) xalloc( st.st_size );
    bufferEnd = (int *) (inBuffer + st.st_size);
    if (fread(inBuffer, st.st_size, 1, fref) != 1) {
        ErrorF("Could not read %qd bytes from keymapping file '%s' (errno %d).\n",
               st.st_size, darwinKeymapFile, errno);
        return FALSE;
    }

    if (strncmp( inBuffer, "KYM1", 4 ) == 0) {
        // Magic number OK.
    } else if (strncmp( inBuffer, "KYMP", 4 ) == 0) {
        ErrorF("Keymapping file '%s' is intended for use with the original NeXT keyboards and cannot be used by XDarwin.\n", darwinKeymapFile);
        return FALSE;
    } else {
        ErrorF("Keymapping file '%s' has a bad magic number and cannot be used by XDarwin.\n", darwinKeymapFile);
        return FALSE;
    }

    // find the keyboard interface and handler id
    {NXEventHandle hid;
     NXEventSystemInfoType info_type;

     size = sizeof( info ) / sizeof( int );
     hid = NXOpenEventStatus ();
     info_type = NXEventSystemInfo (hid, NX_EVS_DEVICE_INFO,
				    (int *) info, &size);
     NXCloseEventStatus (hid);
     if (!info_type) {
	 ErrorF("Error reading event status driver info.\n");
	 return FALSE;
     }}

    size = size * sizeof( int ) / sizeof( info[0] );
    for( i = 0; i < size; i++) {
        if (info[i].dev_type == NX_EVS_DEVICE_TYPE_KEYBOARD) {
            Bool hasInterface = FALSE;
            Bool hasMatch = FALSE;

            interface = info[i].interface;
            handler_id = info[i].id;

            // Find an appropriate keymapping:
            // The first time we try to match both interface and handler_id.
            // If we can't match both, we take the first match for interface.

            do {
                km.charP = inBuffer;
                km.intP++;
                while (km.intP+3 < bufferEnd) {
                    map_interface = NXSwapBigIntToHost(*(km.intP++));
                    map_handler_id = NXSwapBigIntToHost(*(km.intP++));
                    map_size = NXSwapBigIntToHost(*(km.intP++));
                    if (map_interface == interface) {
                        if (map_handler_id == handler_id || hasInterface) {
                            hasMatch = TRUE;
                            break;
                        } else {
                            hasInterface = TRUE;
                        }
                    }
                    km.charP += map_size;
                }
            } while (hasInterface && !hasMatch);

            if (hasMatch) {
                // fill in NXKeyMapping structure
                keyMap->size = map_size;
                keyMap->mapping = (char*) xalloc(map_size);
                memcpy(keyMap->mapping, km.charP, map_size);
                return TRUE;
            }
        } // if dev_id == keyboard device
    } // foreach info struct

    // The keymapping file didn't match any of the info structs
    // returned by NXEventSystemInfo.
    ErrorF("Keymapping file '%s' did not contain appropriate keyboard interface.\n", darwinKeymapFile);
    return FALSE;
}

static Bool DarwinReadKeymapFile(NXKeyMapping *keyMap)
{
    Bool ret;

    ret = _DarwinReadKeymapFile (keyMap);

    if (inBuffer != NULL)
	xfree (inBuffer);
    if (fref != NULL)
	fclose (fref);

    return ret;
}

int DarwinParseKeymapFile (darwin_keyboard_info *info)
{
    short numMods, numKeys, numPadKeys = 0;
    NXKeyMapping keyMap;
    DataStream *keyMapStream;
    unsigned char const *numPadStart = 0;
    Bool haveKeymap;
    int i;
    KeySym *k;

    if (darwinKeymapFile == NULL)
	return FALSE;

    haveKeymap = DarwinReadKeymapFile(&keyMap);
    if (!haveKeymap) {
	ErrorF("Reverting to system keymapping.\n");
	return FALSE;
    }

    keyMapStream = new_data_stream( (unsigned char const*)keyMap.mapping,
                                    keyMap.size );

    // check the type of map
    if (get_word(keyMapStream)) {
        keyMapStream->number_size = 2;
        ErrorF("Current 16-bit keymapping may not be interpreted correctly.\n");
    }

    // Compute the modifier map and
    // insert X modifier KeySyms into keyboard map.
    // Store modifier keycodes in modifierKeycodes.
    numMods = get_number(keyMapStream);
    while (numMods-- > 0) {
        int             left = 1;               // first keycode is left
        short const     charCode = get_number(keyMapStream);
        short           numKeyCodes = get_number(keyMapStream);

        // This is just a marker, not a real modifier.
        // Store numeric keypad keys for later.
        if (charCode == NX_MODIFIERKEY_NUMERICPAD) {
            numPadStart = keyMapStream->data;
            numPadKeys = numKeyCodes;
        }

        while (numKeyCodes-- > 0) {
            const short keyCode = get_number(keyMapStream);
            if (charCode != NX_MODIFIERKEY_NUMERICPAD) {
                switch (charCode) {
                    case NX_MODIFIERKEY_ALPHALOCK:
                        info->key_map[keyCode * GLYPHS_PER_KEY] = XK_Caps_Lock;
                        break;
                    case NX_MODIFIERKEY_SHIFT:
                        info->key_map[keyCode * GLYPHS_PER_KEY] =
                                (left ? XK_Shift_L : XK_Shift_R);
                        break;
                    case NX_MODIFIERKEY_CONTROL:
                        info->key_map[keyCode * GLYPHS_PER_KEY] =
                                (left ? XK_Control_L : XK_Control_R);
                        break;
                    case NX_MODIFIERKEY_ALTERNATE:
                        info->key_map[keyCode * GLYPHS_PER_KEY] =
                                (left ? XK_Alt_L : XK_Alt_R);
                        break;
                    case NX_MODIFIERKEY_COMMAND:
                        info->key_map[keyCode * GLYPHS_PER_KEY] =
                                (left ? XK_Meta_L : XK_Meta_R);
                        break;
                    case NX_MODIFIERKEY_SECONDARYFN:
                        info->key_map[keyCode * GLYPHS_PER_KEY] =
                                (left ? XK_Control_L : XK_Control_R);
                        break;
                    case NX_MODIFIERKEY_HELP:
                        info->key_map[keyCode * GLYPHS_PER_KEY] = XK_Help;
                        break;
                }
            }
            left = 0;
        }
    }

    // Convert the Darwin keyboard map to an X keyboard map.
    // A key can have a different character code for each combination of
    // modifiers. We currently ignore all modifier combinations except
    // those with Shift, AlphaLock, and Alt.
    numKeys = get_number(keyMapStream);
    for (i = 0, k = info->key_map; i < numKeys; i++, k += GLYPHS_PER_KEY) {
        short const     charGenMask = get_number(keyMapStream);
        if (charGenMask != 0xFF) {              // is key bound?
            short       numKeyCodes = 1 << bits_set(charGenMask);

            // Record unmodified case
            parse_next_char_code( keyMapStream, k );
            numKeyCodes--;

            // If AlphaLock and Shift modifiers produce different codes,
            // we record the Shift case since X handles AlphaLock.
            if (charGenMask & 0x01) {		// AlphaLock
                parse_next_char_code( keyMapStream, k+1 );
                numKeyCodes--;
            }

            if (charGenMask & 0x02) {		// Shift
                parse_next_char_code( keyMapStream, k+1 );
                numKeyCodes--;

                if (charGenMask & 0x01) {	// Shift-AlphaLock
                    get_number(keyMapStream); get_number(keyMapStream);
                    numKeyCodes--;
                }
            }

            // Skip the Control cases
            if (charGenMask & 0x04) {		// Control
                get_number(keyMapStream); get_number(keyMapStream);
                numKeyCodes--;

                if (charGenMask & 0x01) {	// Control-AlphaLock
                    get_number(keyMapStream); get_number(keyMapStream);
                    numKeyCodes--;
                }

                if (charGenMask & 0x02) {	// Control-Shift
                    get_number(keyMapStream); get_number(keyMapStream);
                    numKeyCodes--;

                    if (charGenMask & 0x01) {	// Shift-Control-AlphaLock
                        get_number(keyMapStream); get_number(keyMapStream);
                        numKeyCodes--;
                    }
                }
            }

            // Process Alt cases
            if (charGenMask & 0x08) {		// Alt
                parse_next_char_code( keyMapStream, k+2 );
                numKeyCodes--;

                if (charGenMask & 0x01) {	// Alt-AlphaLock
                    parse_next_char_code( keyMapStream, k+3 );
                    numKeyCodes--;
                }

                if (charGenMask & 0x02) {	// Alt-Shift
                    parse_next_char_code( keyMapStream, k+3 );
                    numKeyCodes--;

                    if (charGenMask & 0x01) {	// Alt-Shift-AlphaLock
                        get_number(keyMapStream); get_number(keyMapStream);
                        numKeyCodes--;
                    }
                }
            }

            while (numKeyCodes-- > 0) {
                get_number(keyMapStream); get_number(keyMapStream);
            }

            if (k[3] == k[2]) k[3] = NoSymbol;
            if (k[2] == k[1]) k[2] = NoSymbol;
            if (k[1] == k[0]) k[1] = NoSymbol;
            if (k[0] == k[2] && k[1] == k[3]) k[2] = k[3] = NoSymbol;
        }
    }

    // Now we have to go back through the list of keycodes that are on the
    // numeric keypad and update the X keymap.
    keyMapStream->data = numPadStart;
    while(numPadKeys-- > 0) {
        const short keyCode = get_number(keyMapStream);
        k = &info->key_map[keyCode * GLYPHS_PER_KEY];
        for (i = 0; i < NUM_KEYPAD; i++) {
            if (*k == normal_to_keypad[i].normalSym) {
                k[0] = normal_to_keypad[i].keypadSym;
                break;
            }
        }
    }

    // free Darwin keyboard map
    destroy_data_stream( keyMapStream );
    xfree( keyMap.mapping );

    return TRUE;
}
