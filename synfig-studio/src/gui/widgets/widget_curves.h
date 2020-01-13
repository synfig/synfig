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

#include "selectdraghelper.h"

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
	friend class ChannelPointSD;
	friend class TcbHandleSD;
private:
	struct Channel;
	struct CurveStruct;
	struct ChannelPoint {
		std::list<CurveStruct>::iterator curve_it;
		synfig::TimePoint time_point;
		size_t channel_idx;

		ChannelPoint();
		ChannelPoint(std::list<CurveStruct>::iterator &curve_it, const synfig::TimePoint time_point, size_t channel_idx);

		void invalidate();
		bool is_valid() const;
		bool is_draggable() const;

		bool operator ==(const ChannelPoint &b) const;
		bool operator !=(const ChannelPoint &b) const {return !operator==(b);}

		synfig::Real get_value(synfig::Real time_tolerance) const;
	};

	class ChannelPointSD : public SelectDragHelper<ChannelPoint> {
		Widget_Curves & widget;
	public:
		ChannelPointSD(Widget_Curves &widget);
		virtual ~ChannelPointSD() override {}

		void get_item_position(const ChannelPoint &item, Gdk::Point &position) override;

		bool find_item_at_position(int pos_x, int pos_y, ChannelPoint & cp) override;
		bool find_items_in_rect(Gdk::Rectangle rect, std::vector<ChannelPoint> & list) override;
		void get_all_items(std::vector<ChannelPoint> & items) override;
		void delta_drag(int total_dx, int total_dy, bool by_keys) override;

		// Check if waypoint (of curve_it at time_point no matter channel) is selected
		bool is_waypoint_selected(const ChannelPoint& point) const;
	} channel_point_sd;

	class TcbHandle {
	private:
		synfig::Waypoint waypoint;
		std::shared_ptr<const synfig::Waypoint> previous_waypoint;
		std::shared_ptr<const synfig::Waypoint> next_waypoint;
	public:
		enum Param {Tension, Continuity, Bias, TemporalTension, Invalid} param;
		size_t channel_index;

		TcbHandle();
		TcbHandle(Param param_);

		void set_waypoint(const synfig::Waypoint& waypoint);
		synfig::Waypoint& get_waypoint();
		const synfig::Waypoint* get_previous() const;
		const synfig::Waypoint* get_next() const;

		void invalidate();
		bool is_valid() const;
//		bool is_draggable() const;

		bool operator ==(const TcbHandle &b) const;
		bool operator !=(const TcbHandle &b) const {return !operator==(b);}

		void draw(const Cairo::RefPtr<Cairo::Context> &cr, bool hovered, const TimePlotData &time_plot_data, unsigned int waypoint_edge_length) const;
		bool get_waypoint_position(Gdk::Point &p, const TimePlotData& time_plot_data) const;
		bool get_position(Gdk::Point &p, const TimePlotData& time_plot_data) const;

		synfig::Real get_tangent() const;
	};

	class TcbHandleSD : public SelectDragHelper<TcbHandle> {
		Widget_Curves & widget;
	public:
		TcbHandleSD(Widget_Curves &widget);
		virtual void get_item_position(const TcbHandle &item, Gdk::Point &position) override;
		virtual bool find_item_at_position(int pos_x, int pos_y, TcbHandle& tcb_handle) override;
		virtual bool find_items_in_rect(Gdk::Rectangle /*rect*/, std::vector<TcbHandle>& /*list*/) override {return false;}
		virtual void get_all_items(std::vector<TcbHandle>& /*items*/) override {}
		virtual void delta_drag(int total_dx, int total_dy, bool by_keys) override;
	} tcb_handle_sd;

	etl::handle<synfigapp::CanvasInterface> canvas_interface;

	Glib::RefPtr<Gtk::Adjustment> range_adjustment;

	std::list<CurveStruct> curve_list;

	std::list<sigc::connection> value_desc_changed;

	TimePlotData * time_plot_data;

	int waypoint_edge_length;

	std::vector<std::pair<synfig::Waypoint, std::list<CurveStruct>::iterator> > overlapped_waypoints;

	void on_waypoint_selection_changed();
	void on_waypoint_clicked(const ChannelPoint &cp, unsigned int button, Gdk::Point /*point*/);
	void on_waypoint_double_clicked(const ChannelPoint &cp, unsigned int button, Gdk::Point /*point*/);

	sigc::signal<void, synfigapp::ValueDesc, std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int> signal_waypoint_clicked_;
	sigc::signal<void, synfigapp::ValueDesc, std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int> signal_waypoint_double_clicked_;

	std::vector<TcbHandle> tcb_handles;
	void refresh_tcb_handles();

public:
	Widget_Curves();
	~Widget_Curves();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	const etl::handle<TimeModel>& get_time_model() const;
	void set_time_model(const etl::handle<TimeModel> &x);

	void set_value_descs(etl::handle<synfigapp::CanvasInterface> canvas_interface_, const std::list< std::pair<std::string, synfigapp::ValueDesc> > &data);
	void clear();
	void refresh();

	void zoom_in();
	void zoom_out();
	void zoom_100();
	void set_zoom(double new_zoom_factor);
	double get_zoom() const;

	void scroll_up();
	void scroll_down();

	void pan(int dx, int dy, int total_dx, int total_dy);

	void select_all_points();

	sigc::signal<void, synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int>& signal_waypoint_clicked() { return signal_waypoint_clicked_; }
	sigc::signal<void, synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int>& signal_waypoint_double_clicked() { return signal_waypoint_double_clicked_; }

protected:
	bool on_event(GdkEvent *event);
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	void delete_selected();
	bool add_waypoint_to(int point_x, int point_y);
}; // END of class Widget_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
