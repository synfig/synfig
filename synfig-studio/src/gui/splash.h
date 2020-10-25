/* === S Y N F I G ========================================================= */
/*!	\file splash.h
**	\brief Header File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_SPLASH_H
#define __SYNFIG_GTKMM_SPLASH_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/window.h>

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

	Gtk::Label *versionlabel;
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
