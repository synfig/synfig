/* === S Y N F I G ========================================================= */
/*!	\file valuedescsetinterpolation.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
**	Copyright (c) 2013 Konstantin Dmitriev
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

#include <synfig/general.h>

#include "valuedescsetinterpolation.h"

#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescSetInterpolation);
ACTION_SET_NAME(Action::ValueDescSetInterpolation,"ValueDescSetInterpolation");
ACTION_SET_LOCAL_NAME(Action::ValueDescSetInterpolation,N_("Set Parameter Interpolation"));
ACTION_SET_TASK(Action::ValueDescSetInterpolation,"setinterpolation");
ACTION_SET_CATEGORY(Action::ValueDescSetInterpolation,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescSetInterpolation,0);
ACTION_SET_VERSION(Action::ValueDescSetInterpolation,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescSetInterpolation::ValueDescSetInterpolation():
	value(),
	old_value()
{ }

synfig::String
Action::ValueDescSetInterpolation::get_local_name()const
{
	return strprintf(_("Set interpolation for %s"),
					 value_desc
					 ? value_desc.get_description().c_str()
					 : _("ValueDesc"));
}

Action::ParamVocab
Action::ValueDescSetInterpolation::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_INTERPOLATION)
		.set_local_name(_("Interpolation"))
	);

	return ret;
}

bool
Action::ValueDescSetInterpolation::is_candidate(const ParamList &x)
{
	if(!candidate_check(get_param_vocab(),x))
	    return false;
	return true;
}

bool
Action::ValueDescSetInterpolation::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_INTERPOLATION)
	{
		value=param.get_interpolation();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescSetInterpolation::is_ready()const
{
	if(!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescSetInterpolation::perform()
{
	//synfig::info(value_desc.get_description());
	if(value_desc.get_value_node())
	{
		ValueNode::Handle value_node=value_desc.get_value_node();
		old_value = value_node->get_interpolation();
		set_dirty(true);
		value_node->set_interpolation(value);
		value_node->changed();
	}
	else if (value_desc.parent_is_layer())
	{
		old_value = value_desc.get_value().get_interpolation();
		synfig::Layer::Handle layer;
		layer = value_desc.get_layer();
		ValueBase value_base;
		String param_name;
		param_name = value_desc.get_param_name();
		value_base = layer->get_param(param_name);
		value_base.set_interpolation(value);
		layer->set_param(param_name,value_base);
		//! Signal layer changed
		layer->changed();
		//! Signal that a layer parameter changed
		if(get_canvas_interface())
			get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}

void
Action::ValueDescSetInterpolation::undo()
{
	if(value_desc.get_value_node())
	{
		ValueNode::Handle value_node=value_desc.get_value_node();
		set_dirty(true);
		value_node->set_interpolation(old_value);
		value_node->changed();
	} 
	else if (value_desc.parent_is_layer())
	{
		synfig::Layer::Handle layer;
		layer = value_desc.get_layer();
		ValueBase value_base;
		String param_name;
		param_name = value_desc.get_param_name();
		value_base = layer->get_param(param_name);
		value_base.set_interpolation(old_value);
		layer->set_param(param_name,value_base);
		//! Signal layer changed
		layer->changed();
		//! Signal that a layer parameter changed
		if(get_canvas_interface())
			get_canvas_interface()->signal_layer_param_changed()(layer,param_name);
	}
}
