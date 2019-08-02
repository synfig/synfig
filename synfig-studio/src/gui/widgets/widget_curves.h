/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_curves.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2018 Ivan Mahonin
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

#include <list>

#include <gtkmm/drawingarea.h>
#include <gtkmm/adjustment.h>

#include <synfigapp/value_desc.h>

#include <gui/timemodel.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct TimePlotData;

class Widget_Curves: public Gtk::DrawingArea
{
private:
	struct Channel;
	struct CurveStruct;

	Glib::RefPtr<Gtk::Adjustment> range_adjustment;

	std::list<CurveStruct> curve_list;

	std::list<sigc::connection> value_desc_changed;

	TimePlotData * time_plot_data;

public:
	Widget_Curves();
	~Widget_Curves();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	const etl::handle<TimeModel>& get_time_model() const;
	void set_time_model(const etl::handle<TimeModel> &x);

	void set_value_descs(const std::list<synfigapp::ValueDesc> &value_descs);
	void clear();
	void refresh();

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);
}; // END of class Widget_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
