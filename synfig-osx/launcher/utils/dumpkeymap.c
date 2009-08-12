// $XFree86: xc/programs/Xserver/hw/darwin/utils/dumpkeymap.c,v 1.3 2000/12/05 21:18:34 dawes Exp $
//=============================================================================
//
// Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. The name of the author may not be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
// NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=============================================================================
//-----------------------------------------------------------------------------
// dumpkeymap.c
//
//	Prints a textual representation of each Apple/NeXT .keymapping file
//	mentioned on the command-line.  If no files are mentioned and if the
//	local machine is an Apple or NeXT installation, then the key mapping
//	currently in use by the WindowServer and the AppKit is printed
//	instead.
//
//	Invoke dumpkeymap with one of the options listed below in order to
//	view detailed documentation about .keymapping files and the use of
//	this program.
//
//	--help: Usage summary.
//	--help-keymapping: Detailed discussion of the internal format of a
//	    .keymapping file.
//	--help-output: Explanation of dumpkeymap's output.
//	--help-files: List of key mapping-related files and directories.
//	--help-diagnostics: Explanation of diagnostic messages.
//
// COMPILATION INSTRUCTIONS
//
//	MacOS/X, Darwin
//	cc -Wall -o dumpkeymap dumpkeymap.c -framework IOKit
//
//	MacOS/X DP4 (Developer Preview 4)
//	cc -Wall -o dumpkeymap dumpkeymap.c -FKernel -framework IOKit
//
//	MacOS/X Server, OpenStep, NextStep
//	cc -Wall -o dumpkeymap dumpkeymap.c
//
//	By default, dumpkeymap is configured to interface with the HID driver
//	(Apple) or event-status driver (NeXT), thus allowing it to dump the
//	key mapping which is currently in use by the WindowServer and AppKit.
//	However, these facilities are specific to Apple/NeXT.  In order to
//	build dumpkeymap for non-Apple/NeXT platforms, you must define the
//	DUMPKEYMAP_FILE_ONLY flag when compiling the program.  This flag
//	inhibits use of the HID and event-status drivers and configures
//	dumpkeymap to work strictly with raw key mapping files.
//
//	For example, to compile for Linux:
//	gcc -Wall -DDUMPKEYMAP_FILE_ONLY -o dumpkeymap dumpkeymap.c
//
// CONCLUSION
//
//	This program and its accompanying documentation were written by Eric
//	Sunshine and are copyright (C)1999,2000 by Eric Sunshine
//	<sunshine@sunshineco.com>.
//
//	The implementation of dumpkeymap is based upon information gathered
//	on September 3, 1997 by Eric Sunshine <sunshine@sunshineco.com> and
//	Paul S. McCarthy <zarnuk@zarnuk.com> during an effort to reverse
//	engineer the format of the NeXT .keymapping file.
//
// HISTORY
//
//	v4 2000/12/01 Eric Sunshine <sunshine@sunshineco.com>
//	    Updated manual page to work with `rman', the `man' to `HTML'
//		translator.  Unfortunately, however, rman is missing important
//		roff features such as diversions, indentation, and tab stops,
//		and is also hideously buggy, so getting the manual to work with
//		rman required quite a few work-arounds.
//	    The manual page has now been tested with nroff (plain text), troff
//		(PostScript, etc.), groff (PostScript), and rman (HTML, etc.)
//
//	v3 2000/11/28 Eric Sunshine <sunshine@sunshineco.com>
//	    Considerably expanded the documentation.
//	    Augmented the existing description of .keymapping internals.
//	    Added these new documentation topics:
//		- Output: Very important section describing how to interpret
//		  the output of dumpkeymap.
//		- Files: Lists files and directories related to key mappings.
//		- Diagnostics: Explains diagnostic messages issued by
//		  dumpkeymap.
//	    Created a manual page (dumpkeymap.1) which contains the complete
//		set of documentation for key mapping files and dumpkeymap.
//	    Added command-line options (--help, --help-keymapping,
//		--help-output, --help-files, --help-diagnostics) which allow
//		access to all key mapping documentation.  Previously the
//		description of the internal layout of a .keymapping file was
//		only available as source code comments.
//	    Added --version option.
//	    Ported to non-Apple/NeXT platforms.  Defining the pre-processor
//		flag DUMPKEYMAP_FILE_ONLY at compilation time inhibits use of
//		Apple/NeXT-specific API.
//	    Added a README file.
//
//	v2 2000/11/13 Eric Sunshine <sunshine@sunshineco.com>
//	    Converted from C++ to plain-C.
//	    Now parses and takes into account the "number-size" flag stored
//		with each key map.  This flag indicates the size, in bytes, of
//		all remaining numeric values in the mapping.  Updated all code
//		to respect this flag.  (Previously, the purpose of this field
//		was unknown, and it was thus denoted as
//		`KeyMapping::fill[2]'.)
//	    Updated all documentation; especially the "KEY MAPPING
//		DESCRIPTION" section.  Added discussion of the "number-size"
//		flag and revamped all structure definitions to use the generic
//		data type `number' instead of `uchar' or 'byte'.  Clarified
//		several sections of the documentation and added missing
//		discussions about type definitions and the relationship of
//		`interface' and `handler_id' to .keymapping and .keyboard
//		files.
//	    Updated compilation instructions to include directions for all
//		platforms on which this program might be built.
//	    Now published under the formal BSD license rather than a
//		home-grown license.
//
//	v1 1999/09/08 Eric Sunshine <sunshine@sunshineco.com>
//	    Created.
//-----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#if !defined(DUMPKEYMAP_FILE_ONLY)
#include <drivers/event_status_driver.h>
#endif

#define PROG_NAME "dumpkeymap"
#define PROG_VERSION "4"
#define AUTHOR_NAME "Eric Sunshine"
#define AUTHOR_EMAIL "sunshine@sunshineco.com"
#define AUTHOR_INFO AUTHOR_NAME " <" AUTHOR_EMAIL ">"
#define COPYRIGHT "Copyright (C) 1999,2000 by " AUTHOR_INFO

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int natural;
typedef unsigned long dword;
typedef dword number;

#define ASCII_SET 0x00
#define BIND_FUNCTION 0xfe
#define BIND_SPECIAL 0xff

