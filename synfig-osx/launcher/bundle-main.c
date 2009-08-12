/* bundle-main.c -- X server launcher
   $Id: bundle-main.c,v 1.17 2003/09/11 00:17:10 jharper Exp $

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
   prior written authorization.

   Parts of this file are derived from xdm, which has this copyright:

   Copyright 1988, 1998  The Open Group

   Permission to use, copy, modify, distribute, and sell this software
   and its documentation for any purpose is hereby granted without fee,
   provided that the above copyright notice appear in all copies and
   that both that copyright notice and this permission notice appear in
   supporting documentation.

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of The Open Group shall
   not be used in advertising or otherwise to promote the sale, use or
   other dealings in this Software without prior written authorization
   from The Open Group. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <sys/ioctl.h>

#include <X11/Xlib.h>
#include <X11/Xauth.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

#define X_SERVER "/usr/X11R6/bin/Xquartz"
#define XTERM_PATH "/usr/X11R6/bin/xterm"
#define WM_PATH "/usr/X11R6/bin/quartz-wm"
#define DEFAULT_XINITRC "/usr/X11R6/lib/X11/xinit/xinitrc"
#include <mach-o/dyld.h>

/* what xinit does */
#ifndef SHELL
# define SHELL "sh"
#endif

#undef FALSE
#define FALSE 0
#undef TRUE
#define TRUE 1

#define MAX_DISPLAYS 64

static int server_pid = -1, client_pid = -1;
static int xinit_kills_server = FALSE;
static jmp_buf exit_continuation;
static const char *server_name = NULL;
static Display *server_dpy;

static char *auth_file;

typedef struct addr_list_struct addr_list;

struct addr_list_struct {
    addr_list *next;
    Xauth auth;
};

static addr_list *addresses;


/* Utility functions. */

/* Return the current host name. Matches what Xlib does. */
static char *
host_name (void)
{
#ifdef NEED_UTSNAME
    static struct utsname name;

    uname(&name);

    return name.nodename;
#else
    static char buf[100];

    gethostname(buf, sizeof(buf));

    return buf;
#endif
}

static int
read_boolean_pref (CFStringRef name, int default_)
{
    int value;
    Boolean ok;

    value = CFPreferencesGetAppBooleanValue (name,
					     CFSTR ("com.apple.x11"), &ok);
    return ok ? value : default_;
}

static inline int
binary_equal (const void *a, const void *b, int length)
{
    return memcmp (a, b, length) == 0;
}

static inline void *
binary_dup (const void *a, int length)
{
    void *b = malloc (length);
    if (b != NULL)
	memcpy (b, a, length);
    return b;
}

static inline void
binary_free (void *data, int length)
{
    if (data != NULL)
	free (data);
}


/* Functions for managing the authentication entries. */

/* Returns true if something matching AUTH is in our list of auth items */
static int
check_auth_item (Xauth *auth)
{
    addr_list *a;

    for (a = addresses; a != NULL; a = a->next)
    {
	if (a->auth.family == auth->family
	    && a->auth.address_length == auth->address_length
	    && binary_equal (a->auth.address, auth->address, auth->address_length)
	    && a->auth.number_length == auth->number_length
	    && binary_equal (a->auth.number, auth->number, auth->number_length)
	    && a->auth.name_length == auth->name_length
	    && binary_equal (a->auth.name, auth->name, auth->name_length))
	{
	    return TRUE;
	}
    }

    return FALSE;
}

/* Add one item to our list of auth items. */
static void
add_auth_item (Xauth *auth)
{
    addr_list *a = malloc (sizeof (addr_list));

    a->auth.family = auth->family;
    a->auth.address_length = auth->address_length;
    a->auth.address = binary_dup (auth->address, auth->address_length);
    a->auth.number_length = auth->number_length;
    a->auth.number = binary_dup (auth->number, auth->number_length);
    a->auth.name_length = auth->name_length;
    a->auth.name = binary_dup (auth->name, auth->name_length);
    a->auth.data_length = auth->data_length;
    a->auth.data = binary_dup (auth->data, auth->data_length);

    a->next = addresses;
    addresses = a;
}

