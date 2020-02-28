/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_waypoint.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_GTKMM_WIDGET_WAYPOINT_H
#define __SYNFIG_GTKMM_WIDGET_WAYPOINT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>

#include <synfigapp/value_desc.h>
#include <synfig/waypoint.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include "widgets/widget_enum.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_ValueBase;
class Widget_Time;
class Widget_Waypoint;

class Widget_Waypoint : public Gtk::Alignment
{
	Widget_ValueBase *value_widget;
	Gtk::Label *value_node_label;
	Gtk::Label *label;
	Widget_Time *time_widget;
	mutable synfig::Waypoint waypoint;
	synfig::Canvas::Handle canvas;

	Widget_Enum *before_options, *after_options;

	// TCB Parameter members
	Gtk::Frame *tcbFrame;
	Gtk::Label *tensionLabel, *continuityLabel, *biasLabel, *temporalTensionLabel;
	Gtk::SpinButton *spin_tension, *spin_continuity, *spin_bias, *spin_temporal_tension;
	Glib::RefPtr<Gtk::Adjustment> adj_tension, adj_continuity, adj_bias, adj_temporal_tension;

public:
	Widget_Waypoint(etl::handle<synfig::Canvas> canvas);
	void set_canvas(synfig::Canvas::Handle x);
	void set_waypoint(synfig::Waypoint &x);
	const synfig::Waypoint &get_waypoint()const;
	
	// TCB Parameter functions
	void show_tcb_params(bool ok);
	void on_before_options_changed();
	void on_after_options_changed();
}; // END of class Widget_Waypoint

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
