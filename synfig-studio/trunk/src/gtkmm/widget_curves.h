/* === S Y N F I G ========================================================= */
/*!	\file widget_curves.h
**	\brief Template Header
**
**	$Id: widget_curves.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_STUDIO_WIDGET_CURVES_H
#define __SYNFIG_STUDIO_WIDGET_CURVES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/layout.h>
#include <synfig/color.h>
#include <synfigapp/value_desc.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Curves : public Gtk::DrawingArea
{
	struct Channel;
	struct CurveStruct;

	Gtk::Adjustment* time_adjustment_;
	Gtk::Adjustment* range_adjustment_;
		
	std::list<CurveStruct> curve_list_;
	
public:

	Widget_Curves();
	~Widget_Curves();

	void set_value_descs(std::list<synfigapp::ValueDesc> value_descs);
	void clear();
	void refresh();

	Gtk::Adjustment& get_range_adjustment() { return *range_adjustment_; }
	Gtk::Adjustment& get_time_adjustment() { return *time_adjustment_; }
	void set_time_adjustment(Gtk::Adjustment&);

private:
	bool redraw(GdkEventExpose*bleh);
	bool on_event(GdkEvent *event);

}; // END of class Widget_Curves
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
