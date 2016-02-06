/* === S Y N F I G ========================================================= */
/*!	\file splash.h
**	\brief Header File
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_SPLASH_H
#define __SYNFIG_GTKMM_SPLASH_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ProgressCallback; };

namespace studio {

class SplashProgress;

class Splash : public Gtk::Window
{
	friend class SplashProgress;

	SplashProgress *cb;

	Gtk::Label *tasklabel;
	Gtk::ProgressBar *progressbar;

public:

	synfig::ProgressCallback *get_callback();

	void enable_startup_notification();

	Splash();
	~Splash();
};

}

/* === E N D =============================================================== */

#endif
