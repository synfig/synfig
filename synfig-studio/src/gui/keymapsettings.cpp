/* === S Y N F I G ========================================================= */
/*!	\file keymapsettings.cpp
**	\brief Contains Info for Key Map settings
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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

#include <synfig/general.h>

#include <gtkmm/accelkey.h>
#include <gtkmm/accelmap.h>
#include <gtk/gtk.h>

#include <gui/localization.h>

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
KeyMapSettings::KeyMapSettings()/*:
	unsaved()*/
{ }

KeyMapSettings::~KeyMapSettings()
{ }

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

bool KeyMapSettings::get_key(const char *path, Gtk::AccelKey *key)
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
