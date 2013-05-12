/* === S Y N F I G ========================================================= */
/*!	\file valuedescexport.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "valuenodeadd.h"

#include "canvasadd.h"
#include "valuedescexport.h"
#include "layerparamconnect.h"
#include "layerparamset.h"

#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_const.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescExport);
ACTION_SET_NAME(Action::ValueDescExport,"ValueDescExport");
ACTION_SET_LOCAL_NAME(Action::ValueDescExport,N_("Export"));
ACTION_SET_TASK(Action::ValueDescExport,"export");
ACTION_SET_CATEGORY(Action::ValueDescExport,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescExport,0);
ACTION_SET_VERSION(Action::ValueDescExport,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescExport,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescExport::ValueDescExport()
{
}

synfig::String
Action::ValueDescExport::get_local_name()const
{
	// TRANSLATORS: This is used in the 'history' dialog when a ValueNode is exported.  The first %s is what is exported, the 2nd is the name it is given.
	return strprintf(_("Export '%s' as '%s'"),
					 value_desc.get_description(false).c_str(),
					 name.c_str());
}

Action::ParamVocab
Action::ValueDescExport::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("name",Param::TYPE_STRING)
		.set_local_name(_("Name"))
		.set_desc(_("The name that you want this value to be exported as"))
		.set_user_supplied()
	);

	return ret;
}

bool
Action::ValueDescExport::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc=x.find("value_desc")->second.get_value_desc();
		if(!value_desc)
			return false;
		if(value_desc.get_value_type()==ValueBase::TYPE_CANVAS)
			if(!value_desc.get_value().get(Canvas::Handle()))
				return false;
		if(
			value_desc.parent_is_canvas()
			||
			(value_desc.is_value_node() && value_desc.get_value_node()->is_exported())
			||
			(value_desc.get_value_type()==ValueBase::TYPE_CANVAS && !value_desc.get_value().get(Canvas::Handle())->is_inline())
			)
		{
			return false;
		}
	// Don't allow to export lower and upper boundaries of the WidhtPoint
		if(value_desc.parent_is_linkable_value_node()
			&& value_desc.get_parent_value_node()->get_name()=="composite"
			&& value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_WIDTHPOINT
			&& (value_desc.get_index()==4 || value_desc.get_index()==5))
			return false;
		return true;
	}
	return false;
}

bool
Action::ValueDescExport::set_param(const synfig::String& param_name, const Action::Param &param)
{
	if(param_name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(param_name=="name" && param.get_type()==Param::TYPE_STRING)
	{
		name=param.get_string();

		return true;
	}

	return Action::CanvasSpecific::set_param(param_name,param);
}

bool
Action::ValueDescExport::is_ready()const
{
	if(!value_desc || name.empty())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescExport::prepare()
{
	clear();

	ValueNode::Handle value_node;

	if(value_desc.get_value_type()==ValueBase::TYPE_CANVAS)
	{
		// action: CanvasAdd
		if(!value_desc.is_const())
			throw Error(_("Can only export Canvas when used as constant parameter"));
		Canvas::Handle canvas(value_desc.get_value().get(Canvas::Handle()));

		// clone canvas (all code that clones a canvas has this comment)
		if (canvas) canvas=canvas->clone(GUID(), true);

		Action::Handle action(CanvasAdd::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",canvas);
		action->set_param("id",name);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);

		if(value_desc.parent_is_layer_param() && !value_desc.is_value_node())
		{
			// action: LayerParamSet
			Action::Handle action(LayerParamSet::create());

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("layer",value_desc.get_layer());
			action->set_param("param",value_desc.get_param_name());
			action->set_param("new_value",ValueBase(canvas));

			assert(action->is_ready());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action_front(action);
		}

		return;
	}

	if(value_desc.is_value_node())
	{
		if(value_desc.get_value_node()->is_exported())
			throw Error(_("ValueBase is already exported"));

		value_node=value_desc.get_value_node();
	}
	else
	{
		// action: LayerParamConnect
		if(!value_desc.parent_is_layer_param())
			throw Error(_("Unable to export parameter. (Bug?)"));

		value_node=ValueNode_Const::create(value_desc.get_value());

		Action::Handle action(LayerParamConnect::create());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",value_desc.get_layer());
		action->set_param("param",value_desc.get_param_name());
		action->set_param("value_node",value_node);

		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);

		add_action_front(action);
	}

	// action: ValueNodeAdd
	Action::Handle action(ValueNodeAdd::create());

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("new",value_node);
	action->set_param("name",name);

	assert(action->is_ready());
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);
}
