/* === S Y N F I G ========================================================= */
/*!	\file valuenode_step.cpp
**	\brief Implementation of the "Step" valuenode conversion.
**
**	$Id$
**
**	\legal
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

#include "valuenode_step.h"
#include "valuenode_const.h"
#include "general.h"
#include "color.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Step::ValueNode_Step(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	set_link("width",ValueNode_Const::create(Time(1)));
	set_link("offset",ValueNode_Const::create(Time(0)));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("link",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("link",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("link",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("link",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("link",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("link",ValueNode_Const::create(value.get(Vector())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_Step::create_new()const
{
	return new ValueNode_Step(get_type());
}

ValueNode_Step*
ValueNode_Step::create(const ValueBase &x)
{
	return new ValueNode_Step(x);
}

ValueNode_Step::~ValueNode_Step()
{
	unlink_all();
}

ValueBase
ValueNode_Step::operator()(Time t)const
{
	Time width ((*width_ )(t).get(Time()));
	Time offset((*offset_)(t).get(Time()));

	t = floor((t - offset) / width) * width + offset;

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:   return (*link_)(t).get( Angle());
	case ValueBase::TYPE_COLOR:   return (*link_)(t).get( Color());
	case ValueBase::TYPE_INTEGER: return (*link_)(t).get(   int());
	case ValueBase::TYPE_REAL:    return (*link_)(t).get(  Real());
	case ValueBase::TYPE_TIME:    return (*link_)(t).get(  Time());
	case ValueBase::TYPE_VECTOR:  return (*link_)(t).get(Vector());
	default:
		assert(0);
		return ValueBase();
	}
}


String
ValueNode_Step::get_name()const
{
	return "step";
}

String
ValueNode_Step::get_local_name()const
{
	return _("Step");
}

bool
ValueNode_Step::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_ANGLE		||
		type==ValueBase::TYPE_COLOR		||
		type==ValueBase::TYPE_INTEGER	||
		type==ValueBase::TYPE_REAL		||
		type==ValueBase::TYPE_TIME		||
		type==ValueBase::TYPE_VECTOR	;
}

bool
ValueNode_Step::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,   get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(width_,  ValueBase::TYPE_TIME);
	case 2: CHECK_TYPE_AND_SET_VALUE(offset_, ValueBase::TYPE_TIME);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Step::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_;
	case 1: return width_;
	case 2: return offset_;
	default:
		return 0;
	}
}

int
ValueNode_Step::link_count()const
{
	return 3;
}

String
ValueNode_Step::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "link";
	case 1: return "width";
	case 2: return "offset";
	default:
		return String();
	}
}

String
ValueNode_Step::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Link");
	case 1: return _("Width");
	case 2: return _("Offset");
	default:
		return String();
	}
}

int
ValueNode_Step::get_link_index_from_name(const String &name)const
{
	if(name=="link")  return 0;
	if(name=="width")  return 1;
	if(name=="offset")  return 2;

	throw Exception::BadLinkName(name);
}
