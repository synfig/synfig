/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.cpp
**	\brief Implementation of the "Animated" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <synfig/localization.h>

#include "valuenode_animated.h"
#include "valuenode_const.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Animated::ValueNode_Animated(Type &type):
	ValueNode_AnimatedInterface(*(ValueNode*)this)
{
	ValueNode_AnimatedInterface::set_type(type);
}

ValueNode_Animated::Handle
ValueNode_Animated::create(Type &type)
	{ return new ValueNode_Animated(type); }

ValueNode_Animated::Handle
ValueNode_Animated::create(const ValueBase& value, const Time& time)
	{ return create(ValueNode::Handle(ValueNode_Const::create(value)),time); }

ValueNode_Animated::Handle
ValueNode_Animated::create(ValueNode::Handle value_node, const Time& time)
{
	ValueNode_Animated::Handle ret(create(value_node->get_type()));
	ret->new_waypoint(time,value_node);
	return ret;
}


ValueNode::Handle
ValueNode_Animated::clone(Canvas::LooseHandle canvas, const synfig::GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
	ValueNode_Animated::Handle ret(new ValueNode_Animated(get_type()));
	ret->set_guid(get_guid()^deriv_guid);
	ret->set_parent_canvas(canvas);
	ret->assign(*this, deriv_guid);
	return ret;
}

String
ValueNode_Animated::get_name()const
	{ return "animated"; }

String
ValueNode_Animated::get_local_name()const
	{ return _("Animated"); }

String
ValueNode_Animated::get_string()const
	{ return "ValueNode_Animated"; }

void
ValueNode_Animated::on_changed()
{
	ValueNode::on_changed();
	ValueNode_AnimatedInterface::on_changed();
}

ValueBase
ValueNode_Animated::operator()(Time t) const
	{ return ValueNode_AnimatedInterface::operator()(t); }

void
ValueNode_Animated::get_values_vfunc(std::map<Time, ValueBase> &x) const
	{ ValueNode_AnimatedInterface::get_values_vfunc(x); }

void
ValueNode_Animated::get_times_vfunc(Node::time_set &set) const
	{ ValueNode_AnimatedInterface::get_times_vfunc(set); }

