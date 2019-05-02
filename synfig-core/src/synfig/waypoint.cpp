
/* === S Y N F I G ========================================================= */
/*!	\file waypoint.cpp
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

#include "waypoint.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_animated.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Waypoint::Waypoint(ValueBase value, Time time):
	priority_(0),
	before(INTERPOLATION_TCB),
	after(INTERPOLATION_TCB),
	value_node(ValueNode_Const::create(value)),
	time(time),
	tension(0.0),
	continuity(0.0),
	bias(0),
	time_tension(0.0f)
{
	//!Writeme
	if(value.get_type()==type_angle)
		after=before=INTERPOLATION_LINEAR;
}

Waypoint::Waypoint(etl::handle<ValueNode> value_node, Time time):
	priority_(0),
	before(INTERPOLATION_TCB),
	after(INTERPOLATION_TCB),
	value_node(value_node),
	time(time),
	tension(0.0),
	continuity(0),
	bias(0),
	time_tension(0.0f)
{
	if(value_node->get_type()==type_angle)
		after=before=INTERPOLATION_LINEAR;
}

Waypoint::Waypoint():
	priority_(0),
	before(INTERPOLATION_TCB),
	after(INTERPOLATION_TCB),
	tension(0),
	continuity(0),
	bias(0),
	time_tension(0.0f)
{
}

void
Waypoint::set_value(const ValueBase &x)
{
	//! If the value node is not set and we are setting the value
	//! of an angle, then set both interpolation to linear... why?
	if(!value_node && x.get_type()==type_angle)
		after=before=INTERPOLATION_LINEAR;

	value_node=ValueNode_Const::create(x);
}

void
Waypoint::set_value_node(const etl::handle<ValueNode> &x)
{

	// printf("%s:%d Waypoint::set_value_node(%lx) = %lx (%s)\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(x), x->get_string().c_str());

	//! If the value node is not set and we are setting the value
	//! of an angle, then set both interpolation to linear... why?

	if(!value_node && x->get_type()==type_angle)
		after=before=INTERPOLATION_LINEAR;

	if (value_node == x)
		return;

	ValueNode::Handle vn_parent(get_parent_value_node());
	if (!vn_parent)
	{
		value_node=x;
		return;
	}

	ValueNode_Animated::Handle parent(ValueNode_Animated::Handle::cast_dynamic(vn_parent));
	if (!parent)
	{
		value_node=x;
		return;
	}

	if (parent->waypoint_is_only_use_of_valuenode(*this))
		parent->remove_child(value_node.get());

	value_node=x;
	parent->add_child(value_node.get());
	parent->changed();
}

void
Waypoint::set_parent_value_node(const etl::loose_handle<ValueNode> &x)
{
	// printf("%s:%d Waypoint::set_parent_value_node(%lx) = %lx (%s)\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(x.get()), x->get_string().c_str());
	assert(get_value_node());

	if (parent_ == x)
		return;

	// it seems that the parent is never previously set (unless it was
	// already set to the same value, in which case the previous test
	// will have caused this function to return already).  if the
	// parent was previously set, we may need to call
	// parent_->remove_child() so assert that it isn't, and fix if it
	// is...
	assert(!parent_);

	parent_=x;
	parent_->add_child(get_value_node().get());
}

bool
Waypoint::is_static()const
{
	return static_cast<bool>(ValueNode_Const::Handle::cast_dynamic(value_node)) && value_node && !value_node->is_exported();
}

void
Waypoint::set_time(const Time &x)
{
	time=x;
}

void
Waypoint::apply_model(const Model &x)
{
	if(x.priority_flag) set_priority(x.get_priority());
	if(x.before_flag) set_before(x.get_before());
	if(x.after_flag) set_after(x.get_after());
	if(x.tension_flag) set_tension(x.get_tension());
	if(x.continuity_flag) set_continuity(x.get_continuity());
	if(x.bias_flag) set_bias(x.get_bias());
	if(x.temporal_tension_flag) set_temporal_tension(x.get_temporal_tension());
}

Waypoint
Waypoint::clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid)const
{
	Waypoint ret(*this);
	ret.make_unique();
	if(!ret.value_node->is_exported())
		ret.value_node=value_node->clone(canvas, deriv_guid);
	ret.parent_=0;
	return ret;
}

ValueBase
Waypoint::get_value()const { return (*value_node)(0); }

ValueBase
Waypoint::get_value(const Time &t)const {
	return (*value_node)(t);
}

synfig::GUID
Waypoint::get_guid()const
{
	return GUID::hasher(get_uid());
}
