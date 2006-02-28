/* === S Y N F I G ========================================================= */
/*!	\file waypoint.cpp
**	\brief Template File
**
**	$Id: waypoint.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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
#include "valuenode_const.h"

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
	if(value.get_type()==ValueBase::TYPE_ANGLE)
		after=before=INTERPOLATION_LINEAR;
}

Waypoint::Waypoint(ValueNode::Handle value_node, Time time):
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
	if(value_node->get_type()==ValueBase::TYPE_ANGLE)
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
	if(!value_node && x.get_type()==ValueBase::TYPE_ANGLE)
		after=before=INTERPOLATION_LINEAR;

	value_node=ValueNode_Const::create(x);
}

void
Waypoint::set_value_node(const ValueNode::Handle &x)
{
	if(!value_node && x->get_type()==ValueBase::TYPE_ANGLE)
		after=before=INTERPOLATION_LINEAR;

	value_node=x;
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
Waypoint::clone(const GUID& deriv_guid)const
{
	Waypoint ret(*this);
	ret.make_unique();
	if(!ret.value_node->is_exported())
		ret.value_node=value_node->clone(deriv_guid);
	ret.parent_=0;
	return ret;
}

ValueBase
Waypoint::get_value()const { return (*value_node)(0); }

ValueBase
Waypoint::get_value(const Time &t)const { return (*value_node)(t); }

synfig::GUID
Waypoint::get_guid()const
{
	return GUID::hasher(get_uid());
}
