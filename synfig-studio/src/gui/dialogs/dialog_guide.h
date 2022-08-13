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
#include <gtkmm/spinbutton.h>
#include <gtkmm/comboboxtext.h>

#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Waypoint;
class Widget_ValueBase;
class WorkArea;

class Dialog_Guide : public Gtk::Dialog
{
	typedef std::list<float> GuideList;

	Gtk::SpinButton *angle_widget;
	Glib::RefPtr<Gtk::Adjustment> angle_adjustment;

	Gtk::SpinButton *center_x_widget;
	Glib::RefPtr<Gtk::Adjustment> center_x_widget_adjust;

	Gtk::SpinButton *center_y_widget;
	Glib::RefPtr<Gtk::Adjustment> center_y_widget_adjust;

	Gtk::SpinButton *point_x_widget;
	Glib::RefPtr<Gtk::Adjustment> point_x_widget_adjust;

	Gtk::SpinButton *point_y_widget;
	Glib::RefPtr<Gtk::Adjustment> point_y_widget_adjust;

	Gtk::ComboBoxText angle_type_picker;


	WorkArea *current_work_area;

	etl::handle<synfig::Canvas> canvas;

	void on_ok_or_apply_pressed(bool ok);
	void rotate_ruler();
	void set_new_coordinates();
	void set_angle_type();

	GuideList::iterator curr_guide;
	GuideList::iterator curr_guide_accomp_duckamtic;
	GuideList::iterator curr_guide_accomp_duckamtic_other;

	float angle_rad;
	float angle_deg;

	synfig::ValueBase test_value;

	bool menu_guide_is_x;
	bool degrees;


public:
	Dialog_Guide(Gtk::Window& parent, etl::handle<synfig::Canvas> canvas, WorkArea *work_area);
	~Dialog_Guide();

	void set_current_guide_iterators(GuideList::iterator curr_guide_workarea,
									 GuideList::iterator curr_guide_accomp_duckamtic_workarea,
									 GuideList::iterator curr_guide_accomp_duckamtic_other_workarea);

	void set_rotation_angle(bool curr_guide_is_x);//must be called after set_current_guide_iterators()

}; // END of Dialog_Guide

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // DIALOG_GUIDE_H