/* Free all allocated auth items. */
static void
free_auth_items (void)
{
    addr_list *a;

    while ((a = addresses) != NULL)
    {
	addresses = a->next;

	binary_free (a->auth.address, a->auth.address_length);
	binary_free (a->auth.number, a->auth.number_length);
	binary_free (a->auth.name, a->auth.name_length);
	binary_free (a->auth.data, a->auth.data_length);
	free (a);
    }
}

/* Add the unix domain auth item. */
static void
define_local (Xauth *auth)
{
    char *host = host_name ();

#ifdef DEBUG
    fprintf (stderr, "x11: hostname is %s\n", host);
#endif

    auth->family = FamilyLocal;
    auth->address_length = strlen (host);
    auth->address = host;

    add_auth_item (auth);
}

/* Add the tcp auth item. */
static void
define_named (Xauth *auth, const char *name)
{
    struct ifaddrs *addrs, *ptr;

    if (getifaddrs (&addrs) != 0)
	return;

    for (ptr = addrs; ptr != NULL; ptr = ptr->ifa_next)
    {
	if (ptr->ifa_addr->sa_family != AF_INET)
	    continue;

	auth->family = FamilyInternet;
	auth->address_length = sizeof (struct sockaddr_in);
	auth->address = (char *) &(((struct sockaddr_in *) ptr->ifa_addr)->sin_addr);

#ifdef DEBUG
	fprintf (stderr, "x11: ipaddr is %d.%d.%d.%d\n",
		 (unsigned char) auth->address[0],
		 (unsigned char) auth->address[1],
		 (unsigned char) auth->address[2],
		 (unsigned char) auth->address[3]);
#endif

	add_auth_item (auth);
    }

    freeifaddrs (addrs);
}

/* Parse the display number from NAME and add it to AUTH. */
static void
set_auth_number (Xauth *auth, const char *name)
{
    char *colon;
    char *dot, *number;

    colon = strrchr(name, ':');
    if (colon != NULL)
    {
	colon++;
	dot = strchr(colon, '.');

	if (dot != NULL)
	    auth->number_length = dot - colon;
	else
	    auth->number_length = strlen (colon);

	number = malloc (auth->number_length + 1);
	if (number != NULL)
	{
	    strncpy (number, colon, auth->number_length);
	    number[auth->number_length] = '\0';
	}
	else
	{
	    auth->number_length = 0;
	}

	auth->number = number;
    }
}

/* Put 128 bits of random data into DATA. If possible, it will be "high
   quality" */
static int
generate_mit_magic_cookie (char data[16])
{
    int fd, ret, i;
    long *ldata = (long *) data;

    fd = open ("/dev/random", O_RDONLY);
    if (fd > 0)
    {
	ret = read (fd, data, 16);
	if (ret == 16)
	    return TRUE;

	close (fd);
    }

    /* fall back to the usual crappy rng */

    srand48 (getpid () ^ time (NULL));

    for (i = 0; i < 4; i++)
	ldata[i] = lrand48 ();

    return TRUE;
}

/* Create the keys we'll be using for the display named NAME. */
static int
make_auth_keys (const char *name)
{
    Xauth auth;
    char key[16];

    if (auth_file == NULL)
	return FALSE;

    auth.name = "MIT-MAGIC-COOKIE-1";
    auth.name_length = strlen (auth.name);

    if (!generate_mit_magic_cookie (key))
    {
	auth_file = NULL;
	return FALSE;
    }

    auth.data = key;
    auth.data_length = 16;

    set_auth_number (&auth, name);

    define_named (&auth, host_name ());
    define_local (&auth);

    free (auth.number);

    return TRUE;
}

/* If ADD-ENTRIES is true, merge our auth entries into the existing
   Xauthority file. If ADD-ENTRIES is false, remove our entries. */
