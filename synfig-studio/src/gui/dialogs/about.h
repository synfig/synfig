/* === S Y N F I G ========================================================= */
/*!	\file about.h
**	\brief About dialog class
**
**	\legal
**	Copyright (c) 2008 Paul Wise
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

namespace studio {

class About : public Gtk::AboutDialog
{
public:
	About();
	void close(int);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
