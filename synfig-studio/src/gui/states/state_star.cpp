/* === S Y N F I G ========================================================= */
/*!	\file state_star.cpp
**	\brief Star tool state
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2010 Carlos López
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

#include "state_star.h"

#include "docks/dock_toolbox.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

StateStar studio::state_star;

/* === C L A S S E S & S T R U C T S ======================================= */

/* === M E T H O D S ======================================================= */

StateStar::StateStar():
	StateShape<StateStar_Context>("star")
{
}

StateStar::~StateStar()
{
}

void
StateStar_Context::do_load_settings()
{
	StateShape_Context::do_load_settings();

	set_number_of_points(get_setting("number_of_points", 5));
	set_inner_tangent(get_setting("inner_tangent", 0.0));
	set_outer_tangent(get_setting("outer_tangent", 0.0));
	set_inner_width(get_setting("inner_width", 1.0));
	set_outer_width(get_setting("outer_width", 1.0));
	set_radius_ratio(get_setting("radius_ratio", 0.5));
	set_angle_offset(get_setting("angle_offset", 0.0));
	set_regular_polygon(get_setting("regular_polygon", false));
	set_layer_origins_at_center_flag(get_setting("layer_origins_at_center", true));
}

void
StateStar_Context::do_save_settings()
{
	StateShape_Context::do_save_settings();

	set_setting("number_of_points", get_number_of_points());
	set_setting("inner_tangent", get_inner_tangent());
	set_setting("outer_tangent", get_outer_tangent());
	set_setting("inner_width", get_inner_width());
	set_setting("outer_width", get_outer_width());
	set_setting("radius_ratio", get_radius_ratio());
	set_setting("angle_offset", get_angle_offset());
	set_setting("regular_polygon", get_regular_polygon());
	set_setting("layer_origins_at_center", get_layer_origins_at_center_flag());
}

StateStar_Context::StateStar_Context(CanvasView* canvas_view):
	StateShape_Context(canvas_view),
	number_of_points_adj(Gtk::Adjustment::create(0, 2, 120, 1, 1)),
	number_of_points_spin(number_of_points_adj,1,0),
	radius_ratio_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	radius_ratio_spin(radius_ratio_adj,1,2),
	angle_offset_adj(Gtk::Adjustment::create(0, -360, 360, 0.1, 1)),
	angle_offset_spin(angle_offset_adj,1,1),
	outer_width_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	outer_width_spin(outer_width_adj,1,2),
	inner_tangent_adj(Gtk::Adjustment::create(0,-10, 10, 0.01, 0.1)),
	inner_tangent_spin(inner_tangent_adj,1,2),
	inner_width_adj(Gtk::Adjustment::create(0, -10, 10, 0.01, 0.1)),
	inner_width_spin(inner_width_adj,1,2),
	outer_tangent_adj(Gtk::Adjustment::create(0,-10, 10, 0.01, 0.1)),
	outer_tangent_spin(outer_tangent_adj,1,2)
{
	// 6, star points
	number_of_points_label.set_label(_("Star Points:"));
	number_of_points_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 7, angle offset
	SPACING(angle_offset_indent, INDENTATION);
	angle_offset_label.set_label(_("Offset:"));
	angle_offset_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	angle_offset_box.pack_start(*angle_offset_indent, Gtk::PACK_SHRINK);
	angle_offset_box.pack_start(angle_offset_label, Gtk::PACK_SHRINK);

	// 8, radius ratio
	radius_ratio_label.set_label(_("Radius Ratio:"));
	radius_ratio_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 9, regular polygon
	regular_polygon_label.set_label(_("Regular Polygon"));
	regular_polygon_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	regular_polygon_box.pack_start(regular_polygon_label);
	regular_polygon_box.pack_end(regular_polygon_checkbutton, Gtk::PACK_SHRINK);

	// 10, inner width
	inner_width_label.set_label(_("Inner Width:"));
	inner_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 11, inner tangent
	inner_tangent_label.set_label(_("Inner Tangent:"));
	inner_tangent_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 12, outer width
	outer_width_label.set_label(_("Outer Width:"));
	outer_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 13, outer tangent
	outer_tangent_label.set_label(_("Outer Tangent:"));
	outer_tangent_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 17, spline origins at center
	origins_at_center_label.set_label(_("Spline Origins at Center"));
	origins_at_center_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	origins_at_center_box.pack_start(origins_at_center_label);
	origins_at_center_box.pack_end(layer_origins_at_center_checkbutton, Gtk::PACK_SHRINK);
	origins_at_center_box.set_sensitive(false);
}

