/* === S Y N F I G ========================================================= */
/*!	\file make_region_layer.cpp
**	\brief Make region layer
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

#include "make_region_layer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M E T H O D S ======================================================= */

void
MakeRegionLayer::make_layer(
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

	shape_context->disable_egress_on_selection_change();
	Layer::Handle layer(get_canvas_interface()->add_layer_to("region",canvas,depth));
	shape_context->enable_egress_on_selection_change();
	if (!layer)
	{
		shape_context->get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);
	layer->set_description(shape_context->get_id()+_(" Region"));
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

	layer->set_param("blend_method",shape_context->get_blend());
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",shape_context->get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	layer->set_param("feather",shape_context->get_feather_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"feather");

	layer->set_param("invert",shape_context->get_invert());
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
	if (   shape_context->get_layer_link_origins_flag()
		&& shape_context->layers_to_create() > 1)
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
