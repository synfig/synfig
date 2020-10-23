/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_keyframe.h
**	\brief Keyframe properties dialog Header
**
**	$Id$
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

#ifndef __SYNFIG_STUDIO_DIALOG_KEYFRAME_H
#define __SYNFIG_STUDIO_DIALOG_KEYFRAME_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class Widget_WaypointModel;

class Dialog_Keyframe : public Gtk::Dialog
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface;

	synfig::Keyframe keyframe_;

	Gtk::Entry entry_description;
	Gtk::CheckButton entry_toogle;

	Widget_WaypointModel* widget_waypoint_model;

	void on_ok_pressed();
	void on_apply_pressed();
	void on_delete_pressed();

public:
	Dialog_Keyframe(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~Dialog_Keyframe();

	const synfig::Keyframe& get_keyframe()const;
	void set_keyframe(const synfig::Keyframe& x);

private:

}; // END of class Dialog_Keyframe

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
