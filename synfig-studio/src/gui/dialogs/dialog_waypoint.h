/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_waypoint.h
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

#ifndef __SYNFIG_GTKMM_DIALOG_WAYPOINT_H
#define __SYNFIG_GTKMM_DIALOG_WAYPOINT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>

#include <synfigapp/value_desc.h>
#include <synfig/valuenodes/valuenode_animated.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_ValueBase;
class Widget_Waypoint;

class Dialog_Waypoint : public Gtk::Dialog
{
	Widget_Waypoint *waypointwidget;
	etl::handle<synfig::Canvas> canvas;
	synfigapp::ValueDesc value_desc_;

	sigc::signal<void> signal_changed_;

	sigc::signal<void> signal_delete_;
	void on_ok_pressed();
	void on_apply_pressed();
	void on_delete_pressed();

	sigc::connection value_desc_changed;

	void refresh();

public:
	Dialog_Waypoint(Gtk::Window& parent,etl::handle<synfig::Canvas> canvas);
	~Dialog_Waypoint();

    void reset();

	void set_value_desc(synfigapp::ValueDesc value_desc);
	synfigapp::ValueDesc get_value_desc()const { return value_desc_; }

	void set_waypoint(synfig::ValueNode_Animated::Waypoint x);
	const synfig::ValueNode_Animated::Waypoint &get_waypoint()const;

	sigc::signal<void> &signal_changed()
	{return signal_changed_; }

	sigc::signal<void> &signal_delete()
	{return signal_delete_; }
}; // END of Dialog_Waypoint

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
