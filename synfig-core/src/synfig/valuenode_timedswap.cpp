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

#include "general.h"
#include "valuenode_timedswap.h"
#include "valuenode_const.h"
#include <stdexcept>
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

ValueNode_TimedSwap::ValueNode_TimedSwap(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("before",ValueNode_Const::create(value.get(Angle())));
		set_link("after",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("before",ValueNode_Const::create(value.get(Color())));
		set_link("after",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("before",ValueNode_Const::create(value.get(int())));
		set_link("after",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("before",ValueNode_Const::create(value.get(Real())));
		set_link("after",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("before",ValueNode_Const::create(value.get(Time())));
		set_link("after",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("before",ValueNode_Const::create(value.get(Vector())));
		set_link("after",ValueNode_Const::create(value.get(Vector())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(get_type()));
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

		switch(get_type())
		{
		case ValueBase::TYPE_ANGLE:
			{
				Angle a=(*after)(t).get(Angle());
				Angle b=(*before)(t).get(Angle());
				return (b-a)*amount+a;
			}
		case ValueBase::TYPE_COLOR:
			{
				Color a=(*after)(t).get(Color());
				Color b=(*before)(t).get(Color());
				// note: Shouldn't this use a straight blend?
				return (b-a)*amount+a;
			}
		case ValueBase::TYPE_INTEGER:
			{
				float a=(float)(*after)(t).get(int());
				float b=(float)(*before)(t).get(int());
				return round_to_int((b-a)*amount+a);
			}
		case ValueBase::TYPE_REAL:
			{
				Real a=(*after)(t).get(Real());
				Real b=(*before)(t).get(Real());
				return (b-a)*amount+a;
			}
		case ValueBase::TYPE_TIME:
			{
				Time a=(*after)(t).get(Time());
				Time b=(*before)(t).get(Time());
				return (b-a)*amount+a;
			}
		case ValueBase::TYPE_VECTOR:
			{
				Vector a=(*after)(t).get(Vector());
				Vector b=(*before)(t).get(Vector());
				return (b-a)*amount+a;
			}
		default:
			break;
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
	case 2: CHECK_TYPE_AND_SET_VALUE(swap_time,   ValueBase::TYPE_TIME);
	case 3: CHECK_TYPE_AND_SET_VALUE(swap_length, ValueBase::TYPE_TIME);
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

String
ValueNode_TimedSwap::get_name()const
{
	return "timed_swap";
}

String
ValueNode_TimedSwap::get_local_name()const
{
	return _("Timed Swap");
}

bool
ValueNode_TimedSwap::check_type(ValueBase::Type type)
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
