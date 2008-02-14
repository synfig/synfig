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
	set_link("time",ValueNode_Const::create(Time(0)));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("height",ValueNode_Const::create(Angle::deg(0)));
		set_link("value",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("height",ValueNode_Const::create(Color(0,0,0,0)));
		set_link("value",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("height",ValueNode_Const::create(int(0)));
		set_link("value",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("height",ValueNode_Const::create(Real(0)));
		set_link("value",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("height",ValueNode_Const::create(Time(0)));
		set_link("value",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("height",ValueNode_Const::create(Vector(0,0)));
		set_link("value",ValueNode_Const::create(value.get(Vector())));
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
	int step = floor((t - ((*time_)(t).get(Time()))) /
					 ((*width_)(t).get(Time())));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:   return (*value_)(t).get( Angle()) + (*height_)(t).get( Angle())*step;
	case ValueBase::TYPE_COLOR:   return (*value_)(t).get( Color()) + (*height_)(t).get( Color())*step;
	case ValueBase::TYPE_INTEGER: return (*value_)(t).get(   int()) + (*height_)(t).get(   int())*step;
	case ValueBase::TYPE_REAL:    return (*value_)(t).get(  Real()) + (*height_)(t).get(  Real())*step;
	case ValueBase::TYPE_TIME:    return (*value_)(t).get(  Time()) + (*height_)(t).get(  Time())*step;
	case ValueBase::TYPE_VECTOR:  return (*value_)(t).get(Vector()) + (*height_)(t).get(Vector())*step;
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
	case 0: CHECK_TYPE_AND_SET_VALUE(width_,  ValueBase::TYPE_TIME);
	case 1: CHECK_TYPE_AND_SET_VALUE(height_, get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(time_,   ValueBase::TYPE_TIME);
	case 3: CHECK_TYPE_AND_SET_VALUE(value_,  get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Step::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return width_;
	case 1: return height_;
	case 2: return time_;
	case 3: return value_;
	default:
		return 0;
	}
}

int
ValueNode_Step::link_count()const
{
	return 4;
}

String
ValueNode_Step::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "width";
	case 1: return "height";
	case 2: return "time";
	case 3: return "value";
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
	case 0: return _("Width");
	case 1: return _("Height");
	case 2: return _("Time");
	case 3: return _("Value");
	default:
		return String();
	}
}

int
ValueNode_Step::get_link_index_from_name(const String &name)const
{
	if(name=="width")  return 0;
	if(name=="height") return 1;
	if(name=="time")   return 2;
	if(name=="value")  return 3;

	throw Exception::BadLinkName(name);
}
