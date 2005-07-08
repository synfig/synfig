/* === S Y N F I G ========================================================= */
/*!	\file dialog_waypoint.h
**	\brief Template Header
**
**	$Id: widget_waypoint.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_GTKMM_WIDGET_WAYPOINT_H
#define __SYNFIG_GTKMM_WIDGET_WAYPOINT_H

/* === H E A D E R S ======================================================= */

//#include <gtk/gtk.h>
//#include <gtkmm/arrow.h>
//#include <gtkmm/image.h>
//#include <gdkmm/pixbufloader.h>
//#include <gtkmm/viewport.h>
#include <gtkmm/adjustment.h>
//#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
//#include <gtkmm/progressbar.h>
//#include <atkmm/stateset.h>
//#include <gtkmm/paned.h>
#include <gtkmm/box.h>
//#include <gtkmm/scrollbar.h>
#include <gtkmm/combo.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/spinbutton.h>


#include <synfigapp/value_desc.h>
#include <synfig/waypoint.h>
//#include <synfig/valuenode_dynamiclist.h>
#include <synfig/string.h>
#include <synfig/time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_ValueBase;
class Widget_Time;
class Widget_Waypoint;

class Widget_Waypoint : public Gtk::Table
{
    Widget_ValueBase *value_widget;
	Gtk::Label *value_node_label;
	Gtk::Label *label;
	Widget_Time *time_widget;
	mutable synfig::Waypoint waypoint;
	synfig::Canvas::Handle canvas;
	//Gtk::Adjustment time_adjustment;

	Gtk::Combo *in,*out;
	Gtk::OptionMenu *before, *after;
	Gtk::Menu *before_options,*after_options;

	Gtk::SpinButton *spin_tension, *spin_continuity, *spin_bias, *spin_temporal_tension;
	Gtk::Adjustment adj_tension, adj_continuity, adj_bias, adj_temporal_tension;
//	Gtk::ComboDropDownItem item;

public:
	Widget_Waypoint(etl::handle<synfig::Canvas> canvas);
	void set_canvas(synfig::Canvas::Handle x);
	void set_waypoint(synfig::Waypoint &x);
	const synfig::Waypoint &get_waypoint()const;
}; // END of class Widget_Waypoint

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
