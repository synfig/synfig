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
#include "valuenode_const.h"
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
	set_link("link",x);
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

bool
ValueNode_BoneInfluence::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneInfluence::get_link_vfunc(int i __attribute__ ((unused)))const
{
	assert(i>=0 && i<link_count());

	return link_;
}

int
ValueNode_BoneInfluence::link_count()const
{
	return 1;
}

String
ValueNode_BoneInfluence::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Link");
	}
	return String();
}

String
ValueNode_BoneInfluence::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "link";
	}
	return String();
}

int
ValueNode_BoneInfluence::get_link_index_from_name(const String &name)const
{
	if(name=="link")
		return 0;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_BoneInfluence::operator()(Time t)const
{
	return (*link_)(t);
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
ValueNode_BoneInfluence::check_type(ValueBase::Type type)
{
	if(type)
		return true;
	return false;
}
