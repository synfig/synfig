/* === S Y N F I G ========================================================= */
/*!	\file blinepointtangentmerge.cpp
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

#include "blinepointtangentmerge.h"
#include "valuedescset.h"

#include "activepointset.h"
#include "activepointadd.h"

#include "valuedescconnect.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>
#include <synfig/valuenodes/valuenode_radialcomposite.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentMerge);
ACTION_SET_NAME(Action::BLinePointTangentMerge,"BLinePointTangentMerge");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentMerge,N_("Merge Tangents"));
ACTION_SET_TASK(Action::BLinePointTangentMerge,"connect");
ACTION_SET_CATEGORY(Action::BLinePointTangentMerge,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentMerge,0);
ACTION_SET_VERSION(Action::BLinePointTangentMerge,"0.1");
ACTION_SET_CVS_ID(Action::BLinePointTangentMerge,"$Id$");

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentMergeRadius);
ACTION_SET_NAME(Action::BLinePointTangentMergeRadius,"BLinePointTangentMergeRadius");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentMergeRadius,N_("Merge Tangents's Radius"));
ACTION_SET_TASK(Action::BLinePointTangentMergeRadius,"type_vector");
ACTION_SET_CATEGORY(Action::BLinePointTangentMergeRadius,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentMergeRadius,0);
ACTION_SET_VERSION(Action::BLinePointTangentMergeRadius,"0.0");
ACTION_SET_CVS_ID(Action::BLinePointTangentMergeRadius,"$Id$");

ACTION_INIT_NO_GET_LOCAL_NAME(Action::BLinePointTangentMergeAngle);
ACTION_SET_NAME(Action::BLinePointTangentMergeAngle,"BLinePointTangentMergeAngle");
ACTION_SET_LOCAL_NAME(Action::BLinePointTangentMergeAngle,N_("Merge Tangents's Angle"));
ACTION_SET_TASK(Action::BLinePointTangentMergeAngle,"type_angle");
ACTION_SET_CATEGORY(Action::BLinePointTangentMergeAngle,Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::BLinePointTangentMergeAngle,0);
ACTION_SET_VERSION(Action::BLinePointTangentMergeAngle,"0.0");
ACTION_SET_CVS_ID(Action::BLinePointTangentMergeAngle,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


//// BLINEPOINT TANGENT MERGE //////////
Action::BLinePointTangentMerge::BLinePointTangentMerge()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentMerge::get_local_name()const
{
	return strprintf(_("Merge Tangents of '%s'"), ((ValueNode::Handle)(value_node))->get_description().c_str());
}

Action::ParamVocab
Action::BLinePointTangentMerge::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
		.set_local_name(_("ValueNode of Spline Point"))
	);
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
	);
	return ret;
}

bool
Action::BLinePointTangentMerge::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueNode_Composite::Handle value_node;
		value_node=ValueNode_Composite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
		if(!value_node || value_node->get_type()!=type_bline_point)
		{
			// Before return false, let's check whether the value_node
			// is radial composite and vector type
			ValueNode_RadialComposite::Handle radial_value_node;
			radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
			if(radial_value_node && radial_value_node->get_type()==type_vector)
			// value_node is radial composite and vector (user right click on a tangent)
			{
				ValueNode_Composite::Handle blinepoint=NULL;
				std::set<Node*>::iterator iter;
				// now check if the parent of radial_value_node is a blinepoint type
				for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
					{
						blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
						if(blinepoint && blinepoint->get_type()==type_bline_point)
							break;
					}
				if(blinepoint)
					value_node=blinepoint;
			}
		}
		// at this point we should have a value node and it should be blinepoint
		// if we haven't, then return false
		if(!value_node || value_node->get_type()!=type_bline_point)
			return false;
		synfig::Time time(x.find("time")->second.get_time());
		bool split_radius=(*value_node->get_link("split_radius"))(time).get(bool());
		bool split_angle=(*value_node->get_link("split_angle"))(time).get(bool());
		if(split_radius==false && split_angle==false)
			return false;
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentMerge::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=value_node.cast_dynamic(param.get_value_node());
		if(value_node && value_node->get_type()==type_bline_point)
			return true;
		ValueNode_RadialComposite::Handle radial_value_node;
		radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(param.get_value_node());
		if(radial_value_node && radial_value_node->get_type()==type_vector)
		// value_node is radial composite and vector (user right click on a tangent)
		{
			ValueNode_Composite::Handle blinepoint;
			std::set<Node*>::iterator iter;
			// now check if the parent of value_node is a blinepoint type
			for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
				{
					blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
					if(blinepoint && blinepoint->get_type()==type_bline_point)
					{
						value_node=blinepoint;
						return true;
					}
				}
			return false;
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
Action::BLinePointTangentMerge::is_ready()const
{
	if(!value_node)
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(!value_node || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentMerge::prepare()
{
	clear();
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_radius")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(false));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_angle")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(false));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}



////// BLINEPOINT TANGENT MERGE RADIUS   //////

Action::BLinePointTangentMergeRadius::BLinePointTangentMergeRadius()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentMergeRadius::get_local_name()const
{
	return strprintf(_("Merge Tangents's Radius of '%s'"), ((ValueNode::Handle)(value_node))->get_description().c_str());
}

Action::ParamVocab
Action::BLinePointTangentMergeRadius::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
				  .set_local_name(_("ValueNode of Spline Point"))
				  );
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
				  .set_local_name(_("Time"))
				  );
	return ret;
}

bool
Action::BLinePointTangentMergeRadius::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueNode_Composite::Handle value_node;
		value_node=ValueNode_Composite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
		if(!value_node || value_node->get_type()!=type_bline_point)
		{
			// Before return false, let's check whether the value_node
			// is radial composite and vector type
			ValueNode_RadialComposite::Handle radial_value_node;
			radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
			if(radial_value_node && radial_value_node->get_type()==type_vector)
				// value_node is radial composite and vector (user right click on a tangent)
			{
				ValueNode_Composite::Handle blinepoint=NULL;
				std::set<Node*>::iterator iter;
				// now check if the parent of radial_value_node is a blinepoint type
				for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
				{
					blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
					if(blinepoint && blinepoint->get_type()==type_bline_point)
						break;
				}
				if(blinepoint)
					value_node=blinepoint;
			}
		}
		// at this point we should have a value node and it should be blinepoint
		// if we haven't, then return false
		if(!value_node || value_node->get_type()!=type_bline_point)
			return false;
		synfig::Time time(x.find("time")->second.get_time());
		bool split_radius=(*value_node->get_link("split_radius"))(time).get(bool());
		if(split_radius==false)
			return false;
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentMergeRadius::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=value_node.cast_dynamic(param.get_value_node());
		if(value_node && value_node->get_type()==type_bline_point)
			return true;
		ValueNode_RadialComposite::Handle radial_value_node;
		radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(param.get_value_node());
		if(radial_value_node && radial_value_node->get_type()==type_vector)
			// value_node is radial composite and vector (user right click on a tangent)
		{
			ValueNode_Composite::Handle blinepoint;
			std::set<Node*>::iterator iter;
			// now check if the parent of value_node is a blinepoint type
			for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
			{
				blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
				if(blinepoint && blinepoint->get_type()==type_bline_point)
				{
					value_node=blinepoint;
					return true;
				}
			}
			return false;
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
Action::BLinePointTangentMergeRadius::is_ready()const
{
	if(!value_node)
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(!value_node || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentMergeRadius::prepare()
{
	clear();
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_radius")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(false));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}


////// BLINEPOINT TANGENT MERGE ANGLE   //////

Action::BLinePointTangentMergeAngle::BLinePointTangentMergeAngle()
{
	time=(Time::begin()-1);
	set_dirty(true);
}

synfig::String
Action::BLinePointTangentMergeAngle::get_local_name()const
{
	return strprintf(_("Merge Tangents's Angle of '%s'"), ((ValueNode::Handle)(value_node))->get_description().c_str());
}

Action::ParamVocab
Action::BLinePointTangentMergeAngle::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	ret.push_back(ParamDesc("value_node",Param::TYPE_VALUENODE)
				  .set_local_name(_("ValueNode of Spline Point"))
				  );
	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
				  .set_local_name(_("Time"))
				  );
	return ret;
}

bool
Action::BLinePointTangentMergeAngle::is_candidate(const ParamList &x)
{
	if(candidate_check(get_param_vocab(),x))
	{
		ValueNode_Composite::Handle value_node;
		value_node=ValueNode_Composite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
		if(!value_node || value_node->get_type()!=type_bline_point)
		{
			// Before return false, let's check whether the value_node
			// is radial composite and vector type
			ValueNode_RadialComposite::Handle radial_value_node;
			radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(x.find("value_node")->second.get_value_node());
			if(radial_value_node && radial_value_node->get_type()==type_vector)
				// value_node is radial composite and vector (user right click on a tangent)
			{
				ValueNode_Composite::Handle blinepoint=NULL;
				std::set<Node*>::iterator iter;
				// now check if the parent of radial_value_node is a blinepoint type
				for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
				{
					blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
					if(blinepoint && blinepoint->get_type()==type_bline_point)
						break;
				}
				if(blinepoint)
					value_node=blinepoint;
			}
		}
		// at this point we should have a value node and it should be blinepoint
		// if we haven't, then return false
		if(!value_node || value_node->get_type()!=type_bline_point)
			return false;
		synfig::Time time(x.find("time")->second.get_time());
		bool split_angle=(*value_node->get_link("split_angle"))(time).get(bool());
		if(split_angle==false)
			return false;
		return true;
	}
	return false;
}

bool
Action::BLinePointTangentMergeAngle::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_node" && param.get_type()==Param::TYPE_VALUENODE)
	{
		value_node=value_node.cast_dynamic(param.get_value_node());
		if(value_node && value_node->get_type()==type_bline_point)
			return true;
		ValueNode_RadialComposite::Handle radial_value_node;
		radial_value_node=ValueNode_RadialComposite::Handle::cast_dynamic(param.get_value_node());
		if(radial_value_node && radial_value_node->get_type()==type_vector)
			// value_node is radial composite and vector (user right click on a tangent)
		{
			ValueNode_Composite::Handle blinepoint;
			std::set<Node*>::iterator iter;
			// now check if the parent of value_node is a blinepoint type
			for(iter=radial_value_node->parent_set.begin();iter!=radial_value_node->parent_set.end();++iter)
			{
				blinepoint=ValueNode_Composite::Handle::cast_dynamic(*iter);
				if(blinepoint && blinepoint->get_type()==type_bline_point)
				{
					value_node=blinepoint;
					return true;
				}
			}
			return false;
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
Action::BLinePointTangentMergeAngle::is_ready()const
{
	if(!value_node)
		synfig::error("Missing or bad value_node");
	if(time==(Time::begin()-1))
		synfig::error("Missing time");
	if(!value_node || time==(Time::begin()-1))
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::BLinePointTangentMergeAngle::prepare()
{
	clear();
	{
		Action::Handle action;
		action=Action::create("ValueDescSet");
		if(!action)
			throw Error(_("Couldn't find action \"ValueDescSet\""));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",ValueDesc(value_node,value_node->get_link_index_from_name("split_angle")));
		action->set_param("time",time);
		action->set_param("new_value",synfig::ValueBase(false));
		assert(action->is_ready());
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
	}
}