static int
write_auth_file (int add_entries)
{
    char *home, newname[1024];
    int fd, ret;
    FILE *new_fh, *old_fh;
    addr_list *addr;
    Xauth *auth;

    if (auth_file == NULL)
	return FALSE;

    home = getenv ("HOME");
    if (home == NULL)
    {
	auth_file = NULL;
	return FALSE;
    }

    snprintf (newname, sizeof (newname), "%s/.XauthorityXXXXXX", home);
    mktemp (newname);

    if (XauLockAuth (auth_file, 1, 2, 10) != LOCK_SUCCESS)
    {
	/* FIXME: do something here? */

	auth_file = NULL;
	return FALSE;
    }

    fd = open (newname, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd >= 0)
    {
	new_fh = fdopen (fd, "w");
	if (new_fh != NULL)
	{
	    if (add_entries)
	    {
		for (addr = addresses; addr != NULL; addr = addr->next)
		{
		    XauWriteAuth (new_fh, &addr->auth);
		}
	    }

	    old_fh = fopen (auth_file, "r");
	    if (old_fh != NULL)
	    {
		while ((auth = XauReadAuth (old_fh)) != NULL)
		{
		    if (!check_auth_item (auth))
			XauWriteAuth (new_fh, auth);
		    XauDisposeAuth (auth);
		}
		fclose (old_fh);
	    }

	    fclose (new_fh);
	    unlink (auth_file);

	    ret = rename (newname, auth_file);

	    if (ret != 0)
		auth_file = NULL;

	    XauUnlockAuth (auth_file);
	    return ret == 0;
	}

	close (fd);
    }

    XauUnlockAuth (auth_file);
    auth_file = NULL;
    return FALSE;
}


/* Subprocess management functions. */

static int
start_server (char **xargv)
{
    int child;

    child = fork ();

    switch (child)
    {
    case -1:				/* error */
	perror ("fork");
	return FALSE;

    case 0:				/* child */
	execv (X_SERVER, xargv);
	perror ("Couldn't exec " X_SERVER);
	_exit (1);

    default:				/* parent */
	server_pid = child;
	return TRUE;
    }
}

static int
wait_for_server (void)
{
    int count = 100;

    while (count-- > 0)
    {
	int status;

	server_dpy = XOpenDisplay (server_name);
	if (server_dpy != NULL)
	    return TRUE;

	if (waitpid (server_pid, &status, WNOHANG) == server_pid)
	    return FALSE;

	sleep (1);
    }

    return FALSE;
}

static int
start_client (void)
{
    int child;

    child = fork ();

    switch (child)
    {
	char *tem, buf[1024];

    case -1:				/* error */
	perror ("fork");
	return FALSE;

    case 0:				/* child */
	/* cd $HOME */
	tem = getenv ("HOME");
	if (tem != NULL)
	    chdir (tem);

	/* Setup environment */

	setenv ("DISPLAY", server_name, TRUE);
	tem = getenv ("PATH");
	if (tem != NULL && tem[0] != NULL)
	    snprintf (buf, sizeof (buf), "%s:/usr/X11R6/bin", tem);
	else
	    snprintf (buf, sizeof (buf), "/bin:/usr/bin:/usr/X11R6/bin");
	setenv ("PATH", buf, TRUE);

#if 1
	setenv("GTK_USE_XFT","1",0);
	system(WM_PATH " &");
	execl("/usr/local/bin/synfigstudio","/usr/local/bin/synfigstudio",NULL);

#else
	/* First look for .xinitrc in user's home directory. */

	tem = getenv ("HOME");
	if (tem != NULL)
	{
	    snprintf (buf, sizeof (buf), "%s/.xinitrc", tem);
	    if (access (buf, R_OK) == 0)
		execlp (SHELL, SHELL, buf, NULL);
	}

	/* Then try the default xinitrc in the lib directory. */

	if (access (DEFAULT_XINITRC, R_OK) == 0)
	    execlp (SHELL, SHELL, DEFAULT_XINITRC, NULL);

	/* Then fallback to hardcoding an xterm and the window manager. */

	system (XTERM_PATH " &");
	execl (WM_PATH, WM_PATH, NULL);
	
#endif
	perror ("exec");
	_exit (1);

    default:				/* parent */
	client_pid = child;
	return TRUE;
    }
}

