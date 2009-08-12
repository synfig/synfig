/* X11Controller.m -- connect the IB ui, also the NSApp delegate
   $Id: X11Controller.m,v 1.36 2003/07/24 17:52:29 jharper Exp $

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
#define __DARWIN__

#import "X11Controller.h"
#import "X11Application.h"
#import <Carbon/Carbon.h>


/* ouch! */
#define BOOL X_BOOL
# include "Xproto.h"
#define WindowPtr X_WindowPtr
#define Cursor X_Cursor
# include "quartz.h"
# define _APPLEWM_SERVER_
# include "applewm.h"
# include "X.h"
#undef Cursor
#undef WindowPtr
#undef BOOL

#include <stdio.h>
#include <unistd.h>

#define TRACE() fprintf (stderr, "%s\n", __FUNCTION__)

@implementation X11Controller

- (void) awakeFromNib
{
    X11Application *xapp = NSApp;
    NSArray *array;

    /* Point X11Application at ourself. */
    [xapp set_controller:self];
#if 0
    array = [xapp prefs_get_array:@PREFS_APPSMENU];
    if (array != nil)
    {
	int count;

	/* convert from [TITLE1 COMMAND1 TITLE2 COMMAND2 ...]
	   to [[TITLE1 COMMAND1] [TITLE2 COMMAND2] ...] format. */

	count = [array count];
	if (count > 0
	    && ![[array objectAtIndex:0] isKindOfClass:[NSArray class]])
	{
	    int i;
	    NSMutableArray *copy, *sub;

	    copy = [NSMutableArray arrayWithCapacity:(count / 2)];

	    for (i = 0; i < count / 2; i++)
	    {
		sub = [[NSMutableArray alloc] initWithCapacity:3];
		[sub addObject:[array objectAtIndex:i*2]];
		[sub addObject:[array objectAtIndex:i*2+1]];
		[sub addObject:@""];
		[copy addObject:sub];
		[sub release];
	    }

	    array = copy;
	}

	[self set_apps_menu:array];
    }
	#endif
}

- (void) item_selected:sender
{
    [NSApp activateIgnoringOtherApps:YES];

    QuartzMessageMainThread (kXquartzControllerNotify, 2,
			     AppleWMWindowMenuItem, [sender tag]);
}

- (void) remove_window_menu
{
    NSMenu *menu;
    int first, count, i;

    /* Work backwards so we don't mess up the indices */
    menu = [window_separator menu];
    first = [menu indexOfItem:window_separator] + 1;
    count = [menu numberOfItems];
    for (i = count - 1; i >= first; i--)
	[menu removeItemAtIndex:i];

    menu = [dock_window_separator menu];
    count = [menu indexOfItem:dock_window_separator];
    for (i = 0; i < count; i++)
	[dock_menu removeItemAtIndex:0];
}

- (void) install_window_menu:(NSArray *)list
{
    NSMenu *menu;
    NSMenuItem *item;
    int first, count, i;

    menu = [window_separator menu];
    first = [menu indexOfItem:window_separator] + 1;
    count = [list count];

    for (i = 0; i < count; i++)
    {
	NSString *name, *shortcut;

	name = [[list objectAtIndex:i] objectAtIndex:0];
	shortcut = [[list objectAtIndex:i] objectAtIndex:1];

	item = (NSMenuItem *) [menu addItemWithTitle:name action:@selector
			       (item_selected:) keyEquivalent:shortcut];
	[item setTarget:self];
	[item setTag:i];
	[item setEnabled:YES];

	item = (NSMenuItem *) [dock_menu insertItemWithTitle:name
			       action:@selector
			       (item_selected:) keyEquivalent:shortcut
			       atIndex:i];
	[item setTarget:self];
	[item setTag:i];
	[item setEnabled:YES];
    }

    if (checked_window_item >= 0 && checked_window_item < count)
    {
	item = (NSMenuItem *) [menu itemAtIndex:first + checked_window_item];
	[item setState:NSOnState];
	item = (NSMenuItem *) [dock_menu itemAtIndex:checked_window_item];
	[item setState:NSOnState];
    }
}

- (void) remove_apps_menu
{
    NSMenu *menu;
    NSMenuItem *item;
    int i;

    if (apps == nil || apps_separator == nil)
	return;

    menu = [apps_separator menu];

    if (menu != nil)
    {
	for (i = [menu numberOfItems] - 1; i >= 0; i--)
	{
	    item = (NSMenuItem *) [menu itemAtIndex:i];
	    if ([item tag] != 0)
		[menu removeItemAtIndex:i];
	}
    }

    if (dock_apps_menu != nil)
    {
	for (i = [dock_apps_menu numberOfItems] - 1; i >= 0; i--)
	{
	    item = (NSMenuItem *) [dock_apps_menu itemAtIndex:i];
	    if ([item tag] != 0)
		[dock_apps_menu removeItemAtIndex:i];
	}
    }

    [apps release];
    apps = nil;
}

