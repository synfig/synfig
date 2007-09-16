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

synfig::ValueNode_Subtract::ValueNode_Subtract():LinkableValueNode(synfig::ValueBase::TYPE_NIL)
{
	set_scalar(1.0);
}

LinkableValueNode*
ValueNode_Subtract::create_new()const
{
	return new ValueNode_Subtract();
}

ValueNode_Subtract*
ValueNode_Subtract::create(const ValueBase& x)
{
	ValueBase::Type id(x.get_type());

	ValueNode_Subtract* value_node=new ValueNode_Subtract();
	switch(id)
	{
	case ValueBase::TYPE_NIL:
		return value_node;
	case ValueBase::TYPE_COLOR:
		value_node->set_link("lhs",ValueNode_Const::create(x.get(Color())));
		value_node->set_link("rhs",ValueNode_Const::create(Color(0,0,0,0)));
		break;
	case ValueBase::TYPE_ANGLE:
	case ValueBase::TYPE_INTEGER:
	case ValueBase::TYPE_REAL:
	case ValueBase::TYPE_VECTOR:
		value_node->set_link("lhs",ValueNode_Const::create(ValueBase(id)));
		value_node->set_link("rhs",ValueNode_Const::create(ValueBase(id)));
		break;
	default:
		assert(0);
		throw runtime_error("synfig::ValueNode_Subtract:Bad type "+ValueBase::type_name(id));
	}

	assert(value_node->get_lhs()->get_type()==id);
	assert(value_node->get_rhs()->get_type()==id);

	assert(value_node->get_type()==id);

	return value_node;
}

synfig::ValueNode_Subtract::~ValueNode_Subtract()
{
	unlink_all();
}

void
ValueNode_Subtract::set_scalar(Real x)
{
	set_link("scalar",ValueNode_Const::create(x));
}

bool
synfig::ValueNode_Subtract::set_scalar(ValueNode::Handle x)
{
	if(x->get_type()!=ValueBase::TYPE_REAL&& !PlaceholderValueNode::Handle::cast_dynamic(x))
		return false;
	scalar=x;
	return true;
}

bool
synfig::ValueNode_Subtract::set_lhs(ValueNode::Handle a)
{
	ref_a=a;

	if(PlaceholderValueNode::Handle::cast_dynamic(a))
		return true;

	if(!ref_a || !ref_b)
		set_type(ValueBase::TYPE_NIL);
	else
	if(ref_a->get_type()==ValueBase::TYPE_ANGLE && ref_a->get_type()==ValueBase::TYPE_ANGLE)
		set_type(ValueBase::TYPE_ANGLE);
	else
	if(ref_a->get_type()==ValueBase::TYPE_COLOR && ref_a->get_type()==ValueBase::TYPE_COLOR)
		set_type(ValueBase::TYPE_COLOR);
	else
	if(ref_a->get_type()==ValueBase::TYPE_INTEGER && ref_a->get_type()==ValueBase::TYPE_INTEGER)
		set_type(ValueBase::TYPE_INTEGER);
	else
	if(ref_a->get_type()==ValueBase::TYPE_REAL && ref_a->get_type()==ValueBase::TYPE_REAL)
		set_type(ValueBase::TYPE_REAL);
	else
	if(ref_a->get_type()==ValueBase::TYPE_VECTOR && ref_a->get_type()==ValueBase::TYPE_VECTOR)
		set_type(ValueBase::TYPE_VECTOR);
	else
	{
		synfig::warning(get_id()+":(set_a):"+strprintf(_("Types seem to be off for ValueNodes %s and %s"),ref_a->get_id().c_str(),ref_b->get_id().c_str()));
		set_type(ValueBase::TYPE_NIL);
	}

	return true;
}

bool
synfig::ValueNode_Subtract::set_rhs(ValueNode::Handle b)
{
	ref_b=b;

	if(PlaceholderValueNode::Handle::cast_dynamic(b))
		return true;

	if(!ref_a || !ref_b)
		set_type(ValueBase::TYPE_NIL);
	else
	if(ref_a->get_type()==ValueBase::TYPE_ANGLE && ref_a->get_type()==ValueBase::TYPE_ANGLE)
		set_type(ValueBase::TYPE_ANGLE);
	else
	if(ref_a->get_type()==ValueBase::TYPE_COLOR && ref_a->get_type()==ValueBase::TYPE_COLOR)
		set_type(ValueBase::TYPE_COLOR);
	else
	if(ref_a->get_type()==ValueBase::TYPE_INTEGER && ref_a->get_type()==ValueBase::TYPE_INTEGER)
		set_type(ValueBase::TYPE_INTEGER);
	else
	if(ref_a->get_type()==ValueBase::TYPE_REAL && ref_a->get_type()==ValueBase::TYPE_REAL)
		set_type(ValueBase::TYPE_REAL);
	else
	if(ref_a->get_type()==ValueBase::TYPE_VECTOR && ref_a->get_type()==ValueBase::TYPE_VECTOR)
		set_type(ValueBase::TYPE_VECTOR);
	else
	{
		synfig::warning(get_id()+":(set_b):"+strprintf(_("Types seem to be off for ValueNodes %s and %s"),ref_a->get_id().c_str(),ref_b->get_id().c_str()));
		set_type(ValueBase::TYPE_NIL);
	}

	return true;
}

synfig::ValueBase
synfig::ValueNode_Subtract::operator()(Time t)const
{
	if(!ref_a || !ref_b)
		throw runtime_error(strprintf("ValueNode_Subtract: %s",_("One or both of my parameters aren't set!")));
	if(get_type()==ValueBase::TYPE_ANGLE)
		return ((*ref_a)(t).get(Angle())-(*ref_b)(t).get(Angle()))*(*scalar)(t).get(Real());
	if(get_type()==ValueBase::TYPE_COLOR)
		return ((*ref_a)(t).get(Color())-(*ref_b)(t).get(Color()))*(*scalar)(t).get(Real());
	if(get_type()==ValueBase::TYPE_INTEGER)
		return ((*ref_a)(t).get(int())-(*ref_b)(t).get(int()))*(*scalar)(t).get(Real());
	if(get_type()==ValueBase::TYPE_REAL)
		return ((*ref_a)(t).get(Vector::value_type())-(*ref_b)(t).get(Vector::value_type()))*(*scalar)(t).get(Real());
	if(get_type()==ValueBase::TYPE_VECTOR)
		return ((*ref_a)(t).get(Vector())-(*ref_b)(t).get(Vector()))*(*scalar)(t).get(Real());

	synfig::error(get_id()+':'+strprintf(_("Cannot subtract types of %s and %s"),ValueBase::type_name(ref_a->get_type()).c_str(),ValueBase::type_name(ref_b->get_type()).c_str()));
	return ValueBase();
}

bool
ValueNode_Subtract::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			if(set_lhs(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 1:
			if(set_rhs(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 2:
			scalar=x;
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
		case 0:
			return ref_a;
		case 1:
			return ref_b;
		case 2:
			return scalar;
	}
	return 0;
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
		case 0:
			return _("LHS");
		case 1:
			return _("RHS");
		case 2:
			return _("Scalar");
		default:
			return String();
	}
}

String
ValueNode_Subtract::link_name(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			return "lhs";
		case 1:
			return "rhs";
		case 2:
			return "scalar";
		default:
			return String();
	}
}

int
ValueNode_Subtract::get_link_index_from_name(const String &name)const
{
	if(name=="lhs")
		return 0;
	if(name=="rhs")
		return 1;
	if(name=="scalar")
		return 2;
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
		|| type==ValueBase::TYPE_VECTOR;
}
