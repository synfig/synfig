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
#include <synfigapp/canvasinterface.h>

#include <gui/timemodel.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {
namespace Action {
class PassiveGrouper;
}
}

namespace studio {

struct TimePlotData;

class Widget_Curves: public Gtk::DrawingArea
{
private:
	struct Channel;
	struct CurveStruct;
	struct ChannelPoint {
		std::list<CurveStruct>::iterator curve_it;
		synfig::TimePoint time_point;
		int channel_idx;

		ChannelPoint();
		ChannelPoint(std::list<CurveStruct>::iterator &curve_it, const synfig::TimePoint time_point, int channel_idx);
		void invalidate();
		bool is_valid() const;
		bool is_draggable() const;

		bool operator ==(const ChannelPoint &b) const;
		bool operator !=(const ChannelPoint &b) const {return !operator==(b);}

		synfig::Real get_value(synfig::Real time_tolerance) const;
	};

	etl::handle<synfigapp::CanvasInterface> canvas_interface;

	Glib::RefPtr<Gtk::Adjustment> range_adjustment;

	std::list<CurveStruct> curve_list;

	std::list<sigc::connection> value_desc_changed;

	TimePlotData * time_plot_data;

	int waypoint_edge_length;

	ChannelPoint hovered_point;

	std::vector<ChannelPoint> selected_points;

	bool find_channelpoint_at_position(int pos_x, int pos_y, ChannelPoint & cp);
	bool find_channelpoints_in_rect(Gdk::Rectangle rect, std::vector<ChannelPoint> & list);

	enum {POINTER_NONE, POINTER_DRAGGING, POINTER_SELECTING} pointer_state;
	int pointer_tracking_start_x, pointer_tracking_start_y;
	ChannelPoint active_point;
	int active_point_initial_y;
	void start_dragging(const ChannelPoint &pointed_item);
	void drag(int pointer_x, int pointer_y);
	void finish_dragging();
	void cancel_dragging();
	bool made_dragging_move;
	synfigapp::Action::PassiveGrouper *group;

	static void set_value_base_for_channel_point(synfig::ValueBase &value_base, const ChannelPoint &channel_point, synfig::Real v);

public:
	Widget_Curves();
	~Widget_Curves();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	const etl::handle<TimeModel>& get_time_model() const;
	void set_time_model(const etl::handle<TimeModel> &x);

	void set_value_descs(etl::handle<synfigapp::CanvasInterface> canvas_interface_, const std::list<synfigapp::ValueDesc> &value_descs);
	void clear();
	void refresh();

	void zoom_in();
	void zoom_out();
	void zoom_100();
	void set_zoom(double new_zoom_factor);
	double get_zoom() const;

	void scroll_up();
	void scroll_down();

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);
}; // END of class Widget_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
