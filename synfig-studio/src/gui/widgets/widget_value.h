/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_value.h
**	\brief Template File
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

#ifndef __SYNFIG_GTKMM_VALUE_H
#define __SYNFIG_GTKMM_VALUE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>

#include <synfig/canvas.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>

#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_BoneChooser;
class Widget_ColorEdit;
class Widget_CanvasChooser;
class Widget_Enum;
class Widget_Sublayer;
class Widget_Filename;
class Widget_Vector;
class Widget_Time;
class Widget_Distance;
class Widget_FontFamily;

class Widget_ValueBase : public Gtk::Box
{
	Gtk::Label *label;
	synfig::ValueBase value;

	Widget_Vector *vector_widget;
	Gtk::SpinButton *real_widget;
	Glib::RefPtr<Gtk::Adjustment> real_adjustment;
	Gtk::SpinButton *integer_widget;
	Glib::RefPtr<Gtk::Adjustment> integer_adjustment;
	Gtk::SpinButton *angle_widget;
	Glib::RefPtr<Gtk::Adjustment> angle_adjustment;

	Gtk::CheckButton *bool_widget;
	//Gtk::ColorSelection *color_widget;
	Widget_BoneChooser *bone_widget;
	Widget_ColorEdit *color_widget;
	Widget_CanvasChooser *canvas_widget;
	Widget_Enum *enum_widget;
	Widget_Sublayer *sublayer_widget;
	Widget_Filename *filename_widget;
	Widget_Time *time_widget;
	Gtk::Entry *string_widget;
	Widget_Distance *distance_widget;
	Widget_FontFamily *fontfamily_widget;

//	std::string hint;

	synfig::ParamDesc param_desc;
	synfigapp::ValueDesc value_desc;
	synfig::ParamDesc child_param_desc;
	synfig::Canvas::Handle canvas;
	sigc::signal<void> signal_value_changed_;
	sigc::signal<void> signal_activate_;

public:
	sigc::signal<void> &signal_value_changed() { return signal_value_changed_; }

	void activate();

	sigc::signal<void>& signal_activate() { return signal_activate_; }

	void set_value(const synfig::ValueBase &data);
	const synfig::ValueBase &get_value();

	void on_grab_focus();

	void set_param_desc(const synfig::ParamDesc &x) { param_desc=x; }
	const synfig::ParamDesc &get_param_desc() { return param_desc; }

	void set_value_desc(const synfigapp::ValueDesc &x) { value_desc=x; }
	const synfigapp::ValueDesc &get_value_desc() { return value_desc; }

	void set_child_param_desc(const synfig::ParamDesc &x) { child_param_desc=x; }
	const synfig::ParamDesc &get_child_param_desc() { return child_param_desc; }

	void set_sensitive(bool x);

	/// popup combobox menu if it is an enum entry
	void popup_combobox();

	//void set_hint(std::string x) { hint=x; }
//	std::string get_hint() { return hint; }

	void set_canvas(synfig::Canvas::Handle x);
	void inside_cellrenderer();
	Widget_ValueBase();
	~Widget_ValueBase();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