void
StateStar_Context::enter()
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
	// 6, star points
	options_table.attach(number_of_points_label,
		0, 1, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(number_of_points_spin,
		1, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 7, star points offset
	options_table.attach(angle_offset_box,
		0, 1, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(angle_offset_spin,
		1, 2, 8, 9, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 8, radius ratio
	options_table.attach(radius_ratio_label,
		0, 1, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(radius_ratio_spin,
		1, 2, 9, 10, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 9, regular polygon
	options_table.attach(regular_polygon_box,
		0, 2, 10, 11, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 10, inner width
	options_table.attach(inner_width_label,
		0, 1, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(inner_width_spin,
		1, 2, 11, 12, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 11, inner tangent
	options_table.attach(inner_tangent_label,
		0, 1, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(inner_tangent_spin,
		1, 2, 12, 13, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 12, outer width
	options_table.attach(outer_width_label,
		0, 1, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(outer_width_spin,
		1, 2, 13, 14, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 13, outer tangent
	options_table.attach(outer_tangent_label,
		0, 1, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(outer_tangent_spin,
		1, 2, 14, 15, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 14, invert
	options_table.attach(invert_box,
		0, 2, 15, 16, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 15, feather
	options_table.attach(feather_label,
		0, 1, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(feather_dist,
		1, 2, 16, 17, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 16, link origins
	options_table.attach(link_origins_box,
		0, 2, 17, 18, Gtk::FILL, Gtk::FILL, 0, 0
		);
	// 17, origins at center
	options_table.attach(origins_at_center_box,
		0, 2, 18, 19, Gtk::FILL, Gtk::FILL, 0, 0
		);

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	options_table.set_row_spacing(19, 0); // the final row using border width of table

	finalize_init();
}

StateStar_Context::~StateStar_Context()
{
}

void
StateStar_Context::make_star_layer(
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
	Layer::Handle layer=get_canvas_interface()->add_layer_to("star",canvas,depth);
	enable_egress_on_selection_change();
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);

	Real radius_ratio(get_radius_ratio());
	Real radius1((p2-p1).mag());
	Real radius2(radius1 * radius_ratio);

	Angle::deg offset(get_angle_offset());
	bool regular(get_regular_polygon());
	Angle::deg angle(360.0/get_number_of_points());

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("blend_method",get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("radius1",radius1);
	get_canvas_interface()->signal_layer_param_changed()(layer,"radius1");

	layer->set_param("radius2",radius2);
	get_canvas_interface()->signal_layer_param_changed()(layer,"radius2");

	layer->set_param("angle",offset);
	get_canvas_interface()->signal_layer_param_changed()(layer,"angle");

	layer->set_param("points",get_number_of_points());
	get_canvas_interface()->signal_layer_param_changed()(layer,"points");

	layer->set_param("regular_polygon",regular);
	get_canvas_interface()->signal_layer_param_changed()(layer,"regular_polygon");

	layer->set_param("feather",get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",get_invert());
	get_canvas_interface()->signal_layer_param_changed()(layer,"invert");

	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	// only link the star's origin parameter if the option is selected, we're putting bline
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
			throw String(_("Unable to create Star layer"));
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
StateStar_Context::make_star(const Point& _p1, const Point& _p2)
{
	int depth;
	Canvas::Handle canvas;
	synfigapp::SelectionManager::LayerList layer_selection;

	synfigapp::Action::PassiveGrouper group = init_layer_creation(depth, canvas, layer_selection);

	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
	const Point p1(transform.unperform(_p1));
	const Point p2(transform.unperform(_p2));

	// remove code duplication
	Real radius_ratio(get_radius_ratio());
	Real radius1((p2-p1).mag());
	Real radius2(radius1 * radius_ratio);

	Real inner_tangent = get_inner_tangent() * radius1;
	Real outer_tangent = get_outer_tangent() * radius2;
	Real inner_width = get_inner_width();
	Real outer_width = get_outer_width();

	Angle::deg offset(get_angle_offset());
	bool regular(get_regular_polygon());
	Angle::deg angle(360.0/get_number_of_points());

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

	std::vector<BLinePoint> new_list;
	int point(0);
	for (int i = 0; i < get_number_of_points(); i++)
	{
		new_list.push_back(*(new BLinePoint));
		new_list[point].set_width(outer_width);
		new_list[point].set_vertex(Point(radius1*Angle::cos(angle*i + offset).get() + x,
										 radius1*Angle::sin(angle*i + offset).get() + y));
		new_list[point++].set_tangent(Point(-Angle::sin(angle*i + offset).get(),
											 Angle::cos(angle*i + offset).get()) * outer_tangent);

		if (!regular)
		{
			new_list.push_back(*(new BLinePoint));
			new_list[point].set_width(inner_width);
			new_list[point].set_vertex(Point(radius2*Angle::cos(angle*i + angle/2 + offset).get() + x,
											 radius2*Angle::sin(angle*i + angle/2 + offset).get() + y));
			new_list[point++].set_tangent(Point(-Angle::sin(angle*i + angle/2 + offset).get(),
												 Angle::cos(angle*i + angle/2 + offset).get()) * inner_tangent);
		}
	}

	ValueNode_BLine::Handle value_node_bline(ValueNode_BLine::create(new_list));
	assert(value_node_bline);

	ValueNode::Handle value_node_origin(ValueNode_Const::create(origin));
	assert(value_node_origin);

	// Set the looping flag
	value_node_bline->set_loop(true);

	value_node_bline->set_member_canvas(canvas);

	// Maybe make star layer
	if (get_layer_shape_flag())
	{
		make_star_layer(canvas, depth, group, layer_selection, p1, p2, value_node_origin);
	}

	generate_shape_layers(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	finalize_layer_creation(layer_selection);
}

Smach::event_result
StateStar_Context::event_mouse_click_handler(const Smach::event& x)
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

		make_star(point_holder, point);
		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}

	return Smach::RESULT_OK;
}

void
StateStar_Context::toggle_layer_creation()
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

	// inner/outer width and tangent
	if (get_layer_region_flag() ||
		get_layer_outline_flag() ||
		get_layer_advanced_outline_flag() ||
		get_layer_plant_flag() ||
		get_layer_curve_gradient_flag())
	{
		inner_width_label.set_sensitive(true);
		inner_width_spin.set_sensitive(true);
		inner_tangent_label.set_sensitive(true);
		inner_tangent_spin.set_sensitive(true);
		outer_width_label.set_sensitive(true);
		outer_width_spin.set_sensitive(true);
		outer_tangent_label.set_sensitive(true);
		outer_tangent_spin.set_sensitive(true);
	}
	else
	{
		inner_width_label.set_sensitive(false);
		inner_width_spin.set_sensitive(false);
		inner_tangent_label.set_sensitive(false);
		inner_tangent_spin.set_sensitive(false);
		outer_width_label.set_sensitive(false);
		outer_width_spin.set_sensitive(false);
		outer_tangent_label.set_sensitive(false);
		outer_tangent_spin.set_sensitive(false);
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
