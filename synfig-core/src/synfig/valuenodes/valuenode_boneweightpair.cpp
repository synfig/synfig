/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneweightpair.cpp
**	\brief Implementation of the "BoneWeightPair" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2008 Carlos López
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

#include "valuenode_boneweightpair.h"
#include "valuenode_bone.h"
#include "valuenode_const.h"
#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/boneweightpair.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BoneWeightPair, RELEASE_VERSION_0_62_00, "boneweightpair", "Bone Weight Pair")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneWeightPair::ValueNode_BoneWeightPair(const ValueBase &value, Canvas::LooseHandle canvas):
	LinkableValueNode(value.get_type())
{
	if (value.get_type() == type_bone_weight_pair)
	{
		BoneWeightPair bone_weight_pair(value.get(BoneWeightPair()));
		ValueBase bone(bone_weight_pair.get_bone());
		ValueNode_Bone::Handle bone_value_node;
		bone_value_node = ValueNode_Bone::create(bone, canvas);
		set_link("bone",ValueNode_Const::create(bone_value_node, canvas));
		set_link("weight",ValueNode_Const::create(Real(bone_weight_pair.get_weight())));

		if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
			printf("%s:%d set parent canvas for bwp to %p\n", __FILE__, __LINE__, canvas.get());
		set_parent_canvas(canvas);

		ValueNode_Bone::show_bone_map(canvas, __FILE__, __LINE__, "after making new boneweightpair");
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_BoneWeightPair::create_new()const
{
	return new ValueNode_BoneWeightPair(get_type());
}

ValueNode_BoneWeightPair*
ValueNode_BoneWeightPair::create(const ValueBase &x, Canvas::LooseHandle canvas)
{
	return new ValueNode_BoneWeightPair(x, canvas);
}

ValueNode_BoneWeightPair::~ValueNode_BoneWeightPair()
{
	unlink_all();
}

ValueBase
ValueNode_BoneWeightPair::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	ValueNode_Bone::Handle bone_node((*bone_)(t).get(ValueNode_Bone::Handle()));
	Bone bone((*bone_node)(t).get(Bone()));
	Real weight((*weight_)(t).get(Real()));
	return BoneWeightPair(bone, weight);
}



bool
ValueNode_BoneWeightPair::check_type(Type &type)
{
	return type==type_bone_weight_pair;
}

bool
ValueNode_BoneWeightPair::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bone_,   type_bone_valuenode);
	case 1: CHECK_TYPE_AND_SET_VALUE(weight_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneWeightPair::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0:	return bone_;
	case 1: return weight_;
	}

	return 0;
}


LinkableValueNode::Vocab
ValueNode_BoneWeightPair::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bone")
		.set_local_name(_("Bone"))
		.set_description(_("Bone used to make influence"))
	);

	ret.push_back(ParamDesc(ValueBase(),"weight")
		.set_local_name(_("Weight"))
		.set_description(_("The relative value of influence of the bone"))
	);

	return ret;
}
