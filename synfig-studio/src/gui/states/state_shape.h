/* === S Y N F I G ========================================================= */
/*!	\file state_shape.h
**	\brief Generic shape tool state (header)
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

#ifndef __SYNFIG_STUDIO_STATE_SHAPE_H
#define __SYNFIG_STUDIO_STATE_SHAPE_H

/* === H E A D E R S ======================================================= */

#include "state.h"
#include "state_normal.h"

#include "duck.h"
#include "workarea.h"
#include "app.h"

#include <synfig/vector.h>
#include <synfigapp/action_system.h>
#include <synfigapp/action.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <gui/localization.h>

#include "event_mouse.h"
#include "event_layerclick.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"
#include "docks/dialog_tooloptions.h"

/* === U S I N G =========================================================== */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class MakeRegionLayer;
class MakeOutlineLayer;
class MakeAdvancedOutlineLayer;
class MakeCurveGradientLayer;
class MakePlantLayer;

class StateShape_Context : public State_Context
{
protected:
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	synfig::Point point_holder;

	etl::handle<Duck> point2_duck;

	void refresh_ducks()
	{
		get_work_area()->clear_ducks();
		get_work_area()->queue_draw();
	}

	bool prev_workarea_layer_status_;

	// holder of options
	Gtk::Table options_table;

	// title
	Gtk::Label title_label;

	// layer name:
	Gtk::Label id_label;
	Gtk::HBox id_box;
	Gtk::Entry id_entry;

	// layer types to create:
	Gtk::Label layer_types_label;
	Gtk::ToggleButton layer_shape_togglebutton;
	Gtk::ToggleButton layer_region_togglebutton;
	Gtk::ToggleButton layer_outline_togglebutton;
	Gtk::ToggleButton layer_advanced_outline_togglebutton;
	Gtk::ToggleButton layer_curve_gradient_togglebutton;
	Gtk::ToggleButton layer_plant_togglebutton;
	Gtk::HBox layer_types_box;

	// blend method
	Gtk::Label blend_label;
	Gtk::HBox blend_box;
	Widget_Enum blend_enum;

	// opacity
	Gtk::Label opacity_label;
	Gtk::HScale opacity_hscl;

	// brush size
	Gtk::Label bline_width_label;
	Widget_Distance bline_width_dist;

	// invert
	Gtk::Label invert_label;
	Gtk::CheckButton invert_checkbutton;
	Gtk::HBox invert_box;

	// feather size
	Gtk::Label feather_label;
	Widget_Distance feather_dist;

	// link origins
	Gtk::Label link_origins_label;
	Gtk::CheckButton layer_link_origins_checkbutton;
	Gtk::HBox link_origins_box;

protected:
	MakeRegionLayer* region_maker;
	MakeOutlineLayer* outline_maker;
	MakeAdvancedOutlineLayer* advanced_outline_maker;
	MakePlantLayer* plant_maker;
	MakeCurveGradientLayer* curve_gradient_maker;

public:
	// this only counts the layers which use blines - they're the only
	// ones we link the origins for
	virtual int layers_to_create()const
	{
		return
			get_layer_region_flag() +
			get_layer_outline_flag() +
			get_layer_advanced_outline_flag() +
			get_layer_curve_gradient_flag() +
			get_layer_plant_flag();
	}

	synfig::String get_id()const { return id_entry.get_text(); }
	void set_id(const synfig::String& x) { return id_entry.set_text(x); }

	int get_blend()const { return blend_enum.get_value(); }
	void set_blend(int x) { return blend_enum.set_value(x); }

	synfig::Real get_opacity()const { return opacity_hscl.get_value(); }
	void set_opacity(synfig::Real x) { opacity_hscl.set_value(x); }

