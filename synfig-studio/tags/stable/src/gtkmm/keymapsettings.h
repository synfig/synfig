/* === S I N F G =========================================================== */
/*!	\file keymapsettings.h
**	\brief Defines the structures for managing key map settings
**
**	$Id: keymapsettings.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_KEYMAPSETTINGS_H
#define __SINFG_KEYMAPSETTINGS_H

/* === H E A D E R S ======================================================= */
#include <gtkmm/dialog.h>

#include <set>
#include <map>
#include <string>

#include <gtkmm/accelkey.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//a dialog for viewing and setting options, though it can also be used just as is
class KeyMapSettings : public Gtk::Dialog
{
	struct AcKeyInfo
	{
		guint				key;
		Gdk::ModifierType 	mod;
		
		bool				on;
		
		AcKeyInfo(guint k = 0, Gdk::ModifierType m = Gdk::ModifierType())
		:key(k),mod(m) {}
	};
	
	//std::map<const char *,AcKeyInfo>	pathmap; //uses string info from paths set
	//std::set<std::string>				accelpaths;
	
	bool unsaved; //Assume as such...
		
public:

	KeyMapSettings();
	~KeyMapSettings();

	//void add_path(const char *path);
	
	bool set_key(const char *path, guint key, Gdk::ModifierType mod, bool replace = true);
	bool get_key(const char *path, Gtk::AccelKey *key);

	// These files must be sent a filename without extension (so the key map can be obtained)
	bool load(const char *filename);
	bool save(const char *filename);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
