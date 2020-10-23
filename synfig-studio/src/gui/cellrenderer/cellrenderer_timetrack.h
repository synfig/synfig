/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_timetrack.h
**	\brief Cell renderer for the timetrack. Render all time points (waypoints / keyframes and current time line ...)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_GTKMM_CELLRENDERER_TIMETRACK_H
#define __SYNFIG_GTKMM_CELLRENDERER_TIMETRACK_H

/* === H E A D E R S ======================================================= */

#include <set>

#include <glibmm/property.h>

#include <gtkmm/cellrenderer.h>

#include <synfig/time.h>

#include <synfigapp/value_desc.h>
#include <synfigapp/canvasinterface.h>

#include <gui/timemodel.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_ValueBase;

/*! \class CellRenderer_TimeTrack
**	\brief A cell renderer that displays the waypoints for Animated ValueNodes.
*/
class CellRenderer_TimeTrack :
	public Gtk::CellRenderer
{
	/*
 --	** -- P R I V A T E   D A T A ---------------------------------------------
	*/
private:

	//! Time model
	etl::handle<TimeModel> time_model;

	synfig::UniqueID selected;

	//! selected information for time... (will work for way points etc...)
	//! TODO: make multiple... on both time and value select...
	std::set<synfig::Time> sel_times;
	synfigapp::ValueDesc sel_value;
	synfig::Time actual_time;
	synfig::Time actual_dragtime;
	int mode;

	bool dragging;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;

	/*
 --	** -- P R O P E R T I E S -------------------------------------------------
	*/
private:

	Glib::Property<synfigapp::ValueDesc> property_valuedesc_;
	Glib::Property<synfig::Canvas::Handle> property_canvas_;

	/*
 --	** -- S I G N A L S -------------------------------------------------------
	*/
private:

	//! Signal for when the user clicks on a waypoint
	sigc::signal<void, const etl::handle<synfig::Node>&, const synfig::Time&, const synfig::Time&, const synfig::Time&, int> signal_waypoint_clicked_cellrenderer_;
	sigc::signal<void, synfig::Waypoint&, synfig::ValueNode::Handle> signal_waypoint_changed_;

	/*
 --	** -- P R O P E R T Y   I N T E R F A C E S -------------------------------
	*/
public:

	Glib::PropertyProxy<synfigapp::ValueDesc> property_value_desc();
	Glib::PropertyProxy<synfig::Canvas::Handle> property_canvas();

	/*
 --	** -- S I G N A L   I N T E R F A C E S -----------------------------------
	*/
public:

	sigc::signal<void, const etl::handle<synfig::Node>&, const synfig::Time&, const synfig::Time&, const synfig::Time&, int>& signal_waypoint_clicked_cellrenderer()
		{ return signal_waypoint_clicked_cellrenderer_; }
	sigc::signal<void, synfig::Waypoint&, synfig::ValueNode::Handle>& signal_waypoint_changed()
		{ return signal_waypoint_changed_; }

	/*
 --	** -- P U B L I C   M E T H O D S -----------------------------------------
	*/
public:

	CellRenderer_TimeTrack();
    ~CellRenderer_TimeTrack();

	const etl::handle<TimeModel>& get_time_model() const { return time_model; }
	void set_time_model(const etl::handle<TimeModel> &x);

	const etl::loose_handle<synfigapp::CanvasInterface>& get_canvas_interface() const { return canvas_interface; }
	void set_canvas_interface(const etl::loose_handle<synfigapp::CanvasInterface> &x);

	synfig::Canvas::Handle get_canvas() const
		{ return const_cast<CellRenderer_TimeTrack*>(this)->property_canvas().get_value(); }

	bool is_selected(const synfig::Waypoint &waypoint) const
		{ return selected == waypoint; }

protected:
	virtual void render_vfunc(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags);

	virtual bool activate_vfunc(
		GdkEvent* event,
		Gtk::Widget& widget,
		const Glib::ustring& path,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags );
}; // END of class CellRenderer_TimeTrack

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
