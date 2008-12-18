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
#include "boneweightpair.h"
#include "canvas.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneInfluence::ValueNode_BoneInfluence(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_BoneInfluence::ValueNode_BoneInfluence(const ValueNode::Handle &x, Canvas::LooseHandle canvas):
	LinkableValueNode(x->get_type())
{
	switch(x->get_type())
	{
	case ValueBase::TYPE_VECTOR:
	{
		ValueNode_StaticList::Handle bone_weight_list(ValueNode_StaticList::create(ValueBase::TYPE_BONE_WEIGHT_PAIR, canvas));
		bone_weight_list->add(ValueNode_BoneWeightPair::create(BoneWeightPair(Bone(), 1), canvas));
		set_link("link",				x);
		set_link("bone_weight_list",	bone_weight_list);

		if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
			printf("%s:%d set parent canvas for bone influence to %lx\n", __FILE__, __LINE__, ulong(canvas.get()));
		set_parent_canvas(canvas);

		break;
	}
	default:
		throw Exception::BadType(ValueBase::type_local_name(x->get_type()));
	}

	DCAST_HACK_ENABLE();
}

ValueNode_BoneInfluence*
ValueNode_BoneInfluence::create(const ValueBase &x, Canvas::LooseHandle canvas)
{
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

	Vector link((*link_)(t).get(Vector()));

	Matrix transform;
	transform *= 0;
	vector<ValueBase> bone_weight_list((*bone_weight_list_)(t).get_list());
	Real total_weight = 0;
	for (vector<ValueBase>::iterator iter = bone_weight_list.begin(); iter != bone_weight_list.end(); iter++)
	{
		Bone bone(iter->get(BoneWeightPair()).get_bone());
		Real weight(iter->get(BoneWeightPair()).get_weight());

		if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
		{
			printf("%s  *\n", bone.get_setup_matrix().get_string(15, "t = setup").c_str());
			printf("%s  =\n", bone.get_animated_matrix().get_string(15, "animated", strprintf("* %.2f (weight)", weight)).c_str());
			printf("%s\n",	 (bone.get_setup_matrix() * bone.get_animated_matrix() * weight).get_string(15).c_str());
		}

		transform += (bone.get_setup_matrix() *
					  bone.get_animated_matrix() *
					  weight);
		total_weight += weight;
	}

	if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
	{
		printf("%s:%d transform:\n%s\n", __FILE__, __LINE__, transform.get_string().c_str());
		printf("%s:%d total_weight: %.2f\n", __FILE__, __LINE__, total_weight);;
	}

	if (total_weight) transform *= (1/total_weight);

	if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
		printf("%s:%d final transform:\n%s\n", __FILE__, __LINE__, transform.get_string().c_str());

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


String
ValueNode_BoneInfluence::get_name()const
{
	return "boneinfluence";
}

String
ValueNode_BoneInfluence::get_local_name()const
{
	return _("Bone Influence");
}

bool
ValueNode_BoneInfluence::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,				ValueBase::TYPE_VECTOR);
	case 1: CHECK_TYPE_AND_SET_VALUE(bone_weight_list_,	ValueBase::TYPE_LIST);
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_BoneInfluence::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_;
	case 1: return bone_weight_list_;
	}

	return 0;
}

int
ValueNode_BoneInfluence::link_count()const
{
	return 2;
}

String
ValueNode_BoneInfluence::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("link");
	case 1: return _("bone_weight_list");
	}

	return String();
}

String
ValueNode_BoneInfluence::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Link");
	case 1: return _("Bone Weight List");
	}

	return String();
}

int
ValueNode_BoneInfluence::get_link_index_from_name(const String &name)const
{
	if(name=="link")				return 0;
	if(name=="vertex_setup")		return 0; // todo: this is what it used to be called - it can be deleted once my files are converted
	if(name=="bone_weight_list")	return 1;

	throw Exception::BadLinkName(name);
}

bool
ValueNode_BoneInfluence::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}
