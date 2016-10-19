/* === S Y N F I G ========================================================= */
/*!	\file state_circle.cpp
**	\brief Circle tool state
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/valuenodes/valuenode_dynamiclist.h>

#include "state_circle.h"

#include "docks/dock_toolbox.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
enum CircleFalloff
{
	CIRCLE_SQUARED	=0,
	CIRCLE_INTERPOLATION_LINEAR	=1,
	CIRCLE_COSINE	=2,
	CIRCLE_SIGMOND	=3,
	CIRCLE_SQRT		=4,
	CIRCLE_NUM_FALLOFF
};

/* === G L O B A L S ======================================================= */

StateCircle studio::state_circle;

/* === C L A S S E S & S T R U C T S ======================================= */

/* === M E T H O D S ======================================================= */

StateCircle::StateCircle():
	StateShape<StateCircle_Context>("circle")
{
}

StateCircle::~StateCircle()
{
}

void
StateCircle_Context::do_load_settings()
{
	StateShape_Context::do_load_settings();
	String value;

	if(settings.get_value("circle.fallofftype",value) && value != "")
		set_falloff(atoi(value.c_str()));
	else
		set_falloff(2);

	if(settings.get_value("circle.number_of_bline_points",value))
		set_number_of_bline_points(atof(value.c_str()));
	else
		set_number_of_bline_points(4);

	if(settings.get_value("circle.bline_point_angle_offset",value))
		set_bline_point_angle_offset(atof(value.c_str()));
	else
		set_bline_point_angle_offset(0);

	if(settings.get_value("circle.layer_origins_at_center",value) && value=="0")
		set_layer_origins_at_center_flag(false);
	else
		set_layer_origins_at_center_flag(true);
}

void
StateCircle_Context::do_save_settings()
{
	StateShape_Context::do_save_settings();
	settings.set_value("circle.fallofftype",strprintf("%d",get_falloff()));
	settings.set_value("circle.number_of_bline_points",strprintf("%d",(int)(get_number_of_bline_points() + 0.5)));
	settings.set_value("circle.bline_point_angle_offset",strprintf("%f",(float)get_bline_point_angle_offset()));
	settings.set_value("circle.layer_origins_at_center",get_layer_origins_at_center_flag()?"1":"0");
}

StateCircle_Context::StateCircle_Context(CanvasView* canvas_view):
	StateShape_Context(canvas_view),
	number_of_bline_points_adj(Gtk::Adjustment::create(0, 2, 120, 1, 1)),
	number_of_bline_points_spin(number_of_bline_points_adj, 1, 0),
	bline_point_angle_offset_adj(Gtk::Adjustment::create(0, -360, 360, 0.1, 1)),
	bline_point_angle_offset_spin(bline_point_angle_offset_adj, 1, 1)
{
	// 6, spline points
	bline_points_label.set_label(_("Spline Points:"));
	bline_points_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bline_points_label.set_sensitive(false);
	number_of_bline_points_spin.set_sensitive(false);

	// 7, spline point angle offset
	bline_point_angle_offset_label.set_label(_("Offset:"));
	bline_point_angle_offset_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bline_point_angle_offset_label.set_sensitive(false);
	bline_point_angle_offset_spin.set_sensitive(false);

	SPACING(bline_point_angle_offset_indent, INDENTATION);
	bline_point_angle_offset_box.pack_start(*bline_point_angle_offset_indent, Gtk::PACK_SHRINK);
	bline_point_angle_offset_box.pack_start(bline_point_angle_offset_label, Gtk::PACK_SHRINK);

	// 10, feather falloff for circle layer
	falloff_label.set_label(_("Falloff:"));
	falloff_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	falloff_label.set_sensitive(false);
	SPACING(falloff_indent, INDENTATION);
	falloff_box.pack_start(*falloff_indent, Gtk::PACK_SHRINK);
	falloff_box.pack_start(falloff_label, Gtk::PACK_SHRINK);

	falloff_enum.set_param_desc(ParamDesc("falloff")
		.set_local_name(_("Falloff"))
		.set_description(_("Determines the falloff function for the feather"))
		.set_hint("enum")
		.add_enum_value(CIRCLE_INTERPOLATION_LINEAR,"linear",_("Linear"))
		.add_enum_value(CIRCLE_SQUARED,"squared",_("Squared"))
		.add_enum_value(CIRCLE_SQRT,"sqrt",_("Square Root"))
		.add_enum_value(CIRCLE_SIGMOND,"sigmond",_("Sigmond"))
		.add_enum_value(CIRCLE_COSINE,"cosine",_("Cosine")));
	falloff_enum.set_sensitive(false);

	// 12, spline origins at center
	origins_at_center_label.set_label(_("Spline Origins at Center"));
	origins_at_center_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	origins_at_center_box.pack_start(origins_at_center_label);
	origins_at_center_box.pack_end(layer_origins_at_center_checkbutton, Gtk::PACK_SHRINK);
	origins_at_center_box.set_sensitive(false);
}

