/* === S Y N F I G ========================================================= */
/*!	\file keymapsettings.cpp
**	\brief Contains Info for Key Map settings
**
**	$Id: keymapsettings.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "keymapsettings.h"

#include <gtkmm/accelkey.h>
#include <gtkmm/accelmap.h>
#include <gtk/gtkaccelmap.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
//using namespace synfig;
using namespace studio;

using namespace Gtk;
//using namespace Gtk::Menu_Helpers;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

// KeyMapSettings Definitions
KeyMapSettings::KeyMapSettings()
{
}

KeyMapSettings::~KeyMapSettings()
{
}

bool KeyMapSettings::set_key(const char *path, guint key, Gdk::ModifierType mod, bool replace)
{
	if(gtk_accel_map_lookup_entry(path,NULL))
	{
		return AccelMap::change_entry(path,key,mod,replace);
	}else
	{
		AccelMap::add_entry(path,key,mod);
		return true;
	}
}

bool KeyMapSettings::get_key(const char *path, AccelKey *key)
{
	GtkAccelKey	ac;
	if(gtk_accel_map_lookup_entry(path,&ac))
	{
		*key = AccelKey(ac.accel_key,(Gdk::ModifierType)ac.accel_mods,string(path));
		return true;
	}
	
	return false;
}

bool KeyMapSettings::load(const char *filename)
{
	string n(filename);
	n += ".skm";
	
	AccelMap::load(filename);
	
	return true;	
}

bool KeyMapSettings::save(const char *filename)
{
	string n(filename);
	n += ".skm";
	
	AccelMap::save(filename);
	
	return true;
}
