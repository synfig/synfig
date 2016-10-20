/* === S Y N F I G ========================================================= */
/*!	\file state_shape.cpp
**	\brief Generic shape tool state
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

#include "state_shape.h"
#include "docks/dock_toolbox.h"

#include "make_region_layer.h"
#include "make_outline_layer.h"
#include "make_advanced_outline_layer.h"
#include "make_plant_layer.h"
#include "make_curve_gradient_layer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

void
StateShape_Context::increment_id()
{
	// TODO: improve code here
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id=get_name();

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for (digits = 0;
			 (int)id.size()-1 >= digits && id[id.size()-1-digits] <= '9' && id[id.size()-1-digits] >= '0';
			 digits++)
			;

		String str_number;
		str_number=String(id,id.size()-digits,id.size());
		id=String(id,0,id.size()-digits);

		number=atoi(str_number.c_str());
	}
	else
	{
		number=1;
		digits=3;
	}

	number++;

	// Add the number back onto the id
	{
		const String format(strprintf("%%0%dd",digits));
		id+=strprintf(format.c_str(),number);
	}

	// Set the ID
	set_id(id);
}

void
StateShape_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(get_local_name());
	App::dialog_tool_options->set_name(get_name_lower());
}

Smach::event_result
StateShape_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateShape_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateShape_Context::event_stop_handler(const Smach::event& /*x*/)
{
	throw &state_normal;
	return Smach::RESULT_OK;
}

void
StateShape_Context::do_load_settings()
{
	State_Context::do_load_settings();

	// TODO: make this more generic somehow
	set_id(get_setting("id", get_name()));
	set_blend(get_setting("blend", 0));//(int)Color::BLEND_COMPOSITE); //0 should be blend composites value
	set_opacity(get_setting("opacity", 1.0));
	set_bline_width(get_setting("bline_width", synfig::Distance(1, App::distance_system))); // default width
	set_feather_size(get_setting("feather", synfig::Distance(0, App::distance_system))); // default feather
	set_invert(get_setting("invert", false));
	set_layer_shape_flag(get_setting("layer_shape", true));
	set_layer_region_flag(get_setting("layer_region", false));
	set_layer_outline_flag(get_setting("layer_outline", false));
	set_layer_advanced_outline_flag(get_setting("layer_advanced_outline", false));
	set_layer_curve_gradient_flag(get_setting("layer_curve_gradient", false));
	set_layer_plant_flag(get_setting("layer_plant", false));
	set_layer_link_origins_flag(get_setting("layer_link_origins", true));

	// determine layer flags
	layer_shape_flag = get_layer_shape_flag();
	layer_region_flag = get_layer_region_flag();
	layer_outline_flag = get_layer_outline_flag();
	layer_advanced_outline_flag = get_layer_outline_flag();
	layer_curve_gradient_flag = get_layer_curve_gradient_flag();
	layer_plant_flag = get_layer_plant_flag();
}

void
StateShape_Context::do_save_settings()
{
	synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
	set_setting("id",get_id());
	set_setting("blend",get_blend());
	set_setting("opacity",(float)get_opacity());
	set_setting("bline_width", bline_width_dist.get_value());
	set_setting("feather", feather_dist.get_value());
	set_setting("invert",get_invert());
	set_setting("layer_shape",get_layer_shape_flag());
	set_setting("layer_outline",get_layer_outline_flag());
	set_setting("layer_advanced_outline",get_layer_advanced_outline_flag());
	set_setting("layer_region",get_layer_region_flag());
	set_setting("layer_curve_gradient",get_layer_curve_gradient_flag());
	set_setting("layer_plant",get_layer_plant_flag());
	set_setting("layer_link_origins",get_layer_link_origins_flag());
}

