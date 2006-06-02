/* X11Controller.h -- connect the IB ui
   $Id: X11Controller.h,v 1.21 2003/07/24 17:52:29 jharper Exp $

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

#ifndef X11CONTROLLER_H
#define X11CONTROLLER_H 1

#if __OBJC__

#import <Cocoa/Cocoa.h>
#include "x-list.h"

@interface X11Controller : NSObject
{
    NSPanel *prefs_panel;

    NSButton *fake_buttons;
    NSButton *enable_fullscreen;
    NSButton *use_sysbeep;
    NSButton *enable_keyequivs;
    NSButton *sync_keymap;
    NSButton *enable_auth;
    NSButton *enable_tcp;
    NSPopUpButton *depth;

    NSMenuItem *x11_about_item;
    NSMenuItem *window_separator;
    NSMenuItem *dock_window_separator;
    NSMenuItem *apps_separator;
    NSMenuItem *toggle_fullscreen_item;
    NSMenu *dock_apps_menu;
    NSTableView *apps_table;

    NSArray *apps;
    NSMutableArray *table_apps;

    NSMenu *dock_menu;

    int checked_window_item;
    x_list *pending_apps;

    BOOL finished_launching;
    BOOL can_quit;
}

- (void) set_window_menu:(NSArray *)list;
- (void) set_window_menu_check:(NSNumber *)n;
- (void) set_apps_menu:(NSArray *)list;
- (void) set_can_quit:(BOOL)state;
- (void) server_ready;

@end

#endif /* __OBJC__ */

extern void X11ControllerMain (int argc, const char *argv[],
			       void (*server_thread) (void *),
			       void *server_arg);

#endif /* X11CONTROLLER_H */
