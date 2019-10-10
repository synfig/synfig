/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_color.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_COLOR_H
#define __SYNFIG_STUDIO_WIDGET_COLOR_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {


void render_color_to_window(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &ca, const synfig::Color &color);

class Widget_Color : public Gtk::DrawingArea
{
private:
	synfig::Color color;
	synfig::Gamma gamma;
	sigc::signal<void> signal_activate_;
	sigc::signal<void> signal_middle_click_;
	sigc::signal<void> signal_right_click_;

public:
	sigc::signal<void>& signal_activate() { return signal_activate_; }
	sigc::signal<void>& signal_clicked() { return signal_activate_; }
	sigc::signal<void>& signal_middle_click() { return signal_middle_click_; }
	sigc::signal<void>& signal_right_click() { return signal_right_click_; }

	const synfig::Color& get_value() const;
	void set_value(const synfig::Color &x);

	Widget_Color();
	~Widget_Color();
	
protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);
}; // END of class Widget_Color

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
