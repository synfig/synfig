/* === S Y N F I G ========================================================= */
/*!	\file valuenode_linear.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "valuenode_linear.h"
#include "valuenode_const.h"
#include "general.h"
#include "color.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Linear::ValueNode_Linear(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("slope",ValueNode_Const::create(Angle::deg(0)));
		set_link("offset",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("slope",ValueNode_Const::create(Color(0,0,0,0)));
		set_link("offset",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("slope",ValueNode_Const::create(int(0)));
		set_link("offset",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("slope",ValueNode_Const::create(Real(0)));
		set_link("offset",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("slope",ValueNode_Const::create(Time(0)));
		set_link("offset",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("slope",ValueNode_Const::create(Vector(0,0)));
		set_link("offset",ValueNode_Const::create(value.get(Vector())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_name(get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_Linear::create_new()const
{
	return new ValueNode_Linear(get_type());
}

ValueNode_Linear*
ValueNode_Linear::create(const ValueBase &x)
{
	return new ValueNode_Linear(x);
}

ValueNode_Linear::~ValueNode_Linear()
{
	unlink_all();
}

ValueBase
ValueNode_Linear::operator()(Time t)const
{
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return (*m_)(t).get( Angle())*t+(*b_)(t).get( Angle());
	case ValueBase::TYPE_COLOR:
		return (*m_)(t).get( Color())*t+(*b_)(t).get( Color());
	case ValueBase::TYPE_INTEGER:
	{
		Real ret = (*m_)(t).get(int())*t+(*b_)(t).get(int()) + 0.5f;
		if (ret < 0) return static_cast<int>(ret-1);
		return static_cast<int>(ret);
	}
	case ValueBase::TYPE_REAL:
		return (*m_)(t).get(  Real())*t+(*b_)(t).get(  Real());
	case ValueBase::TYPE_TIME:
		return (*m_)(t).get(  Time())*t+(*b_)(t).get(  Time());
	case ValueBase::TYPE_VECTOR:
		return (*m_)(t).get(Vector())*t+(*b_)(t).get(Vector());
	default:
		assert(0);
		break;
	}
	return ValueBase();
}


String
ValueNode_Linear::get_name()const
{
	return "linear";
}

String
ValueNode_Linear::get_local_name()const
{
	return _("Linear");
}

bool
ValueNode_Linear::check_type(ValueBase::Type type)
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
ValueNode_Linear::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i==0 || i==1);
	if(i==0)
	{
		m_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	if(i==1)
	{
		b_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Linear::get_link_vfunc(int i)const
{
	assert(i==0 || i==1);
	if(i==0) return m_;
	if(i==1) return b_;
	return 0;
}

int
ValueNode_Linear::link_count()const
{
	return 2;
}

String
ValueNode_Linear::link_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0) return "slope";
	if(i==1) return "offset";
	return String();
}

String
ValueNode_Linear::link_local_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		switch(get_type())
		{
		case ValueBase::TYPE_ANGLE:
		case ValueBase::TYPE_COLOR:
		case ValueBase::TYPE_INTEGER:
		case ValueBase::TYPE_REAL:
		case ValueBase::TYPE_TIME:
			return _("Rate");
		case ValueBase::TYPE_VECTOR:
		default:
			return _("Slope");
		}
	if(i==1)
		return _("Offset");
	return String();
}

int
ValueNode_Linear::get_link_index_from_name(const String &name)const
{
	if(name=="slope")  return 0;
	if(name=="offset") return 1;

	throw Exception::BadLinkName(name);
}
