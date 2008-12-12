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

ValueNode_BoneInfluence::ValueNode_BoneInfluence(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	switch(x->get_type())
	{
	case ValueBase::TYPE_VECTOR:
	{
		ValueNode_StaticList::Handle bone_weight_list(ValueNode_StaticList::create(ValueBase::TYPE_MATRIX));
		bone_weight_list->add(ValueNode_BoneWeightPair::create(BoneWeightPair(Bone(), 1)));
		set_link("vertex_free",			ValueNode_Const::create(Vector()));
		set_link("vertex_setup",		x);
		set_link("bone_weight_list",	bone_weight_list);
		break;
	}
	default:
		throw Exception::BadType(ValueBase::type_local_name(x->get_type()));
	}

	DCAST_HACK_ENABLE();
}

ValueNode_BoneInfluence*
ValueNode_BoneInfluence::create(const ValueBase &x)
{
	return new ValueNode_BoneInfluence(ValueNode_Const::create(x));
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

	Vector vertex_free((*vertex_free_)(t).get(Vector()));
	Vector vertex_setup((*vertex_setup_)(t).get(Vector()));

	Matrix transform;
	vector<ValueBase> bone_weight_list((*bone_weight_list_)(t).get_list());
	Real total_weight = 0;
	for (vector<ValueBase>::iterator iter = bone_weight_list.begin(); iter != bone_weight_list.end(); iter++)
	{
		Bone bone(iter->get(BoneWeightPair()).get_bone());
		Real weight(iter->get(BoneWeightPair()).get_weight());
		transform += (bone.get_setup_matrix() *
					  bone.get_animated_matrix() *
					  weight);
		total_weight += weight;
	}

	if (total_weight) transform *= (1/total_weight);

	return vertex_free + transform.get_transformed(vertex_setup);
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
	case 0: CHECK_TYPE_AND_SET_VALUE(vertex_free_,		ValueBase::TYPE_VECTOR);
	case 1: CHECK_TYPE_AND_SET_VALUE(vertex_setup_,		ValueBase::TYPE_VECTOR);
	case 2: CHECK_TYPE_AND_SET_VALUE(bone_weight_list_,	ValueBase::TYPE_LIST);
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_BoneInfluence::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return vertex_free_;
	case 1: return vertex_setup_;
	case 2: return bone_weight_list_;
	}

	return 0;
}

int
ValueNode_BoneInfluence::link_count()const
{
	return 3;
}

String
ValueNode_BoneInfluence::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("vertex_free");
	case 1: return _("vertex_setup");
	case 2: return _("bone_weight_list");
	}

	return String();
}

String
ValueNode_BoneInfluence::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Vertex Free");
	case 1: return _("Vertex Setup");
	case 2: return _("Bone Weight List");
	}

	return String();
}

int
ValueNode_BoneInfluence::get_link_index_from_name(const String &name)const
{
	if(name=="vertex_free")			return 0;
	if(name=="vertex_setup")		return 1;
	if(name=="bone_weight_list")	return 2;

	throw Exception::BadLinkName(name);
}

bool
ValueNode_BoneInfluence::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}
