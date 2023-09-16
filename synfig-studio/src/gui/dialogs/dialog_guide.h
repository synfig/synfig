/* === S Y N F I G ========================================================= */
/*!	\file dialogs/Dialog_Guide.h
**	\brief Dialog for editing guides.
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

#ifndef SYNFIG_STUDIO_DIALOG_GUIDE_H
#define SYNFIG_STUDIO_DIALOG_GUIDE_H


/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/spinbutton.h>
#include <gui/widgets/widget_distance.h>
#include <gtkmm/comboboxtext.h>

#include <synfigapp/value_desc.h>
#include <synfig/real.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Waypoint;
class Widget_ValueBase;
class WorkArea;
struct Guide;

class Dialog_Guide : public Gtk::Dialog
{
	typedef std::list<Guide> GuideList;

	synfig::Canvas::Handle canvas;
	WorkArea* current_work_area;

	Gtk::SpinButton* angle_widget;
	Widget_Distance* x_widget;
	Widget_Distance* y_widget;
	Glib::RefPtr<Gtk::Adjustment> angle_adjustment;
	Glib::RefPtr<Gtk::Adjustment> x_adjustment;
	Glib::RefPtr<Gtk::Adjustment> y_adjustment;

	Gtk::ComboBoxText angle_type_picker;
	Gtk::ComboBoxText distance_system_picker;

	void on_ok_or_apply_pressed(bool ok);
	void set_angle_type();
	void set_distance_system();
	void init_widget_values();

	GuideList::iterator curr_guide;

	bool menu_guide_is_x;
	bool degrees;

public:
	Dialog_Guide(Gtk::Window& parent, etl::handle<synfig::Canvas> canvas, WorkArea* work_area);
	~Dialog_Guide();
	void set_current_guide(GuideList::iterator current_guide);

}; // END of Dialog_Guide

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // DIALOG_GUIDE_H
