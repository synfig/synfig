/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bone.cpp
**	\brief Implementation of the "Bone" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_bone.h"
#include "valuenode_const.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static map<GUID, ValueNode_Bone::Handle> bone_map;
static int bone_counter;

/* === P R O C E D U R E S ================================================= */

static void
show_bone_map()
{
	printf("we now have %d bones:\n", int(bone_map.size()));
	map<GUID, ValueNode_Bone::Handle>::iterator iter;
	for (iter = bone_map.begin(); iter != bone_map.end(); iter++)
	{
		GUID guid(iter->first);
		ValueNode_Bone::Handle bone(iter->second);
		printf("%s : %s (%d)\n",
			   guid.get_string().substr(0,GUID_PREFIX_LEN).c_str(),
			   (*bone)(0).get(Bone()).get_string().c_str(),
			   bone->rcount());
	}
	printf("\n");
}

/* === M E T H O D S ======================================================= */

ValueNode_Bone::ValueNode_Bone(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_BONE:
	{
		Bone bone(value.get(Bone()));
		set_link("name",ValueNode_Const::create(bone.get_name().empty() ? strprintf(_("Bone %d"), ++bone_counter) : bone.get_name()));
		set_link("origin",ValueNode_Const::create(bone.get_origin()));
		set_link("origin0",ValueNode_Const::create(bone.get_origin0()));
		set_link("angle",ValueNode_Const::create(bone.get_angle()));
		set_link("angle0",ValueNode_Const::create(bone.get_angle0()));
		set_link("scale",ValueNode_Const::create(bone.get_scale()));
		set_link("length",ValueNode_Const::create(bone.get_length()));
		set_link("strength",ValueNode_Const::create(bone.get_strength()));
		set_link("parent",ValueNode_Const::create(bone.get_parent()));

		bone_map[get_guid()] = this;
		show_bone_map();

		break;
	}
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_Bone::create_new()const
{
	return new ValueNode_Bone(get_type());
}

ValueNode_Bone*
ValueNode_Bone::create(const ValueBase &x)
{
	return new ValueNode_Bone(x);
}

ValueNode_Bone::~ValueNode_Bone()
{
	unlink_all();
}

ValueBase
ValueNode_Bone::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Bone ret;
	ret.set_name		((*name_	)(t).get(String()));
	ret.set_origin		((*origin_	)(t).get(Point()));
	ret.set_origin0		((*origin0_	)(t).get(Point()));
	ret.set_angle		((*angle_	)(t).get(Angle()));
	ret.set_angle0		((*angle0_	)(t).get(Angle()));
	ret.set_scale		((*scale_	)(t).get(Real()));
	ret.set_length		((*length_	)(t).get(Real()));
	ret.set_strength	((*strength_)(t).get(Real()));
	ret.set_parent		((*parent_	)(t).get(GUID()));

	return ret;
}

String
ValueNode_Bone::get_name()const
{
	return "bone";
}

String
ValueNode_Bone::get_local_name()const
{
	return _("Bone");
}

bool
ValueNode_Bone::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_BONE;
}

bool
ValueNode_Bone::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(name_,		ValueBase::TYPE_STRING);
	case 1: CHECK_TYPE_AND_SET_VALUE(origin_,	ValueBase::TYPE_VECTOR);
	case 2: CHECK_TYPE_AND_SET_VALUE(origin0_,	ValueBase::TYPE_VECTOR);
	case 3: CHECK_TYPE_AND_SET_VALUE(angle_,	ValueBase::TYPE_ANGLE);
	case 4: CHECK_TYPE_AND_SET_VALUE(angle0_,	ValueBase::TYPE_ANGLE);
	case 5: CHECK_TYPE_AND_SET_VALUE(scale_,	ValueBase::TYPE_REAL);
	case 6: CHECK_TYPE_AND_SET_VALUE(length_,	ValueBase::TYPE_REAL);
	case 7: CHECK_TYPE_AND_SET_VALUE(strength_,	ValueBase::TYPE_REAL);
	case 8: CHECK_TYPE_AND_SET_VALUE(parent_,	ValueBase::TYPE_GUID);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Bone::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return name_;
	case 1: return origin_;
	case 2: return origin0_;
	case 3: return angle_;
	case 4: return angle0_;
	case 5: return scale_;
	case 6: return length_;
	case 7: return strength_;
	case 8: return parent_;
	}

	return 0;
}

int
ValueNode_Bone::link_count()const
{
	return 9;
}

String
ValueNode_Bone::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "name";
	case 1: return "origin";
	case 2: return "origin0";
	case 3: return "angle";
	case 4: return "angle0";
	case 5: return "scale";
	case 6: return "length";
	case 7: return "strength";
	case 8: return "parent";
	}

	return String();
}

String
ValueNode_Bone::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Name");
	case 1: return _("Origin");
	case 2: return _("Origin0");
	case 3: return _("Angle");
	case 4: return _("Angle0");
	case 5: return _("Scale");
	case 6: return _("Length");
	case 7: return _("Strength");
	case 8: return _("Parent");
	}

	return String();
}

int
ValueNode_Bone::get_link_index_from_name(const String &name)const
{
	if (name == "name") return 0;
	if (name == "origin") return 1;
	if (name == "origin0") return 2;
	if (name == "angle") return 3;
	if (name == "angle0") return 4;
	if (name == "scale") return 5;
	if (name == "length") return 6;
	if (name == "strength") return 7;
	if (name == "parent") return 8;

	throw Exception::BadLinkName(name);
}

ValueNode_Bone::BoneMap::const_iterator
ValueNode_Bone::map_begin()
{
	return bone_map.begin();
}

ValueNode_Bone::BoneMap::const_iterator
ValueNode_Bone::map_end()
{
	return bone_map.end();
}

ValueNode_Bone::Handle
ValueNode_Bone::find(GUID guid)
{
	return bone_map[guid];
}

#ifdef _DEBUG
void
ValueNode_Bone::rref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s   rref %d -> ", __FILE__, __LINE__, get_guid().get_string().substr(0,GUID_PREFIX_LEN).c_str(), rcount());

	LinkableValueNode::rref();

	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%d\n", rcount());
}

void
ValueNode_Bone::runref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s runref %d -> ", __FILE__, __LINE__, get_guid().get_string().substr(0,GUID_PREFIX_LEN).c_str(), rcount());

	LinkableValueNode::runref();

	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%d\n", rcount());
}
#endif
