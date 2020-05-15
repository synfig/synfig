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

/**
 * \brief Base class for widgets that are graph-like representations with time axis
 *
 * Derive this class in order to create graph-like widgets that must be in sync
 * with the time set for the animation: it allows to easily react to time-line
 * zooming and panning/scrolling.
 *
 * It also supports a vertical adjustment, allowing to zoom and pan/scroll in
 * Y-axis too. It can be set for an external Gtk::ScrollBar to control this widget.
 * Retrieve it by Widget_TimeGraphBase::get_range_adjustment().
 *
 * A derived class can support the zoom/pan/scroll actions by checking Widget_TimeGraphBase#time_plot_data
 * methods that provide conversion between (horizontal) pixels <-> time values
 * as well between (vertical) pixels <-> real Y values (the meaning of Y depends on each case).
 *
 * The effects of those three actions are automatically mapped in Widget_TimeGraphBase#time_plot_data.
 *
 * After class instancing, it is mandatory to set a TimeModel via Widget_TimeGraphBase::set_time_model(),
 * otherwise it is impossible to do the pixel coordinates <-> time value conversions.
 */
class Widget_TimeGraphBase : public Gtk::DrawingArea
{
public:
	Widget_TimeGraphBase();
	virtual ~Widget_TimeGraphBase();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	virtual const etl::handle<TimeModel>& get_time_model() const;
	virtual void set_time_model(const etl::handle<TimeModel> &x);

	//! Zoom in along vertical-axis. \sa Widget_TimeGraphBase#zoom_changing_factor
	virtual void zoom_in();
	//! Zoom out along vertical-axis. \sa Widget_TimeGraphBase#zoom_changing_factor
	virtual void zoom_out();
	//! Alias for set_zoom(1.0);
	virtual void zoom_100();
	virtual void set_zoom(double new_zoom_factor);
	virtual double get_zoom() const;

	//! Scroll vertically by step_increment units of Widget_TimeGraphBase#range_adjustment
	virtual void scroll_up();
	virtual void scroll_down();

	virtual void pan(int dx, int dy, int /*total_dx*/, int /*total_dy*/);

	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() const;
	void set_canvas_interface(const etl::handle<synfigapp::CanvasInterface>& value);

protected:
	etl::handle<synfigapp::CanvasInterface> canvas_interface;
	virtual void on_canvas_interface_changed();

	Glib::RefPtr<Gtk::Adjustment> range_adjustment;
	TimePlotData * time_plot_data;

	//! Multiplier zoom factor for Widget_TimeGraphBase::zoom_in() and Widget_TimeGraphBase::zoom_out()
	//! Example: if it equals to 2.0, zoom_in() doubles current zoom value, whereas zoom_out() reduces by half
	double zoom_changing_factor;

	//! Set the page size of Widget_TimeGraphBase#range_adjustment when zoom is set to 100%
	void set_default_page_size(double new_value);
	double get_default_page_size() const;

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	virtual void on_time_model_changed();

	//! Draw a vertical line representing the current time in Synfig Studio
	//! To be used in Widget_TimeGraphBase::on_draw() override implementation.
	void draw_current_time(const Cairo::RefPtr<Cairo::Context> &cr) const;
	//! Draw a vertical line marking the keyframe time
	//! To be used in Widget_TimeGraphBase::on_draw() override implementation.
	void draw_keyframe_line(const Cairo::RefPtr<Cairo::Context> &cr, const synfig::Keyframe& keyframe) const;

private:
	double default_page_size;
	sigc::connection time_model_changed_connection;
};

}

#endif // SYNFIG_STUDIO_WIDGET_TIMEGRAPHBASE_H
