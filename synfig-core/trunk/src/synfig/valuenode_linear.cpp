/* === S Y N F I G ========================================================= */
/*!	\file valuenode_linear.cpp
**	\brief Template File
**
**	$Id: valuenode_linear.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Linear::ValueNode_Linear(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	switch(x)
	{
	case ValueBase::TYPE_REAL:
		set_link("slope",ValueNode_Const::create(Real(1)));
		set_link("offset",ValueNode_Const::create(Real(0)));
		break;
	case ValueBase::TYPE_TIME:
		set_link("slope",ValueNode_Const::create(Time(1)));
		set_link("offset",ValueNode_Const::create(Time(0)));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("slope",ValueNode_Const::create(Vector(1.0,1.0)));
		set_link("offset",ValueNode_Const::create(Vector(0.0,0.0)));
		break;
	case ValueBase::TYPE_ANGLE:
		set_link("slope",ValueNode_Const::create(Angle::deg(90)));
		set_link("offset",ValueNode_Const::create(Angle::deg(0)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_name(x));
	}

	DCAST_HACK_ENABLE();
}

ValueNode_Linear*
ValueNode_Linear::create(const ValueBase &x)
{
	return new ValueNode_Linear(x.get_type());
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
	case ValueBase::TYPE_TIME:
		return (*m_)(t).get(Time())*t+(*b_)(t).get(Time());
	case ValueBase::TYPE_REAL:
		return (*m_)(t).get(Real())*t+(*b_)(t).get(Real());
	case ValueBase::TYPE_VECTOR:
		return (*m_)(t).get(Vector())*t+(*b_)(t).get(Vector());
	case ValueBase::TYPE_ANGLE:
		return (*m_)(t).get(Angle())*t+(*b_)(t).get(Angle());
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
	return type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_VECTOR
		|| type==ValueBase::TYPE_TIME
		|| type==ValueBase::TYPE_ANGLE;
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
	if(i==0)
		return m_;
	if(i==1)
		return b_;

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
	if(i==0)
		return "slope";
	if(i==1)
		return "offset";
	return String();
}

String
ValueNode_Linear::link_local_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		switch(get_type())
		{
		case ValueBase::TYPE_REAL:
		case ValueBase::TYPE_TIME:
		case ValueBase::TYPE_ANGLE:
			return _("Rate");
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
	if(name=="slope")
		return 0;
	if(name=="offset")
		return 1;
	
	throw Exception::BadLinkName(name);
}

LinkableValueNode*
ValueNode_Linear::create_new()const
{
	return new ValueNode_Linear(get_type());
}
