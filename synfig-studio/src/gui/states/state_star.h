/* === S Y N F I G ========================================================= */
/*!	\file state_star.h
**	\brief Star creation state
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2016 caryoscelus
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

#ifndef __SYNFIG_STUDIO_STATE_STAR_H
#define __SYNFIG_STUDIO_STATE_STAR_H

/* === H E A D E R S ======================================================= */

#include "state_shape.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateStar_Context : public StateShape_Context
{
	// star points
	Gtk::Label number_of_points_label;
	Glib::RefPtr<Gtk::Adjustment> number_of_points_adj;
	Gtk::SpinButton	number_of_points_spin;

	// radius ratio
	Gtk::Label radius_ratio_label;
	Glib::RefPtr<Gtk::Adjustment> radius_ratio_adj;
	Gtk::SpinButton	radius_ratio_spin;

	// angle offset
	Gtk::Label angle_offset_label;
	Glib::RefPtr<Gtk::Adjustment> angle_offset_adj;
	Gtk::SpinButton	angle_offset_spin;
	Gtk::HBox angle_offset_box;

	// regular polygon
	Gtk::Label regular_polygon_label;
	Gtk::CheckButton regular_polygon_checkbutton;
	Gtk::HBox regular_polygon_box;

	// inner width
	Gtk::Label outer_width_label;
	Glib::RefPtr<Gtk::Adjustment> outer_width_adj;
	Gtk::SpinButton	outer_width_spin;

	// inner tangent
	Gtk::Label inner_tangent_label;
	Glib::RefPtr<Gtk::Adjustment> inner_tangent_adj;
	Gtk::SpinButton	inner_tangent_spin;

	// outer width
	Gtk::Label inner_width_label;
	Glib::RefPtr<Gtk::Adjustment> inner_width_adj;
	Gtk::SpinButton	inner_width_spin;

	// outer tangent
	Gtk::Label outer_tangent_label;
	Glib::RefPtr<Gtk::Adjustment> outer_tangent_adj;
	Gtk::SpinButton	outer_tangent_spin;

	// spline origins at center
	Gtk::Label origins_at_center_label;
	Gtk::CheckButton layer_origins_at_center_checkbutton;
	Gtk::HBox origins_at_center_box;

public:
	virtual int layers_to_create()const
	{
		return
			StateShape_Context::layers_to_create()+
			(get_layer_shape_flag() && get_layer_origins_at_center_flag());
	}

	int get_number_of_points()const { return number_of_points_adj->get_value(); }
	void set_number_of_points(int n) { number_of_points_adj->set_value(n); }

	synfig::Real get_inner_tangent()const { return inner_tangent_adj->get_value(); }
	void set_inner_tangent(synfig::Real f) { inner_tangent_adj->set_value(f); }

	synfig::Real get_outer_tangent()const { return outer_tangent_adj->get_value(); }
	void set_outer_tangent(synfig::Real f) { outer_tangent_adj->set_value(f); }

	synfig::Real get_inner_width()const { return inner_width_adj->get_value(); }
	void set_inner_width(synfig::Real f) { inner_width_adj->set_value(f); }

	synfig::Real get_outer_width()const { return outer_width_adj->get_value(); }
	void set_outer_width(synfig::Real f) { outer_width_adj->set_value(f); }

	synfig::Real get_radius_ratio()const { return radius_ratio_adj->get_value(); }
	void set_radius_ratio(synfig::Real f) { radius_ratio_adj->set_value(f); }

	synfig::Real get_angle_offset()const { return angle_offset_adj->get_value(); }
	void set_angle_offset(synfig::Real f) { angle_offset_adj->set_value(f); }

	bool get_regular_polygon()const { return regular_polygon_checkbutton.get_active(); }
	void set_regular_polygon(bool i) { regular_polygon_checkbutton.set_active(i); }

protected:
	bool get_layer_origins_at_center_flag()const { return layer_origins_at_center_checkbutton.get_active(); }
	void set_layer_origins_at_center_flag(bool x) { return layer_origins_at_center_checkbutton.set_active(x); }

	virtual const synfig::String get_name_lower() const { return "star"; }
	virtual const synfig::String get_name() const { return "Star"; }
	virtual const synfig::String get_name_local() const { return _("Star tool"); }
	virtual const synfig::String get_local_create() const { return _("Create a star layer"); }
	virtual const synfig::String get_local_new() const { return _("New Star"); }

	virtual const Gdk::CursorType get_cursor() const { return Gdk::STAR; }

	virtual void do_load_settings();
	virtual void do_save_settings();

	void make_star_layer(
		synfig::Canvas::Handle canvas,
		int depth,
		synfigapp::Action::PassiveGrouper& group,
		synfigapp::SelectionManager::LayerList& layer_selection,
		const synfig::Point& p1,
		const synfig::Point& p2,
		synfig::ValueNode::Handle value_node_origin
	);

public:
	//events
	virtual Smach::event_result event_mouse_click_handler(const Smach::event& x);

	void make_star(const synfig::Point& p1, const synfig::Point& p2);

	virtual void toggle_layer_creation();

	virtual void enter();

	//constructor destructor
	StateStar_Context(CanvasView* canvas_view);
	virtual ~StateStar_Context();

};	// END of class StateStar_Context

class StateStar : public StateShape<StateStar_Context>
{
public:
	StateStar();
	~StateStar();
}; // END of class StateStar

extern StateStar state_star;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
