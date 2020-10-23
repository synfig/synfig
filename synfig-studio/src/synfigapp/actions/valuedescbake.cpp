/* === S Y N F I G ========================================================= */
/*!	\file valuedescbake.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2019 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenodes/valuenode_animated.h>

#include <synfigapp/canvasinterface.h>

#include "valuedescconnect.h"

#include "valuedescbake.h"

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescBake);
ACTION_SET_NAME(Action::ValueDescBake,"ValueDescBake");
ACTION_SET_LOCAL_NAME(Action::ValueDescBake,N_("Bake"));
ACTION_SET_TASK(Action::ValueDescBake,"convert");
ACTION_SET_CATEGORY(Action::ValueDescBake,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescBake,0);
ACTION_SET_VERSION(Action::ValueDescBake,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescBake::ValueDescBake()
	{ }

synfig::String
Action::ValueDescBake::get_local_name()const
	{ return _("Bake"); }

Action::ParamVocab
Action::ValueDescBake::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);
	return ret;
}

bool
Action::ValueDescBake::is_candidate(const ParamList &x)
{
	if (!candidate_check(get_param_vocab(), x))
		return false;
	const ValueDesc value_desc = x.find("value_desc")->second.get_value_desc();
	if (!value_desc && !value_desc.is_value_node())
		return false;
	ValueNode::Handle value_node = value_desc.get_value_node();
	if (!value_node)
		return false;
	if (ValueNode_Const::Handle::cast_dynamic(value_node))
		return false;
	if (!is_type_supported(value_node->get_type()))
		return false;
	return true;
}

bool
Action::ValueDescBake::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc = param.get_value_desc();
		return true;
	}
	return Action::CanvasSpecific::set_param(name, param);
}

bool
Action::ValueDescBake::is_ready()const
{
	if (!value_desc)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescBake::prepare()
{
	clear();

	const RendDesc &renddesc = get_canvas()->rend_desc();
	
	ValueNode::Handle value_node = value_desc.get_value_node();
	ValueNode::Handle baked = bake(
		value_node,
		renddesc.get_time_start(),
		renddesc.get_time_end(),
		renddesc.get_frame_rate() );
	if (!baked)
		throw Error(_("Unable to bake"));

	Action::Handle action(ValueDescConnect::create());
	action->set_param("canvas", get_canvas());
	action->set_param("canvas_interface", get_canvas_interface());
	action->set_param("src", baked);
	action->set_param("dest", value_desc);
	assert(action->is_ready());
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);

	add_action_front(action);
}


bool
Action::ValueDescBake::is_type_supported(synfig::Type &type)
{
	try { ValueNode_Animated::create(type); }
	catch(...) { return false; }
	return true;
}

ValueNode::Handle
Action::ValueDescBake::bake(
	const ValueNode::Handle &value_node,
	Time begin,
	Time end,
	float fps )
{
	if (!value_node)
		return ValueNode::Handle();
	if (!approximate_greater_lp(fps, 0.f))
		fps = end - begin;
	if (end < begin) {
		end = begin;
		fps = 1;
	}
	double step = 1/(double)fps;
	
	Type &type = value_node->get_type();
	ValueNode_Animated::Handle animated;
	try {
		animated = ValueNode_Animated::create(type);
	} catch(...) {
		error("ValueDescBake: Cannot create ValueNode_Animated, may be unsopported value type.");
		return ValueNode::Handle();
	}
	
	Interpolation interpolation = INTERPOLATION_CONSTANT;
	if ( type == type_time
	  || type == type_real
	  || type == type_angle
	  || type == type_vector
	  || type == type_color
	  || type == type_gradient )
		interpolation = INTERPOLATION_CLAMPED;
	
	ValueBase prev_value;
	int index = 0;
	while(true) {
		Time t = begin + step*index++;
		if (index > 10000000) {
			error("ValueDescBake: Reached limit of iterations.");
			return ValueNode::Handle();
		}
		if (approximate_greater_lp((double)t, (double)end))
			break;
		if (t > end) // set exact end, for the last iteration
			t = end;

		ValueBase value = (*value_node)(t);
		if (prev_value != value) {
			Waypoint &wp = *animated->new_waypoint(t, value);
			wp.set_before(interpolation);
			wp.set_after(interpolation);
			prev_value = value;
		}

		if (t == end) // stop the cycle (see upper comment)
			break;
	}
	
	assert(!animated->waypoint_list().empty());
	if (animated->waypoint_list().size() == 1)
		return ValueNode_Const::create(prev_value);
	
	return animated;
}
