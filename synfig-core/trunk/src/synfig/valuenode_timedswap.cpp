/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timedswap.cpp
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
#include "valuenode_timedswap.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"

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
	switch(get_type())
	{
	case ValueBase::TYPE_REAL:
		set_link("before",ValueNode_Const::create(value.get(Real())));
		set_link("after",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("before",ValueNode_Const::create(value.get(Vector())));
		set_link("after",ValueNode_Const::create(value.get(Vector())));
		break;
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
	case ValueBase::TYPE_TIME:
		set_link("before",ValueNode_Const::create(value.get(Time())));
		set_link("after",ValueNode_Const::create(value.get(Time())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_name(get_type()));
	}

	set_link("time",ValueNode_Const::create(Time(2)));
	set_link("length",ValueNode_Const::create(Time(1)));

	DCAST_HACK_ENABLE();
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



bool
ValueNode_TimedSwap::set_before(const ValueNode::Handle &x)
{
	if(!x || x->get_type()!=get_type()
		&& !PlaceholderValueNode::Handle::cast_dynamic(x))
		return false;

	before=x;

	return true;
}

ValueNode::Handle
ValueNode_TimedSwap::get_before()const
{
	return before;
}


bool
ValueNode_TimedSwap::set_after(const ValueNode::Handle &x)
{
	if(!x || x->get_type()!=get_type()
		&& !PlaceholderValueNode::Handle::cast_dynamic(x))
		return false;

	after=x;

	return true;
}

ValueNode::Handle
ValueNode_TimedSwap::get_after()const
{
	return after;
}


bool
ValueNode_TimedSwap::set_swap_time(const ValueNode::Handle &x)
{
	if(!x || (!ValueBase(x->get_type()).same_type_as(ValueBase::TYPE_TIME) &&
			  !PlaceholderValueNode::Handle::cast_dynamic(x)))
		return false;

	swap_time=x;
	return true;
}

ValueNode::Handle
ValueNode_TimedSwap::get_swap_time()const
{
	return swap_time;
}

bool
ValueNode_TimedSwap::set_swap_length(const ValueNode::Handle &x)
{
	if(!x || (!ValueBase(x->get_type()).same_type_as(ValueBase::TYPE_TIME) &&
			  !PlaceholderValueNode::Handle::cast_dynamic(x)))
		return false;

	swap_length=x;
	return true;
}

ValueNode::Handle
ValueNode_TimedSwap::get_swap_length()const
{
	return swap_length;
}



synfig::ValueBase
synfig::ValueNode_TimedSwap::operator()(Time t)const
{
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
		case ValueBase::TYPE_REAL:
			{
				Real a=(*after)(t).get(Real());
				Real b=(*before)(t).get(Real());
				return (b-a)*amount+a;
			}
		case ValueBase::TYPE_VECTOR:
			{
				Vector a=(*after)(t).get(Vector());
				Vector b=(*before)(t).get(Vector());
				return (b-a)*amount+a;
			}
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
				return static_cast<int>((b-a)*amount+a+0.5f);
			}
		case ValueBase::TYPE_TIME:
			{
				Time a=(*after)(t).get(Time());
				Time b=(*before)(t).get(Time());
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
ValueNode_TimedSwap::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0: return set_before(x);
	case 1: return set_after(x);
	case 2: return set_swap_time(x);
	case 3: return set_swap_length(x);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TimedSwap::get_link_vfunc(int i)const
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0: return get_before();
	case 1: return get_after();
	case 2: return get_swap_time();
	case 3: return get_swap_length();
	}
	return 0;
}

int
ValueNode_TimedSwap::link_count()const
{
	return 4;
}

String
ValueNode_TimedSwap::link_local_name(int i)const
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0: return _("Before");
	case 1: return _("After");
	case 2: return _("Swap Time");
	case 3: return _("Swap Duration");
	default:return String();
	}
}

String
ValueNode_TimedSwap::link_name(int i)const
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0: return "before";
	case 1: return "after";
	case 2: return "time";
	case 3: return "length";
	default:return String();
	}
}

int
ValueNode_TimedSwap::get_link_index_from_name(const String &name)const
{
	if(name=="before")	return 0;
	if(name=="after")	return 1;
	if(name=="time")	return 2;
	if(name=="length")	return 3;

	throw Exception::BadLinkName(name);
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
		type==ValueBase::TYPE_REAL ||
		type==ValueBase::TYPE_VECTOR ||
		type==ValueBase::TYPE_ANGLE ||
		type==ValueBase::TYPE_COLOR ||
		type==ValueBase::TYPE_INTEGER ||
		type==ValueBase::TYPE_TIME;
}
