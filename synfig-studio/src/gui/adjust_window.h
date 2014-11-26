/* === S Y N F I G ========================================================= */
/*!	\file adjust_window.h
**	\brief Adjustment Window Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_TEMPLATE_H
#define __SYNFIG_TEMPLATE_H

/* === H E A D E R S ======================================================= */
#include <gtkmm/adjustment.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

/* Sets up an adjustment that controls/communicates with another adjustment
	(could be expanded to multiple children)

	The current value and pagesize define the lower and upper bounds of the
	child adjustment.

	NEED TO REPLACE FUNCTIONALITY IN:
	refresh_rend_desc
	refresh_time_window
	on_time_changed - possibly....

	time_zoom_in - possibly...
	time_zoom_out - possibly...

	play - possibly...

	THINGS TO CHECK:
	disp_audio's use of time_adjustment
	children_tree's use of time_adjustment
	layer_tree's use of time_adjustment
*/
class Adjust_Window : public Gtk::Adjustment
{
	Glib::RefPtr<Gtk::Adjustment> adj_child;

	sigc::connection childchanged; //we only care about the non-value parts of the child

	virtual void on_changed(); //value+pagesize corresponds to child upper
	virtual void on_value_changed(); //value corresponds to child lower

protected: //update interface
	virtual void update_child();
	virtual void update_fromchild();

public: //structors
	Adjust_Window(double value, double lower, double upper,
				  double step_increment=1, double page_increment=10, double page_size=0,
				  const Glib::RefPtr<Gtk::Adjustment> &adj = Glib::RefPtr<Gtk::Adjustment>());

	virtual ~Adjust_Window();

public: //child interface
	Glib::RefPtr<Gtk::Adjustment> get_child_adjustment();
	Glib::RefPtr<const Gtk::Adjustment> get_child_adjustment() const;
	void set_child_adjustment(const Glib::RefPtr<Gtk::Adjustment> &child);

public: //Sub value interface
	double get_sub_lower() const;
	double get_sub_upper() const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
