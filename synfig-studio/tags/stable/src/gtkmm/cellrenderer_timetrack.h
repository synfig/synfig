/* === S I N F G =========================================================== */
/*!	\file cellrenderer_timetrack.h
**	\brief Template Header
**
**	$Id: cellrenderer_timetrack.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_GTKMM_CELLRENDERER_TIMETRACK_H
#define __SINFG_GTKMM_CELLRENDERER_TIMETRACK_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/ruler.h>
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
#include <atkmm/stateset.h>
#include <gtkmm/paned.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/cellrenderer.h>

#include <gtkmm/dialog.h>
#include <gtkmm/menu.h>


#include <sinfgapp/canvasinterface.h>
#include <sinfgapp/value_desc.h>
#include <sinfg/valuenode_animated.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/string.h>
#include <sinfg/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_ValueBase;

enum Side
{
	SIDE_LEFT,
	SIDE_RIGHT
};

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
	Gtk::Adjustment adjustment_;
	
	//! Signal for when the user clicks on a waypoint
	sigc::signal<void, const Glib::ustring&,sinfg::Waypoint, int> signal_waypoint_clicked_;

	sigc::signal<void, sinfg::Waypoint, sinfg::ValueNode::Handle> signal_waypoint_changed_;

	//! Iterator for selected waypoint. (Should this be an UniqueID instead?)
	sinfg::ValueNode_Animated::WaypointList::iterator selected_waypoint;
	
	sinfg::UniqueID selected;

	//! selected information for time... (will work for way points etc...)
	//TODO: make multiple... on both time and value select...
	std::set<sinfg::Time>	sel_times;
	sinfgapp::ValueDesc		sel_value;
	sinfg::Time				actual_time;
	sinfg::Time				actual_dragtime;
	int						mode;

	//! ???
	sinfg::Time selected_time;
    
	//! The path to the current item in the tree model
	Glib::ustring path;
	
	//! ???
	bool selection;

	bool dragging;

	sinfg::Time drag_time;
	
	etl::loose_handle<sinfgapp::CanvasInterface>	canvas_interface_;

	/*
 --	** -- P R O P E R T I E S -------------------------------------------------
	*/

private:
	
	//! ValueBase Desc
	Glib::Property<sinfgapp::ValueDesc> property_valuedesc_;

	//! Canvas
	Glib::Property<sinfg::Canvas::Handle> property_canvas_;

	//! ??? \see adjustment_
	Glib::Property<Gtk::Adjustment* > property_adjustment_;

	//! \writeme
	Glib::Property<bool> property_enable_timing_info_;

	/*
 --	** -- P R O P E R T Y   I N T E R F A C E S -------------------------------
	*/

public:

	Glib::PropertyProxy<sinfgapp::ValueDesc> property_value_desc();

	Glib::PropertyProxy<sinfg::Canvas::Handle> property_canvas();

	Glib::PropertyProxy<Gtk::Adjustment* > property_adjustment();

	/*
 --	** -- S I G N A L   I N T E R F A C E S -----------------------------------
	*/

public:

	sigc::signal<void, const Glib::ustring&,sinfg::Waypoint,int> &signal_waypoint_clicked()
	{return signal_waypoint_clicked_; }

	sigc::signal<void, sinfg::Waypoint, sinfg::ValueNode::Handle> &signal_waypoint_changed()
	{return signal_waypoint_changed_; }

	/*
 --	** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	CellRenderer_TimeTrack();
    ~CellRenderer_TimeTrack();
	
	void show_timepoint_menu(const etl::handle<sinfg::Node>& node, const sinfg::Time& time, Side side=SIDE_RIGHT);

	void set_adjustment(Gtk::Adjustment &x);
	Gtk::Adjustment *get_adjustment();
	const Gtk::Adjustment *get_adjustment()const;

	etl::loose_handle<sinfgapp::CanvasInterface>	canvas_interface()const {return canvas_interface_;}
	void set_canvas_interface(etl::loose_handle<sinfgapp::CanvasInterface> h); //this should only be called by smart people
	
	sinfg::Canvas::Handle get_canvas()const;
	
	bool is_selected(const sinfg::Waypoint& waypoint)const;

	sinfg::ValueNode_Animated::WaypointList::iterator find_waypoint(const sinfg::Time& t, const sinfg::Time& scope=sinfg::Time::end());

	virtual void
	render_vfunc(
		const Glib::RefPtr<Gdk::Drawable>& window,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& ca,
		const Gdk::Rectangle& expose_area,
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
