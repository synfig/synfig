/* === S Y N F I G ========================================================= */
/*!	\file valuenode_linear.cpp
**	\brief Implementation of the "Linear" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

ValueNode_Linear::ValueNode_Linear(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
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
		throw Exception::BadType(ValueBase::type_local_name(get_type()));
	}
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
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return (*m_)(t).get( Angle())*t+(*b_)(t).get( Angle());
	case ValueBase::TYPE_COLOR:
		return (*m_)(t).get( Color())*t+(*b_)(t).get( Color());
	case ValueBase::TYPE_INTEGER:
		return round_to_int((*m_)(t).get(int())*t+(*b_)(t).get(int()));
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
ValueNode_Linear::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(m_, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(b_, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Linear::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return m_;
	if(i==1) return b_;
	return 0;
}

LinkableValueNode::Vocab
ValueNode_Linear::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
	case ValueBase::TYPE_COLOR:
	case ValueBase::TYPE_INTEGER:
	case ValueBase::TYPE_REAL:
	case ValueBase::TYPE_TIME:
		ret.push_back(ParamDesc(ValueBase(),"slope")
			.set_local_name(_("Rate"))
			.set_description(_("Value that is multiplied by the current time (in seconds)"))
		);
	break;
	case ValueBase::TYPE_VECTOR:
	default:
		ret.push_back(ParamDesc(ValueBase(),"slope")
			.set_local_name(_("Slope"))
			.set_description(_("Value that is multiplied by the current time (in seconds)"))
		);
	}

	ret.push_back(ParamDesc(ValueBase(),"offset")
		.set_local_name(_("Offset"))
		.set_description(_("Returned value when the current time is zero"))
	);

	return ret;
}
