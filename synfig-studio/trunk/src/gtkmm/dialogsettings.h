/* === S I N F G =========================================================== */
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

#ifndef __SINFG_DIALOGSETTINGS_H
#define __SINFG_DIALOGSETTINGS_H

/* === H E A D E R S ======================================================= */

#include <sinfgapp/settings.h>
#include <gtkmm/window.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class DialogSettings : public sinfgapp::Settings
{
	Gtk::Window* window;
	sinfg::String name;
public:
	DialogSettings(Gtk::Window* window,const sinfg::String& name);
	virtual ~DialogSettings();

	virtual bool get_value(const sinfg::String& key, sinfg::String& value)const;
	virtual bool set_value(const sinfg::String& key,const sinfg::String& value);
	virtual KeyList get_key_list()const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
