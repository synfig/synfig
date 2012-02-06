/* === S Y N F I G ========================================================= */
/*!	\file valuenode_subtract.cpp
**	\brief Implementation of the "Subtract" valuenode conversion.
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

#include "general.h"
#include "valuenode_subtract.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"
#include "gradient.h"
#include "vector.h"
#include "angle.h"
#include "real.h"
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

synfig::ValueNode_Subtract::ValueNode_Subtract(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("scalar",ValueNode_Const::create(Real(1.0)));
	ValueBase::Type id(value.get_type());

	switch(id)
	{
	case ValueBase::TYPE_ANGLE:
		set_link("lhs",ValueNode_Const::create(value.get(Angle())));
		set_link("rhs",ValueNode_Const::create(Angle::deg(0)));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("lhs",ValueNode_Const::create(value.get(Color())));
		set_link("rhs",ValueNode_Const::create(Color(0,0,0,0)));
		break;
	case ValueBase::TYPE_GRADIENT:
		set_link("lhs",ValueNode_Const::create(value.get(Gradient())));
		set_link("rhs",ValueNode_Const::create(Gradient()));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("lhs",ValueNode_Const::create(value.get(int())));
		set_link("rhs",ValueNode_Const::create(int(0)));
		break;
	case ValueBase::TYPE_REAL:
		set_link("lhs",ValueNode_Const::create(value.get(Real())));
		set_link("rhs",ValueNode_Const::create(Real(0)));
		break;
	case ValueBase::TYPE_TIME:
		set_link("lhs",ValueNode_Const::create(value.get(Time())));
		set_link("rhs",ValueNode_Const::create(Time(0)));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("lhs",ValueNode_Const::create(value.get(Vector())));
		set_link("rhs",ValueNode_Const::create(Vector(0,0)));
		break;
	default:
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(id));
	}

	assert(ref_a->get_type()==id);
	assert(ref_b->get_type()==id);
	assert(get_type()==id);
}

LinkableValueNode*
ValueNode_Subtract::create_new()const
{
	return new ValueNode_Subtract(get_type());
}

ValueNode_Subtract*
ValueNode_Subtract::create(const ValueBase& value)
{
	return new ValueNode_Subtract(value);
}

synfig::ValueNode_Subtract::~ValueNode_Subtract()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Subtract::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!ref_a || !ref_b)
		throw runtime_error(strprintf("ValueNode_Subtract: %s",_("One or both of my parameters aren't set!")));
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return ((*ref_a)(t).get(Angle())-(*ref_b)(t).get(Angle()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_COLOR:
		return ((*ref_a)(t).get(Color())-(*ref_b)(t).get(Color()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_GRADIENT:
		return ((*ref_a)(t).get(Gradient())-(*ref_b)(t).get(Gradient()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_INTEGER:
		return round_to_int(((*ref_a)(t).get(int())-(*ref_b)(t).get(int()))*(*scalar)(t).get(Real()));
	case ValueBase::TYPE_REAL:
		return ((*ref_a)(t).get(Vector::value_type())-(*ref_b)(t).get(Vector::value_type()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_TIME:
		return ((*ref_a)(t).get(Time())-(*ref_b)(t).get(Time()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_VECTOR:
		return ((*ref_a)(t).get(Vector())-(*ref_b)(t).get(Vector()))*(*scalar)(t).get(Real());
	default:
		assert(0);
		break;
	}
	return ValueBase();
}

bool
ValueNode_Subtract::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_a,  get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_b,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(scalar, ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Subtract::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return ref_a;
		case 1: return ref_b;
		case 2: return scalar;
		default: return 0;
	}
}

String
ValueNode_Subtract::get_name()const
{
	return "subtract";
}

String
ValueNode_Subtract::get_local_name()const
{
	return _("Subtract");
}

bool
ValueNode_Subtract::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_ANGLE
		|| type==ValueBase::TYPE_COLOR
		|| type==ValueBase::TYPE_GRADIENT
		|| type==ValueBase::TYPE_INTEGER
		|| type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_TIME
		|| type==ValueBase::TYPE_VECTOR;
}

LinkableValueNode::Vocab
ValueNode_Subtract::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"lhs")
		.set_local_name(_("LHS"))
		.set_description(_("Left Hand Side of the subtraction"))
	);

	ret.push_back(ParamDesc(ValueBase(),"rhs")
		.set_local_name(_("RHS"))
		.set_description(_("Right Hand Side of the subtraction"))
	);

		ret.push_back(ParamDesc(ValueBase(),"scalar")
		.set_local_name(_("Scalar"))
		.set_description(_("Value that multiplies the subtraction"))
	);

	return ret;
}