static void
sigchld_handler (int sig)
{
    int pid, status;

again:
    pid = waitpid (WAIT_ANY, &status, WNOHANG);

    if (pid > 0)
    {
	if (pid == server_pid)
	{
	    server_pid = -1;

	    if (client_pid >= 0)
		kill (client_pid, SIGTERM);
	}
	else if (pid == client_pid)
	{
	    client_pid = -1;

	    if (server_pid >= 0 && xinit_kills_server)
		kill (server_pid, SIGTERM);
	}
	goto again;
    }

    if (server_pid == -1 && client_pid == -1)
	longjmp (exit_continuation, 1);

    signal (SIGCHLD, sigchld_handler);
}


/* Server utilities. */

static Boolean
display_exists_p (int number)
{
    char buf[64];
    void *conn;
    char *fullname = NULL;
    int idisplay, iscreen;
    char *conn_auth_name, *conn_auth_data;
    int conn_auth_namelen, conn_auth_datalen;

    extern void *_X11TransConnectDisplay ();
    extern void _XDisconnectDisplay ();

    /* Since connecting to the display waits for a few seconds if the
       display doesn't exist, check for trivial non-existence - if the
       socket in /tmp exists or not.. (note: if the socket exists, the
       server may still not, so we need to try to connect in that case..) */

    sprintf (buf, "/tmp/.X11-unix/X%d", number);
    if (access (buf, F_OK) != 0)
	return FALSE;

    /* This is a private function that we shouldn't really be calling,
       but it's the best way to see if the server exists (without
       needing to hold the necessary authentication to use it) */

    sprintf (buf, ":%d", number);
    conn = _X11TransConnectDisplay (buf, &fullname, &idisplay, &iscreen,
				    &conn_auth_name, &conn_auth_namelen,
				    &conn_auth_data, &conn_auth_datalen);
    if (conn == NULL)
	return FALSE;

    _XDisconnectDisplay (conn);
    return TRUE;
}


/* Monitoring when the system's ip addresses change. */

static Boolean pending_timer;

static void
timer_callback (CFRunLoopTimerRef timer, void *info)
{
    pending_timer = FALSE;

    /* Update authentication names. Need to write .Xauthority file first
       without the existing entries, then again with the new entries.. */

    write_auth_file (FALSE);

    free_auth_items ();
    make_auth_keys (server_name);

    write_auth_file (TRUE);
}

/* This function is called when the system's ip addresses may have changed. */
static void
ipaddr_callback (SCDynamicStoreRef store, CFArrayRef changed_keys, void *info)
{
    if (auth_file != NULL && !pending_timer)
    {
	CFRunLoopTimerRef timer;

	timer = CFRunLoopTimerCreate (NULL, CFAbsoluteTimeGetCurrent () + 1.0,
				      0.0, 0, 0, timer_callback, NULL);
	CFRunLoopAddTimer (CFRunLoopGetCurrent (), timer,
			   kCFRunLoopDefaultMode);
	CFRelease (timer);

	pending_timer = TRUE;
    }
}

/* This code adapted from "Living in a Dynamic TCP/IP Environment" technote. */
static Boolean
install_ipaddr_source (void)
{
    CFRunLoopSourceRef source = NULL;

    SCDynamicStoreContext context = {0};
    SCDynamicStoreRef ref;

    ref = SCDynamicStoreCreate (NULL,
				CFSTR ("AddIPAddressListChangeCallbackSCF"),
				ipaddr_callback, &context);

    if (ref != NULL)
    {
	const void *keys[2];

	/* This should tell us when any IPV4 address changes */
	keys[0] = (SCDynamicStoreKeyCreateNetworkServiceEntity
		   (NULL, kSCDynamicStoreDomainState,
		    kSCCompAnyRegex, kSCEntNetIPv4));

	/* This should tell us when the hostname(s) change */
	keys[1] = SCDynamicStoreKeyCreateHostNames (NULL);

	if (keys[0] != NULL && keys[1] != NULL)
	{
	    CFArrayRef pattern_array;

	    pattern_array = CFArrayCreate (NULL, keys, 2,
					   &kCFTypeArrayCallBacks);

	    if (pattern_array != NULL)
	    {
		SCDynamicStoreSetNotificationKeys (ref, NULL, pattern_array);
		source = SCDynamicStoreCreateRunLoopSource (NULL, ref, 0);

		CFRelease (pattern_array);
	    }

	    if (keys[0] != NULL)
		CFRelease (keys[0]);
	    if (keys[1] != NULL)
		CFRelease (keys[1]);
	}

	CFRelease (ref); 
   }

    if (source != NULL)
    {
	CFRunLoopAddSource (CFRunLoopGetCurrent (),
			    source, kCFRunLoopDefaultMode);
	CFRelease (source);
    }

    return source != NULL;
}



