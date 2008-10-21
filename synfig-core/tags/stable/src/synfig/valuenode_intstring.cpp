/* === S Y N F I G ========================================================= */
/*!	\file valuenode_intstring.cpp
**	\brief Implementation of the "IntString" valuenode conversion.
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

#include "valuenode_intstring.h"
#include "valuenode_const.h"
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

ValueNode_IntString::ValueNode_IntString(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_STRING:
		set_link("int",ValueNode_Const::create(int(0)));
		set_link("width",ValueNode_Const::create(int(0)));
		set_link("zero_pad",ValueNode_Const::create(bool(false)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_IntString::create_new()const
{
	return new ValueNode_IntString(get_type());
}

ValueNode_IntString*
ValueNode_IntString::create(const ValueBase &x)
{
	return new ValueNode_IntString(x);
}

ValueNode_IntString::~ValueNode_IntString()
{
	unlink_all();
}

ValueBase
ValueNode_IntString::operator()(Time t)const
{
	int integer((*int_)(t).get(int()));
	int width((*width_)(t).get(int()));
	int zero_pad((*zero_pad_)(t).get(bool()));

	switch (get_type())
	{
	case ValueBase::TYPE_STRING:
		return strprintf(strprintf("%%%s%dd",
								   zero_pad ? "0" : "",
								   width).c_str(), integer);
	default:
		break;
	}

	assert(0);
	return ValueBase();
}

String
ValueNode_IntString::get_name()const
{
	return "intstring";
}

String
ValueNode_IntString::get_local_name()const
{
	return _("Int String");
}

bool
ValueNode_IntString::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(int_, ValueBase::TYPE_INTEGER);
	case 1: CHECK_TYPE_AND_SET_VALUE(width_, ValueBase::TYPE_INTEGER);
	case 2: CHECK_TYPE_AND_SET_VALUE(zero_pad_, ValueBase::TYPE_BOOL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_IntString::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return int_;
	case 1: return width_;
	case 2: return zero_pad_;
	}

	return 0;
}

int
ValueNode_IntString::link_count()const
{
	return 3;
}

String
ValueNode_IntString::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "int";
		case 1: return "width";
		case 2: return "zero_pad";
	}
	return String();
}

String
ValueNode_IntString::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("Int");
		case 1: return _("Width");
		case 2: return _("Zero Padded");
	}
	return String();
}

int
ValueNode_IntString::get_link_index_from_name(const String &name)const
{
	if (name=="int") return 0;
	if (name=="width") return 1;
	if (name=="zero_pad") return 2;

	throw Exception::BadLinkName(name);
}

bool
ValueNode_IntString::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_STRING;
}
