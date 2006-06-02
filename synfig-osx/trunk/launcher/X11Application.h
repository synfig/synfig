/* X11Application.h -- subclass of NSApplication to multiplex events
   $Id: X11Application.h,v 1.26 2003/08/08 19:16:13 jharper Exp $

   Copyright (c) 2002 Apple Computer, Inc. All rights reserved.

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

#ifndef X11APPLICATION_H
#define X11APPLICATION_H 1

#if __OBJC__

#import <Cocoa/Cocoa.h>
#import "X11Controller.h"

@interface X11Application : NSApplication {
    X11Controller *_controller;

    unsigned int _x_active :1;
}

- (void) set_controller:controller;
- (void) set_window_menu:(NSArray *)list;

- (int) prefs_get_integer:(NSString *)key default:(int)def;
- (const char *) prefs_get_string:(NSString *)key default:(const char *)def;
- (float) prefs_get_float:(NSString *)key default:(float)def;
- (int) prefs_get_boolean:(NSString *)key default:(int)def;
- (NSArray *) prefs_get_array:(NSString *)key;
- (void) prefs_set_integer:(NSString *)key value:(int)value;
- (void) prefs_set_float:(NSString *)key value:(float)value;
- (void) prefs_set_boolean:(NSString *)key value:(int)value;
- (void) prefs_set_array:(NSString *)key value:(NSArray *)value;
- (void) prefs_set_string:(NSString *)key value:(NSString *)value;
- (void) prefs_synchronize;

- (BOOL) x_active;

@end

extern X11Application *X11App;

#endif /* __OBJC__ */

extern void X11ApplicationSetWindowMenu (int nitems, const char **items,
					 const char *shortcuts);
extern void X11ApplicationSetWindowMenuCheck (int idx);
extern void X11ApplicationSetFrontProcess (void);
extern void X11ApplicationSetCanQuit (int state);
extern void X11ApplicationServerReady (void);
extern void X11ApplicationShowHideMenubar (int state);

extern void X11ApplicationMain (int argc, const char *argv[],
				void (*server_thread) (void *),
				void *server_arg);

extern int X11EnableKeyEquivalents;

#define APP_PREFS "com.apple.x11"

#define PREFS_APPSMENU		"apps_menu"
#define PREFS_FAKEBUTTONS	"enable_fake_buttons"
#define PREFS_SYSBEEP		"enable_system_beep"
#define PREFS_KEYEQUIVS		"enable_key_equivalents"
#define PREFS_KEYMAP_FILE	"keymap_file"
#define PREFS_SYNC_KEYMAP	"sync_keymap"
#define PREFS_DEPTH		"depth"
#define PREFS_NO_AUTH		"no_auth"
#define PREFS_NO_TCP		"nolisten_tcp"
#define PREFS_DONE_XINIT_CHECK	"done_xinit_check"
#define PREFS_NO_QUIT_ALERT	"no_quit_alert"
#define PREFS_FAKE_BUTTON2	"fake_button2"
#define PREFS_FAKE_BUTTON3	"fake_button3"
#define PREFS_ROOTLESS		"rootless"
#define PREFS_FULLSCREEN_HOTKEYS "fullscreen_hotkeys"
#define PREFS_SWAP_ALT_META	"swap_alt_meta"
#define PREFS_XP_OPTIONS	"xp_options"

#endif /* X11APPLICATION_H */