/* Entrypoint. */
int
main (int argc, char **argv)
{
    char **xargv;
    int i, j;
    int fd;






    xargv = alloca (sizeof (char *) * (argc + 32));

    if (!read_boolean_pref (CFSTR ("no_auth"), FALSE))
	auth_file = XauFileName ();

    /* The standard X11 behaviour is for the server to quit when the first
       client exits. But it can be useful for debugging (and to mimic our
       behaviour in the beta releases) to not do that. */

    xinit_kills_server = read_boolean_pref (CFSTR ("xinit_kills_server"), TRUE);

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] == ':')
	    server_name = argv[i];
    }

    if (server_name == NULL)
    {
	static char name[8];

	/* No display number specified, so search for the first unused.

	   There's a big old race condition here if two servers start at
	   the same time, but that's fairly unlikely. We could create
	   lockfiles or something, but that's seems more likely to cause
	   problems than the race condition itself.. */

	for (i = 2; i < MAX_DISPLAYS; i++)
	{
	    if (!display_exists_p (i))
		break;
	}

	if (i == MAX_DISPLAYS)
	{
	    fprintf (stderr, "%s: couldn't allocate a display number", argv[0]);
	    exit (1);
	}

	sprintf (name, ":%d", i);
	server_name = name;
    }

    if (auth_file != NULL)
    {
	/* Create new Xauth keys and add them to the .Xauthority file */

	make_auth_keys (server_name);
	write_auth_file (TRUE);
    }

    /* Construct our new argv */

    i = j = 0;

    xargv[i++] = argv[j++];

    if (auth_file != NULL)
    {
	xargv[i++] = "-auth";
	xargv[i++] = auth_file;
    }

    /* By default, don't listen on tcp sockets if Xauth is disabled. */

    if (read_boolean_pref (CFSTR ("nolisten_tcp"), auth_file == NULL))
    {
	xargv[i++] = "-nolisten";
	xargv[i++] = "tcp";
    }

    while (j < argc)
    {
	if (argv[j++][0] != ':')
	    xargv[i++] = argv[j-1];
    }

    xargv[i++] = (char *) server_name;
    xargv[i++] = NULL;

    /* Detach from any controlling terminal and connect stdin to /dev/null */

#ifdef TIOCNOTTY
    fd = open ("/dev/tty", O_RDONLY);
    if (fd != -1)
    {
	ioctl (fd, TIOCNOTTY, 0);
	close (fd);
    }
#endif

    fd = open ("/dev/null", O_RDWR, 0);
    if (fd >= 0)
    {
	dup2 (fd, 0);
	if (fd > 0)
	    close (fd);
    }

    if (!start_server (xargv))
	return 1;

    if (!wait_for_server ())
    {
	kill (server_pid, SIGTERM);
	return 1;
    }

    if (!start_client ())
    {
	kill (server_pid, SIGTERM);
	return 1;
    }

    signal (SIGCHLD, sigchld_handler);

	if (NSIsSymbolNameDefined("_ASKInitialize"))
	{
		NSSymbol *symbol = NSLookupAndBindSymbol("_ASKInitialize");
		if (symbol)
		{
			void (*initializeASKFunc)(void) = NSAddressOfSymbol(symbol);
			if (initializeASKFunc)
			{
				initializeASKFunc();
			}
			else
				return 666;
		}
		else return 667;
	}else
	return 668;

    if (setjmp (exit_continuation) == 0)
    {
	if (install_ipaddr_source ())
	    CFRunLoopRun ();
	else
	    while (1) pause ();
    }

    signal (SIGCHLD, SIG_IGN);

    if (auth_file != NULL)
    {
	/* Remove our Xauth keys */

	write_auth_file (FALSE);
    }

    free_auth_items ();

    return 0;
}
