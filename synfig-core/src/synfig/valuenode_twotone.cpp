/* === S Y N F I G ========================================================= */
/*!	\file valuenode_twotone.cpp
**	\brief Implementation of the "Two-Tone" valuenode conversion.
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

#include "general.h"
#include "valuenode_twotone.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"
#include "gradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_TwoTone::ValueNode_TwoTone(const ValueBase &value):LinkableValueNode(synfig::ValueBase::TYPE_GRADIENT)
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_GRADIENT:
		set_link("color1",ValueNode_Const::create(value.get(Gradient())(0)));
		set_link("color2",ValueNode_Const::create(value.get(Gradient())(1)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}
}

LinkableValueNode*
ValueNode_TwoTone::create_new()const
{
	return new ValueNode_TwoTone(get_type());
}

ValueNode_TwoTone*
ValueNode_TwoTone::create(const ValueBase& x)
{
	return new ValueNode_TwoTone(x);
}

synfig::ValueNode_TwoTone::~ValueNode_TwoTone()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_TwoTone::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return Gradient((*ref_a)(t).get(Color()),(*ref_b)(t).get(Color()));
}

bool
ValueNode_TwoTone::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_a, ValueBase::TYPE_COLOR);
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_b, ValueBase::TYPE_COLOR);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TwoTone::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return ref_a;
		case 1:
			return ref_b;
	}
	return 0;
}

int
ValueNode_TwoTone::link_count()const
{
	return 2;
}

String
ValueNode_TwoTone::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return _("Color1");
		case 1:
			return _("Color2");
		default:
			return String();
	}
}

String
ValueNode_TwoTone::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return "color1";
		case 1:
			return "color2";
		default:
			return String();
	}
}

int
ValueNode_TwoTone::get_link_index_from_name(const String &name)const
{
	if(name=="color1")
		return 0;
	if(name=="color2")
		return 1;
	throw Exception::BadLinkName(name);
}

String
ValueNode_TwoTone::get_name()const
{
	return "twotone";
}

String
ValueNode_TwoTone::get_local_name()const
{
	return _("Two-Tone");
}

bool
ValueNode_TwoTone::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_GRADIENT;
}
