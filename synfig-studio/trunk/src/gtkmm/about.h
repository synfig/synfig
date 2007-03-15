/* === S Y N F I G ========================================================= */
/*!	\file about.h
**	\brief Header File
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_ABOUT_H
#define __SYNFIG_GTKMM_ABOUT_H

/* === H E A D E R S ======================================================= */

//#include <gtk/gtk.h>
#include <gtkmm/window.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ProgressCallback; };

namespace studio {

class AboutProgress;

class About : public Gtk::Window
{
	friend class AboutProgress;

	AboutProgress *cb;

	Gtk::Tooltips _tooltips;

	Gtk::Label *tasklabel;
	Gtk::ProgressBar *progressbar;
	Gtk::Button *CloseButton;

	void close();

	bool can_self_destruct;

public:

	synfig::ProgressCallback *get_callback();

	void set_can_self_destruct(bool x);

	About();
	~About();
};

}

/* === E N D =============================================================== */

#endif
