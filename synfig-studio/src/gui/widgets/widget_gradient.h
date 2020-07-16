/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_gradient.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_GRADIENT_H
#define __SYNFIG_STUDIO_WIDGET_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <synfig/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

void render_gradient_to_window(const Cairo::RefPtr<Cairo::Context> &cr,const Gdk::Rectangle& ca,const synfig::Gradient &gradient);

class Widget_Gradient : public Gtk::DrawingArea
{
	sigc::signal<void> signal_value_changed_;
	sigc::signal<void> signal_clicked_;

	sigc::signal<void,synfig::Gradient::CPoint> signal_cpoint_selected_;

	synfig::Gradient gradient_;

	bool editable_;

	bool changed_;

	synfig::Gradient::CPoint	selected_cpoint;

	void popup_menu(float x);

	void insert_cpoint(float x);

	void remove_cpoint(float x);

public:

	Widget_Gradient();

	~Widget_Gradient();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }
	sigc::signal<void>& signal_clicked() { return signal_clicked_; }

	sigc::signal<void,synfig::Gradient::CPoint>& signal_cpoint_selected() { return signal_cpoint_selected_; }

	void set_value(const synfig::Gradient& x);

	const synfig::Gradient& get_value()const { return gradient_; }

	void set_editable(bool x=true) { editable_=x; }

	bool get_editable()const { return editable_; }



	void set_selected_cpoint(const synfig::Gradient::CPoint &x);

	const synfig::Gradient::CPoint& get_selected_cpoint() { return selected_cpoint; }

	void update_cpoint(const synfig::Gradient::CPoint &x);

protected:
	bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr);

	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