void
StateCircle_Context::enter()
{
	StateShape_Context::enter();
	// pack all options to the options_table

	// 0, title
	options_table.attach(title_label,
		0, 2, 0, 1, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 1, name
	options_table.attach(id_box,
		0, 2, 1, 2, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 2, layer types creation
	options_table.attach(layer_types_label,
		0, 2, 2, 3, Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(layer_types_box,
		0, 2, 3, 4, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 3, blend method
	options_table.attach(blend_box,
		0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(blend_enum,
		1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 4, opacity
	options_table.attach(opacity_label,
		0, 1, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(opacity_hscl,
		1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 5, brush size
	options_table.attach(bline_width_label,
		0, 1, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(bline_width_dist,
		1, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 6, spline points
	options_table.attach(bline_points_label,
		0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(number_of_bline_points_spin,
		1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, spline points offset
	options_table.attach(bline_point_angle_offset_box,
		0, 1, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(bline_point_angle_offset_spin,
		1, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, invert
	options_table.attach(invert_box,
		0, 2, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 9, feather
	options_table.attach(feather_label,
		0, 1, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(feather_dist,
		1, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
  // 10, falloff
  options_table.attach(falloff_box,
		0, 1, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(falloff_enum,
		1, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 11, link origins
	options_table.attach(link_origins_box,
		0, 2, 12, 13, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 12, origins at center
	options_table.attach(origins_at_center_box,
		0, 2, 13, 14, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	options_table.set_row_spacing(14, 0); // the final row using border width of table

	finalize_init();
}

StateCircle_Context::~StateCircle_Context()
{
}

void
StateCircle_Context::make_circle_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	const Point& p1,
	const Point& p2,
	ValueNode::Handle value_node_origin
)
{
	disable_egress_on_selection_change();
	Layer::Handle layer=get_canvas_interface()->add_layer_to("circle",canvas,depth);
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);

	layer->set_param("radius",(p2-p1).mag());
	get_canvas_interface()->signal_layer_param_changed()(layer,"radius");

	layer->set_param("falloff",get_falloff());
	get_canvas_interface()->signal_layer_param_changed()(layer,"falloff");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("feather",get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	// only link the circle's origin parameter if the option is selected, we're putting bline
	// origins at their centers, and we're creating more than one layer
	if (get_layer_link_origins_flag() && get_layer_origins_at_center_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Circle layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",p1);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_curve_gradient_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	ValueNode_BLine::Handle value_node_bline,
	Vector& origin,
	ValueNode::Handle value_node_origin
)
{
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("curve_gradient",canvas,depth));
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(get_id()+_(" Gradient"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("width",get_bline_width());
	get_canvas_interface()->signal_layer_param_changed()(layer,"width");

	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Gradient layer"));
			return;
		}
	}

	// only link the curve gradient's origin parameter if the option is selected and we're creating more than one layer
	if (get_layer_link_origins_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Gradient layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",origin);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_plant_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	ValueNode_BLine::Handle value_node_bline,
	Vector& origin,
	ValueNode::Handle value_node_origin
)
{
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("plant",canvas,depth));
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(get_id()+_(" Plant"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Plant layer"));
			return;
		}
	}

	// only link the plant's origin parameter if the option is selected and we're creating more than one layer
	if (get_layer_link_origins_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Plant layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",origin);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_region_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	ValueNode_BLine::Handle value_node_bline,
	Vector& origin,
	ValueNode::Handle value_node_origin
)
{
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(get_id()+_(" Region"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("feather",get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

	// I don't know if it's safe to reuse the same LayerParamConnect action, so I'm
	// using 2 separate ones.
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
			group.cancel();
			throw String(_("Unable to create Region layer"));
			return;
		}
	}

	// only link the region's origin parameter if the option is selected and we're creating more than one layer
	if (get_layer_link_origins_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
			group.cancel();
			throw String(_("Unable to create Region layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",origin);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_outline_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	ValueNode_BLine::Handle value_node_bline,
	Vector& origin,
	ValueNode::Handle value_node_origin
)
{
	disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("outline",canvas,depth));
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(get_id()+_(" Outline"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("width",get_bline_width());
	get_canvas_interface()->signal_layer_param_changed()(layer,"width");

	layer->set_param("feather",get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Outline layer"));
			return;
		}
	}

	// only link the outline's origin parameter if the option is selected and we're creating more than one layer
	if (get_layer_link_origins_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Outline layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",origin);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_advanced_outline_layer(
	Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	ValueNode_BLine::Handle value_node_bline,
	Vector& origin,
	ValueNode::Handle value_node_origin
)
{
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);
	disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth));
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(get_id()+_(" Advanced Outline"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("width",get_bline_width());
	get_canvas_interface()->signal_layer_param_changed()(layer,"width");

	layer->set_param("feather",get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Advanced Outline layer"));
			return;
		}
	}

	// only link the outline's origin parameter if the option is selected and we're creating more than one layer
	if (get_layer_link_origins_flag() && layers_to_create() > 1)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("origin")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_origin)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			group.cancel();
			throw String(_("Unable to create Advanced Outline layer"));
			return;
		}
	}
	else
	{
		layer->set_param("origin",origin);
		get_canvas_interface()->signal_layer_param_changed()(layer,"origin");
	}
}

void
StateCircle_Context::make_circle(const Point& _p1, const Point& _p2)
{
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Circle"));
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	Layer::Handle layer;

	Canvas::Handle canvas;
	int depth(0);

	// we are temporarily using the layer to hold something
	layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	} else
	{
		canvas=get_canvas_view()->get_canvas();
	}

	synfigapp::SelectionManager::LayerList layer_selection;
	if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION"))
		layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();

	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));

	Real radius((p2-p1).mag());
	int points = get_number_of_bline_points();
	Angle::deg offset(get_bline_point_angle_offset());
	Angle::deg angle(360.0/points);
	Real tangent(4 * ((points == 2)
					  ? 1
					  : ((2 * Angle::cos(angle/2).get() - Angle::cos(angle).get() - 1) / Angle::sin(angle).get())));
	Vector origin;
	Real x, y;

	if (get_layer_origins_at_center_flag())
	{
		x = y = 0;
		origin = p1;
	}
	else
	{
		x = p1[0];
		y = p1[1];
	}

	// Calculate bline points
	// TODO: only do this if we create anything that needs them
	std::vector<BLinePoint> new_list;
	for (int i = 0; i < points; i++)
	{
		new_list.push_back(*(new BLinePoint));
		new_list[i].set_width(1);
		new_list[i].set_vertex(Point(radius*Angle::cos(angle*i + offset).get() + x,
									 radius*Angle::sin(angle*i + offset).get() + y));
		new_list[i].set_tangent(Point(-radius*tangent*Angle::sin(angle*i + offset).get(),
									   radius*tangent*Angle::cos(angle*i + offset).get()));
	}

	ValueNode_BLine::Handle value_node_bline(ValueNode_BLine::create(new_list));
	assert(value_node_bline);

	ValueNode::Handle value_node_origin(ValueNode_Const::create(origin));
	assert(value_node_origin);

	// Set the looping flag
	value_node_bline->set_loop(true);

	value_node_bline->set_member_canvas(canvas);

	///////////////////////////////////////////////////////////////////////////
	//   C I R C L E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_shape_flag() &&
		get_falloff() >= 0 && get_falloff() < CIRCLE_NUM_FALLOFF)
	{
		make_circle_layer(canvas, depth, group, layer_selection, p1, p2, value_node_origin);
	}

	///////////////////////////////////////////////////////////////////////////
	//   C U R V E   G R A D I E N T
	///////////////////////////////////////////////////////////////////////////

	if(get_layer_curve_gradient_flag())
	{
		make_curve_gradient_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	///////////////////////////////////////////////////////////////////////////
	//   P L A N T
	///////////////////////////////////////////////////////////////////////////

	if(get_layer_plant_flag())
	{
		make_plant_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	///////////////////////////////////////////////////////////////////////////
	//   R E G I O N
	///////////////////////////////////////////////////////////////////////////

	if(get_layer_region_flag())
	{
		make_region_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	///////////////////////////////////////////////////////////////////////////
	//   O U T L I N E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_outline_flag())
	{
		make_outline_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	///////////////////////////////////////////////////////////////////////////
	//   A D V A N C E D   O U T L I N E
	///////////////////////////////////////////////////////////////////////////

	if (get_layer_advanced_outline_flag())
	{
		make_advanced_outline_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	disable_egress_on_selection_change();
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	enable_egress_on_selection_change();

	reset();
	increment_id();
}

Smach::event_result
StateCircle_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DOWN && event.button==BUTTON_LEFT)
	{
		point_holder=get_work_area()->snap_point_to_grid(event.pos);
		etl::handle<Duck> duck=new Duck();
		duck->set_point(point_holder);
		duck->set_name("p1");
		duck->set_type(Duck::TYPE_POSITION);
		duck->set_editable(false);
		get_work_area()->add_duck(duck);

		point2_duck=new Duck();
		point2_duck->set_point(Vector(0,0));
		point2_duck->set_name("radius");
		point2_duck->set_origin(duck);
		point2_duck->set_radius(true);
		point2_duck->set_scalar(-1);
		point2_duck->set_type(Duck::TYPE_RADIUS);
		point2_duck->set_hover(true);
		get_work_area()->add_duck(point2_duck);

		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_DRAG && event.button==BUTTON_LEFT)
	{
		if (!point2_duck) return Smach::RESULT_OK;
		point2_duck->set_point(point_holder-get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}

	if(event.key==EVENT_WORKAREA_MOUSE_BUTTON_UP && event.button==BUTTON_LEFT)
	{
		Point point(get_work_area()->snap_point_to_grid(event.pos));

		if (App::restrict_radius_ducks)
		{
			if ((point[0] - point_holder[0]) < 0) point[0] = point_holder[0];
			if ((point[1] - point_holder[1]) < 0) point[1] = point_holder[1];
		}

		if (point_holder != point)
			make_circle(point_holder, point);
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}

void
StateCircle_Context::toggle_layer_creation()
{
  // don't allow none layer creation
  if (get_layer_shape_flag() +
     get_layer_region_flag() +
     get_layer_outline_flag() +
     get_layer_advanced_outline_flag() +
     get_layer_curve_gradient_flag() +
     get_layer_plant_flag() == 0)
  {
    if(layer_shape_flag) set_layer_shape_flag(true);
    else if(layer_region_flag) set_layer_region_flag(true);
    else if(layer_outline_flag) set_layer_outline_flag(true);
    else if(layer_advanced_outline_flag) set_layer_advanced_outline_flag(true);
    else if(layer_curve_gradient_flag) set_layer_curve_gradient_flag(true);
    else if(layer_plant_flag) set_layer_plant_flag(true);
  }

	// brush size
	if (get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_curve_gradient_flag())
	{
		bline_width_label.set_sensitive(true);
		bline_width_dist.set_sensitive(true);
	}
	else
	{
		bline_width_label.set_sensitive(false);
		bline_width_dist.set_sensitive(false);
	}

	// spline points and offset angle
	if (!get_layer_region_flag() &&
		!get_layer_outline_flag() &&
		!get_layer_advanced_outline_flag() &&
		!get_layer_plant_flag() &&
		!get_layer_curve_gradient_flag())
	{
		bline_points_label.set_sensitive(false);
		number_of_bline_points_spin.set_sensitive(false);
		bline_point_angle_offset_label.set_sensitive(false);
		bline_point_angle_offset_spin.set_sensitive(false);
	}
	else
	{
		bline_points_label.set_sensitive(true);
		number_of_bline_points_spin.set_sensitive(true);
		bline_point_angle_offset_label.set_sensitive(true);
		bline_point_angle_offset_spin.set_sensitive(true);
	}

	// invert
	if (get_layer_shape_flag() ||
		get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag())
	{
		invert_box.set_sensitive(true);
	}
	else
		invert_box.set_sensitive(false);

	// feather size
	if (get_layer_shape_flag() ||
		get_layer_shape_flag() ||
		get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag())
	{
		feather_label.set_sensitive(true);
		feather_dist.set_sensitive(true);
	}
	else
	{
		feather_label.set_sensitive(false);
		feather_dist.set_sensitive(false);
	}

	// falloff type for circle layer only
	if (get_layer_shape_flag())
	{
		feather_dist.set_sensitive(true);
		feather_label.set_sensitive(true);

		falloff_label.set_sensitive(true);
		falloff_enum.set_sensitive(true);
	}
	else
	{
		falloff_label.set_sensitive(false);
		falloff_enum.set_sensitive(false);
	}

	// orignis at center
	if (get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_plant_flag() ||
		get_layer_curve_gradient_flag())
	{
		origins_at_center_box.set_sensitive(true);
	}
	else
		origins_at_center_box.set_sensitive(false);

	// link origins
	if (get_layer_region_flag() +
		get_layer_outline_flag() +
		get_layer_advanced_outline_flag() +
		get_layer_plant_flag() +
		get_layer_curve_gradient_flag() +
		get_layer_shape_flag() >= 2)
		{
			link_origins_box.set_sensitive(true);
		}
	else link_origins_box.set_sensitive(false);

  // update layer flags
  layer_shape_flag = get_layer_shape_flag();
  layer_region_flag = get_layer_region_flag();
  layer_outline_flag = get_layer_outline_flag();
  layer_advanced_outline_flag = get_layer_advanced_outline_flag();
  layer_curve_gradient_flag = get_layer_curve_gradient_flag();
  layer_plant_flag = get_layer_plant_flag();
}
