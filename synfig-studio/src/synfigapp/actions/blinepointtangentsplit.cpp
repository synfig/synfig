/* === S Y N F I G ========================================================= */
/*!	\file blinepointtangentsplit.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
**	Copyright (c) 2012 Carlos López
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

#include "blinepointtangentsplit.h"
#include "valuedescset.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_radialcomposite.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentSplit);
ACTION_SET_NAME(Action::BLinePointTangentSplit,"BLinePointTangentSplit");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentSplit,N_("Split Tangents"));
ACTION_SET_TASK(Action::BLinePointTangentSplit,"disconnect");
ACTION_SET_CATEGORY(Action::BLinePointTangentSplit,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentSplit,0);
ACTION_SET_VERSION(Action::BLinePointTangentSplit,"0.2");

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentSplitRadius);
ACTION_SET_NAME(Action::BLinePointTangentSplitRadius,"BLinePointTangentSplitRadius");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentSplitRadius,N_("Split Tangents's Radius"));
ACTION_SET_TASK(Action::BLinePointTangentSplitRadius,"type_vector");
ACTION_SET_CATEGORY(Action::BLinePointTangentSplitRadius,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentSplitRadius,0);
ACTION_SET_VERSION(Action::BLinePointTangentSplitRadius,"0.1");

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentSplitAngle);
ACTION_SET_NAME(Action::BLinePointTangentSplitAngle,"BLinePointTangentSplitAngle");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentSplitAngle,N_("Split Tangents's Angle"));
ACTION_SET_TASK(Action::BLinePointTangentSplitAngle,"type_angle");
ACTION_SET_CATEGORY(Action::BLinePointTangentSplitAngle,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentSplitAngle,0);
ACTION_SET_VERSION(Action::BLinePointTangentSplitAngle,"0.1");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/// Return the param valuenode if it is a blinepoint itself or
///  the related blinepoint valuenode if param is a radial composite and vector type (ie. a tangent duck)
static ValueNode_Composite::Handle search_for_related_blinepoint(const Action::Param& param) {
	ValueNode_Composite::Handle blinepoint;
	blinepoint=ValueNode_Composite::Handle::cast_dynamic(param.get_value_node());
	if(blinepoint && blinepoint->get_type()==type_bline_point)
		return blinepoint;

	ValueNode_RadialComposite::Handle radial_value_node;
	radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(param.get_value_node());
	if(radial_value_node && radial_value_node->get_type()==type_vector)
	// value_node is radial composite and vector (user right click on a tangent)
	{
		std::set<Node*>::iterator iter;
		// now check if the parent of radial_value_node is a blinepoint type
		for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
		{
			blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
			if(blinepoint && blinepoint->get_type()==type_bline_point)
				return blinepoint;
		}
	}
	return nullptr;
}

/* === M E T H O D S ======================================================= */


//// BLINEPOINT TANGENT SPLIT //////////
Action::BLinePointTangentSplit::BLinePointTangentSplit()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentSplit::get_local_name()const
{
	if (value_nodes.size() == 1)
		return strprintf(_("Split Tangents of '%s'"), ValueNode::Handle(*value_nodes.begin())->get_description().c_str());
	else {
		std::string descriptions;
		for (const auto& value_node : value_nodes)
			descriptions.append(ValueNode::Handle(value_node)->get_description());
		return strprintf(_("Split Tangents of %zu vertices: '%s'"), value_nodes.size(), descriptions.c_str());
	}
}

Action::ParamVocab
Action::BLinePointTangentSplit::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode of Spline Point"))
		.set_supports_multiple()
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);
	return ret;
}

