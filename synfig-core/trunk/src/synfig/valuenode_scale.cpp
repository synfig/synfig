/* === S Y N F I G ========================================================= */
/*!	\file valuenode_scale.cpp
**	\brief Implementation of the "Scale" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
#include "valuenode_scale.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"
#include "vector.h"
#include "time.h"
#include "angle.h"
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

ValueNode_Scale::ValueNode_Scale(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	set_scalar(1.0);
	ValueBase::Type id(value.get_type());

	switch(id)
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
		assert(0);
		throw runtime_error("synfig::ValueNode_Scale:Bad type "+ValueBase::type_name(value.get_type()));
	}

	assert(value_node);
	assert(get_value_node()->get_type()==id);
	assert(get_type()==id);
}

LinkableValueNode*
ValueNode_Scale::create_new()const
{
	return new ValueNode_Scale(get_type());
}

ValueNode_Scale*
ValueNode_Scale::create(const ValueBase& value)
{
	return new ValueNode_Scale(value);
}

synfig::ValueNode_Scale::~ValueNode_Scale()
{
	unlink_all();
}

void
ValueNode_Scale::set_scalar(Real x)
{
	set_link("scalar",ValueNode::Handle(ValueNode_Const::create(x)));
}

bool
ValueNode_Scale::set_scalar(const ValueNode::Handle &x)
{
	if(!x
		|| x->get_type()!=ValueBase::TYPE_REAL
		&& !PlaceholderValueNode::Handle::cast_dynamic(x)
	)
		return false;
	scalar=x;
	return true;
}

ValueNode::Handle
ValueNode_Scale::get_scalar()const
{
	return scalar;
}

bool
ValueNode_Scale::set_value_node(const ValueNode::Handle &x)
{
	assert(get_type());

	// if this isn't a proper value
	if(!x ||
	   // or we don't have a type, and this value isn't one of the types we accept
	   (get_type()==ValueBase::TYPE_NIL && !check_type(x->get_type())) ||
	   // or we have a type and this value is a different type and (placeholder?)
	   (get_type()!=ValueBase::TYPE_NIL && x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x)))
		// then fail to set the value
		return false;

	value_node=x;

	return true;
}

ValueNode::Handle
ValueNode_Scale::get_value_node()const
{
	return value_node;
}


synfig::ValueBase
synfig::ValueNode_Scale::operator()(Time t)const
{
	if(!value_node || !scalar)
		throw runtime_error(strprintf("ValueNode_Scale: %s",_("One or both of my parameters aren't set!")));
	else if(get_type()==ValueBase::TYPE_ANGLE)
		return (*value_node)(t).get(Angle())*(*scalar)(t).get(Real());
	else if(get_type()==ValueBase::TYPE_COLOR)
	{
		Color ret((*value_node)(t).get(Color()));
		Real s((*scalar)(t).get(Real()));
		ret.set_r(ret.get_r()*s);
		ret.set_g(ret.get_g()*s);
		ret.set_b(ret.get_b()*s);
		return ret;
	}
	else if(get_type()==ValueBase::TYPE_INTEGER)
		return round_to_int((*value_node)(t).get(int())*(*scalar)(t).get(Real()));
	else if(get_type()==ValueBase::TYPE_REAL)
		return (*value_node)(t).get(Real())*(*scalar)(t).get(Real());
	else if(get_type()==ValueBase::TYPE_TIME)
		return (*value_node)(t).get(Time())*(*scalar)(t).get(Time());
	else if(get_type()==ValueBase::TYPE_VECTOR)
		return (*value_node)(t).get(Vector())*(*scalar)(t).get(Real());

	assert(0);
	return ValueBase();
}


bool
ValueNode_Scale::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());

	if(i==0 && !set_value_node(x))
		return false;
	else
	if(i==1 && !set_scalar(x))
		return false;

	signal_child_changed()(i);signal_value_changed()();

	return true;
}

ValueNode::LooseHandle
ValueNode_Scale::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return value_node;
	else if(i==1)
		return scalar;
	return 0;
}

int
ValueNode_Scale::link_count()const
{
	return 2;
}

String
ValueNode_Scale::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return _("Link");
	else if(i==1)
		return _("Scalar");
	return String();
}

String
ValueNode_Scale::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return "link";
	else if(i==1)
		return "scalar";
	return String();
}

int
ValueNode_Scale::get_link_index_from_name(const String &name)const
{
	if(name=="link")
		return 0;
	if(name=="scalar")
		return 1;

	throw Exception::BadLinkName(name);
}

String
ValueNode_Scale::get_name()const
{
	return "scale";
}

String
ValueNode_Scale::get_local_name()const
{
	return _("Scale");
}

bool
ValueNode_Scale::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_ANGLE ||
		type==ValueBase::TYPE_COLOR ||
		type==ValueBase::TYPE_INTEGER ||
		type==ValueBase::TYPE_REAL ||
		type==ValueBase::TYPE_TIME ||
		type==ValueBase::TYPE_VECTOR;
}
