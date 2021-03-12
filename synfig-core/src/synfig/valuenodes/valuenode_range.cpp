/* === S Y N F I G ========================================================= */
/*!	\file valuenode_range.cpp
**	\brief Implementation of the "Range" valuenode conversion.
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
#include "valuenode_range.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/real.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Range, RELEASE_VERSION_0_61_07, "range", "Range")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Range::ValueNode_Range(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(value.get_type());

	if (type == type_angle)
	{
		set_link("min",ValueNode_Const::create(value.get(Angle())));
		set_link("max",ValueNode_Const::create(value.get(Angle())));
		set_link("link",ValueNode_Const::create(value.get(Angle())));
	}
	else
	if (type == type_integer)
	{
		set_link("min",ValueNode_Const::create(value.get(int())));
		set_link("max",ValueNode_Const::create(value.get(int())));
		set_link("link",ValueNode_Const::create(value.get(int())));
	}
	else
	if (type == type_real)
	{
		set_link("min",ValueNode_Const::create(value.get(Real())));
		set_link("max",ValueNode_Const::create(value.get(Real())));
		set_link("link",ValueNode_Const::create(value.get(Real())));
	}
	else
	if (type == type_time)
	{
		set_link("min",ValueNode_Const::create(value.get(Time())));
		set_link("max",ValueNode_Const::create(value.get(Time())));
		set_link("link",ValueNode_Const::create(value.get(Time())));
	}
	else
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}

	assert(min_->get_type()==type);
	assert(max_->get_type()==type);
	assert(link_->get_type()==type);
	assert(get_type()==type);
}

LinkableValueNode*
ValueNode_Range::create_new()const
{
	return new ValueNode_Range(get_type());
}

ValueNode_Range*
ValueNode_Range::create(const ValueBase& value)
{
	return new ValueNode_Range(value);
}

synfig::ValueNode_Range::~ValueNode_Range()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Range::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!min_ || !max_ || !link_)
		throw runtime_error(strprintf("ValueNode_Range: %s",_("Some of my parameters aren't set!")));

	Type &type(get_type());
	if (type == type_angle)
	{
		Angle minimum = (* min_)(t).get(Angle());
		Angle maximum = (* max_)(t).get(Angle());
		Angle link    = (*link_)(t).get(Angle());
// This code was removed because it didn't work with link < minimum
// It is sane to completely delete it if the replacement code is fine.
/* ***********************************************
		// if link is between min and max, use it
		if (Angle::deg((link-minimum).mod()).get() < Angle::deg((maximum-minimum).mod()).get())
			return link;
		// otherwise use whichever of min and max is closest to link
		else if (link.dist(minimum).abs() < link.dist(maximum).abs())
			return minimum;
		else
			return maximum;
*********************************************** */
		if(Angle::rad(maximum).get()>=Angle::rad(link).get() && Angle::rad(link).get()>=Angle::rad(minimum).get())
			return link;
		else if (Angle::rad(minimum).get()>Angle::rad(link).get())
			return minimum;
		else
			return maximum;
	}
	else
	if (type == type_integer)
		return std::max((*min_)(t).get(int()),  std::min((*max_)(t).get(int()),  (*link_)(t).get(int())));
	else
	if (type == type_real)
		return std::max((*min_)(t).get(Real()), std::min((*max_)(t).get(Real()), (*link_)(t).get(Real())));
	else
	if (type == type_time)
		return std::max((*min_)(t).get(Time()), std::min((*max_)(t).get(Time()), (*link_)(t).get(Time())));

	assert(0);
	return ValueBase();
}

LinkableValueNode::InvertibleStatus
synfig::ValueNode_Range::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (!t.is_valid())
		return INVERSE_ERROR_BAD_TIME;

	const Type& type = target_value.get_type();
	if (type != type_angle && type != type_vector)
		return INVERSE_ERROR_BAD_TYPE;

	if (link_index)
		*link_index = get_link_index_from_name("link");
	return INVERSE_OK;
}

ValueBase
ValueNode_Range::get_inverse(const Time& t, const ValueBase& target_value) const
{
	const Type& type = target_value.get_type();
	if (type == type_angle)
		return get_inverse(t, target_value.get(Angle()));
	if (type == type_vector)
		return get_inverse(t, target_value.get(Vector()));
	throw runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Invalid value type")));
}

synfig::ValueBase
synfig::ValueNode_Range::get_inverse(const Time& t, const synfig::Vector &target_value) const
{
	Type &type(get_type());
	if (type == type_integer)
	{
		int max_value((*max_)(t).get(int()));
		int min_value((*min_)(t).get(int()));
		return std::max(min_value, std::min(max_value, int(target_value.mag())));
	}
	else
	if (type == type_real)
	{
		Real max_value((*max_)(t).get(Real()));
		Real min_value((*min_)(t).get(Real()));
		return std::max(min_value, std::min(max_value, target_value.mag()));
	}
	else
	if (type == type_angle)
	{
		Angle max_value((*max_)(t).get(Angle()));
		Angle min_value((*min_)(t).get(Angle()));
		Angle target_angle(Angle::tan(target_value[1],target_value[0]));
		return target_angle>max_value?max_value:target_angle<min_value?min_value:target_angle;
	}
	else
	if (type == type_time)
	{
		Real max_value((*max_)(t).get(Time()));
		Real min_value((*min_)(t).get(Time()));
		return std::max(min_value, std::min(max_value, target_value.mag()));
	}

	return target_value;
}

synfig::ValueBase
synfig::ValueNode_Range::get_inverse(const Time& t, const synfig::Angle &target_value) const
{
	Angle minimum = (* min_)(t).get(Angle());
	Angle maximum = (* max_)(t).get(Angle());
	// Angle link = (*link_)(t).get(Angle());
	if(Angle::rad(maximum).get()>=Angle::rad(target_value).get() && Angle::rad(target_value).get()>=Angle::rad(minimum).get())
		return target_value;
	else if (Angle::rad(minimum).get()>Angle::rad(target_value).get())
		return minimum;
	else
		return maximum;
	return ValueBase();
}


bool
ValueNode_Range::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(min_,  get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(max_,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(link_, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Range::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return min_;
		case 1: return max_;
		case 2: return link_;
		default: return 0;
	}
}



bool
ValueNode_Range::check_type(Type &type)
{
	return type==type_angle
		|| type==type_integer
		|| type==type_real
		|| type==type_time;
}

LinkableValueNode::Vocab
ValueNode_Range::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"min")
		.set_local_name(_("Min"))
		.set_description(_("Returned value when 'Link' is smaller"))
	);

	ret.push_back(ParamDesc(ValueBase(),"max")
		.set_local_name(_("Max"))
		.set_description(_("Returned value when 'Link' is greater"))
	);

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node to limit its range"))
	);

	return ret;
}
