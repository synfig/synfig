/* === S Y N F I G ========================================================= */
/*!	\file dialog_gradient.h
**	\brief Template Header
**
**	$Id: dialog_gradient.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/checkbutton.h>

#include <synfig/gamma.h>
#include <synfig/time.h>

#include "widget_gradient.h"
#include "widget_coloredit.h"

#include <synfigapp/value_desc.h>
#include <synfig/time.h>

#include "dialogsettings.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; class SpinButton; class Adjustment; };

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

	Gtk::Adjustment adjustment_pos;


	sigc::signal<void,synfig::Gradient> signal_edited_;

	sigc::connection value_changed_connection;

	void on_ok_pressed();
	void on_apply_pressed();
	void on_grab_pressed();

	void on_cpoint_selected(synfig::Gradient::CPoint x);
	void on_values_adjusted();

	Widget_Gradient* widget_gradient;
	Widget_ColorEdit* widget_color;

	void on_changed();

public:

	sigc::signal<void,synfig::Gradient>& signal_edited() { return signal_edited_; }

	void set_gradient(const synfig::Gradient& x);

	const synfig::Gradient& get_gradient()const { return widget_gradient->get_value(); }

	void reset();


	Dialog_Gradient();
	~Dialog_Gradient();

	void edit(const synfigapp::ValueDesc &x, etl::handle<synfigapp::CanvasInterface> canvas_interface, synfig::Time x=0);
}; // END of Dialog_Gradient

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
