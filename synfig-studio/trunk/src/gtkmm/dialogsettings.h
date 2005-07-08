/* === S Y N F I G ========================================================= */
/*!	\file dialogsettings.h
**	\brief Template Header
**
**	$Id: dialogsettings.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_DIALOGSETTINGS_H
#define __SYNFIG_DIALOGSETTINGS_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/settings.h>
#include <gtkmm/window.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class DialogSettings : public synfigapp::Settings
{
	Gtk::Window* window;
	synfig::String name;
public:
	DialogSettings(Gtk::Window* window,const synfig::String& name);
	virtual ~DialogSettings();

	virtual bool get_value(const synfig::String& key, synfig::String& value)const;
	virtual bool set_value(const synfig::String& key,const synfig::String& value);
	virtual KeyList get_key_list()const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
