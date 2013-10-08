/* === S Y N F I G ========================================================= */
/*!	\file valuedescset.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2013 Konstantin Dmitriev
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

#include "layerparamset.h"
#include "valuenodeconstset.h"
#include "valuedescconnect.h"
#include "waypointsetsmart.h"

#include "valuedescset.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_wplist.h>
#include <synfig/valuenode_blinecalctangent.h>
#include <synfig/valuenode_blinecalcvertex.h>
#include <synfig/valuenode_blinecalcwidth.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_radialcomposite.h>
#include <synfig/valuenode_range.h>
#include <synfig/valuenode_reference.h>
#include <synfig/valuenode_boneinfluence.h>
#include <synfig/valuenode_scale.h>
#include <synfig/valuenode_integer.h>
#include <synfig/valuenode_real.h>
#include <synfigapp/main.h>

#include <synfigapp/general.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescSet);
ACTION_SET_NAME(Action::ValueDescSet,"ValueDescSet");
ACTION_SET_LOCAL_NAME(Action::ValueDescSet,N_("Set ValueDesc"));
ACTION_SET_TASK(Action::ValueDescSet,"set");
ACTION_SET_CATEGORY(Action::ValueDescSet,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescSet,0);
ACTION_SET_VERSION(Action::ValueDescSet,"0.0");
ACTION_SET_CVS_ID(Action::ValueDescSet,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescSet::ValueDescSet():
	time(0),
	recursive(true)
{
}

synfig::String
Action::ValueDescSet::get_local_name()const
{
	return strprintf(_("Set %s"),
					 value_desc
					 ? value_desc.get_description().c_str()
					 : _("ValueDesc"));
}

Action::ParamVocab
Action::ValueDescSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("recursive", Param::TYPE_BOOL)
		.set_local_name(_("Recursive"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueDescSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();

		return true;
	}

	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		value=param.get_value();

		return true;
	}

	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();

		return true;
	}
	if(name=="recursive" && param.get_type()==Param::TYPE_BOOL)
	{
		recursive=param.get_bool();

		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescSet::is_ready()const
{
	if(!value_desc || !value.is_valid())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescSet::prepare()
{
	clear();

	// If our tangents are merged, and
	// our first tangent is being manipulated,
	// then we also need to adjust the other
	// tangent.
	if(	value_desc.parent_is_value_node()
		&&
		value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT
		&&
		(value_desc.get_name()=="t1" || value_desc.get_name()=="t2")
		&&
		recursive
	)
	{
		ValueNode_Composite::Handle parent_value_node;
		parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());
		assert(parent_value_node);
		BLinePoint bp((*parent_value_node)(time).get(BLinePoint()));
		if(!bp.get_split_tangent_radius() || !bp.get_split_tangent_angle())
		{
			if (value_desc.get_name()=="t1")
			{
				// We are modifying t1 so we need to update t2 accordingly. Let's ask it to the BLinePoint
				// We modify the tangent1 (and the tangent2 is updated)
				bp.set_tangent1(value);
				// then we retrieve the correct tangent2
				ValueBase new_value(bp.get_tangent2());
				Action::Handle action(Action::create("ValueDescSet"));
				if(!action)
					throw Error(_("Unable to find action ValueDescSet (bug)"));
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",new_value);
				action->set_param("value_desc",ValueDesc(parent_value_node, parent_value_node->get_link_index_from_name("t2")));
				action->set_param("recursive", false);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action(action);
			}
			if (value_desc.get_name()=="t2") {
				// We are modifying t2 so we need to update t1 accordingly. Let's ask it to the BLinePoint
				// Create a new BLinePoint
				BLinePoint nbp;
				// Terporary set the flags for the new BLinePoint to all split
				nbp.set_split_tangent_both(true);
				// Now we can set the tangents. Tangent2 won't be modified by tangent1
				nbp.set_tangent1(value);
				// bp.tangent1 is the one we want to update based on value
				nbp.set_tangent2(bp.get_tangent1());
				// Now update the flags (nbp.tangent2 will be updated)
				nbp.set_split_tangent_radius(bp.get_split_tangent_radius());
				nbp.set_split_tangent_angle(bp.get_split_tangent_angle());
				// Now retrieve the updated tangent2 (which will be stored later as t1, see below)
				ValueBase new_value(nbp.get_tangent2());
				Action::Handle action(Action::create("ValueDescSet"));
				if(!action)
					throw Error(_("Unable to find action ValueDescSet (bug)"));
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",new_value);
				action->set_param("value_desc",ValueDesc(parent_value_node, parent_value_node->get_link_index_from_name("t1")));
				action->set_param("recursive", false);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action(action);
			}
		}
	}
	// If we are a reference value node, then
	// we need to distribute the changes to the
	// referenced value node
	if(value_desc.is_value_node() && ValueNode_Reference::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueDesc reference_value_desc(ValueNode_Reference::Handle::cast_dynamic(value_desc.get_value_node()),0);
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",value);
		action->set_param("value_desc",reference_value_desc);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}

	// if we are a boneinfluence value node, then we need to distribute the changes to the linked value node
	// This is only valid for vector type converted to bone influence
	// Not valid for a whole blinepoint converted to bone influence
	if(value_desc.is_value_node() && value.get_type() == ValueBase::TYPE_VECTOR)
	{
		if (ValueNode_BoneInfluence::Handle bone_influence_value_node =
			ValueNode_BoneInfluence::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueDesc bone_influence_value_desc(bone_influence_value_node,
												bone_influence_value_node->get_link_index_from_name("link"));

			if (bone_influence_value_node->has_inverse_transform())
				value = bone_influence_value_node->get_inverse_transform().get_transformed(value);
			else
				throw Error(_("this node isn't editable - in the future it will be greyed to prevent editing"));

			Action::Handle action(Action::create("ValueDescSet"));

			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",value);
			action->set_param("value_desc",bone_influence_value_desc);

			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);

			add_action(action);

			return;
		}
	}

	// if we are a boneinfluence value node, then we need to distribute the changes
	/// to the linked value node.
	// This is only valid for blinepoint type converted to bone influence
	// Not valid for a vector converted to bone influence
	if(value_desc.is_value_node() && value.get_type() == ValueBase::TYPE_BLINEPOINT)
	{
		if (ValueNode_BoneInfluence::Handle bone_influence_value_node =
			ValueNode_BoneInfluence::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueDesc bone_influence_value_desc(bone_influence_value_node,
												bone_influence_value_node->get_link_index_from_name("link"));
			ValueBase old_value(value);
			BLinePoint bp, nbp;
			nbp=bp=old_value.get(BLinePoint());
			if (bone_influence_value_node->has_inverse_transform())
			{
				nbp.set_vertex(bone_influence_value_node->get_inverse_transform().get_transformed(bp.get_vertex()));
				nbp.set_tangent1(bone_influence_value_node->get_inverse_transform().get_transformed(bp.get_tangent1()));
				nbp.set_tangent2(bone_influence_value_node->get_inverse_transform().get_transformed(bp.get_tangent2()));
			}
			else
				throw Error(_("this node isn't editable - in the future it will be greyed to prevent editing"));
			
			Action::Handle action(Action::create("ValueDescSet"));
			
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",ValueBase(nbp));
			action->set_param("value_desc",bone_influence_value_desc);
			
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			
			add_action(action);
			
			return;
		}
	}


	// If we are a composite value node, then
	// we need to distribute the changes to the
	// individual parts
	// except if we are TYPE WIDTHPOINT which is handled individually
	// except if we are TYPE BLINEPOINT which is handled individually
	if(value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
	 && value_desc.get_value_node()->get_type()!=ValueBase::TYPE_WIDTHPOINT
	 && value_desc.get_value_node()->get_type()!=ValueBase::TYPE_BLINEPOINT)
	{
		ValueBase components[8];
		int n_components(0);
		switch(value.get_type())
		{
		case ValueBase::TYPE_VECTOR:
			components[0]=value.get(Vector())[0];
			components[1]=value.get(Vector())[1];
			n_components=2;
			break;
		case ValueBase::TYPE_COLOR:
			components[0]=value.get(Color()).get_r();
			components[1]=value.get(Color()).get_g();
			components[2]=value.get(Color()).get_b();
			components[3]=value.get(Color()).get_a();
			n_components=4;
			break;
		case ValueBase::TYPE_SEGMENT:
			components[0]=value.get(Segment()).p1;
			components[1]=value.get(Segment()).t1;
			components[2]=value.get(Segment()).p2;
			components[3]=value.get(Segment()).t2;
			n_components=4;
			break;
		default:
			throw Error(_("Bad type for composite (%s)"),ValueBase::type_local_name(value.get_type()).c_str());
			break;
		}
		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()),i);
			Action::Handle action(Action::create("ValueDescSet"));
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",components[i]);
			action->set_param("value_desc",component_value_desc);
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action(action);
		}
		return;
	}

	// If we are a composite value node type BLINEPOINT, then
	// we need to distribute the changes to the
	// individual parts in a proper way
	if(value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
	 && value_desc.get_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT)
	{
		ValueBase components[8];
		int n_components(0);
		int order[8]={0,1,2,3,6,7,4,5};
		BLinePoint bline_point(value);
		components[0]=bline_point.get_vertex();
		components[1]=bline_point.get_width();
		components[2]=bline_point.get_origin();
		components[3]=bline_point.get_split_tangent_both();
		components[4]=bline_point.get_tangent1();
		components[5]=bline_point.get_tangent2();
		components[6]=bline_point.get_split_tangent_radius();
		components[7]=bline_point.get_split_tangent_angle();
		n_components=8;
		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()),order[i]);
			Action::Handle action(Action::create("ValueDescSet"));
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",components[order[i]]);
			action->set_param("value_desc",component_value_desc);
			action->set_param("recursive", false);
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action(action);
		}
		return;
	}

	// If we are a RADIAL composite value node, then
	// we need to distribute the changes to the
	// individual parts
	if(value_desc.is_value_node() && ValueNode_RadialComposite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase components[6];
		int n_components(0);
		switch(value.get_type())
		{
		case ValueBase::TYPE_VECTOR:
		{
			Angle old_angle = (*(ValueNode_RadialComposite::Handle::cast_dynamic(
									 value_desc.get_value_node())->get_link("theta")))(time).get(Angle());
			Vector vect(value.get(Vector()));
			components[0]=vect.mag();
			Angle change = Angle(Angle::tan(vect[1],vect[0])) - old_angle;
			while (change < Angle::deg(-180)) change += Angle::deg(360);
			while (change > Angle::deg(180)) change -= Angle::deg(360);
			components[1]=old_angle + change;
			n_components=2;
		}
			break;
		case ValueBase::TYPE_COLOR:
			components[0]=value.get(Color()).get_y();
			components[1]=value.get(Color()).get_s();
			components[2]=value.get(Color()).get_hue();
			components[3]=value.get(Color()).get_a();
			n_components=4;
			break;
		default:
			throw Error(_("Bad type for radial composite (%s)"),ValueBase::type_local_name(value.get_type()).c_str());
			break;
		}
		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(ValueNode_RadialComposite::Handle::cast_dynamic(value_desc.get_value_node()),i);
			Action::Handle action(Action::create("ValueDescSet"));
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",components[i]);
			action->set_param("value_desc",component_value_desc);
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action(action);
		}
		return;
	}

	// Perform reverse manipulations

	// If we are a scale value node, then edit the link
	// such that it will scale to our target value
	if (ValueNode_Scale::Handle scale_value_node = ValueNode_Scale::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		if(! scale_value_node->is_invertible(time))
		{
			synfig::warning(_("Attempt to edit scale ValueNode with a scale factor of zero."));
			return;
		}
		ValueBase new_value;
		if (value.get_type() == ValueBase::TYPE_ANGLE)
			new_value = scale_value_node->get_inverse(time, value.get(Angle()));
		else if(value.get_type() == ValueBase::TYPE_VECTOR)
			new_value = scale_value_node->get_inverse(time, value.get(Vector()));
		else if(value.get_type() == ValueBase::TYPE_REAL)
			new_value = scale_value_node->get_inverse(time, value.get(Real()));
		else
			throw Error(_("Inverse manipulation of %s scale values not implemented in core."), value.type_name().c_str());
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(scale_value_node, scale_value_node->get_link_index_from_name("link")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
    // Range: disallow values outside the range
	if (ValueNode_Range::Handle range_value_node = ValueNode_Range::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase new_value;
		if (value.get_type() == ValueBase::TYPE_ANGLE)
			new_value = range_value_node->get_inverse(time, value.get(Angle()));
		else
			throw Error(_("Inverse manipulation of %s range values not implemented in core."), value.type_name().c_str());
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(range_value_node,range_value_node->get_link_index_from_name("link")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	// Integer: integer values only
	if (ValueNode_Integer::Handle integer_value_node = ValueNode_Integer::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase new_value;
		if (value.get_type() == ValueBase::TYPE_ANGLE)
			new_value = integer_value_node->get_inverse(time, value.get(Angle()));
		else if(value.get_type() == ValueBase::TYPE_REAL)
			new_value = integer_value_node->get_inverse(time, value.get(Real()));
		else
			throw Error(_("Inverse manipulation of %s scale values not implemented in core."), value.type_name().c_str());
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(integer_value_node,integer_value_node->get_link_index_from_name("link")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	// Real: Reverse manipulations for Real->Angle convert
	if (ValueNode_Real::Handle real_value_node = ValueNode_Real::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase new_value;
		if (value.get_type() == ValueBase::TYPE_ANGLE)
			new_value = real_value_node->get_inverse(time, value.get(Angle()));
		else
			throw Error(_("Inverse manipulation of %s scale values not implemented in core."), value.type_name().c_str());
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_value);
		action->set_param("value_desc",ValueDesc(real_value_node,real_value_node->get_link_index_from_name("link")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	// BlineCalcWidth: modify the scale value node
	// so that the target width is achieved
	if (ValueNode_BLineCalcWidth::Handle bline_width = ValueNode_BLineCalcWidth::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		Real old_width((*bline_width)(time).get(Real()));
		Real scale((*(bline_width->get_link("scale")))(time).get(Real()));
		ValueBase new_width(value.get(Real()) * scale / old_width);
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_width);
		action->set_param("value_desc",ValueDesc(bline_width, bline_width->get_link_index_from_name("scale")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	// BLineCalcVertex: snap the point to the nearest
	// allowed position.
	if (ValueNode_BLineCalcVertex::Handle bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_BLine::Handle bline = ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link("bline"));
		Real radius = 0.0;
		ValueBase new_amount;
		if (((*(bline_vertex->get_link("loop")))(time).get(bool())))
		{
			// The bline is looped. Animation may require an amount parameter
			// outside the range of 0-1, so make sure that the amount does
			// not change drastically.
			Real amount_old((*(bline_vertex->get_link("amount")))(time).get(Real()));

			Real amount_new = synfig::find_closest_point((*bline)(time), value, radius, bline->get_loop());
			Real difference = fmod( fmod(amount_new - amount_old, 1.0) + 1.0 , 1.0);
			//fmod is called twice to avoid negative values
			if (difference > 0.5)
				difference=difference-1.0;
			new_amount = amount_old+difference;
		}
		else
			new_amount = synfig::find_closest_point((*bline)(time), value, radius, bline->get_loop());
		bool homogeneous((*(bline_vertex->get_link("homogeneous")))(time).get(bool()));
		if(homogeneous)
			new_amount=std_to_hom((*bline)(time), new_amount, ((*(bline_vertex->get_link("loop")))(time).get(bool())), bline->get_loop() );
		Action::Handle action(Action::create("ValueDescSet"));
		if(!action)
			throw Error(_("Unable to find action ValueDescSet (bug)"));
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("time",time);
		action->set_param("new_value",new_amount);
		action->set_param("value_desc",ValueDesc(bline_vertex, bline_vertex->get_link_index_from_name("amount")));
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	// BLineCalcTangent: adjust scale and offset
	// to achieve the desired tangent
	if (ValueNode_BLineCalcTangent::Handle bline_tangent = ValueNode_BLineCalcTangent::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase new_scale;
		ValueDesc scale_value_desc(bline_tangent,bline_tangent->get_link_index_from_name("scale"));
		ValueDesc offset_value_desc(bline_tangent,bline_tangent->get_link_index_from_name("offset"));
		switch(value_desc.get_value_type())
		{
		case ValueBase::TYPE_REAL:
		{
			Real old_length = (*bline_tangent)(time).get(Real());
			Real new_length = value.get(Vector()).mag();
			Real scale((*(bline_tangent->get_link("scale")))(time).get(Real()));
			bool fixed_length((*(bline_tangent->get_link("fixed_length")))(time).get(bool()));
			if (fixed_length)
			{
				new_scale = new_length;
				break;
			}
			if (old_length == 0)
				return;
			new_scale = new_length * scale / old_length;
		}
		case ValueBase::TYPE_VECTOR:
		{
			Vector old_tangent = (*bline_tangent)(time).get(Vector());
			Angle old_angle = old_tangent.angle();
			Real old_length = old_tangent.mag();
			Angle new_angle = value.get(Vector()).angle();
			Real new_length = value.get(Vector()).mag();
			Angle old_offset((*(bline_tangent->get_link("offset")))(time).get(Angle()));
			Real scale((*(bline_tangent->get_link("scale")))(time).get(Real()));
			bool fixed_length((*(bline_tangent->get_link("fixed_length")))(time).get(bool()));
			if (fixed_length)
			{
				new_scale = new_length;
				break;
			}
			if (old_length != 0)
			{
				new_scale = new_length * scale / old_length;
				Action::Handle action(Action::create("ValueDescSet"));
				if(!action)
					throw Error(_("Unable to find action ValueDescSet (bug)"));
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value", ValueBase(old_offset + new_angle - old_angle));
				action->set_param("value_desc",offset_value_desc);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action(action);
			}
		}
		break;
		case ValueBase::TYPE_ANGLE:
		{
			Angle old_angle = (*bline_tangent)(time).get(Angle());
			Angle new_angle = value.get(Angle());
			Angle old_offset((*(bline_tangent->get_link("offset")))(time).get(Angle()));
			Action::Handle action(Action::create("ValueDescSet"));
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value", ValueBase(old_offset + new_angle - old_angle));
			action->set_param("value_desc",offset_value_desc);
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action(action);
			return;
		}
		default:
			break;
		}
		if (new_scale)
		{
			Action::Handle action(Action::create("ValueDescSet"));
			if(!action)
				throw Error(_("Unable to find action ValueDescSet (bug)"));
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("time",time);
			action->set_param("new_value",new_scale);
			action->set_param("value_desc",scale_value_desc);
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action(action);
		}
		return;
	}

	// end reverse manipulations

	// WidthPoint Composite: adjust the width point position
	// to achieve the desired point on bline
	// (Code copied from BLineCalcVertex above)
	if (value_desc.parent_is_linkable_value_node() && value_desc.get_parent_value_node()->get_type() == ValueBase::TYPE_LIST)
	{
		if(ValueNode_DynamicList::Handle::cast_dynamic(value_desc.get_parent_value_node())->get_contained_type() == ValueBase::TYPE_WIDTHPOINT)
		{
			ValueNode_WPList::Handle wplist=ValueNode_WPList::Handle::cast_dynamic(value_desc.get_parent_value_node());
			if(wplist)
			{
				bool wplistloop(wplist->get_loop());
				ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(wplist->get_bline()));
				ValueNode_Composite::Handle wpoint_composite(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
				if(bline && wpoint_composite)
				{
					bool blineloop(bline->get_loop());
					// Retrieve the homogeneous layer parameter
					bool homogeneous=false;
					Layer::Handle layer_parent;
					std::set<Node*>::iterator iter;
					for(iter=wplist->parent_set.begin();iter!=wplist->parent_set.end();++iter)
						{
							Layer::Handle layer;
							layer=Layer::Handle::cast_dynamic(*iter);
							if(layer && layer->get_name() == "advanced_outline")
							{
								homogeneous=layer->get_param("homogeneous").get(bool());
								break;
							}
						}
					Real radius = 0.0;
					ValueBase new_amount;
					WidthPoint wp((*wpoint_composite)(time));
					if (wplistloop)
					{
						// The wplist is looped. Animation may require a position parameter
						// outside the range of 0-1, so make sure that the position doesn't
						// change drastically.
						Real amount_old(wp.get_norm_position(wplistloop));
						Real amount_old_b(wp.get_bound_position(wplistloop));
						// If it is homogeneous then convert it to standard
						amount_old=homogeneous?hom_to_std((*bline)(time), amount_old, wplistloop, blineloop):amount_old;
						// grab a new position given by duck's position on the bline
						Real amount_new = synfig::find_closest_point((*bline)(time), value, radius, blineloop);
						// calculate the difference between old and new amounts
						Real difference = fmod( fmod(amount_new - amount_old, 1.0) + 1.0 , 1.0);
						//fmod is called twice to avoid negative values
						if (difference > 0.5)
							difference=difference-1.0;
						// calculate a new value for the position
						new_amount=amount_old+difference;
						// restore the homogeneous value if needed
						new_amount = homogeneous?ValueBase(std_to_hom((*bline)(time), new_amount, wplistloop, blineloop)):new_amount;
						// this is the difference between the new amount and the old amount inside the boundaries
						Real bound_diff((wp.get_lower_bound() + new_amount*(wp.get_upper_bound()-wp.get_lower_bound()))-amount_old_b);
						// add the new diff to the current amount
						new_amount = wp.get_position() + bound_diff;
					}
					else
					{
						// grab a new amount given by duck's position on the bline
						new_amount = synfig::find_closest_point((*bline)(time), value , radius, blineloop);
						// if it is homogeneous then convert to it
						new_amount=homogeneous?ValueBase(std_to_hom((*bline)(time), new_amount, wplistloop, blineloop)):new_amount;
						// convert the value inside the boundaries
						new_amount = wp.get_lower_bound()+new_amount*(wp.get_upper_bound()-wp.get_lower_bound());
					}
					Action::Handle action(Action::create("ValueDescSet"));
					if(!action)
						throw Error(_("Unable to find action ValueDescSet (bug)"));
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("time",time);
					action->set_param("new_value",new_amount);
					action->set_param("value_desc",ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("position")));
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);
					add_action(action);
					return;
				}
			}
		}
	}

	// end reverse manipulations

	// If we are merging the tangents of a BLinePoint,
	// we must also set the second tangent for things
	// to interpolate properly
	if (value_desc.parent_is_value_node() &&
	    value_desc.get_parent_value_node()->get_type()==ValueBase::TYPE_BLINEPOINT)
	{
		ValueNode_Composite::Handle parent_value_node;
		parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());
		assert(parent_value_node);
		int split_radius=parent_value_node->get_link_index_from_name("split_radius");
		int split_angle=parent_value_node->get_link_index_from_name("split_angle");
		int index_value_desc=value_desc.get_index();
		if(index_value_desc==split_radius)
		{
			// are we splitting or merging the radius?
			if (value.get(bool()))
			{
				// we are splitting radius
				Vector t2new((*parent_value_node->get_link("t1"))(time).get(Vector()).mag(),(*parent_value_node->get_link("t2"))(time).get(Vector()).angle());
				Action::Handle action(Action::create("ValueDescSet"));
				if(!action)
					throw Error(_("Unable to find action ValueDescSet (bug)"));
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",ValueBase(t2new));
				action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t2")));
				action->set_param("recursive", false);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action(action);
			}
			else
			{
				// we are merging radius
				// the merged tangent should be the average radius of the 2 tangents we're merging
				Real average(((*parent_value_node->get_link("t1"))(time).get(Vector()).mag() +
							  (*parent_value_node->get_link("t2"))(time).get(Vector()).mag()) / 2);
				Vector t1new(Vector(average, (*parent_value_node->get_link("t1"))(time).get(Vector()).angle()));
				Vector t2new(Vector(average, (*parent_value_node->get_link("t2"))(time).get(Vector()).angle()));
				{
					Action::Handle action(Action::create("ValueDescSet"));
					if(!action)
						throw Error(_("Unable to find action ValueDescSet (bug)"));
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("time",time);
					action->set_param("new_value",ValueBase(t1new));
					action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t1")));
					action->set_param("recursive", false);
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);
					add_action(action);
				}
				{
					Action::Handle action(Action::create("ValueDescSet"));
					if(!action)
						throw Error(_("Unable to find action ValueDescSet (bug)"));
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("time",time);
					action->set_param("new_value",ValueBase(t2new));
					action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t2")));
					action->set_param("recursive", false);
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);
					add_action(action);
				}
			}
		}
		else if(index_value_desc==split_angle)
		{
			// are we splitting or merging the angle?
			if (value.get(bool()))
			{
				// we are splitting angle
				Vector t2new((*parent_value_node->get_link("t2"))(time).get(Vector()).mag(),(*parent_value_node->get_link("t1"))(time).get(Vector()).angle());
				Action::Handle action(Action::create("ValueDescSet"));
				if(!action)
					throw Error(_("Unable to find action ValueDescSet (bug)"));
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("time",time);
				action->set_param("new_value",ValueBase(t2new));
				action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t2")));
				action->set_param("recursive", false);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action(action);
			}
			else
			{
				// we are merging angle
				// the merged tangent should be the average angle of the 2 tangents we're merging
				Angle average(((*parent_value_node->get_link("t1"))(time).get(Vector()).angle() +
							  (*parent_value_node->get_link("t2"))(time).get(Vector()).angle()) / 2);
				Vector t1new(Vector((*parent_value_node->get_link("t1"))(time).get(Vector()).mag(), average));
				Vector t2new(Vector((*parent_value_node->get_link("t2"))(time).get(Vector()).mag(), average));
				{
					Action::Handle action(Action::create("ValueDescSet"));
					if(!action)
						throw Error(_("Unable to find action ValueDescSet (bug)"));
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("time",time);
					action->set_param("new_value",ValueBase(t1new));
					action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t1")));
					action->set_param("recursive", false);
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);
					add_action(action);
				}
				{
					Action::Handle action(Action::create("ValueDescSet"));
					if(!action)
						throw Error(_("Unable to find action ValueDescSet (bug)"));
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->set_param("time",time);
					action->set_param("new_value",ValueBase(t2new));
					action->set_param("value_desc",ValueDesc(parent_value_node,parent_value_node->get_link_index_from_name("t2")));
					action->set_param("recursive", false);
					if(!action->is_ready())
						throw Error(Error::TYPE_NOTREADY);
					add_action(action);
				}
			}
		}
	}

	// if value desc has parent value node and parent is composite widthpoint type and index is 4 or 5
	// then we are changing the value of a widthpoint boundary.
	// It is needed to check that we aren't doing the boundary range zero

	if(value_desc.parent_is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()))
	{
		ValueNode_Composite::Handle parent_value_node;
		parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());
		assert(parent_value_node);
		int i=value_desc.get_index();
		if(parent_value_node->get_type() == ValueBase::TYPE_WIDTHPOINT && (i==4 || i==5))
		{
			ValueNode::Handle low(parent_value_node->get_link("lower_bound"));
			ValueNode::Handle upp(parent_value_node->get_link("upper_bound"));
			Real new_value(value.get(Real()));
			Real lower = (*low)(Time(0.0)).get(Real());
			Real upper = (*upp)(Time(0.0)).get(Real());
			if( (i==4 && new_value > (upper- 0.00000001))
			||  (i==5 && new_value < (lower+ 0.00000001)) )
			{
				throw Error(_("It is forbidden to set lower boundary equal or bigger than upper boundary"));
				return;
			}
		}
	}

	// If we are changing the z_depth range parameters
	// send a signal to the canvas interface to say that the layer has changed
	if(value_desc.parent_is_layer_param()
	   &&
	   (value_desc.get_param_name() =="z_range"
		||
		value_desc.get_param_name() =="z_range_position"
		||
		value_desc.get_param_name() =="z_range_depth")
	   )
	{
		if (get_canvas_interface() && recursive)
		{
			get_canvas_interface()->signal_layer_z_range_changed()(value_desc.get_layer(),true);
		}
	}

	// If we are in animate editing mode
	// TODO: Can we replace local_value to value after all parameters will be converted into ValueBase type?
	if(get_edit_mode()&MODE_ANIMATE && !value_desc.get_static())
	{
		ValueNode_Animated::Handle& value_node(value_node_animated);
		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();
			Interpolation interp=value.get_interpolation();
			if(!value_node)value_node=ValueNode_Animated::create(value,time);
			// Be sure that the newly created waypoint is set with the default
			// interpolations.
			synfig::ValueNode_Animated::WaypointList::iterator iter(value_node->find(time));
			iter->set_before(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
			iter->set_after(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
			value_node->set_interpolation(interp);
			Action::Handle action;
			if(!value_desc.is_value_node())
			{
				action=(ValueDescConnect::create());
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=Action::create("ValueNodeReplace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else
		{
			value_node=value_node.cast_dynamic(value_desc.get_value_node());
		}
		if(!value_node)
			throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));
		synfig::ValueNode_Animated::WaypointList::iterator iter;
		Waypoint waypoint;
		Action::Handle action(WaypointSetSmart::create());
		try
		{
			iter=value_node->find(time);
			// value_node->find throws an exception
			// when no waypoint is found at given time
			waypoint=*iter;
		}catch(Exception::NotFound)
		{
			waypoint=value_node->new_waypoint_at_time(time);
			Interpolation inter=value_node->get_interpolation();
			waypoint.set_before(inter==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():inter);
			waypoint.set_after(inter==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():inter);
		}
		waypoint.set_value(value);
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	else						// We are not in animate editing mode
	{
		if(value_desc.is_value_node())
		{
			if(ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				Action::Handle action(ValueNodeConstSet::create());
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",value_desc.get_value_node());
				action->set_param("new_value",value);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action);
				return;
			}
			else
			if(ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()))
				throw Error(_("You must be in Animate-Editing-Mode to directly manipulate this value"));
			else
				throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));
		}
		else
		if(value_desc.parent_is_layer_param() && !value_desc.is_value_node())
		{
			Action::Handle layer_param_set(LayerParamSet::create());
			layer_param_set->set_param("canvas",get_canvas());
			layer_param_set->set_param("canvas_interface",get_canvas_interface());
			layer_param_set->set_param("layer",value_desc.get_layer());
			layer_param_set->set_param("param",value_desc.get_param_name());
			layer_param_set->set_param("new_value",value);
			if(!layer_param_set->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(layer_param_set);
			return;
		}
		throw Error(_("Unsupported ValueDesc type"));
	}

	
}