- (void) prepend_apps_item:(NSArray *)list index:(int)i menu:(NSMenu *)menu
{
    NSString *title, *shortcut = @"";
    NSArray *group;
    NSMenuItem *item;

    group = [list objectAtIndex:i];
    title = [group objectAtIndex:0];
    if ([group count] >= 3)
	shortcut = [group objectAtIndex:2];

    if ([title length] != 0)
    {
	item = (NSMenuItem *) [menu insertItemWithTitle:title
			       action:@selector (app_selected:)
			       keyEquivalent:shortcut atIndex:0];
	[item setTarget:self];
	[item setEnabled:YES];
    }
    else
    {
	item = (NSMenuItem *) [NSMenuItem separatorItem];
	[menu insertItem:item atIndex:0];
    }

    [item setTag:i+1];			/* can't be zero, so add one */
}

- (void) install_apps_menu:(NSArray *)list
{
    NSMenu *menu;
    int i, count;

    count = [list count];

    if (count == 0 || apps_separator == nil)
	return;

    menu = [apps_separator menu];

    for (i = count - 1; i >= 0; i--)
    {
	if (menu != nil)
	    [self prepend_apps_item:list index:i menu:menu];
	if (dock_apps_menu != nil)
	    [self prepend_apps_item:list index:i menu:dock_apps_menu];
    }

    apps = [list retain];
}

- (void) set_window_menu:(NSArray *)list
{
    [self remove_window_menu];
    [self install_window_menu:list];

    QuartzMessageMainThread (kXquartzControllerNotify, 1,
			     AppleWMWindowMenuNotify);
}

- (void) set_window_menu_check:(NSNumber *)nn
{
    NSMenu *menu;
    NSMenuItem *item;
    int first, count;
    int n = [nn intValue];

    menu = [window_separator menu];
    first = [menu indexOfItem:window_separator] + 1;
    count = [menu numberOfItems] - first;

    if (checked_window_item >= 0 && checked_window_item < count)
    {
	item = (NSMenuItem *) [menu itemAtIndex:first + checked_window_item];
	[item setState:NSOffState];
	item = (NSMenuItem *) [dock_menu itemAtIndex:checked_window_item];
	[item setState:NSOffState];
    }
    if (n >= 0 && n < count)
    {
	item = (NSMenuItem *) [menu itemAtIndex:first + n];
	[item setState:NSOnState];
	item = (NSMenuItem *) [dock_menu itemAtIndex:n];
	[item setState:NSOnState];
    }
    checked_window_item = n;
}

- (void) set_apps_menu:(NSArray *)list
{
    [self remove_apps_menu];
    [self install_apps_menu:list];
}

- (void) launch_client:(NSString *)command
{
    QuartzRunClient ([command cString]);
}

- (void) app_selected:sender
{
    int tag;
    NSString *item;

    tag = [sender tag] - 1;
    if (apps == nil || tag < 0 || tag >= [apps count])
	return;

    item = [[apps objectAtIndex:tag] objectAtIndex:1];

    [self launch_client:item];
}

- (IBAction) apps_table_show:sender
{
    NSArray *columns;

    if (table_apps == nil)
    {
	table_apps = [[NSMutableArray alloc] initWithCapacity:1];

	if (apps != nil)
	    [table_apps addObjectsFromArray:apps];
    }

    columns = [apps_table tableColumns];
    [[columns objectAtIndex:0] setIdentifier:@"0"];
    [[columns objectAtIndex:1] setIdentifier:@"1"];
    [[columns objectAtIndex:2] setIdentifier:@"2"];

    [apps_table setDataSource:self];
    [apps_table selectRow:0 byExtendingSelection:NO];

    [[apps_table window] makeKeyAndOrderFront:sender];
}

- (IBAction) apps_table_cancel:sender
{
    [[apps_table window] orderOut:sender];

    [table_apps release];
    table_apps = nil;
}

- (IBAction) apps_table_done:sender
{
    [apps_table deselectAll:sender];	/* flush edits? */

    [self remove_apps_menu];
    [self install_apps_menu:table_apps];

    [NSApp prefs_set_array:@PREFS_APPSMENU value:table_apps];
    [NSApp prefs_synchronize];

    [[apps_table window] orderOut:sender];

    [table_apps release];
    table_apps = nil;
}

