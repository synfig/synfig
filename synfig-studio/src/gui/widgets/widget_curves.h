/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_curves.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2018 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_WIDGET_CURVES_H
#define __SYNFIG_STUDIO_WIDGET_CURVES_H

/* === H E A D E R S ======================================================= */

#include <gui/selectdraghelper.h>
#include <gui/widgets/widget_timegraphbase.h>
#include <list>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {
namespace Action {
class PassiveGrouper;
}
}

namespace studio {

class Widget_Curves: public Widget_TimeGraphBase
{
	friend class ChannelPointSD;
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

	std::list<CurveStruct> curve_list;

	std::list<sigc::connection> value_desc_changed;

	int waypoint_edge_length;

	std::vector<std::pair<synfig::Waypoint, std::list<CurveStruct>::iterator> > overlapped_waypoints;

	void on_waypoint_clicked(const ChannelPoint &cp, unsigned int button, Gdk::Point /*point*/);
	void on_waypoint_double_clicked(const ChannelPoint &cp, unsigned int button, Gdk::Point /*point*/);

	sigc::signal<void, synfigapp::ValueDesc, std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int> signal_waypoint_clicked_;
	sigc::signal<void, synfigapp::ValueDesc, std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int> signal_waypoint_double_clicked_;

public:
	Widget_Curves();
	~Widget_Curves();

	const Glib::RefPtr<Gtk::Adjustment>& get_range_adjustment() const { return range_adjustment; }

	void set_value_descs(etl::handle<synfigapp::CanvasInterface> canvas_interface_, const std::list< std::pair<std::string, synfigapp::ValueDesc> > &data);
	void clear();
	void refresh();

	void select_all_points();

	sigc::signal<void, synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int>& signal_waypoint_clicked() { return signal_waypoint_clicked_; }
	sigc::signal<void, synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int>& signal_waypoint_double_clicked() { return signal_waypoint_double_clicked_; }

protected:
	bool on_event(GdkEvent *event);
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	void on_channel_point_drag_canceled();
	void on_channel_point_drag_finished(bool /*started_by_keys*/);
	void on_channel_point_selection_changed();

	void delete_selected();
	bool add_waypoint_to(int point_x, int point_y);
}; // END of class Widget_Curves

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
