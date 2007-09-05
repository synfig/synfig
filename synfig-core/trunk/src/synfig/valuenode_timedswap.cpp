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

ValueNode_TimedSwap::ValueNode_TimedSwap(ValueBase::Type type):
	LinkableValueNode(type)
{
	set_before(ValueNode_Const::create(type));
	set_after(ValueNode_Const::create(type));
	set_swap_time_real(1.0);
	set_swap_length_real(1.0);

	DCAST_HACK_ENABLE();
}

ValueNode_TimedSwap*
ValueNode_TimedSwap::create(const ValueBase& x)
{
	return new ValueNode_TimedSwap(x.get_type());
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


void
ValueNode_TimedSwap::set_swap_time_real(Time x)
{
	set_swap_time(ValueNode_Const::create(x));
}

bool
ValueNode_TimedSwap::set_swap_time(const ValueNode::Handle &x)
{
	if(!x
		|| !ValueBase(ValueBase::TYPE_TIME).same_type_as(x->get_type())
		&& !PlaceholderValueNode::Handle::cast_dynamic(x)
	)
		return false;
	swap_time=x;
	return true;
}

ValueNode::Handle
ValueNode_TimedSwap::get_swap_time()const
{
	return swap_time;
}

void
ValueNode_TimedSwap::set_swap_length_real(Time x)
{
	set_swap_length(ValueNode_Const::create(x));
}

bool
ValueNode_TimedSwap::set_swap_length(const ValueNode::Handle &x)
{
	if(!x || (
		!ValueBase(ValueBase::TYPE_TIME).same_type_as(x->get_type())
		&& !PlaceholderValueNode::Handle::cast_dynamic(x)
		)
	)
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
	case 0:
		return set_before(x);
	case 1:
		return set_after(x);
	case 2:
		return set_swap_time(x);
	case 3:
		return set_swap_length(x);
	}
	return 0;
}

ValueNode::LooseHandle
ValueNode_TimedSwap::get_link_vfunc(int i)const
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0:
		return get_before();
	case 1:
		return get_after();
	case 2:
		return get_swap_time();
	case 3:
		return get_swap_length();
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
	case 0:
		return _("Before");
	case 1:
		return _("After");
	case 2:
		return _("Swap Time");
	case 3:
		return _("Swap Duration");
	default:
		return 0;
	}
}

String
ValueNode_TimedSwap::link_name(int i)const
{
	assert(i>=0 && i<4);
	switch(i)
	{
	case 0:
		return "before";
	case 1:
		return "after";
	case 2:
		return "time";
	case 3:
		return "length";
	}
	return 0;
}

int
ValueNode_TimedSwap::get_link_index_from_name(const String &name)const
{
	if(name=="before")
		return 0;
	if(name=="after")
		return 1;
	if(name=="time")
		return 2;
	if(name=="length")
		return 3;

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
	if(!type)
		return false;
	return true;
}
