/* === S Y N F I G ========================================================= */
/*!	\file valuenode_range.cpp
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
#include "valuenode_range.h"
#include "valuenode_const.h"
#include <stdexcept>
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

synfig::ValueNode_Range::ValueNode_Range(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	ValueBase::Type id(value.get_type());

	switch(id)
	{
	case ValueBase::TYPE_ANGLE:
		set_link("min",ValueNode_Const::create(value.get(Angle())));
		set_link("max",ValueNode_Const::create(value.get(Angle())));
		set_link("link",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("min",ValueNode_Const::create(value.get(int())));
		set_link("max",ValueNode_Const::create(value.get(int())));
		set_link("link",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("min",ValueNode_Const::create(value.get(Real())));
		set_link("max",ValueNode_Const::create(value.get(Real())));
		set_link("link",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("min",ValueNode_Const::create(value.get(Time())));
		set_link("max",ValueNode_Const::create(value.get(Time())));
		set_link("link",ValueNode_Const::create(value.get(Time())));
		break;
	default:
		assert(0);
		throw runtime_error("synfig::ValueNode_Range:Bad type "+ValueBase::type_name(id));
	}

	assert(min_->get_type()==id);
	assert(max_->get_type()==id);
	assert(link_->get_type()==id);
	assert(get_type()==id);

	DCAST_HACK_ENABLE();
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

#define min(x,y) (x>y ? y : x)
#define max(x,y) (x>y ? x : y)
#define range(low,high,input) (min(high,max(low,input)))

synfig::ValueBase
synfig::ValueNode_Range::operator()(Time t)const
{
	if(!min_ || !max_ || !link_)
		throw runtime_error(strprintf("ValueNode_Range: %s",_("Some of my parameters aren't set!")));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
	{
		Angle minimum = (* min_)(t).get(Angle());
		Angle maximum = (* max_)(t).get(Angle());
		Angle link    = (*link_)(t).get(Angle());

		// if link is between min and max, use it
		if (Angle::deg((link-minimum).mod()).get() < Angle::deg((maximum-minimum).mod()).get())
			return link;
		// otherwise use whichever of min and max is closest to link
		else if (link.dist(minimum).abs() < link.dist(maximum).abs())
			return minimum;
		else
			return maximum;
	}
	case ValueBase::TYPE_INTEGER:
		return range((*min_)(t).get(int()),   (*max_)(t).get(int()),   (*link_)(t).get(int()));
	case ValueBase::TYPE_REAL:
		return range((*min_)(t).get(Real()),  (*max_)(t).get(Real()),  (*link_)(t).get(Real()));
	case ValueBase::TYPE_TIME:
		return range((*min_)(t).get(Time()),  (*max_)(t).get(Time()),  (*link_)(t).get(Time()));
	default:
		assert(0);
		break;
	}
	return ValueBase();
}

bool
ValueNode_Range::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			min_=value;
			signal_child_changed()(i);signal_value_changed()();
			return true;
		case 1:
			max_=value;
			signal_child_changed()(i);signal_value_changed()();
			return true;
		case 2:
			link_=value;
			signal_child_changed()(i);signal_value_changed()();
			return true;
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_Range::get_link_vfunc(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return min_;
		case 1: return max_;
		case 2: return link_;
		default: return 0;
	}
}

int
ValueNode_Range::link_count()const
{
	return 3;
}

String
ValueNode_Range::link_local_name(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return _("Min");
		case 1: return _("Max");
		case 2: return _("Link");
		default: return String();
	}
}

String
ValueNode_Range::link_name(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0: return "min";
		case 1: return "max";
		case 2: return "link";
		default: return String();
	}
}

int
ValueNode_Range::get_link_index_from_name(const String &name)const
{
	if(name=="min") return 0;
	if(name=="max") return 1;
	if(name=="link") return 2;
	throw Exception::BadLinkName(name);
}

String
ValueNode_Range::get_name()const
{
	return "range";
}

String
ValueNode_Range::get_local_name()const
{
	return _("Range");
}

bool
ValueNode_Range::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_ANGLE
		|| type==ValueBase::TYPE_INTEGER
		|| type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_TIME;
}
