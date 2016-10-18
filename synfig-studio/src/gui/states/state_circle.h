/* === S Y N F I G ========================================================= */
/*!	\file state_circle.h
**	\brief Circle tool state (header)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_STUDIO_STATE_CIRCLE_H
#define __SYNFIG_STUDIO_STATE_CIRCLE_H

/* === H E A D E R S ======================================================= */

#include "state_shape.h"
#include "smach.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateCircle_Context : public StateShape_Context
{
	// spline points
	Gtk::Label bline_points_label;
	Glib::RefPtr<Gtk::Adjustment> number_of_bline_points_adj;
	Gtk::SpinButton	number_of_bline_points_spin;

	// spline point angle offset
	Gtk::Label bline_point_angle_offset_label;
	Glib::RefPtr<Gtk::Adjustment> bline_point_angle_offset_adj;
	Gtk::SpinButton	bline_point_angle_offset_spin;
	Gtk::HBox bline_point_angle_offset_box;

	// falloff of feather of circle layer
	Gtk::Label falloff_label;
	Gtk::HBox falloff_box;
	Widget_Enum falloff_enum;

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

	int get_falloff()const { return falloff_enum.get_value(); }
	void set_falloff(int x) { return falloff_enum.set_value(x); }

	synfig::Real get_number_of_bline_points()const { return number_of_bline_points_adj->get_value(); }
	void set_number_of_bline_points(synfig::Real f) { number_of_bline_points_adj->set_value(f); }

	synfig::Real get_bline_point_angle_offset()const { return bline_point_angle_offset_adj->get_value(); }
	void set_bline_point_angle_offset(synfig::Real f) { bline_point_angle_offset_adj->set_value(f); }

protected:
	bool get_layer_origins_at_center_flag()const { return layer_origins_at_center_checkbutton.get_active(); }
	void set_layer_origins_at_center_flag(bool x) { return layer_origins_at_center_checkbutton.set_active(x); }

	virtual const synfig::String get_name_lower() const { return "circle"; }
	virtual const synfig::String get_name() const { return "Circle"; }
	virtual const synfig::String get_name_local() const { return _("Circle tool"); }

	virtual const Gdk::CursorType get_cursor() const { return Gdk::CROSSHAIR; }

	virtual void do_load_settings();
	virtual void do_save_settings();

	// parts of make_circle
	void make_circle_layer(
		synfig::Canvas::Handle canvas,
		int depth,
		synfigapp::Action::PassiveGrouper& group,
		synfigapp::SelectionManager::LayerList& layer_selection,
		const synfig::Point& p1,
		const synfig::Point& p2,
		synfig::ValueNode::Handle value_node_origin
	);
	void make_curve_gradient_layer(
		synfig::Canvas::Handle canvas,
		int depth,
		synfigapp::Action::PassiveGrouper& group,
		synfigapp::SelectionManager::LayerList& layer_selection,
		synfig::ValueNode_BLine::Handle value_node_bline,
		synfig::Vector& origin,
		synfig::ValueNode::Handle value_node_origin
	);
	void make_plant_layer();
	void make_region_layer();
	void make_outline_layer();
	void make_advanced_outline_layer();

public:
	//events
	virtual Smach::event_result event_mouse_click_handler(const Smach::event& x);

	void make_circle(const synfig::Point& p1, const synfig::Point& p2);

	virtual void toggle_layer_creation();

	virtual void enter();

	//constructor destructor
	StateCircle_Context(CanvasView* canvas_view);
	virtual ~StateCircle_Context();

};	// END of class StateCircle_Context

class StateCircle : public StateShape<StateCircle_Context>
{
public:
	StateCircle();
	~StateCircle();
}; // END of class StateCircle

extern StateCircle state_circle;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
