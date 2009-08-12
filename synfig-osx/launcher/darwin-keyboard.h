/* darwin-keyboard.h
   $Id: darwin-keyboard.h,v 1.1 2003/01/22 01:54:10 jharper Exp $

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

#ifndef DARWIN_KEYBOARD_H
#define DARWIN_KEYBOARD_H 1

#include "X.h"

/* Each key can generate 4 glyphs. They are, in order:
   unshifted, shifted, modeswitch unshifted, modeswitch shifted */

#ifndef MIN_KEYCODE
# define MIN_KEYCODE 8
#endif

#define GLYPHS_PER_KEY  4
#define NUM_KEYCODES    248
#define MAX_KEYCODE     NUM_KEYCODES + MIN_KEYCODE - 1

typedef struct darwin_keyboard_info_struct darwin_keyboard_info;

struct darwin_keyboard_info_struct {
    unsigned char mod_map[MAX_KEYCODE+1];
    KeySym key_map[NUM_KEYCODES * GLYPHS_PER_KEY];
    unsigned char modifier_keycodes[32][2];
};

extern int DarwinReadSystemKeymap (darwin_keyboard_info *info);
extern int DarwinParseKeymapFile (darwin_keyboard_info *info);

#endif /* DARWIN_KEYBOARD_H */
