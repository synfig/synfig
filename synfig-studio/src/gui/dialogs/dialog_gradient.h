/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_gradient.h
**	\brief Template Header
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DIALOG_GRADIENT_H
#define __SYNFIG_STUDIO_DIALOG_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>

#include <gui/dialogsettings.h>

#include <synfig/gradient.h>
#include <synfig/time.h>

#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; class SpinButton; };

namespace synfigapp {
class CanvasInterface;
};

namespace studio {

class Widget_Gradient;
class Widget_ColorEdit;

class Dialog_Gradient : public Gtk::Dialog
{
	DialogSettings dialog_settings;

	Gtk::SpinButton *spinbutton_pos;

	Glib::RefPtr<Gtk::Adjustment> adjustment_pos;


	sigc::signal<void,synfig::Gradient> signal_edited_;

	sigc::connection value_changed_connection;

	void on_set_default_pressed();

	void on_cpoint_selected(synfig::Gradient::CPoint x);
	void on_values_adjusted();

	Widget_Gradient* widget_gradient;
	Widget_ColorEdit* widget_color;
	Gtk::Button *set_default_button;

	void on_changed();

public:

	sigc::signal<void,synfig::Gradient>& signal_edited() { return signal_edited_; }

	void set_gradient(const synfig::Gradient& x);

	const synfig::Gradient& get_gradient()const;

	void set_default_button_set_sensitive(bool sensitive) { set_default_button->set_sensitive(sensitive); }

	void reset();


	Dialog_Gradient();
	~Dialog_Gradient();
	//! Interface to external calls to fill in the Gradient Editor Dialog
	//! when a Constant ValueNode or a Animated ValueNode is double cliked.
	void edit(const synfigapp::ValueDesc &x, etl::handle<synfigapp::CanvasInterface> canvas_interface, synfig::Time time=0);
}; // END of Dialog_Gradient

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