bool
Action::BLinePointTangentSplit::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		auto valuenode_range = x.equal_range("value_node");
		for (auto param_iter = valuenode_range.first; param_iter != valuenode_range.second; ++param_iter) {
			const Action::Param& param_valuenode = param_iter->second;
			ValueNode_Composite::Handle value_node;
			value_node=search_for_related_blinepoint(param_valuenode);
			// at this point we should have a value node and it should be blinepoint
			// if we haven't, then return false
			if(!value_node)
				return false;
			synfig::Time time(x.find("time")->second.get_time());
			bool split_radius=(*value_node->get_link("split_radius"))(time).get(bool());
			bool split_angle=(*value_node->get_link("split_angle"))(time).get(bool());
			if(split_radius==true && split_angle==true)
				return false;
		}
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentSplit::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		ValueNode_Composite::Handle value_node;
		value_node = search_for_related_blinepoint(param.get_value_node());
		if (value_node) {
			value_nodes.insert(value_node);
			return true;
		}
		return false;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::BLinePointTangentSplit::is_ready()const
{
	if(value_nodes.empty())
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(value_nodes.empty() || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentSplit::prepare()
{
	clear();
	for (const auto& value_node : value_nodes)
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_radius")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(true));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);


		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_angle")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(true));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}


//// BLINEPOINT TANGENT SPLIT RADIUS //////////
Action::BLinePointTangentSplitRadius::BLinePointTangentSplitRadius()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentSplitRadius::get_local_name()const
{
	if (value_nodes.size() == 1)
		return strprintf(_("Split Tangents' Radius of '%s'"), ValueNode::Handle(*value_nodes.begin())->get_description().c_str());
	else {
		std::string descriptions;
		for (const auto& value_node : value_nodes)
			descriptions.append(ValueNode::Handle(value_node)->get_description());
		return strprintf(_("Split Tangents' Radius of %zu vertices: '%s'"), value_nodes.size(), descriptions.c_str());
	}
}

Action::ParamVocab
Action::BLinePointTangentSplitRadius::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
				  .set_local_name(_("ValueNode of Spline Point"))
				  .set_supports_multiple()
				  );
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
				  .set_local_name(_("Time"))
				  );
	return ret;
}

bool
Action::BLinePointTangentSplitRadius::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		auto valuenode_range = x.equal_range("value_node");
		for (auto param_iter = valuenode_range.first; param_iter != valuenode_range.second; ++param_iter) {
			const Action::Param& param_valuenode = param_iter->second;
			ValueNode_Composite::Handle value_node;
			value_node=search_for_related_blinepoint(param_valuenode);
			// at this point we should have a value node and it should be blinepoint
			// if we haven't, then return false
			if(!value_node)
				return false;
			synfig::Time time(x.find("time")->second.get_time());
			bool split_radius=(*value_node->get_link("split_radius"))(time).get(bool());
			if(split_radius==true)
				return false;
		}
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentSplitRadius::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		ValueNode_Composite::Handle value_node;
		value_node = search_for_related_blinepoint(param.get_value_node());
		if (value_node) {
			value_nodes.insert(value_node);
			return true;
		}
		return false;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::BLinePointTangentSplitRadius::is_ready()const
{
	if(value_nodes.empty())
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(value_nodes.empty() || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentSplitRadius::prepare()
{
	clear();
	for (const auto& value_node : value_nodes)
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_radius")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(true));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}

//// BLINEPOINT TANGENT SPLIT ANGLE //////////
Action::BLinePointTangentSplitAngle::BLinePointTangentSplitAngle()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentSplitAngle::get_local_name()const
{
	if (value_nodes.size() == 1)
		return strprintf(_("Split Tangents' Angle of '%s'"), ValueNode::Handle(*value_nodes.begin())->get_description().c_str());
	else {
		std::string descriptions;
		for (const auto& value_node : value_nodes)
			descriptions.append(ValueNode::Handle(value_node)->get_description());
		return strprintf(_("Split Tangents' Angle of %zu vertices: '%s'"), value_nodes.size(), descriptions.c_str());
	}
}

Action::ParamVocab
Action::BLinePointTangentSplitAngle::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
				  .set_local_name(_("ValueNode of Spline Point"))
				  .set_supports_multiple()
				  );
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
				  .set_local_name(_("Time"))
				  );
	return ret;
}

bool
Action::BLinePointTangentSplitAngle::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		auto valuenode_range = x.equal_range("value_node");
		for (auto param_iter = valuenode_range.first; param_iter != valuenode_range.second; ++param_iter) {
			const Action::Param& param_valuenode = param_iter->second;
			ValueNode_Composite::Handle value_node;
			value_node=search_for_related_blinepoint(param_valuenode);
			// at this point we should have a value node and it should be blinepoint
			// if we haven't, then return false
			if(!value_node)
				return false;
			synfig::Time time(x.find("time")->second.get_time());
			bool split_angle=(*value_node->get_link("split_angle"))(time).get(bool());
			if(split_angle==true)
				return false;
		}
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentSplitAngle::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		ValueNode_Composite::Handle value_node;
		value_node = search_for_related_blinepoint(param.get_value_node());
		if (value_node) {
			value_nodes.insert(value_node);
			return true;
		}
		return false;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::BLinePointTangentSplitAngle::is_ready()const
{
	if(value_nodes.empty())
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(value_nodes.empty() || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentSplitAngle::prepare()
{
	clear();
	for (const auto& value_node : value_nodes)
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_angle")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(true));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}
