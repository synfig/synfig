/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_timetrack.h
**	\brief Cell renderer for the timetrack. Render all time points (waypoints / keyframes and current time line ...)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_GTKMM_CELLRENDERER_TIMETRACK_H
#define __SYNFIG_GTKMM_CELLRENDERER_TIMETRACK_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/arrow.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbufloader.h>
#include <gtkmm/viewport.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/paned.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/box.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/cellrenderer.h>

#include <gtkmm/dialog.h>
#include <gtkmm/menu.h>

#include <glibmm/property.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/value_desc.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_dynamiclist.h>
#include <synfig/string.h>
#include <synfig/time.h>

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
	//! Time adjustment window
	Glib::RefPtr<Gtk::Adjustment> adjustment_;

	//! Signal for when the user clicks on a waypoint
	sigc::signal<void, const etl::handle<synfig::Node>&, const synfig::Time&, const synfig::Time&, const synfig::Time&, int> signal_waypoint_clicked_cellrenderer_;

	sigc::signal<void, synfig::Waypoint, synfig::ValueNode::Handle> signal_waypoint_changed_;

	//! Iterator for selected waypoint. (Should this be an UniqueID instead?)
	synfig::ValueNode_Animated::WaypointList::iterator selected_waypoint;

	synfig::UniqueID selected;

	//! selected information for time... (will work for way points etc...)
	//TODO: make multiple... on both time and value select...
	std::set<synfig::Time>	sel_times;
	synfigapp::ValueDesc		sel_value;
	synfig::Time				actual_time;
	synfig::Time				actual_dragtime;
	int						mode;

	//! ???
	synfig::Time selected_time;

	//! The path to the current item in the tree model
	Glib::ustring path;

	//! ???
	bool selection;

	bool dragging;

	synfig::Time drag_time;

	etl::loose_handle<synfigapp::CanvasInterface>	canvas_interface_;

	/*
 --	** -- P R O P E R T I E S -------------------------------------------------
	*/

private:

	//! ValueBase Desc
	Glib::Property<synfigapp::ValueDesc> property_valuedesc_;

	//! Canvas
	Glib::Property<synfig::Canvas::Handle> property_canvas_;

	//! ??? \see adjustment_
	Glib::Property< Glib::RefPtr<Gtk::Adjustment> > property_adjustment_;

	//! \writeme
	Glib::Property<bool> property_enable_timing_info_;

	/*
 -- ** -- P R I V A T E   M E T H O D S --------------------------------------
	*/

private:
	//! Render the inactive waypoint line ( active point off )
	void draw_activepoint_off(
			const ::Cairo::RefPtr< ::Cairo::Context>& cr,
			Gdk::Color inactive_color,
			int line_width,
			int from_x,
			int from_y,
			int to_x,
			int to_y);

	/*
 --	** -- P R O P E R T Y   I N T E R F A C E S -------------------------------
	*/

public:

	Glib::PropertyProxy<synfigapp::ValueDesc> property_value_desc();

	Glib::PropertyProxy<synfig::Canvas::Handle> property_canvas();

	Glib::PropertyProxy< Glib::RefPtr<Gtk::Adjustment> > property_adjustment();

	/*
 --	** -- S I G N A L   I N T E R F A C E S -----------------------------------
	*/

public:

	sigc::signal<void, const etl::handle<synfig::Node>&, const synfig::Time&, const synfig::Time&, const synfig::Time&, int> &signal_waypoint_clicked_cellrenderer()
	{return signal_waypoint_clicked_cellrenderer_; }

	sigc::signal<void, synfig::Waypoint, synfig::ValueNode::Handle> &signal_waypoint_changed()
	{return signal_waypoint_changed_; }

	/*
 --	** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	CellRenderer_TimeTrack();
    ~CellRenderer_TimeTrack();

	void set_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x);
	Glib::RefPtr<Gtk::Adjustment> get_adjustment()const;

	etl::loose_handle<synfigapp::CanvasInterface>	canvas_interface()const {return canvas_interface_;}
	void set_canvas_interface(etl::loose_handle<synfigapp::CanvasInterface> h); //this should only be called by smart people

	synfig::Canvas::Handle get_canvas()const;

	bool is_selected(const synfig::Waypoint& waypoint)const;

	synfig::ValueNode_Animated::WaypointList::iterator find_editable_waypoint(const synfig::Time& t, const synfig::Time& scope=synfig::Time::end());

	virtual void
	render_vfunc(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags);

	virtual bool
	activate_vfunc(	GdkEvent* event,
					Gtk::Widget& widget,
					const Glib::ustring& path,
					const Gdk::Rectangle& background_area,
					const Gdk::Rectangle& cell_area,
					Gtk::CellRendererState flags);

}; // END of class CellRenderer_TimeTrack

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
