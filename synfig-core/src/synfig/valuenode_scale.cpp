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
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("scalar",ValueNode::Handle(ValueNode_Const::create(Real(1.0))));
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
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(id));
	}

	assert(value_node);
	assert(value_node->get_type()==id);
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

synfig::ValueBase
synfig::ValueNode_Scale::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

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

synfig::ValueBase
synfig::ValueNode_Scale::get_inverse(Time t, const synfig::Vector &target_value) const
{
	Real scalar_value((*scalar)(t).get(Real()));
	if(scalar_value==0)
			throw runtime_error(strprintf("ValueNode_Scale: %s",_("Attempting to get the inverse of a non invertible Valuenode")));
	else
		{
			switch (get_type())
			{
				case ValueBase::TYPE_REAL:
					return target_value.mag() / scalar_value;
				case ValueBase::TYPE_ANGLE:
					return Angle::tan(target_value[1] / scalar_value ,target_value[0] / scalar_value);
				default:
					return target_value / scalar_value;
			}
		}
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Scale::get_inverse(Time t, const synfig::Angle &target_value) const
{
	Real scalar_value((*scalar)(t).get(Real()));
	if(scalar_value==0)
			throw runtime_error(strprintf("ValueNode_Scale: %s",_("Attempting to get the inverse of a non invertible Valuenode")));
	else
		{
			switch (get_type())
			{
					default:
					return target_value / scalar_value;
			}
		}
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Scale::get_inverse(Time t, const synfig::Real &target_value) const
{
	Real scalar_value((*scalar)(t).get(Real()));
	if(scalar_value==0)
			throw runtime_error(strprintf("ValueNode_Scale: %s",_("Attempting to get the inverse of a non invertible Valuenode")));
	else
		{
			switch (get_type())
			{
					default:
					return target_value / scalar_value;
			}
		}
	return ValueBase();
}

bool
synfig::ValueNode_Scale::is_invertible(Time t) const
{
	Real scalar_value((*scalar)(t).get(Real()));
	return (!scalar_value==0);
}

bool
ValueNode_Scale::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(value_node, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(scalar,     ValueBase::TYPE_REAL);
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
