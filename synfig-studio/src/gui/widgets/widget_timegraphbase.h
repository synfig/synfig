/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timegraphbase.h
**	\brief Base class for widgets that are graph-like representations with time axis
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_TIMEGRAPHBASE_H
#define SYNFIG_STUDIO_WIDGET_TIMEGRAPHBASE_H

#include <gtkmm/drawingarea.h>
#include <gtkmm/adjustment.h>

#include <synfigapp/canvasinterface.h>
#include <gui/timemodel.h>

namespace studio {

class TimePlotData;

class Widget_TimeGraphBase : public Gtk::DrawingArea
{
public:
	Widget_TimeGraphBase();
	virtual ~Widget_TimeGraphBase();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	virtual const etl::handle<TimeModel>& get_time_model() const;
	virtual void set_time_model(const etl::handle<TimeModel> &x);

	virtual void zoom_in();
	virtual void zoom_out();
	virtual void zoom_100();
	virtual void set_zoom(double new_zoom_factor);
	virtual double get_zoom() const;

	virtual void scroll_up();
	virtual void scroll_down();

	virtual void pan(int dx, int dy, int /*total_dx*/, int /*total_dy*/);

protected:
	etl::handle<synfigapp::CanvasInterface> canvas_interface;

	Glib::RefPtr<Gtk::Adjustment> range_adjustment;
	TimePlotData * time_plot_data;

	double zoom_changing_factor;

	void set_default_page_size(double new_value);
	double get_default_page_size() const;

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	void draw_current_time(const Cairo::RefPtr<Cairo::Context> &cr);

private:
	double default_page_size;
};

}

#endif // SYNFIG_STUDIO_WIDGET_TIMEGRAPHBASE_H
