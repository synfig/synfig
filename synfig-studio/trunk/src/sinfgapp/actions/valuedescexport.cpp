/* === S I N F G =========================================================== */
/*!	\file valuedescset.cpp
**	\brief Template File
**
**	$Id: valuedescexport.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include <sinfgapp/canvasinterface.h>
#include <sinfg/valuenode_const.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::ValueDescExport);
ACTION_SET_NAME(Action::ValueDescExport,"value_desc_export");
ACTION_SET_LOCAL_NAME(Action::ValueDescExport,"Export");
ACTION_SET_TASK(Action::ValueDescExport,"export");
ACTION_SET_CATEGORY(Action::ValueDescExport,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescExport,0);
ACTION_SET_VERSION(Action::ValueDescExport,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescExport,"$Id: valuedescexport.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescExport::ValueDescExport()
{
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
Action::ValueDescExport::is_canidate(const ParamList &x)
{
	if(canidate_check(get_param_vocab(),x))
	{
		ValueDesc value_desc=x.find("value_desc")->second.get_value_desc();
		if(!value_desc || value_desc.parent_is_canvas() || (value_desc.is_value_node() && value_desc.get_value_node()->is_exported()))
			return false;
		return true;
	}
	return false;		
}

bool
Action::ValueDescExport::set_param(const sinfg::String& param_name, const Action::Param &param)
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
		if(!value_desc.is_const())
			throw Error(_("Can only export Canvas when used as constant parameter"));
		Canvas::Handle canvas(value_desc.get_value().get(Canvas::Handle()));
		
		Action::Handle action(CanvasAdd::create());
		
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("src",canvas);
		action->set_param("id",name);
	
		assert(action->is_ready());		
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
	
		add_action_front(action);		
		
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
