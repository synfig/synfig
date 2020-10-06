/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timedswap.cpp
**	\brief Implementation of the "Timed Swap" valuenode conversion.
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
#include "valuenode_timedswap.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/color.h>
#include <synfig/vector.h>

#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_TimedSwap, RELEASE_VERSION_0_61_07, "timed_swap", "Timed Swap")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_TimedSwap::ValueNode_TimedSwap(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(get_type());
	if (type == type_angle)
	{
		set_link("before",ValueNode_Const::create(value.get(Angle())));
		set_link("after",ValueNode_Const::create(value.get(Angle())));
	}
	else
	if (type == type_color)
	{
		set_link("before",ValueNode_Const::create(value.get(Color())));
		set_link("after",ValueNode_Const::create(value.get(Color())));
	}
	else
	if (type == type_integer)
	{
		set_link("before",ValueNode_Const::create(value.get(int())));
		set_link("after",ValueNode_Const::create(value.get(int())));
	}
	else
	if (type == type_real)
	{
		set_link("before",ValueNode_Const::create(value.get(Real())));
		set_link("after",ValueNode_Const::create(value.get(Real())));
	}
	else
	if (type == type_time)
	{
		set_link("before",ValueNode_Const::create(value.get(Time())));
		set_link("after",ValueNode_Const::create(value.get(Time())));
	}
	else
	if (type == type_vector)
	{
		set_link("before",ValueNode_Const::create(value.get(Vector())));
		set_link("after",ValueNode_Const::create(value.get(Vector())));
	}
	else
	{
		throw Exception::BadType(type.description.local_name);
	}

	set_link("time",ValueNode_Const::create(Time(2)));
	set_link("length",ValueNode_Const::create(Time(1)));
}

ValueNode_TimedSwap*
ValueNode_TimedSwap::create(const ValueBase& x)
{
	return new ValueNode_TimedSwap(x);
}

LinkableValueNode*
ValueNode_TimedSwap::create_new()const
{
	return new ValueNode_TimedSwap(get_type());
}

synfig::ValueNode_TimedSwap::~ValueNode_TimedSwap()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_TimedSwap::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Time swptime=(*swap_time)(t).get(Time());
	Time swplength=(*swap_length)(t).get(Time());

	if(t>swptime)
		return (*after)(t);

	if(t<=swptime && t>swptime-swplength)
	{
		Real amount=(swptime-t)/swplength;
		// if amount==0.0, then we are after
		// if amount==1.0, then we are before

		Type &type(get_type());
		if (type == type_angle)
		{
			Angle a=(*after)(t).get(Angle());
			Angle b=(*before)(t).get(Angle());
			return (b-a)*amount+a;
		}
		else
		if (type == type_color)
		{
			Color a=(*after)(t).get(Color());
			Color b=(*before)(t).get(Color());
			// note: Shouldn't this use a straight blend?
			return (b-a)*amount+a;
		}
		else
		if (type == type_integer)
		{
			float a=(float)(*after)(t).get(int());
			float b=(float)(*before)(t).get(int());
			return round_to_int((b-a)*amount+a);
		}
		else
		if (type == type_real)
		{
			Real a=(*after)(t).get(Real());
			Real b=(*before)(t).get(Real());
			return (b-a)*amount+a;
		}
		else
		if (type == type_time)
		{
			Time a=(*after)(t).get(Time());
			Time b=(*before)(t).get(Time());
			return (b-a)*amount+a;
		}
		else
		if (type == type_vector)
		{
			Vector a=(*after)(t).get(Vector());
			Vector b=(*before)(t).get(Vector());
			return (b-a)*amount+a;
		}
	}

	/*! \todo this should interpolate from
	**	before to after over the period defined
	**	by swap_length */

	return (*before)(t);
}

bool
ValueNode_TimedSwap::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(before,      get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(after,       get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(swap_time,   type_time);
	case 3: CHECK_TYPE_AND_SET_VALUE(swap_length, type_time);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TimedSwap::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return before;
	case 1: return after;
	case 2: return swap_time;
	case 3: return swap_length;
	}
	return 0;
}



bool
ValueNode_TimedSwap::check_type(Type &type)
{
	return
		type==type_angle	||
		type==type_color	||
		type==type_integer	||
		type==type_real 	||
		type==type_time 	||
		type==type_vector;
}

LinkableValueNode::Vocab
ValueNode_TimedSwap::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"before")
		.set_local_name(_("Before"))
		.set_description(_("The value node returned when current time is before 'time' - 'length'"))
	);

	ret.push_back(ParamDesc(ValueBase(),"after")
		.set_local_name(_("After"))
		.set_description(_("The value node returned when current time is after 'time'"))
	);

	ret.push_back(ParamDesc(ValueBase(),"time")
		.set_local_name(_("Time"))
		.set_description(_("The time when the linear interpolation ends"))
	);

	ret.push_back(ParamDesc(ValueBase(),"length")
		.set_local_name(_("Length"))
		.set_description(_("The length of time when the linear interpolation between 'Before' and 'After' is made"))
	);

	return ret;
}
