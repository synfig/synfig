/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timetrack.h
**	\brief Widget to displaying layer parameter waypoints along time
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2020 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_TIMETRACK_H
#define SYNFIG_STUDIO_WIDGET_TIMETRACK_H


#include <gui/widgets/widget_timegraphbase.h>
#include <gui/selectdraghelper.h>

#include <gtkmm/treeview.h>

namespace studio {

class LayerParamTreeStore;

/**
 * \brief The Widget_Timetrack class to displaying layer parameter waypoints along time
 *
 * This widget is vertically synchronized with a Gtk::TreeView that uses
 * LayerParamTreeStore as its model: it mimics its height and vertical scrolling.
 *
 * The minimum setup requires three info:
 *
 * \code{.cpp}
 * Widget_Timetrack widget_timetrack;
 * widget_timetrack.set_time_model(canvas_view->time_model());
 * widget_timetrack.set_canvas_interface(canvas_view->canvas_interface());
 * // layer_param_treeview is a Gtk::TreeView with LayerParamTreeStore as model
 * widget_timetrack.set_params_view(layer_param_treeview);
 * \endcode
 *
 * Alternatively, if everything follows the Synfig Studio convention (ie. docks
 * and models names), a helper method can be used instead:
 * \code{.cpp}
 * Widget_Timetrack widget_timetrack;
 * widget_timetrack.use_canvas_view(canvas_view);
 * \endcode
 */
class Widget_Timetrack : public Widget_TimeGraphBase
{
public:
	Widget_Timetrack();
	virtual ~Widget_Timetrack() override;

	bool set_params_view(Gtk::TreeView *treeview);
	Gtk::TreeView * get_params_view() const;
	Glib::RefPtr<LayerParamTreeStore> get_params_model() const;

	bool use_canvas_view(etl::loose_handle<CanvasView> canvas_view);

protected:
	virtual bool on_event(GdkEvent* event) override;
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
	virtual void on_size_allocate(Gtk::Allocation &allocation) override;

private:

	struct WaypointItem {
		synfig::TimePoint time_point;
		Gtk::TreePath path;

		WaypointItem() {}
		WaypointItem(const synfig::TimePoint time_point, const Gtk::TreePath &path);

		bool is_draggable() const;

		bool operator ==(const WaypointItem &b) const;
		bool operator !=(const WaypointItem &b) const {return !operator==(b);}
	};

	// Handle mouse actions for panning/zooming/scrolling and waypoint selection
	struct WaypointSD : SelectDragHelper<WaypointItem>
	{
		Widget_Timetrack &widget;
		// SelectDragHelper interface
	public:
		WaypointSD(Widget_Timetrack &widget);
		virtual ~WaypointSD() override;
		virtual void get_item_position(const WaypointItem& item, Gdk::Point& p) override;
		virtual bool find_item_at_position(int pos_x, int pos_y, WaypointItem& item) override;
		virtual bool find_items_in_rect(Gdk::Rectangle, std::vector<WaypointItem>&) override { return false; }
		virtual void get_all_items(std::vector<WaypointItem>&) override {}
		virtual void delta_drag(int, int, bool) override {}
	} waypoint_sd;
	void setup_mouse_handler();

	Gtk::TreeView *params_treeview;
	Glib::RefPtr<LayerParamTreeStore> params_store;

	std::vector<sigc::connection> treeview_connections;
	std::vector<sigc::connection> treestore_connections;

	void setup_params_store();
	void teardown_params_store();
	void setup_params_view();
	void teardown_params_view();
	void setup_adjustment();

	struct Geometry {
		int y;
		int h;
	};

	struct RowInfo {
		RowInfo();
		RowInfo(synfigapp::ValueDesc value_desc, Geometry geometry);
		~RowInfo();

		sigc::signal<void> signal_changed() {return signal_changed_;}

		const synfigapp::ValueDesc& get_value_desc() const;

		Geometry get_geometry() const;
		void set_geometry(const Geometry& value);

	protected:
		synfigapp::ValueDesc value_desc;
		Geometry geometry;
		sigc::signal<void> signal_changed_;

		void refresh();
	private:
		std::vector<sigc::connection> value_desc_connections;
	};

	std::map<std::string, RowInfo*> param_info_map;

	std::mutex param_list_mutex;
	bool is_rebuild_param_info_list_queued;
	void queue_rebuild_param_info_list();
	void rebuild_param_info_list();
	bool is_update_param_list_geometries_queued;
	void queue_update_param_list_geometries();
	void update_param_list_geometries();

	void draw_static_intervals_for_row(const Cairo::RefPtr<Cairo::Context> &cr, const RowInfo *row_info, const std::vector<std::pair<synfig::TimePoint, synfig::Time>> &waypoints);
	void draw_waypoints(const Cairo::RefPtr<Cairo::Context> &cr, const Gtk::TreePath& path, const std::vector<std::pair<synfig::TimePoint, synfig::Time>> &waypoints);
};

}

#endif // SYNFIG_STUDIO_WIDGET_TIMETRACK_H
