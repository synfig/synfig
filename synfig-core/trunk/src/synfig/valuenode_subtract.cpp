/* === S Y N F I G ========================================================= */
/*!	\file valuenode_subtract.cpp
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

#include "general.h"
#include "valuenode_subtract.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"
#include "vector.h"
#include "angle.h"
#include "real.h"

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
	set_scalar(1.0);
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
		throw runtime_error("synfig::ValueNode_Subtract:Bad type "+ValueBase::type_name(id));
	}

	assert(get_lhs()->get_type()==id);
	assert(get_rhs()->get_type()==id);
	assert(get_type()==id);

	DCAST_HACK_ENABLE();
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

void
ValueNode_Subtract::set_scalar(Real value)
{
	set_link("scalar",ValueNode_Const::create(value));
}

bool
synfig::ValueNode_Subtract::set_scalar(ValueNode::Handle value)
{
	if(value->get_type()!=ValueBase::TYPE_REAL&& !PlaceholderValueNode::Handle::cast_dynamic(value))
		return false;
	scalar=value;
	return true;
}

bool
synfig::ValueNode_Subtract::set_lhs(ValueNode::Handle x)
{
	assert(get_type());

	if(!x ||
	   (get_type()==ValueBase::TYPE_NIL && !check_type(x->get_type())) ||
	   (get_type()!=ValueBase::TYPE_NIL && x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x)))
		return false;

	ref_a=x;

	return true;
}

bool
synfig::ValueNode_Subtract::set_rhs(ValueNode::Handle x)
{
	assert(get_type());

	if(!x ||
	   (get_type()==ValueBase::TYPE_NIL && !check_type(x->get_type())) ||
	   (get_type()!=ValueBase::TYPE_NIL && x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x)))
		return false;

	ref_b=x;

	return true;
}

synfig::ValueBase
synfig::ValueNode_Subtract::operator()(Time t)const
{
	if(!ref_a || !ref_b)
		throw runtime_error(strprintf("ValueNode_Subtract: %s",_("One or both of my parameters aren't set!")));
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return ((*ref_a)(t).get(Angle())-(*ref_b)(t).get(Angle()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_COLOR:
		return ((*ref_a)(t).get(Color())-(*ref_b)(t).get(Color()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_INTEGER:
	{
		Real ret = ((*ref_a)(t).get(int())-(*ref_b)(t).get(int()))*(*scalar)(t).get(Real()) + 0.5f;
		if (ret < 0) return static_cast<int>(ret-1);
		return static_cast<int>(ret); 
	}
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
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			if(set_lhs(value)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			return false;
		case 1:
			if(set_rhs(value)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			return false;
		case 2:
			scalar=value;
			signal_child_changed()(i);signal_value_changed()();
			return true;
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_Subtract::get_link_vfunc(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return ref_a;
		case 1: return ref_b;
		case 2: return scalar;
		default: return 0;
	}
}

int
ValueNode_Subtract::link_count()const
{
	return 3;
}

String
ValueNode_Subtract::link_local_name(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return _("LHS");
		case 1: return _("RHS");
		case 2: return _("Scalar");
		default: return String();
	}
}

String
ValueNode_Subtract::link_name(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return "lhs";
		case 1: return "rhs";
		case 2: return "scalar";
		default: return String();
	}
}

int
ValueNode_Subtract::get_link_index_from_name(const String &name)const
{
	printf("%s:%d link_index_from_name\n", __FILE__, __LINE__);
	if(name=="lhs") return 0;
	if(name=="rhs") return 1;
	if(name=="scalar") return 2;
	throw Exception::BadLinkName(name);
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
		|| type==ValueBase::TYPE_INTEGER
		|| type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_TIME
		|| type==ValueBase::TYPE_VECTOR;
}
