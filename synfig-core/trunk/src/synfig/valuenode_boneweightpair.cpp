/* === S Y N F I G ========================================================= */
/*!	\file valuenode_boneweightpair.cpp
**	\brief Implementation of the "BoneWeightPair" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008 Carlos López
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

#include "valuenode_boneweightpair.h"
#include "valuenode_const.h"
#include "general.h"
#include "matrix.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneWeightPair::ValueNode_BoneWeightPair(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_MATRIX:
		set_link("bone",ValueNode_Const::create(Bone()));
		set_link("weight",ValueNode_Const::create(Real(1.0)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_BoneWeightPair::create_new()const
{
	return new ValueNode_BoneWeightPair(get_type());
}

ValueNode_BoneWeightPair*
ValueNode_BoneWeightPair::create(const ValueBase &x)
{
	return new ValueNode_BoneWeightPair(x);
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
	Bone b(((*bone_)(t).get(Bone())));
	return
		Matrix(
				b.get_setup_matrix()
				*
				b.get_animated_matrix()
				*
				(*weight_)(t).get(Real())
			   )
	;
}


String
ValueNode_BoneWeightPair::get_name()const
{
	return "boneweightpair";
}

String
ValueNode_BoneWeightPair::get_local_name()const
{
	return _("BoneWeightPair");
}

bool
ValueNode_BoneWeightPair::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_MATRIX;
}

bool
ValueNode_BoneWeightPair::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bone_, ValueBase::TYPE_BONE);
	case 1: CHECK_TYPE_AND_SET_VALUE(weight_,   ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneWeightPair::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return bone_;
	if(i==1)
		return weight_;

	return 0;
}

int
ValueNode_BoneWeightPair::link_count()const
{
	return 2;
}

String
ValueNode_BoneWeightPair::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return "bone";
	if(i==1)
		return "weight";
	return String();
}

String
ValueNode_BoneWeightPair::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return _("Bone");
	if(i==1)
		return _("Weight");
	return String();
}

int
ValueNode_BoneWeightPair::get_link_index_from_name(const String &name)const
{
	if(name=="bone")
		return 0;
	if(name=="weight")
		return 1;

	throw Exception::BadLinkName(name);
}