	synfig::Real get_bline_width() const {
		return bline_width_dist.get_value().get(
			synfig::Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_bline_width(synfig::Distance x) { return bline_width_dist.set_value(x);}

	synfig::Real get_feather_size() const {
		return feather_dist.get_value().get(
			synfig::Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_feather_size(synfig::Distance x) { return feather_dist.set_value(x);}

	bool get_invert()const { return invert_checkbutton.get_active(); }
	void set_invert(bool i) { invert_checkbutton.set_active(i); }

protected:
	bool get_layer_shape_flag()const { return layer_shape_togglebutton.get_active(); }
	void set_layer_shape_flag(bool x) { return layer_shape_togglebutton.set_active(x); }

	bool get_layer_region_flag()const { return layer_region_togglebutton.get_active(); }
	void set_layer_region_flag(bool x) { return layer_region_togglebutton.set_active(x); }

	bool get_layer_outline_flag()const { return layer_outline_togglebutton.get_active(); }
	void set_layer_outline_flag(bool x) { return layer_outline_togglebutton.set_active(x); }

	bool get_layer_advanced_outline_flag()const { return layer_advanced_outline_togglebutton.get_active(); }
	void set_layer_advanced_outline_flag(bool x) { return layer_advanced_outline_togglebutton.set_active(x); }

	bool get_layer_curve_gradient_flag()const { return layer_curve_gradient_togglebutton.get_active(); }
	void set_layer_curve_gradient_flag(bool x) { return layer_curve_gradient_togglebutton.set_active(x); }

	bool get_layer_plant_flag()const { return layer_plant_togglebutton.get_active(); }
	void set_layer_plant_flag(bool x) { return layer_plant_togglebutton.set_active(x); }

public:
	bool get_layer_link_origins_flag()const { return layer_link_origins_checkbutton.get_active(); }
protected:
	void set_layer_link_origins_flag(bool x) { return layer_link_origins_checkbutton.set_active(x); }

	bool layer_shape_flag;
	bool layer_region_flag;
	bool layer_outline_flag;
	bool layer_advanced_outline_flag;
	bool layer_curve_gradient_flag;
	bool layer_plant_flag;

protected:
	virtual const synfig::String get_name_lower() const { return "shape"; }
	virtual const synfig::String get_name() const { return "Shape"; }
	virtual const synfig::String get_local_name() const { return _("Generic Shape tool"); }

	virtual const Gdk::CursorType get_cursor() const { return Gdk::LEFT_PTR; }

	//! Load settings unsafe implementation
	virtual void do_load_settings();
	//! Save settings unsafe implementation
	virtual void do_save_settings();

public:
	void refresh_tool_options(); //to refresh the toolbox

	//events
	virtual Smach::event_result event_stop_handler(const Smach::event& x);
	virtual Smach::event_result event_refresh_handler(const Smach::event& x);
	virtual Smach::event_result event_mouse_click_handler(const Smach::event& x) = 0;
	virtual Smach::event_result event_refresh_tool_options(const Smach::event& x);

	virtual void enter();
	virtual void leave();

	void finalize_init();
	void reset() { refresh_ducks(); }
	void increment_id();

	void generate_shape_layers(
		synfig::Canvas::Handle canvas,
		int depth,
		synfigapp::Action::PassiveGrouper& group,
		synfigapp::SelectionManager::LayerList& layer_selection,
		synfig::ValueNode_BLine::Handle value_node_bline,
		synfig::Vector& origin,
		synfig::ValueNode::Handle value_node_origin
	);

private:
	bool egress_on_selection_change = false;

public:
	inline void set_egress_on_selection_change(bool e) { egress_on_selection_change=e; }
	void enable_egress_on_selection_change() { set_egress_on_selection_change(true); }
	void disable_egress_on_selection_change() { set_egress_on_selection_change(false); }

	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal;
		return Smach::RESULT_OK;
	}

	StateShape_Context(CanvasView* canvas_view);
	virtual ~StateShape_Context();
};

template <class Context> class StateShape : public Smach::state<Context>
{
public:
	// This is required because looking up Smach's typedef fails
	typedef Smach::event_def_internal<Context> event_def;
	
	StateShape(const char* name) :
		Smach::state<Context>(name)
	{
		this->insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&Context::event_layer_selection_changed_handler));
		this->insert(event_def(EVENT_STOP,&Context::event_stop_handler));
		this->insert(event_def(EVENT_REFRESH,&Context::event_refresh_handler));
		this->insert(event_def(EVENT_REFRESH_DUCKS,&Context::event_refresh_handler));
		this->insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&Context::event_mouse_click_handler));
		this->insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&Context::event_mouse_click_handler));
		this->insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&Context::event_mouse_click_handler));
		this->insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&Context::event_refresh_tool_options));
	}
	~StateShape(){}
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
