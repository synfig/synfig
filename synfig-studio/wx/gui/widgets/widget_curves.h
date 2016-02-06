/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_curves.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_CURVES_H
#define __SYNFIG_STUDIO_WIDGET_CURVES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/layout.h>
#include <synfig/color.h>
#include <synfigapp/value_desc.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Curves : public Gtk::DrawingArea
{
	struct Channel;
	struct CurveStruct;

	Glib::RefPtr<Gtk::Adjustment> time_adjustment_;
	Glib::RefPtr<Gtk::Adjustment> range_adjustment_;

	std::list<CurveStruct> curve_list_;

public:

	Widget_Curves();
	~Widget_Curves();

	void set_value_descs(std::list<synfigapp::ValueDesc> value_descs);
	void clear();
	void refresh();

	Glib::RefPtr<Gtk::Adjustment> get_range_adjustment() { return range_adjustment_; }
	Glib::RefPtr<Gtk::Adjustment> get_time_adjustment() { return time_adjustment_; }
	void set_time_adjustment(const Glib::RefPtr<Gtk::Adjustment>&);

private:
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);

}; // END of class Widget_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