StateShape_Context::StateShape_Context(CanvasView* canvas_view) :
	State_Context(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	duckmatic_push(get_work_area()),
	opacity_hscl(0.0f, 1.0125f, 0.0125f)
{
	region_maker = new MakeRegionLayer(this);
	outline_maker = new MakeOutlineLayer(this);
	advanced_outline_maker = new MakeAdvancedOutlineLayer(this);
	plant_maker = new MakePlantLayer(this);
	curve_gradient_maker = new MakeCurveGradientLayer(this);

	enable_egress_on_selection_change();


	/* Set up the tool options dialog */

	// 0, title
	title_label.set_label(_((get_name()+" Creation").c_str()));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	// 1, layer name label and entry
	id_label.set_label(_("Name:"));
	id_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	SPACING(id_gap, GAP);
	id_box.pack_start(id_label, Gtk::PACK_SHRINK);
	id_box.pack_start(*id_gap, Gtk::PACK_SHRINK);

	id_box.pack_start(id_entry);
}

void
StateShape_Context::enter()
{

	// 2, layer types creation
	layer_types_label.set_label(_("Layer Type:"));
	layer_types_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	layer_creation(layer_shape_togglebutton,
		("synfig-layer_geometry_"+get_name_lower()), get_local_create());

	layer_creation(layer_region_togglebutton,
		("synfig-layer_geometry_region"), _("Create a region layer"));

	layer_creation(layer_outline_togglebutton,
		("synfig-layer_geometry_outline"), _("Create a outline layer"));

	layer_creation(layer_advanced_outline_togglebutton,
		("synfig-layer_geometry_advanced_outline"), _("Create a advanced outline layer"));

	layer_creation(layer_plant_togglebutton,
		("synfig-layer_other_plant"), _("Create a plant layer"));

	layer_creation(layer_curve_gradient_togglebutton,
		("synfig-layer_gradient_curve"), _("Create a gradient layer"));

	SPACING(layer_types_indent, INDENTATION);

	layer_types_box.pack_start(*layer_types_indent, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_shape_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_region_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_outline_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_advanced_outline_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_plant_togglebutton, Gtk::PACK_SHRINK);
	layer_types_box.pack_start(layer_curve_gradient_togglebutton, Gtk::PACK_SHRINK);

	// 3, blend method label and dropdown list
	blend_label.set_label(_("Blend Method:"));
	blend_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	SPACING(blend_gap, GAP);
	blend_box.pack_start(blend_label, Gtk::PACK_SHRINK);
	blend_box.pack_start(*blend_gap, Gtk::PACK_SHRINK);

	blend_enum.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method")));

	// 4, opacity label and slider
	opacity_label.set_label(_("Opacity:"));
	opacity_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	opacity_hscl.set_digits(2);
	opacity_hscl.set_value_pos(Gtk::POS_LEFT);
	opacity_hscl.set_tooltip_text(_("Opacity"));

	// 5, brush size
	bline_width_label.set_label(_("Brush Size:"));
	bline_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bline_width_label.set_sensitive(false);
	bline_width_dist.set_tooltip_text(_("Brush size"));
	bline_width_dist.set_digits(2);
	bline_width_dist.set_range(0,10000000);
	bline_width_dist.set_sensitive(false);

	// 6, invert
	invert_label.set_label(_("Invert"));
	invert_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	invert_box.pack_start(invert_label);
	invert_box.pack_end(invert_checkbutton, Gtk::PACK_SHRINK);
	invert_box.set_sensitive(false);

	// 7, feather
	feather_label.set_label(_("Feather:"));
	feather_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	feather_label.set_sensitive(false);

	feather_dist.set_digits(2);
	feather_dist.set_range(0,10000000);
	feather_dist.set_sensitive(false);

	// 9, link origins
	link_origins_label.set_label(_("Link Origins"));
	link_origins_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);

	link_origins_box.pack_start(link_origins_label);
	link_origins_box.pack_end(layer_link_origins_checkbutton, Gtk::PACK_SHRINK);
	link_origins_box.set_sensitive(false);

	load_settings();
}

void
StateShape_Context::finalize_init()
{
	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_work_area()->set_cursor(get_cursor());

	App::dock_toolbox->refresh();
}

StateShape_Context::~StateShape_Context()
{
}

void
StateShape_Context::leave()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	// Refresh the work area
	get_work_area()->queue_draw();

	get_canvas_view()->queue_rebuild_ducks();

	App::dock_toolbox->refresh();
}

void
StateShape_Context::generate_shape_layers(
	synfig::Canvas::Handle canvas,
	int depth,
	synfigapp::Action::PassiveGrouper& group,
	synfigapp::SelectionManager::LayerList& layer_selection,
	synfig::ValueNode_BLine::Handle value_node_bline,
	synfig::Vector& origin,
	synfig::ValueNode::Handle value_node_origin
)
{
	if(get_layer_curve_gradient_flag())
	{
		curve_gradient_maker->make_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	if(get_layer_plant_flag())
	{
		plant_maker->make_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	if(get_layer_region_flag())
	{
		region_maker->make_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	if (get_layer_outline_flag())
	{
		outline_maker->make_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}

	if (get_layer_advanced_outline_flag())
	{
		advanced_outline_maker->make_layer(canvas, depth, group, layer_selection, value_node_bline, origin, value_node_origin);
	}
}

// TODO: fix this
synfigapp::Action::PassiveGrouper
StateShape_Context::init_layer_creation(int& depth, Canvas::Handle& canvas, synfigapp::SelectionManager::LayerList& layer_selection)
{
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	Layer::Handle layer = get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	} else
	{
		depth = 0;
		canvas=get_canvas_view()->get_canvas();
	}

	if (!getenv("SYNFIG_TOOLS_CLEAR_SELECTION")) {
		layer_selection = get_canvas_view()->get_selection_manager()->get_selected_layers();
	} else
	{
		layer_selection = synfigapp::SelectionManager::LayerList();
	}

	return synfigapp::Action::PassiveGrouper(
		get_canvas_interface()->get_instance().get(),
		get_local_new()
	);
}

void
StateShape_Context::finalize_layer_creation(synfigapp::SelectionManager::LayerList layer_selection)
{
	disable_egress_on_selection_change();
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	enable_egress_on_selection_change();

	reset();
	increment_id();
}
