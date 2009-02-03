/* === S Y N F I G ========================================================= */
/*!	\file dialog_color.h
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

#ifndef __SYNFIG_STUDIO_DIALOG_COLOR_H
#define __SYNFIG_STUDIO_DIALOG_COLOR_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/tooltips.h>

#include <synfig/gamma.h>
#include <synfig/time.h>

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

class Widget_Color;

class Dialog_Color : public Gtk::Dialog
{
	DialogSettings dialog_settings;
	Gtk::Tooltips tooltips;

	sigc::signal<void,synfig::Color> signal_edited_;
	//sigc::signal<void,synfig::Color> signal_apply_;

	bool on_close_pressed();
	void on_apply_pressed();
	void on_set_fg_pressed();
	void on_set_bg_pressed();
	void on_color_changed();

	Widget_ColorEdit* widget_color;

	bool busy_;

public:
	bool busy()const { return busy_; }

	sigc::signal<void,synfig::Color>& signal_edited() { return signal_edited_; }

	//sigc::signal<void,synfig::Color>& signal_apply() { return signal_apply_; }

	void set_color(const synfig::Color& x) { widget_color->set_value(x); }

	synfig::Color get_color()const { return widget_color->get_value(); }

	void reset();


	Dialog_Color();
	~Dialog_Color();

//	void edit(const synfigapp::ValueDesc &x, etl::handle<synfigapp::CanvasInterface> canvas_interface, synfig::Time x=0);
}; // END of Dialog_Color

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