- (IBAction) apps_table_new:sender
{
    NSMutableArray *item;

    int row = [apps_table selectedRow], i;

    if (row < 0)
	row = 0;
    else
	row = row + 1;

    i = row;
    if (i > [table_apps count])
	return;				/* avoid exceptions */

    item = [[NSMutableArray alloc] initWithCapacity:3];
    [item addObject:@""];
    [item addObject:@""];
    [item addObject:@""];

    [table_apps insertObject:item atIndex:i];
    [item release];

    [apps_table noteNumberOfRowsChanged];
    [apps_table selectRow:row byExtendingSelection:NO];
}

- (IBAction) apps_table_duplicate:sender
{
    int row = [apps_table selectedRow], i;
    NSObject *a;

    if (row < 0)
    {
	[self apps_table_new:sender];
	return;
    }

    i = row;
    if (i > [table_apps count] - 1)
	return;				/* avoid exceptions */

    a = [table_apps objectAtIndex:i];
    [table_apps insertObject:[a copy] atIndex:i];

    [apps_table noteNumberOfRowsChanged];
    [apps_table selectRow:row+1 byExtendingSelection:NO];
}

- (IBAction) apps_table_delete:sender
{
    int row = [apps_table selectedRow];

    if (row >= 0)
    {
	int i = row;

	if (i > [table_apps count] - 1)
	    return;			/* avoid exceptions */

	[table_apps removeObjectAtIndex:i];
    }

    [apps_table noteNumberOfRowsChanged];
}

- (int) numberOfRowsInTableView:(NSTableView *)tableView
{
    if (table_apps == nil)
	return 0;

    return [table_apps count];
}

- (id) tableView:(NSTableView *)tableView
       objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSArray *item;
    int col;

    if (table_apps == nil)
	return nil;

    col = [[tableColumn identifier] intValue];

    item = [table_apps objectAtIndex:row];
    if ([item count] > col)
	return [item objectAtIndex:col];
    else
	return @"";
}

- (void) tableView:(NSTableView *)tableView setObjectValue:(id)object
         forTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSMutableArray *item;
    int col;

    if (table_apps == nil)
	return;

    col = [[tableColumn identifier] intValue];

    item = [table_apps objectAtIndex:row];
    [item replaceObjectAtIndex:col withObject:object];
}

- (void) hide_window:sender
{
    if ([X11App x_active])
	QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMHideWindow);
    else
	NSBeep ();			/* FIXME: something here */
}

- (IBAction)bring_to_front:sender
{
    QuartzMessageMainThread (kXquartzControllerNotify, 1,
			     AppleWMBringAllToFront);
}

- (IBAction)close_window:sender
{
    if ([X11App x_active])
	QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMCloseWindow);
    else
	[[NSApp keyWindow] performClose:sender];
}

- (IBAction)minimize_window:sender
{
    if ([X11App x_active])
	QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMMinimizeWindow);
    else
	[[NSApp keyWindow] performMiniaturize:sender];
}

- (IBAction)zoom_window:sender
{
    if ([X11App x_active])
	QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMZoomWindow);
    else
	[[NSApp keyWindow] performZoom:sender];
}

- (IBAction) next_window:sender
{
    QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMNextWindow);
}

- (IBAction) previous_window:sender
{
    QuartzMessageMainThread (kXquartzControllerNotify,
			     1, AppleWMPreviousWindow);
}

- (IBAction) enable_fullscreen_changed:sender
{
    int value = ![enable_fullscreen intValue];

    QuartzMessageMainThread (kXquartzSetRootless, 1, value);

    [NSApp prefs_set_boolean:@PREFS_ROOTLESS value:value];
    [NSApp prefs_synchronize];
}

- (IBAction) toggle_fullscreen:sender
{
    QuartzMessageMainThread (kXquartzToggleFullscreen, 0);
}

- (void) set_can_quit:(BOOL)state
{
    can_quit = state;
}

- (IBAction)prefs_changed:sender
{
    darwinFakeButtons = [fake_buttons intValue];
    quartzUseSysBeep = [use_sysbeep intValue];
    X11EnableKeyEquivalents = [enable_keyequivs intValue];
    darwinSyncKeymap = [sync_keymap intValue];

    /* after adding prefs here, also add to [X11Application read_defaults]
       and below */

    [NSApp prefs_set_boolean:@PREFS_FAKEBUTTONS value:darwinFakeButtons];
    [NSApp prefs_set_boolean:@PREFS_SYSBEEP value:quartzUseSysBeep];
    [NSApp prefs_set_boolean:@PREFS_KEYEQUIVS value:X11EnableKeyEquivalents];
    [NSApp prefs_set_boolean:@PREFS_SYNC_KEYMAP value:darwinSyncKeymap];
    [NSApp prefs_set_boolean:@PREFS_NO_AUTH value:![enable_auth intValue]];
    [NSApp prefs_set_boolean:@PREFS_NO_TCP value:![enable_tcp intValue]];
    [NSApp prefs_set_integer:@PREFS_DEPTH value:[depth selectedTag]];

    [NSApp prefs_synchronize];
}

