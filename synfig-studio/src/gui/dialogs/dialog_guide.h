/* === S Y N F I G ========================================================= */
/*!	\file dialogs/Dialog_Guide.h
**	\brief Template Header
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_DIALOG_GUIDE_H
#define __SYNFIG_GTKMM_DIALOG_GUIDE_H


/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>


#include <synfigapp/value_desc.h>
#include <synfig/valuenodes/valuenode_animated.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Waypoint;

class Dialog_Guide : public Gtk::Dialog
{
	Widget_Waypoint *waypointwidget;//what is this
	etl::handle<synfig::Canvas> canvas;

	void on_ok_pressed();

public:
	Dialog_Guide(Gtk::Window& parent,etl::handle<synfig::Canvas> canvas);
	~Dialog_Guide();


}; // END of Dialog_Guide

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // DIALOG_GUIDE_H
