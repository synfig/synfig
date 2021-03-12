/* === S Y N F I G ========================================================= */
/*!	\file valuenode_add.cpp
**	\brief Implementation of the "Add" valuenode conversion.
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

#include "valuenode_add.h"
#include "valuenode_const.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/color.h>
#include <synfig/gradient.h>
#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/real.h>

#include <ETL/misc>

#include <stdexcept>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Add, RELEASE_VERSION_0_61_07, "add", "Add")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Add::ValueNode_Add(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("scalar",ValueNode_Const::create(Real(1.0)));
	Type& type(value.get_type());

	if (type == type_angle)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Angle())));
		set_link("rhs",ValueNode_Const::create(Angle::deg(0)));
	}
	else
	if (type == type_color)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Color())));
		set_link("rhs",ValueNode_Const::create(Color(0,0,0,0)));
	}
	else
	if (type == type_gradient)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Gradient())));
		set_link("rhs",ValueNode_Const::create(Gradient()));
	}
	else
	if (type == type_integer)
	{
		set_link("lhs",ValueNode_Const::create(value.get(int())));
		set_link("rhs",ValueNode_Const::create(int(0)));
	}
	else
	if (type == type_real)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Real())));
		set_link("rhs",ValueNode_Const::create(Real(0)));
	}
	else
	if (type == type_time)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Time())));
		set_link("rhs",ValueNode_Const::create(Time(0)));
	}
	else
	if (type == type_vector)
	{
		set_link("lhs",ValueNode_Const::create(value.get(Vector())));
		set_link("rhs",ValueNode_Const::create(Vector(0,0)));
	}
	else
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}
}

LinkableValueNode*
ValueNode_Add::create_new()const
{
	return new ValueNode_Add(get_type());
}

ValueNode_Add*
ValueNode_Add::create(const ValueBase& value)
{
	return new ValueNode_Add(value);
}

synfig::ValueNode_Add::~ValueNode_Add()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Add::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!ref_a || !ref_b)
		throw runtime_error(strprintf("ValueNode_Add: %s",_("One or both of my parameters aren't set!")));
	Type &type(get_type());
	if (type == type_angle)
		return ((*ref_a)(t).get(Angle())+(*ref_b)(t).get(Angle()))*(*scalar)(t).get(Real());
	if (type == type_color)
		return ((*ref_a)(t).get(Color())+(*ref_b)(t).get(Color()))*(*scalar)(t).get(Real());
	if (type == type_gradient)
		return ((*ref_a)(t).get(Gradient())+(*ref_b)(t).get(Gradient()))*(*scalar)(t).get(Real());
	if (type == type_integer)
		return round_to_int(((*ref_a)(t).get(int())+(*ref_b)(t).get(int()))*(*scalar)(t).get(Real()));
	if (type == type_real)
		return ((*ref_a)(t).get(Vector::value_type())+(*ref_b)(t).get(Vector::value_type()))*(*scalar)(t).get(Real());
	if (type == type_time)
		return ((*ref_a)(t).get(Time())+(*ref_b)(t).get(Time()))*(*scalar)(t).get(Real());
	if (type == type_vector)
		return ((*ref_a)(t).get(Vector())+(*ref_b)(t).get(Vector()))*(*scalar)(t).get(Real());

	assert(0);
	return ValueBase();
}

bool
ValueNode_Add::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_a,  get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_b,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(scalar, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Add::get_link_vfunc(int i)const
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

bool
ValueNode_Add::check_type(Type &type)
{
	return type==type_angle
		|| type==type_color
		|| type==type_gradient
		|| type==type_integer
		|| type==type_real
		|| type==type_time
		|| type==type_vector;
}

LinkableValueNode::Vocab
ValueNode_Add::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"lhs")
		.set_local_name(_("Link"))
		.set_description(_("Left Hand Side of the add"))
	);

	ret.push_back(ParamDesc(ValueBase(),"rhs")
		.set_local_name(_("Addition"))
		.set_description(_("Right Hand Side of the add"))
	);

		ret.push_back(ParamDesc(ValueBase(),"scalar")
		.set_local_name(_("Scalar"))
		.set_description(_("Value that multiplies the add"))
	);

	return ret;
}

LinkableValueNode::InvertibleStatus
ValueNode_Add::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (!t.is_valid())
		return INVERSE_ERROR_BAD_TIME;
	if (approximate_zero((*scalar)(t).get(Real()))) {
		if (link_index)
			*link_index = get_link_index_from_name("scalar");
		return INVERSE_ERROR_BAD_PARAMETER;
	}
	const Type& type = target_value.get_type();
	if (type != type_real && type != type_angle && type != type_vector)
		return INVERSE_ERROR_BAD_TYPE;
	if (link_index)
		*link_index = get_link_index_from_name("lhs");
	return INVERSE_OK;
}

ValueBase
ValueNode_Add::get_inverse(const Time& t, const ValueBase& target_value) const
{
	Real scalar_value = (*scalar)(t).get(Real());

	if (approximate_zero(scalar_value))
		throw runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Scalar is zero")));

	const Type& type = target_value.get_type();
	if (type == type_real)
		return target_value.get(Real()) / scalar_value - (*ref_b)(t).get(Real());
	if (type == type_angle)
		return target_value.get(Angle()) / scalar_value - (*ref_b)(t).get(Angle());
	if (type == type_vector)
		return target_value.get(Vector()) / scalar_value - (*ref_b)(t).get(Vector());
	throw runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Invalid value type")));
}