- (IBAction) prefs_show:sender
{
    [fake_buttons setIntValue:darwinFakeButtons];
    [use_sysbeep setIntValue:quartzUseSysBeep];
    [enable_keyequivs setIntValue:X11EnableKeyEquivalents];
    [sync_keymap setIntValue:darwinSyncKeymap];
    [sync_keymap setEnabled:darwinKeymapFile == NULL];

    [enable_auth setIntValue:![NSApp prefs_get_boolean:@PREFS_NO_AUTH default:NO]];
    [enable_tcp setIntValue:![NSApp prefs_get_boolean:@PREFS_NO_TCP default:NO]];
    [depth selectItemAtIndex:[depth indexOfItemWithTag:[NSApp prefs_get_integer:@PREFS_DEPTH default:-1]]];

    [enable_fullscreen setIntValue:!quartzEnableRootless];

    [prefs_panel makeKeyAndOrderFront:sender];
}

- (IBAction) quit:sender
{
    QuartzMessageMainThread (kXdarwinQuit, 0);
}

- (IBAction) x11_help:sender
{
    AHLookupAnchor (CFSTR ("Mac Help"), CFSTR ("mchlp2276"));
}

- (BOOL) validateMenuItem:(NSMenuItem *)item
{
    NSMenu *menu = [item menu];

    if (item == toggle_fullscreen_item)
    {
	return !quartzEnableRootless;
    }
    else if (menu == [window_separator menu] || menu == dock_menu
	     || (menu == [x11_about_item menu] && [item tag] == 42))
    {
	return (AppleWMSelectedEvents () & AppleWMControllerNotifyMask) != 0;
    }
    else
    {
	return TRUE;
    }
}

- (void) applicationDidHide:(NSNotification *)notify
{
    QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMHideAll);
}

- (void) applicationDidUnhide:(NSNotification *)notify
{
    QuartzMessageMainThread (kXquartzControllerNotify, 1, AppleWMShowAll);
}

- (NSApplicationTerminateReply) applicationShouldTerminate:sender
{
    NSString *msg;

    if (can_quit || [X11App prefs_get_boolean:@PREFS_NO_QUIT_ALERT default:NO])
    {
	return NSTerminateNow;
    }

    /* Make sure we're frontmost. */
    [NSApp activateIgnoringOtherApps:YES];

    msg = NSLocalizedString (@"Are you sure you want to quit X11?\n\n\
If you quit X11, any X11 applications you are running will stop immediately \
and you will lose any changes you have not saved.", @"");

    /* FIXME: safe to run the alert in here? Or should we return Later
       and then run the alert on a timer? It seems to work here, so.. */

    return (NSRunAlertPanel (nil, msg, NSLocalizedString (@"Quit", @""),
			     NSLocalizedString (@"Cancel", @""), nil)
	    == NSAlertDefaultReturn) ? NSTerminateNow : NSTerminateCancel;
}

- (void) applicationWillTerminate:(NSNotification *)aNotification
{
    [X11App prefs_synchronize];

    /* shutdown the X server, it will exit () for us. */
    QuartzMessageMainThread (kXdarwinQuit, 0);

    /* In case it doesn't, exit anyway after a while. */
    while (sleep (10) != 0) ;
    exit (1);
}

- (void) server_ready
{
    x_list *node;

    finished_launching = YES;

    for (node = pending_apps; node != NULL; node = node->next)
    {
	NSString *filename = node->data;
	QuartzRunClient ([filename UTF8String]);
	[filename release];
    }

    x_list_free (pending_apps);
    pending_apps = NULL;
}

- (BOOL) application:(NSApplication *)app openFile:(NSString *)filename
{
#if 0
    const char *name = [filename UTF8String];

    if (finished_launching)
	QuartzRunClient (name);
    else if (name[0] != ':')		/* ignore display names */
	pending_apps = x_list_prepend (pending_apps, [filename retain]);

    /* FIXME: report failures. */
    return YES;
#endif
	return NO;
}

@end

void X11ControllerMain (int argc, const char *argv[],
			void (*server_thread) (void *), void *server_arg)
{
    X11ApplicationMain (argc, argv, server_thread, server_arg);
}
