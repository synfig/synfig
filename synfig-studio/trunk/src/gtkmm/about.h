/* === S Y N F I G ========================================================= */
/*!	\file about.h
**	\brief About dialog class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Paul Wise
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_ABOUT_H
#define __SYNFIG_GTKMM_ABOUT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/aboutdialog.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

#

namespace studio {

class About : public Gtk::AboutDialog
{
public:

	About();
	void close(int);
	void on_link_clicked(Gtk::AboutDialog&, const Glib::ustring &url);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
