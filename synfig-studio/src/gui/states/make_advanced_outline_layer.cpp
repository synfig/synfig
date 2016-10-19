/* === S Y N F I G ========================================================= */
/*!	\file make_advanced_outline_layer.cpp
**	\brief Make advanced outline layer
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

#include "make_advanced_outline_layer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M E T H O D S ======================================================= */
void
MakeAdvancedOutlineLayer::make_layer(
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