#define OPT_SWITCH(X) { char const* switched_str__=(X); if (0) {
#define OPT_CASE(X,Y) } else if (strcmp(switched_str__,(#X)) == 0 || \
    strcmp(switched_str__,(#Y)) == 0) {
#define OPT_DEFAULT } else {
#define OPT_SWITCH_END }}

//-----------------------------------------------------------------------------
// Translation Tables
//-----------------------------------------------------------------------------
static char const* const SPECIAL_CODE[] =
    {
    "sound-up",
    "sound-down",
    "brightness-up",
    "brightness-down",
    "alpha-lock",
    "help",
    "power",
    "secondary-up-arrow",
    "secondary-down-arrow"
    };
#define N_SPECIAL_CODE (sizeof(SPECIAL_CODE) / sizeof(SPECIAL_CODE[0]))

static char const* const MODIFIER_CODE[] =
    {
    "alpha-lock",
    "shift",
    "control",
    "alternate",
    "command",
    "keypad",
    "help"
    };
#define N_MODIFIER_CODE (sizeof(MODIFIER_CODE) / sizeof(MODIFIER_CODE[0]))

static char const* const MODIFIER_MASK[] =
    {
    "-----",	// R = carriage-return
    "----L",	// A = alternate
    "---S-",	// C = control
    "---SL",	// S = shift
    "--C--",	// L = alpha-lock
    "--C-L",
    "--CS-",
    "--CSL",
    "-A---",
    "-A--L",
    "-A-S-",
    "-A-SL",
    "-AC--",
    "-AC-L",
    "-ACS-",
    "-ACSL",
    "R----",
    "R---L",
    "R--S-",
    "R--SL",
    "R-C--",
    "R-C-L",
    "R-CS-",
    "R-CSL",
    "RA---",
    "RA--L",
    "RA-S-",
    "RA-SL",
    "RAC--",
    "RAC-L",
    "RACS-",
    "RACSL",
    };
#define N_MODIFIER_MASK (sizeof(MODIFIER_MASK) / sizeof(MODIFIER_MASK[0]))

#define FUNCTION_KEY_FIRST 0x20
static char const* const FUNCTION_KEY[] =
    {
    "F1",			// 0x20
    "F2",			// 0x21
    "F3",			// 0x22
    "F4",			// 0x23
    "F5",			// 0x24
    "F6",			// 0x25
    "F7",			// 0x26
    "F8",			// 0x27
    "F9",			// 0x28
    "F10",			// 0x29
    "F11",			// 0x2a
    "F12",			// 0x2b
    "insert",			// 0x2c
    "delete",			// 0x2d
    "home",			// 0x2e
    "end",			// 0x2f
    "page up",			// 0x30
    "page down",		// 0x31
    "print screen",		// 0x32
    "scroll lock",		// 0x33
    "pause",			// 0x34
    "sys-request",		// 0x35
    "break",			// 0x36
    "reset (HIL)",		// 0x37
    "stop (HIL)",		// 0x38
    "menu (HIL)",		// 0x39
    "user (HIL)",		// 0x3a
    "system (HIL)",		// 0x3b
    "print (HIL)",		// 0x3c
    "clear line (HIL)",		// 0x3d
    "clear display (HIL)",	// 0x3e
    "insert line (HIL)",	// 0x3f
    "delete line (HIL)",	// 0x40
    "insert char (HIL)",	// 0x41
    "delete char (HIL)",	// 0x42
    "prev (HIL)",		// 0x43
    "next (HIL)",		// 0x44
    "select (HIL)",		// 0x45
    };
#define N_FUNCTION_KEY (sizeof(FUNCTION_KEY) / sizeof(FUNCTION_KEY[0]))


//-----------------------------------------------------------------------------
// Data Stream Object
//	Can be configured to treat embedded "numbers" as being composed of
//	either 1, 2, or 4 bytes, apiece.
//-----------------------------------------------------------------------------
typedef struct _DataStream
    {
    byte const* data;
    byte const* data_end;
    natural number_size; // Size in bytes of a "number" in the stream.
    } DataStream;

static DataStream* new_data_stream( byte const* data, int size )
    {
    DataStream* s = (DataStream*)malloc( sizeof(DataStream) );
    s->data = data;
    s->data_end = data + size;
    s->number_size = 1; // Default to byte-sized numbers.
    return s;
    }

static void destroy_data_stream( DataStream* s )
    {
    free(s);
    }

static int end_of_stream( DataStream* s )
    {
    return (s->data >= s->data_end);
    }

static void expect_nbytes( DataStream* s, int nbytes )
    {
    if (s->data + nbytes > s->data_end)
	{
	fputs( "Insufficient data in keymapping data stream.\n", stderr );
	exit(-1);
	}
    }

static byte get_byte( DataStream* s )
    {
    expect_nbytes( s, 1 );
    return *s->data++;
    }

static word get_word( DataStream* s )
    {
    word hi, lo;
    expect_nbytes( s, 2 );
    hi = *s->data++;
    lo = *s->data++;
    return ((hi << 8) | lo);
    }

static dword get_dword( DataStream* s )
    {
    dword b1, b2, b3, b4;
    expect_nbytes( s, 4 );
    b4 = *s->data++;
    b3 = *s->data++;
    b2 = *s->data++;
    b1 = *s->data++;
    return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
    }

static number get_number( DataStream* s )
    {
    switch (s->number_size)
	{
	case 4:  return get_dword(s);
	case 2:  return get_word(s);
	default: return get_byte(s);
	}
    }


//-----------------------------------------------------------------------------
// Translation Utility Functions
//-----------------------------------------------------------------------------
static char const* special_code_desc( number n )
    {
    if (n < N_SPECIAL_CODE)
	return SPECIAL_CODE[n];
    else
	return "invalid";
    }

static char const* modifier_code_desc( number n )
    {
    if (n < N_MODIFIER_CODE)
	return MODIFIER_CODE[n];
    else
	return "invalid";
    }

static char const* modifier_mask_desc( number n )
    {
    if (n < N_MODIFIER_MASK)
	return MODIFIER_MASK[n];
    else
	return "?????";
    }

static char const* function_key_desc( number n )
    {
    if (n >= FUNCTION_KEY_FIRST && n < N_FUNCTION_KEY + FUNCTION_KEY_FIRST)
	return FUNCTION_KEY[ n - FUNCTION_KEY_FIRST ];
    else
	return "unknown";
    }

static number bits_set( number mask )
    {
    number n = 0;
    for ( ; mask != 0; mask >>= 1)
	if ((mask & 0x01) != 0)
	    n++;
    return n;
    }


//-----------------------------------------------------------------------------
// Unparse a list of Modifier records.
//-----------------------------------------------------------------------------
static void unparse_modifiers( DataStream* s )
    {
    number nmod = get_number(s); // Modifier count
    printf( "MODIFIERS [%lu]\n", nmod );
    while (nmod-- > 0)
	{
	number nscan;
	number const code = get_number(s);
	printf( "%s:", modifier_code_desc(code) );
	nscan = get_number(s);
	while (nscan-- > 0)
	    printf( " 0x%02x", (natural)get_number(s) );
	putchar( '\n' );
	}
    putchar( '\n' );
    }


//-----------------------------------------------------------------------------
// Unparse a list of Character records.
//-----------------------------------------------------------------------------
typedef void (*UnparseSpecialFunc)( number code );

static void unparse_char_codes(
    DataStream* s, number ncodes, UnparseSpecialFunc unparse_special )
    {
    if (ncodes != 0)
	{
	while (ncodes-- > 0)
	    {
	    number const char_set = get_number(s);
	    number const code = get_number(s);
	    putchar(' ');
	    switch (char_set)
		{
		case ASCII_SET:
		    {
		    int const c = (int)code;
		    if (isprint(c))
			printf( "\"%c\"", c );
		    else if (code < ' ')
			printf( "\"^%c\"", c + '@' );
		    else
			printf( "%02x", c );
		    break;
		    }
		case BIND_FUNCTION:
		    printf( "[%s]", function_key_desc(code) );
		    break;
		case BIND_SPECIAL:
		    unparse_special( code );
		    break;
		default:
		    printf( "%02x/%02x", (natural)char_set, (natural)code );
		    break;
		}
	    }
	}
    }


//-----------------------------------------------------------------------------
// Unparse a list of scan code bindings.
//-----------------------------------------------------------------------------
static void unparse_key_special( number code )
    {
    printf( "{seq#%lu}", code );
    }

static void unparse_characters( DataStream* s )
    {
    number const NOT_BOUND = 0xff;
    number const nscans = get_number(s);
    number scan;
    printf( "CHARACTERS [%lu]\n", nscans );
    for (scan = 0; scan < nscans; scan++)
	{
	number const mask = get_number(s);
	printf( "scan 0x%02x: ", (natural)scan );
	if (mask == NOT_BOUND)
	    fputs( "not-bound\n", stdout );
	else
	    {
	    number const bits = bits_set( mask );
	    number const codes = 1 << bits;
	    printf( "%s ", modifier_mask_desc(mask) );
	    unparse_char_codes( s, codes, unparse_key_special );
	    putchar( '\n' );
	    }
	}
    putchar( '\n' );
    }


//-----------------------------------------------------------------------------
// Unparse a list of key sequences.
//-----------------------------------------------------------------------------
static void unparse_sequence_special( number code )
    {
    printf( "{%s}", (code == 0 ? "unmodify" : modifier_code_desc(code)) );
    }

static void unparse_sequences( DataStream* s )
    {
    number const nseqs = get_number(s);
    number seq;
    printf( "SEQUENCES [%lu]\n", nseqs );
    for (seq = 0; seq < nseqs; seq++)
	{
	number const nchars = get_number(s);
	printf( "sequence %lu:", seq );
	unparse_char_codes( s, nchars, unparse_sequence_special );
	putchar( '\n' );
	}
    putchar( '\n' );
    }


//-----------------------------------------------------------------------------
// Unparse a list of special keys.
//-----------------------------------------------------------------------------
static void unparse_specials( DataStream* s )
    {
    number nspecials = get_number(s);
    printf( "SPECIALS [%lu]\n", nspecials );
    while (nspecials-- > 0)
	{
	number const special = get_number(s);
	number const scan = get_number(s);
	printf( "%s: 0x%02x\n", special_code_desc(special), (natural)scan );
	}
    putchar( '\n' );
    }


//-----------------------------------------------------------------------------
// Unparse the number-size flag.
//-----------------------------------------------------------------------------
static void unparse_numeric_size( DataStream* s )
    {
    word const numbers_are_shorts = get_word(s);
    s->number_size = numbers_are_shorts ? 2 : 1;
    }


//-----------------------------------------------------------------------------
// Unparse an entire key map.
//-----------------------------------------------------------------------------
static void unparse_keymap_data( DataStream* s )
    {
    unparse_numeric_size(s);
    unparse_modifiers(s);
    unparse_characters(s);
    unparse_sequences(s);
    unparse_specials(s);
    }


//-----------------------------------------------------------------------------
// Unparse the active key map.
//-----------------------------------------------------------------------------
#if !defined(DUMPKEYMAP_FILE_ONLY)
static int unparse_active_keymap( void )
    {
    int rc = 1;
    NXEventHandle const h = NXOpenEventStatus();
    if (h == 0)
	fputs( "Unable to open event status driver.\n", stderr );
    else
	{
	NXKeyMapping km;
	km.size = NXKeyMappingLength(h);
	if (km.size <= 0)
	    fprintf( stderr, "Bad key mapping length (%d).\n", km.size );
	else
	    {
	    km.mapping = (char*)malloc( km.size );
	    if (NXGetKeyMapping( h, &km ) == 0)
		fputs( "Unable to get current key mapping.\n", stderr );
	    else
		{
		DataStream* stream =
		    new_data_stream( (byte const*)km.mapping, km.size );
		fputs( "=============\nACTIVE KEYMAP\n=============\n\n",
		    stdout);
		unparse_keymap_data( stream );
		destroy_data_stream( stream );
		rc = 0;
		}
	    free( km.mapping );
	    }
	NXCloseEventStatus(h);
	}
    return rc;
    }
#endif


//-----------------------------------------------------------------------------
// Unparse one key map from a keymapping file.
//-----------------------------------------------------------------------------
static void unparse_keymap( DataStream* s )
    {
    dword const interface = get_dword(s);
    dword const handler_id = get_dword(s);
    dword const map_size = get_dword(s);
    printf( "interface: 0x%02lx\nhandler_id: 0x%02lx\nmap_size: %lu bytes\n\n",
	interface, handler_id, map_size );
    unparse_keymap_data(s);
    }


//-----------------------------------------------------------------------------
// Check the magic number of a keymapping file.
//-----------------------------------------------------------------------------
static int check_magic_number( DataStream* s )
    {
    return (get_byte(s) == 'K' &&
	    get_byte(s) == 'Y' &&
	    get_byte(s) == 'M' &&
	    get_byte(s) == '1');
    }


//-----------------------------------------------------------------------------
// Unparse all key maps within a keymapping file.
//-----------------------------------------------------------------------------
static int unparse_keymaps( DataStream* s )
    {
    int rc = 0;
    if (check_magic_number(s))
	{
	int n = 1;
	while (!end_of_stream(s))
	    {
	    printf( "---------\nKEYMAP #%d\n---------\n", n++ );
	    unparse_keymap(s);
	    }
	}
    else
	{
	fputs( "Bad magic number.\n", stderr );
	rc = 1;
	}
    return rc;
    }


//-----------------------------------------------------------------------------
// Unparse a keymapping file.
//-----------------------------------------------------------------------------
static int unparse_keymap_file( char const* const path )
    {
    int rc = 1;
    FILE* file;
    printf( "===========\nKEYMAP FILE\n===========\n%s\n\n", path );
    file = fopen( path, "rb" );
    if (file == 0)
	perror( "Unable to open key mapping file" );
    else
	{
	struct stat st;
	if (fstat( fileno(file), &st ) != 0)
	    perror( "Unable to determine key mapping file size" );
	else
	    {
	    byte* buffer = (byte*)malloc( st.st_size );
	    if (fread( buffer, st.st_size, 1, file ) != 1)
		perror( "Unable to read key mapping file" );
	    else
		{
		DataStream* stream = new_data_stream(buffer, (int)st.st_size);
		fclose( file ); file = 0;
		rc = unparse_keymaps( stream );
		destroy_data_stream( stream );
		}
	    free( buffer );
	    }
	if (file != 0)
	    fclose( file );
	}
    return rc;
    }


//-----------------------------------------------------------------------------
// Handle the case when no documents are mentioned on the command-line.  For
// Apple/NeXT platforms, dump the currently active key mapping; else display
// an error message.
//-----------------------------------------------------------------------------
static int handle_empty_document_list( void )
    {
#if !defined(DUMPKEYMAP_FILE_ONLY)
    return unparse_active_keymap();
#else
    fputs( "ERROR: Must specify at least one .keymapping file.\n\n", stderr );
    return 1;
#endif
    }


//-----------------------------------------------------------------------------
// Print a detailed description of the internal layout of a key mapping.
//-----------------------------------------------------------------------------
static void print_internal_layout_info( FILE* f )
    {
    fputs(
"What follows is a detailed descriptions of the internal layout of an\n"
"Apple/NeXT .keymapping file.\n"
"\n"
"Types and Data\n"
"--------------\n"
"The following type definitions are employed throughout this discussion:\n"
"\n"
"    typedef unsigned char  byte;\n"
"    typedef unsigned short word;\n"
"    typedef unsigned long  dword;\n"
"\n"
"Additionally, the type definition `number' is used generically to indicate\n"
"a numeric value.  The actual size of the `number' type may be one or two\n"
"bytes depending upon how the data is stored in the key map.  Although most\n"
"key maps use byte-sized numeric values, word-sized values are also allowed.\n"
"\n"
"Multi-byte values in a key mapping file are stored in big-endian byte\n"
"order.\n"
"\n"
"Key Mapping File and Device Mapping\n"
"-----------------------------------\n"
"A key mapping file begins with a magic-number and continues with a variable\n"
"number of device-specific key mappings.\n"
"\n"
"    struct KeyMappingFile {\n"
"        char magic_number[4];    // `KYM1'\n"
"        DeviceMapping maps[...]; // Variable number of maps\n"
"    };\n"
"\n"
"    struct DeviceMapping {\n"
"        dword interface;  // Interface type\n"
"        dword handler_id; // Interface subtype\n"
"        dword map_size;   // Byte count of `map' (below)\n"
"        KeyMapping map;\n"
"    };\n"
"\n"
"The value of `interface' represents a family of keyboard device types\n"
"(such as Intel PC, ADB, NeXT, Sun Type5, etc.), and is generally\n"
"specified as one of the constant values NX_EVS_DEVICE_INTERFACE_ADB,\n"
"NX_EVS_DEVICE_INTERFACE_ACE, etc., which are are defined in IOHIDTypes.h on\n"
"MacOS/X and Darwin, and in ev_types.h on MacOS/X Server, OpenStep, and\n"
"NextStep.\n"
"\n"
"The value of `handler_id' represents a specific keyboard layout within the\n"
"much broader `interface' family.  For instance, for a 101-key Intel PC\n"
"keyboard (of type NX_EVS_DEVICE_INTERFACE_ACE) the `handler_id' is '0',\n"
"whereas for a 102-key keyboard it is `1'.\n"
"\n"
"Together, `interface' and `handler_id' identify the exact keyboard hardware\n"
"to which this mapping applies.  Programs which display a visual\n"
"representation of a keyboard layout, match `interface' and `handler_id'\n"
"from the .keymapping file against the `interface' and `handler_id' values\n"
"found in each .keyboard file.\n"
"\n"
"Key Mapping\n"
"-----------\n"
"A key mapping completely defines the relationship of all scan codes with\n"
"their associated functionality.  A KeyMapping structure is embedded within\n"
"the DeviceMapping structure in a KeyMappingFile.  The key mapping currently\n"
"in use by the WindowServer and AppKit is also represented by a KeyMapping\n"
"structure, and can be referred to directly by calling NXGetKeyMapping() and\n"
"accessing the `mapping' data member of the returned NXKeyMapping structure.\n"
"\n"
"    struct KeyMapping {\n"
"        word number_size;                   // 0=1 byte, non-zero=2 bytes\n"
"        number num_modifier_groups;         // Modifier groups\n"
"        ModifierGroup modifier_groups[...];\n"
"        number num_scan_codes;              // Scan groups\n"
"        ScanGroup scan_table[...];\n"
"        number num_sequence_lists;          // Sequence lists\n"
"        Sequence sequence_lists[...];\n"
"        number num_special_keys;            // Special keys\n"
"        SpecialKey special_key[...];\n"
"    };\n"
"\n"
"The `number_size' flag determines the size, in bytes, of all remaining\n"
"numeric values (denoted by the type definition `number') within the key\n"
"mapping.  If its value is zero, then numbers are represented by a single\n"
"byte.  If it is non-zero, then numbers are represented by a word (two\n"
"bytes).\n"
"\n"
"Modifier Group\n"
"--------------\n"
"A modifier group defines all scan codes which map to a particular type of\n"
"modifier, such as `shift', `control', etc.\n"
"\n"
"    enum Modifier {\n"
"        ALPHALOCK = 0,\n"
"        SHIFT,\n"
"        CONTROL,\n"
"        ALTERNATE,\n"
"        COMMAND,\n"
"        KEYPAD,\n"
"        HELP\n"
"    };\n"
"\n"
"    struct ModifierGroup {\n"
"        number modifier;        // A Modifier constant\n"
"        number num_scan_codes;\n"
"        number scan_codes[...]; // Variable number of scan codes\n"
"    };\n"
"\n"
"The scan_codes[] array contains a list of all scan codes which map to the\n"
"specified modifier.  The `shift', `command', and `alternate' modifiers are\n"
"frequently mapped to two different scan codes, apiece, since these\n"
"modifiers often appear on both the left and right sides of the keyboard.\n"
"\n"
"Scan Group\n"
"----------\n"
"There is one ScanGroup for each scan code generated by the given keyboard.\n"
"This number is given by KeyMapping::num_scan_codes.  The first scan group\n"
"represents hardware scan code 0, the second represents scan code 1, etc.\n"
"\n"
"    enum ModifierMask {\n"
"        ALPHALOCK_MASK       = 1 << 0,\n"
"        SHIFT_MASK           = 1 << 1,\n"
"        CONTROL_MASK         = 1 << 2,\n"
"        ALTERNATE_MASK       = 1 << 3,\n"
"        CARRIAGE_RETURN_MASK = 1 << 4\n"
"    };\n"
"    #define NOT_BOUND 0xff\n"
"\n"
"    struct ScanGroup {\n"
"        number mask;\n"
"        Character characters[...];\n"
"    };\n"
"\n"
"For each scan code, `mask' defines which modifier combinations generate\n"
"characters.  If `mask' is NOT_BOUND (0xff) then then this scan code does\n"
"not generate any characters ever, and its characters[] array is zero\n"
"length.  Otherwise, the characters[] array contains one Character record\n"
"for each modifier combination.\n"
"\n"
"The number of records in characters[] is determined by computing (1 <<\n"
"bits_set_in_mask).  In other words, if mask is zero, then zero bits are\n"
"set, so characters[] contains only one record.  If `mask' is (SHIFT_MASK |\n"
"CONTROL_MASK), then two bits are set, so characters[] contains four\n"
"records.\n"
"\n"
"The first record always represents the character which is generated by that\n"
"key when no modifiers are active.  The remaining records represent\n"
"characters generated by the various modifier combinations.  Using the\n"
"example with the `shift' and `control' masks set, record two would\n"
"represent the character with the `shift' modifier active; record three, the\n"
"`control' modifier active; and record four, both the `shift' and `control'\n"
"modifiers active.\n"
"\n"
"As a special case, ALPHALOCK_MASK implies SHIFT_MASK, though only\n"
"ALPHALOCK_MASK appears in `mask'.  In this case the same character is\n"
"generated for both the `shift' and `alpha-lock' modifiers, but only needs\n"
"to appear once in the characters[] array.\n"
"\n"
"CARRIAGE_RETURN_MASK does not actually refer to a modifier key.  Instead,\n"
"it is used to distinguish the scan code which is given the special\n"
"pseudo-designation of `carriage return' key.  Typically, this mask appears\n"
"solo in a ScanGroup record and only the two Character records for control-M\n"
"and control-C follow.  This flag may be a throwback to an earlier time or\n"
"may be specially interpreted by the low-level keyboard driver, but its\n"
"purpose is otherwise enigmatic.\n"
"Character\n"
"---------\n"
"Each Character record indicates the character generated when this key is\n"
"pressed, as well as the character set which contains the character.  Well\n"
"known character sets are `ASCII' and `Symbol'.  The character set can also\n"
"be one of the meta values FUNCTION_KEY or KEY_SEQUENCE.  If it is\n"
"FUNCTION_KEY then `char_code' represents a generally well-known function\n"
"key such as those enumerated by FunctionKey.  If the character set is\n"
"KEY_SEQUENCE then `char_code' represents a zero-base index into\n"
"KeyMapping::sequence_lists[].\n"
"\n"
"    enum CharacterSet {\n"
"        ASCII        = 0x00,\n"
"        SYMBOL       = 0x01,\n"
"        ...\n"
"        FUNCTION_KEY = 0xfe,\n"
"        KEY_SEQUENCE = 0xff\n"
"    };\n"
"\n"
"    struct Character {\n"
"        number set;       // CharacterSet of generated character\n"
"        number char_code; // Actual character generated\n"
"    };\n"
"\n"
"    enum FunctionKey {\n"
"        F1 = 0x20, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,\n"
"        INSERT, DELETE, HOME, END, PAGE_UP, PAGE_DOWN, PRINT_SCREEN,\n"
"        SCROLL_LOCK, PAUSE, SYS_REQUEST, BREAK, RESET, STOP, MENU, USER,\n"
"        SYSTEM, PRINT, CLEAR_LINE, CLEAR_DISPLAY, INSERT_LINE,\n"
"        DELETE_LINE, INSERT_CHAR, DELETE_CHAR, PREV, NEXT, SELECT\n"
"    };\n"
"\n"
"Sequence\n"
"--------\n"
"When Character::set contains the meta value KEY_SEQUENCE, the scan code is\n"
"bound to a sequence of keys rather than a single character.  A sequence is\n"
"a series of modifiers and characters which are automatically generated when\n"
"the associated key is depressed.\n"
"\n"
"    #define MODIFIER_KEY 0xff\n"
"\n"
"    struct Sequence {\n"
"        number num_chars;\n"
"        Character characters[...];\n"
"    };\n"
"\n"
"Each generated Character is represented as previously described, with the\n"
"exception that MODIFIER_KEY may appear in place of KEY_SEQUENCE.  When the\n"
"value of Character::set is MODIFIER_KEY then Character::char_code\n"
"represents a modifier key rather than an actual character.  If the modifier\n"
"represented by `char_code' is non-zero, then it indicates that the\n"
"associated modifier key has been depressed.  In this case, the value is one\n"
"of the constants enumerated by Modifier (SHIFT, CONTROL, ALTERNATE, etc.).\n"
"If the value is zero then it means that the modifier keys have been\n"
"released.\n"
"\n"
"Special Key\n"
"-----------\n"
"A special key is one which is scanned directly by the Mach kernel rather\n"
"than by the WindowServer.  In general, events are not generated for special\n"
"keys.\n"
"\n"
"    enum SpecialKeyType {\n"
"        VOLUME_UP = 0,\n"
"        VOLUME_DOWN,\n"
"        BRIGHTNESS_UP,\n"
"        BRIGHTNESS_DOWN,\n"
"        ALPHA_LOCK,\n"
"        HELP,\n"
"        POWER,\n"
"        SECONDARY_ARROW_UP,\n"
"        SECONDARY_ARROW_DOWN\n"
"    };\n"
"\n"
"    struct SpecialKey {\n"
"        number type;      // A SpecialKeyType constant\n"
"        number scan_code; // Actual scan code\n"
"    };\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print an explanation of the output generated by this program.
//-----------------------------------------------------------------------------
static void print_output_info( FILE* f )
    {
    fputs(
"What follows is an explanation and description of the various pieces of\n"
"information emitted by dumpkeymap.\n"
"\n"
"For a more thorough discussion of any particular piece of information\n"
"described here, refer to the detailed description of the internal layout of\n"
"a key mapping given by the --help-layout option.\n"
"\n"
"Conventions\n"
"-----------\n"
"Depending upon context, some numeric values are displayed in decimal\n"
"notation, whereas others are displayed in hexadecimal notation.\n"
"Hexadecimal numbers are denoted by a `0x' prefix (for instance, `0x7b'),\n"
"except when explicitly noted otherwise.\n"
"\n"
"Key Mapping Source\n"
"------------------\n"
"The first piece of information presented about a particular key mapping is\n"
"the source from which the data was gleaned.  For a .keymapping file, the\n"
"title `KEYMAP FILE' is emitted along with the path and name of the file in\n"
"question.  For the key mapping currently in use by the WindowServer and\n"
"AppKit, the title `ACTIVE KEYMAP' is emitted instead.\n"
"\n"
"Device Information\n"
"------------------\n"
"Each .keymapping file may contain one or more raw key mappings.  For\n"
"example, a file which maps keys to a Dvorak-style layout might contain raw\n"
"mappings for Intel PC, ADB, NeXT, and Sun Type5 keyboards.\n"
"\n"
"For each raw mapping, the following information is emitted:\n"
"\n"
"    o The title `KEYMAP' along with the mapping's relative position in the\n"
"      .keymapping file.\n"
"    o The `interface' identifier.\n"
"    o The `handler_id' sub-identifier.\n"
"    o The size of the raw mapping resource counted in bytes.\n"
"\n"
"The `interface' and `handler_id' values, taken together, define a specific\n"
"keyboard device.  A .keyboard file, which describes the visual layout of a\n"
"keyboard, also contains `interface' and `handler_id' identifiers.  The\n"
".keyboard file corresponding to a particular key mapping can be found by\n"
"matching the `interface' and `handler_id' values from each resource.\n"
"\n"
"Modifiers\n"
"---------\n"
"Each mapping may contain zero or more modifier records which associate\n"
"hardware scan codes with modifier descriptions such as `shift', `control',\n"
"`alternate', etc.  The title `MODIFIERS' is printed along with the count of\n"
"modifier records which follow.  For each modifier record, the modifier's\n"
"name is printed along with a list of scan codes, in hexadecimal format,\n"
"which generate that modifier value.  For example:\n"
"\n"
"    MODIFIERS [4]\n"
"    alternate: 0x1d 0x60\n"
"    control: 0x3a\n"
"    keypad: 0x52 0x53 ... 0x63 0x62\n"
"    shift: 0x2a 0x36\n"
"\n"
"Characters\n"
"----------\n"
"Each mapping may contain zero or more character records which associate\n"
"hardware scan codes with the actual characters generated by those scan\n"
"codes in the presence or absence of various modifier combinations.  The\n"
"title `CHARACTERS' is printed along with the count of character records\n"
"which follow.  Here is a highly abbreviated example:\n"
"\n"
"    CHARACTERS [9]\n"
"    scan 0x00: -AC-L  \"a\" \"A\" \"^A\" \"^A\" ca c7 \"^A\" \"^A\"\n"
"    scan 0x07: -AC-L  \"x\" \"X\" \"^X\" \"^X\" 01/b4 01/ce \"^X\" \"^X\"\n"
"    scan 0x0a: ---S-  \"<\" \">\"\n"
"    scan 0x13: -ACS-  \"2\" \"@\" \"^@\" \"^@\" b2 b3 \"^@\" \"^@\"\n"
"    scan 0x24: R----  \"^M\" \"^C\"\n"
"    scan 0x3e: -----  [F4]\n"
"    scan 0x4a: -----  [page up]\n"
"    scan 0x60: -----  {seq#3}\n"
"    scan 0x68: not-bound\n"
"\n"
"For each record, the hexadecimal value of the hardware scan code is\n"
"printed, followed by a list of modifier flag combinations and the actual\n"
"characters generated by this scan code with and without modifiers applied.\n"
"\n"
"The modifier flags field is composed of a combination of single letter\n"
"representations of the various modifier types.  The letters stand for:\n"
"\n"
"    L - alpha-lock\n"
"    S - shift\n"
"    C - control\n"
"    A - alternate\n"
"    R - carriage-return\n"
"\n"
"As a special case, the `alpha-lock' flag also implies the `shift' flag, so\n"
"these two flags never appear together in the same record.\n"
"\n"
"The combination of modifier flags determines the meaning and number of\n"
"fields which follow.  The first field after the modifier flags always\n"
"represents the character that will be generated if no modifier keys are\n"
"depressed.  The remaining fields represent characters generated by the\n"
"various modifier combinations.  The order of the fields follows this\n"
"general pattern:\n"
"\n"
"    o The character generated by this scan code when no modifiers are in\n"
"      effect is listed first.\n"
"\n"
"    o If the `L' or `S' flag is active, then the shifted character\n"
"      generated by this scan code is listed next.\n"
"\n"
"    o If the `C' flag is active, then the control-character generated by\n"
"      this scan code is listed next.  Furthermore, if the `L' or `S' flag\n"
"      is also active, then the shifted control-character is listed after\n"
"      that.\n"
"\n"
"    o If the `A' flag is active, then the alternate-character generated by\n"
"      this scan code is listed next.  Furthermore, if the `L' or `S' flag\n"
"      is active, then the shifted alternate-character is listed after that.\n"
"      If the `C' flag is also active, then the alternate-control-character\n"
"      is listed next.  Finally, if the `C' and `L' or `C' and `S' flags are\n"
"      also active, then the shifted alternate-control-character is listed.\n"
"\n"
"The `R' flag does not actually refer to a modifier key.  Instead, it is\n"
"used to distinguish the scan code which is given the special\n"
"pseudo-designation of `carriage return' key.  Typically, this mask appears\n"
"solo and only the two fields for control-M and control-C follow.  This flag\n"
"may be a throwback to an earlier time or may be specially interpreted by\n"
"the low-level keyboard driver, but its purpose is otherwise enigmatic.\n"
"\n"
"Recalling the example from above, the following fields can be identified:\n"
"\n"
"    scan 0x00: -AC-L  \"a\" \"A\" \"^A\" \"^A\" ca c7 \"^A\" \"^A\"\n"
"\n"
"    o Lower-case `a' is generated when no modifiers are active.\n"
"    o Upper-case `A' is generated when `shift' or `alpha-lock' are active.\n"
"    o Control-A is generated when `control' is active.\n"
"    o Control-A is generated when `control' and `shift' are active.\n"
"    o The character represented by the hexadecimal code 0xca is generated\n"
"      when `alternate' is active.\n"
"    o The character represented by 0xc7 is generated when `alternate' and\n"
"      `shift' (or `alpha-lock') are active.\n"
"    o Control-A is generated when `alternate' and `control' are active.\n"
"    o Control-A is generated when `alternate', `control' and `shift' (or\n"
"      `alpha-lock') are active.\n"
"\n"
"The notation used to represent a particular generated character varies.\n"
"\n"
"    o Printable ASCII characters are quoted, as in \"x\" or \"X\".\n"
"\n"
"    o Control-characters are quoted and prefixed with `^', as in \"^X\".\n"
"\n"
"    o Characters with values greater than 127 (0x7f) are displayed as\n"
"      hexadecimal values without the `0x' prefix.\n"
"\n"
"    o Characters in a non-ASCII character set (such as `Symbol') are\n"
"      displayed as two hexadecimal numbers separated by a slash, as in\n"
"      `01/4a'.  The first number is the character set's identification code\n"
"      (such as `01' for the `Symbol' set), and the second number is the\n"
"      value of the generated character.\n"
"\n"
"    o Non-printing special function characters are displayed with the\n"
"      function's common name enclosed in brackets, as in `[page up]' or\n"
"      `[F4]'.\n"
"\n"
"    o If the binding represents a key sequence rather than a single\n"
"      character, then the sequence's identification number is enclosed in\n"
"      braces, as in `{seq#3}'.\n"
"\n"
"Recalling a few examples from above, the following interpretations can be\n"
"made:\n"
"\n"
"    scan 0x07: -AC-L  \"x\" \"X\" \"^X\" \"^X\" 01/b4 01/ce \"^X\" \"^X\"\n"
"    scan 0x3e: -----  [F4]\n"
"    scan 0x4a: -----  [page up]\n"
"    scan 0x60: -----  {seq#3}\n"
"\n"
"    o \"x\" and \"X\" are printable ASCII characters.\n"
"    o \"^X\" is a control-character.\n"
"    o `01/b4' and `01/ce' represent the character codes 0xb4 and 0xce in\n"
"      the `Symbol' character set.\n"
"    o Scan code 0x3e generates function-key `F4', and scan code 0x4a\n"
"      generates function-key `page up'.\n"
"    o Scan code 0x60 is bound to key sequence #3.\n"
"\n"
"Finally, if a scan code is not bound to any characters, then it is\n"
"annotated with the label `not-bound', as with example scan code 0x68 from\n"
"above.\n"
"\n"
"Sequences\n"
"---------\n"
"A scan code (modified and unmodified) can be bound to a key sequence rather\n"
"than generating a single character or acting as a modifier.  When it is\n"
"bound to a key sequence, a series of character invocations and modifier\n"
"actions are automatically generated rather than a single keystroke.\n"
"\n"
"Each mapping may contain zero or more key sequence records.  The title\n"
"`SEQUENCES' is printed along with the count of sequence records which\n"
"follow.  For example:\n"
"\n"
"    SEQUENCES [3]\n"
"    sequence 0: \"f\" \"o\" \"o\"\n"
"    sequence 1: {alternate} \"b\" \"a\" \"r\" {unmodify}\n"
"    sequence 2: [home] \"b\" \"a\" \"z\"\n"
"\n"
"The notation used to represent the sequence of generated characters is\n"
"identical to the notation already described in the `Characters' section\n"
"above, with the exception that modifier actions may be interposed between\n"
"generated characters.  Such modifier actions are represented by the\n"
"modifier's name enclosed in braces.  The special name `{unmodify}'\n"
"indicates the release of the modifier keys.\n"
"\n"
"Thus, the sequences in the above example can be interpreted as follows:\n"
"\n"
"    o Sequence #0 generates `foo'.\n"
"    o Sequence #1 invokes the `alternate' modifier, generates `bar', and\n"
"      then releases `alternate'.\n"
"    o Sequence #2 invokes the `home' key and then generates `baz'.  In a\n"
"      text editor, this would probably result in `baz' being prepended to\n"
"      the line of text on which the cursor resides.\n"
"\n"
"Special Keys\n"
"------------\n"
"Certain keyboards feature keys which perform some type of special purpose\n"
"function rather than generating a character or acting as a modifier.  For\n"
"instance, Apple keyboards often contain a `power' key, and NeXT keyboards\n"
"have historically featured screen brightness and volume control keys.\n"
"\n"
"Each mapping may contain zero or more special-key records which associate\n"
"hardware scan codes with such special purpose functions.  The title\n"
"`SPECIALS' is printed along with the count of records which follow.  For\n"
"each record, the special function's name is printed along with a list of\n"
"scan codes, in hexadecimal format, which are bound to that function.  For\n"
"example:\n"
"\n"
"    SPECIALS [6]\n"
"    alpha-lock: 0x39\n"
"    brightness-down: 0x79\n"
"    brightness-up: 0x74\n"
"    power: 0x7f\n"
"    sound-down: 0x77\n"
"    sound-up: 0x73\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print a summary of the various files and directories which are related to
// key mappings.
//-----------------------------------------------------------------------------
static void print_files_info( FILE* f )
    {
    fputs(
"This is a summary of the various files and directories which are related to\n"
"key mappings.\n"
"\n"
"*.keymapping\n"
"    A key mapping file which precisely defines the relationship of all\n"
"    hardware-specific keyboard scan-codes with their associated\n"
"    functionality.\n"
"\n"
"*.keyboard\n"
"    A file describing the physical layout of keys on a particular type of\n"
"    keyboard.  Each `key' token in this file defines the position and shape\n"
"    of the key on the keyboard, as well as the associated scan code which\n"
"    that key generates.  A .keymapping file, on the other hand, defines the\n"
"    characters which are generated by a particular scan code depending upon\n"
"    the state of the various modifier keys (such as shift, control, etc.).\n"
"    The `interface' and `handler_id' values from a .keymapping file are\n"
"    matched against those in each .keyboard file in order to associate a\n"
"    particular .keyboard file with a key mapping.  Various GUI programs use\n"
"    the .keyboard file to display a visual representation of a keyboard for\n"
"    the user.  Since these files are just plain text, they can be easily\n"
"    viewed and interpreted without the aid of a specialized program, thus\n"
"    dumpkeymap leaves these files alone.\n"
"\n"
"/System/Library/Keyboards\n"
"/Network/Library/Keyboards\n"
"/Local/Library/Keyboards\n"
"/Library/Keyboards\n"
"    Repositories for .keymapping and .keyboard files for MacOS/X, Darwin,\n"
"    and MacOS/X Server.\n"
"\n"
"/NextLibrary/Keyboards\n"
"/LocalLibrary/Keyboards\n"
"    Repositories for .keymapping and .keyboard files for OpenStep and\n"
"    NextStep.\n"
"\n"
"$(HOME)/Library/Keyboards\n"
"    Repository for personal .keymapping and .keyboard files.\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print a list of the various diagnostic messages which may be emitted.
//-----------------------------------------------------------------------------
static void print_diagnostics_info( FILE* f )
    {
    fputs(
"The following diagnostic messages may be issued to the standard error\n"
"stream.\n"
"\n"
"Unrecognized option.\n"
"    An unrecognized option was specified on the command-line.  Invoke\n"
"    dumpkeymap with the --help option to view a list of valid options.\n"
"\n"
"Insufficient data in keymapping data stream.\n"
"    The key mapping file or data stream is corrupt.  Either the file has\n"
"    been incorrectly truncated or a field, such as those which indicates\n"
"    the number of variable records which follow, contains a corrupt value.\n"
"\n"
"The following diagnostic messages have significance only when trying to\n"
"print .keymapping files mentioned on the command-line.\n"
"\n"
"Bad magic number.\n"
"    The mentioned file is not a .keymapping file.  The file's content does\n"
"    not start with the string `KYM1'.\n"
"\n"
"Unable to open key mapping file.\n"
"    The call to fopen() failed; probably because the specified path is\n"
"    invalid or dumpkeymap does not have permission to read the file.\n"
"\n"
"Unable to determine key mapping file size.\n"
"    The call to fstat() failed, thus memory can not be allocated for\n"
"    loading the file.\n"
"\n"
"Unable to read key mapping file.\n"
"    The call to fread() failed.\n"
"\n"
"The following diagnostic messages have significance only when trying to\n"
"print the currently active key mapping when no .keymapping files have been\n"
"mentioned on the command-line.\n"
"\n"
"Unable to open event status driver.\n"
"    The call to NXOpenEventStatus() failed.\n"
"\n"
"Bad key mapping length.\n"
"    The call to NXKeyMappingLength() returned a bogus value.\n"
"\n"
"Unable to get current key mapping.\n"
"    The call to NXGetKeyMapping() failed.\n"
"\n"
"The following diagnostic messages have significance only when using\n"
"dumpkeymap on a non-Apple/NeXT platform.\n"
"\n"
"Must specify at least one .keymapping file.\n"
"    No .keymapping files were mentioned on the command-line.  On\n"
"    non-Apple/NeXT platforms, there is no concept of a currently active\n"
"    .keymapping file, so at least one file must be mentioned on the\n"
"    command-line.\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print warranty.
//-----------------------------------------------------------------------------
static void print_warranty( FILE* f )
    {
    fputs(
"This software is provided by the author `AS IS' and any express or implied\n"
"WARRANTIES, including, but not limited to, the implied warranties of\n"
"MERCHANTABILITY and FITNESS FOR A PARTICULAR PURPOSE are DISCLAIMED.  In NO\n"
"EVENT shall the author be LIABLE for any DIRECT, INDIRECT, INCIDENTAL,\n"
"SPECIAL, EXEMPLARY, or CONSEQUENTIAL damages (including, but not limited\n"
"to, procurement of substitute goods or services; loss of use, data, or\n"
"profits; or business interruption) however caused and on any theory of\n"
"liability, whether in contract, strict liability, or tort (including\n"
"negligence or otherwise) arising in any way out of the use of this\n"
"software, even if advised of the possibility of such damage.\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print this program's version number.
//-----------------------------------------------------------------------------
static void print_version( FILE* f )
    {
    fputs( "Version " PROG_VERSION " (built " __DATE__ ")\n\n", f );
    }


//-----------------------------------------------------------------------------
// Print a usage summary.
//-----------------------------------------------------------------------------
static void print_usage( FILE* f )
    {
    fputs(
"Usage: dumpkeymap [options] [-] [file ...]\n"
"\n"
"Prints a textual representation of each Apple/NeXT .keymapping file\n"
"mentioned on the command-line.  If no files are mentioned and if the local\n"
"machine is an Apple or NeXT installation, then the key mapping currently in\n"
"use by the WindowServer and the AppKit is printed instead.\n"
"\n"
"Options:\n"
"    -h --help\n"
"        Display general program instructions and option summary.\n"
"\n"
"    -k --help-keymapping\n"
"        Display a detailed description of the internal layout of a\n"
"        .keymapping file.\n"
"\n"
"    -o --help-output\n"
"        Display an explanation of the output generated by dumpkeymap when\n"
"        dissecting a .keymapping file.\n"
"\n"
"    -f --help-files\n"
"        Display a summary of the various files and directories which are\n"
"        related to key mappings.\n"
"\n"
"    -d --help-diagnostics\n"
"        Display a list of the various diagnostic messages which may be\n"
"        emitted by dumpkeymap.\n"
"\n"
"    -v --version\n"
"        Display the dumpkeymap version number and warranty information.\n"
"\n"
"    - --\n"
"        Inhibit processing of options at this point in the argument list.\n"
"        An occurrence of `-' or `--' in the argument list causes all\n"
"        following arguments to be treated as file names even if an argument\n"
"        begins with a `-' character.\n"
"\n", f );
    }


//-----------------------------------------------------------------------------
// Print an informational banner.
//-----------------------------------------------------------------------------
static void print_banner( FILE* f )
    {
    fputs( "\n" PROG_NAME " v" PROG_VERSION " by " AUTHOR_INFO "\n"
	COPYRIGHT "\n\n", f );
    }


//-----------------------------------------------------------------------------
// Process command-line arguments.  Examine options first; collecting files
// along the way.  If all is well, process collected file list.
//-----------------------------------------------------------------------------
int main( int const argc, char const* const argv[] )
    {
    int rc = 0, i, nfiles = 0, more_options = 1, process_files = 1;
    int* files = (int*)calloc( argc - 1, sizeof(int) );
    print_banner( stdout );

    for (i = 1; i < argc; i++)
	{
	char const* const s = argv[i];
	if (!more_options || *s != '-')
	    files[ nfiles++ ] = i;
	else
	    {
	    OPT_SWITCH(s)
		OPT_CASE(-,--)
		    more_options = 0;
		OPT_CASE(-h,--help)
		    print_usage( stdout );
		    process_files = 0;
		OPT_CASE(-k,--help-keymapping)
		    print_internal_layout_info( stdout );
		    process_files = 0;
		OPT_CASE(-o,--help-output)
		    print_output_info( stdout );
		    process_files = 0;
		OPT_CASE(-f,--help-files)
		    print_files_info( stdout );
		    process_files = 0;
		OPT_CASE(-d,--help-diagnostics)
		    print_diagnostics_info( stdout );
		    process_files = 0;
		OPT_CASE(-v,--version)
		    print_version( stdout );
		    print_warranty( stdout );
		    process_files = 0;
		OPT_DEFAULT
		    fprintf( stderr, "ERROR: Unrecognized option: %s\n\n", s );
		    process_files = 0;
		    rc = 1;
	    OPT_SWITCH_END
	    }
	}

    if (process_files)
	{
	if (nfiles == 0)
	    rc = handle_empty_document_list();
	else
	    for (i = 0; i < nfiles; i++)
		rc |= unparse_keymap_file( argv[files[i]] );
	}

    free( files );
    return rc;
    }
