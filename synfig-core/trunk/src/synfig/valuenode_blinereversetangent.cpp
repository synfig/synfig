/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinereversetangent.cpp
**	\brief Implementation of the "Reverse Tangent" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "valuenode_blinereversetangent.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include <ETL/hermite>
#include <ETL/calculus>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineRevTangent::ValueNode_BLineRevTangent(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_BLineRevTangent::ValueNode_BLineRevTangent(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	if(x->get_type()!=ValueBase::TYPE_BLINEPOINT)
		throw Exception::BadType(ValueBase::type_local_name(x->get_type()));

	set_link("reference",x);
	set_link("reverse",ValueNode_Const::create(bool(false)));
}

ValueNode_BLineRevTangent*
ValueNode_BLineRevTangent::create(const ValueBase &x)
{
	return new ValueNode_BLineRevTangent(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_BLineRevTangent::create_new()const
{
	return new ValueNode_BLineRevTangent(get_type());
}

ValueNode_BLineRevTangent::~ValueNode_BLineRevTangent()
{
	unlink_all();
}

ValueBase
ValueNode_BLineRevTangent::operator()(Time t)const
{
	if ((*reverse_)(t).get(bool()))
	{
		BLinePoint reference((*reference_)(t));
		BLinePoint ret(reference);
		if(ret.get_split_tangent_flag())
		{
			ret.set_tangent1(-reference.get_tangent2());
			ret.set_tangent2(-reference.get_tangent1());
		}
		else
		{
			// \todo what should we do here really?
			// it seems that there's some pre-existing bug
			// with the 'reference' convert, too - referencing
			// a non-split blinepoint causes some problems
			ret.set_tangent1(-reference.get_tangent1());
		}
		return ret;
	}
	else
		return (*reference_)(t);
}

String
ValueNode_BLineRevTangent::get_name()const
{
	return "blinerevtangent";
}

String
ValueNode_BLineRevTangent::get_local_name()const
{
	return _("Reverse Tangent");
}

bool
ValueNode_BLineRevTangent::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(reference_, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(reverse_,   ValueBase::TYPE_BOOL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineRevTangent::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return reference_;
		case 1: return reverse_;
	}

	return 0;
}

int
ValueNode_BLineRevTangent::link_count()const
{
	return 2;
}

String
ValueNode_BLineRevTangent::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "reference";
		case 1: return "reverse";
	}
	return String();
}

String
ValueNode_BLineRevTangent::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("Reference");
		case 1: return _("Reverse");
	}
	return String();
}

int
ValueNode_BLineRevTangent::get_link_index_from_name(const String &name)const
{
	if(name=="reference")	return 0;
	if(name=="reverse")		return 1;
	throw Exception::BadLinkName(name);
}

bool
ValueNode_BLineRevTangent::check_type(ValueBase::Type type)
{
	return (type==ValueBase::TYPE_BLINEPOINT);
}
