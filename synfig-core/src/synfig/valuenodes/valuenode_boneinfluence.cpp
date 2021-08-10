/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneinfluence.cpp
**	\brief Implementation of the "BoneInfluence" valuenode conversion.
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

#include "valuenode_boneinfluence.h"
#include "valuenode_boneweightpair.h"
#include "valuenode_staticlist.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/boneweightpair.h>
#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/blinepoint.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define epsilon 1e-6

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BoneInfluence, RELEASE_VERSION_0_62_00, "boneinfluence", "Bone Influence")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneInfluence::ValueNode_BoneInfluence(Type &x):
	LinkableValueNode(x),
	checked_inverse_(),
	has_inverse_()
{
}

ValueNode_BoneInfluence::ValueNode_BoneInfluence(const ValueNode::Handle &x, Canvas::LooseHandle canvas):
	LinkableValueNode(x->get_type()),
	checked_inverse_(),
	has_inverse_()
{
	Type &type(x->get_type());
	if (type == type_vector || type == type_bline_point)
	{
		ValueNode_StaticList::Handle bone_weight_list(ValueNode_StaticList::create_on_canvas(type_bone_weight_pair, canvas));
		bone_weight_list->add(std::shared_ptr<ValueNode>(ValueNode_BoneWeightPair::create(BoneWeightPair(Bone(), 1), canvas)));
		set_link("bone_weight_list",	bone_weight_list);
		set_link("link",				x);

		if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
			printf("%s:%d set parent canvas for bone influence to %p\n", __FILE__, __LINE__, canvas.get());
		set_parent_canvas(canvas);
	}
	else
	{
		throw Exception::BadType(type.description.local_name);
	}
}

ValueNode_BoneInfluence*
ValueNode_BoneInfluence::create(const ValueBase &x, Canvas::LooseHandle canvas)
{
	if (x.get_type() == type_bline_point)
		return new ValueNode_BoneInfluence(std::shared_ptr<ValueNode>(ValueNode_Composite::create(x, canvas)), canvas);

	return new ValueNode_BoneInfluence(ValueNode_Const::create(x, canvas), canvas);
}

LinkableValueNode*
ValueNode_BoneInfluence::create_new()const
{
	return new ValueNode_BoneInfluence(get_type());
}

ValueNode_BoneInfluence::~ValueNode_BoneInfluence()
{
	unlink_all();
}

ValueBase
ValueNode_BoneInfluence::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Matrix transform(get_transform(true, t));
	Type &type(link_->get_type());
	if (type == type_vector)
	{
		Vector link((*link_)(t).get(Vector()));

		if (getenv("SYNFIG_DEBUG_BONE_VECTOR_TRANSFORMATION"))
			printf("%s\n", transform.get_string(35,
												strprintf("transform (%7.2f %7.2f) using",
														  link[0],
														  link[1]),
												strprintf("= (%7.2f %7.2f)",
														  transform.get_transformed(link)[0],
														  transform.get_transformed(link)[1])).c_str());

		return transform.get_transformed(link);
	}
	if (type == type_bline_point)
	{
		BLinePoint link((*link_)(t).get(BLinePoint()));
		Point v(link.get_vertex());
		Point vt(transform.get_transformed(v));
		link.set_vertex(vt);

		if (!getenv("SYNFIG_COMPLEX_TANGENT_BONE_INFLUENCE"))
		{
			link.set_tangent1(transform.get_transformed(link.get_tangent1() + v) - vt);
			if (link.get_split_tangent_both())
				link.set_tangent2(transform.get_transformed(link.get_tangent2() + v) - vt);
		}
		else
		{
			link.set_boned_vertex_flag(true);
			link.set_vertex_setup(v);
		}

		if (getenv("SYNFIG_DEBUG_BONE_BLINEPOINT_TRANSFORMATION"))
			printf("%s\n", transform.get_string(35,
												strprintf("transform v(%7.2f %7.2f) using",
														  v[0],
														  v[1]),
												strprintf("= (%7.2f %7.2f)",
														  vt[0],
														  vt[1]
														  )).c_str());
		return link;
	}

	assert(0);
	return ValueBase();
}




bool
ValueNode_BoneInfluence::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bone_weight_list_,	type_list);
	case 1: CHECK_TYPE_AND_SET_VALUE(link_,				get_type());
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_BoneInfluence::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return bone_weight_list_;
	case 1: return link_;
	}

	return 0;
}

bool
ValueNode_BoneInfluence::check_type(Type &type)
{
	return 	type==type_vector ||
			type==type_bline_point;
}

LinkableValueNode::Vocab
ValueNode_BoneInfluence::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bone_weight_list")
		.set_local_name(_("Bone Weight List"))
		.set_description(_("List of bones used to calculate the influence"))
	);

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node being bone influenced"))
	);

	return ret;
}


Matrix
ValueNode_BoneInfluence::calculate_transform(Time t)const
{
	Matrix transform;
	transform *= 0.0;
	std::vector<ValueBase> bone_weight_list((*bone_weight_list_)(t).get_list());
	Real total_weight = 0;
	for (std::vector<ValueBase>::iterator iter = bone_weight_list.begin(); iter != bone_weight_list.end(); iter++)
	{
		Bone bone(iter->get(BoneWeightPair()).get_bone());
		Real weight(iter->get(BoneWeightPair()).get_weight());

		if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
		{
			printf("%s  *\n", Matrix().set_scale(bone.get_local_scale()).get_string(15, "local scale").c_str());
			printf("%s  =\n", bone.get_animated_matrix().get_string(15, "animated", strprintf("* %.2f (weight)", weight)).c_str());
		}

		transform += ( bone.get_animated_matrix()
				     * Matrix().set_scale(bone.get_local_scale()) ) * weight;
		total_weight += weight;
	}

	if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
	{
		printf("%s:%d transform:\n%s\n", __FILE__, __LINE__, transform.get_string().c_str());
		printf("%s:%d total_weight: %.2f\n", __FILE__, __LINE__, total_weight);
	}

	if (abs(total_weight) > epsilon)
		transform *= (1.0/total_weight);
	else
		transform = Matrix();

	if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
		printf("%s:%d final transform:\n%s\n", __FILE__, __LINE__, transform.get_string().c_str());

	return transform;
}

Matrix&
ValueNode_BoneInfluence::get_transform(bool rebuild, Time t)const
{
	if (rebuild) set_transform(calculate_transform(t));

	return transform_;
}

bool
ValueNode_BoneInfluence::has_inverse_transform()const
{
	if (checked_inverse_)
	{
//		printf("%s:%d returning stored value %d for has_inverse\n", __FILE__, __LINE__, has_inverse_);
		return has_inverse_;
	}

	inverse_transform_ = get_transform();
	if ((has_inverse_ = inverse_transform_.is_invertible()))
		inverse_transform_.invert();

//	printf("%s:%d returning calculated value %d for has_inverse\n", __FILE__, __LINE__, has_inverse_);

	checked_inverse_ = true;
	return has_inverse_;
}

Matrix&
ValueNode_BoneInfluence::get_inverse_transform()const
{
	if (has_inverse_transform())
		return inverse_transform_;
	error("get_inverse_transform() called when no inverse is available");
	assert(0);
	return inverse_transform_;
}
