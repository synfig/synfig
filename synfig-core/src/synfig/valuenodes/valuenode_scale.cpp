/* === S Y N F I G ========================================================= */
/*!	\file valuenode_scale.cpp
**	\brief Implementation of the "Scale" valuenode conversion.
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_scale.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/time.h>
#include <synfig/angle.h>
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Scale, RELEASE_VERSION_0_61_06, "scale", "Scale")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Scale::ValueNode_Scale(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("scalar",ValueNode::Handle(ValueNode_Const::create(Real(1.0))));
	Type &type(value.get_type());

	if (type == type_angle)
		set_link("link",ValueNode_Const::create(value.get(Angle())));
	else
	if (type == type_color)
		set_link("link",ValueNode_Const::create(value.get(Color())));
	else
	if (type == type_integer)
		set_link("link",ValueNode_Const::create(value.get(int())));
	else
	if (type == type_real)
		set_link("link",ValueNode_Const::create(value.get(Real())));
	else
	if (type == type_time)
		set_link("link",ValueNode_Const::create(value.get(Time())));
	else
	if (type == type_vector)
		set_link("link",ValueNode_Const::create(value.get(Vector())));
	else
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}

	assert(value_node);
	assert(value_node->get_type()==type);
	assert(get_type()==type);
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

synfig::ValueBase
synfig::ValueNode_Scale::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!value_node || !scalar)
		throw runtime_error(strprintf("ValueNode_Scale: %s",_("One or both of my parameters aren't set!")));
	else if(get_type()==type_angle)
		return (*value_node)(t).get(Angle())*(*scalar)(t).get(Real());
	else if(get_type()==type_color)
	{
		Color ret((*value_node)(t).get(Color()));
		Real s((*scalar)(t).get(Real()));
		ret.set_r(ret.get_r()*s);
		ret.set_g(ret.get_g()*s);
		ret.set_b(ret.get_b()*s);
		return ret;
	}
	else if(get_type()==type_integer)
		return round_to_int((*value_node)(t).get(int())*(*scalar)(t).get(Real()));
	else if(get_type()==type_real)
		return (*value_node)(t).get(Real())*(*scalar)(t).get(Real());
	else if(get_type()==type_time)
		return (*value_node)(t).get(Time())*(*scalar)(t).get(Time());
	else if(get_type()==type_vector)
		return (*value_node)(t).get(Vector())*(*scalar)(t).get(Real());

	assert(0);
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Scale::get_inverse(const Time& t, const synfig::ValueBase &target_value) const
{
	Real scalar_value((*scalar)(t).get(Real()));
	if(approximate_zero(scalar_value))
		throw runtime_error(strprintf("ValueNode_%s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Scalar is zero")));
	const Type& target_type = target_value.get_type();
	if (target_type == type_real)
		return target_value.get(Real()) / scalar_value;
	if (target_type == type_angle)
		return target_value.get(Angle()) / scalar_value;
	if (target_type == type_vector) {
		Vector target_vector = target_value.get(Vector());
		if (get_type() == type_real)
			return target_vector.mag() / scalar_value;
		if (get_type() == type_angle)
			return Angle::tan(target_vector[1] / scalar_value ,target_vector[0] / scalar_value);
		return target_vector / scalar_value;
	}
	throw runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Invalid value type")));
}

LinkableValueNode::InvertibleStatus
synfig::ValueNode_Scale::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (!t.is_valid())
		return INVERSE_ERROR_BAD_TIME;

	Real scalar_value((*scalar)(t).get(Real()));
	if (approximate_zero(scalar_value)) {
		if (link_index)
			*link_index = get_link_index_from_name("scalar");
		return INVERSE_ERROR_BAD_PARAMETER;
	}

	const Type& type = target_value.get_type();
	if (type != type_real && type != type_angle && type != type_vector)
		return INVERSE_ERROR_BAD_TYPE;

	if (link_index)
		*link_index = get_link_index_from_name("link");
	return INVERSE_OK;
}

bool
ValueNode_Scale::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(value_node, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(scalar,     type_real);
	}
	return false;
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



bool
ValueNode_Scale::check_type(Type &type)
{
	return
		type==type_angle	||
		type==type_color	||
		type==type_integer	||
		type==type_real		||
		type==type_time		||
		type==type_vector;
}

LinkableValueNode::Vocab
ValueNode_Scale::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node used to scale"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scalar")
		.set_local_name(_("Scalar"))
		.set_description(_("Value that multiplies the value node"))
	);

	return ret;
}
