/* === S Y N F I G ========================================================= */
/*!	\file valuenode_angle.cpp
**	\brief Implementation of the "Angle" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_angle.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/angle.h>
#include <synfig/real.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Angle, RELEASE_VERSION_1_6_0, "fromangle", N_("Angle"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Angle::ValueNode_Angle(Type& x):
	LinkableValueNode(x)
{
	init_children_vocab();
}

ValueNode_Angle::ValueNode_Angle(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	init_children_vocab();
	if (value.get_type() == type_real)
	{
		set_link("link", ValueNode_Const::create(Angle::deg(value.get(Real()))));
	}
	else
	if (value.get_type() == type_integer)
	{
		set_link("link", ValueNode_Const::create(Angle::deg(value.get(int()))));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_Angle::create_new()const
{
	return new ValueNode_Angle(get_type());
}

ValueNode_Angle*
ValueNode_Angle::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_Angle(x);
}

ValueNode_Angle::~ValueNode_Angle()
{
	unlink_all();
}

ValueBase
ValueNode_Angle::operator()(Time t)const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);

	const Type& type(get_type());
	if (type == type_real)
		return Angle::deg((*angle_)(t).get(Angle())).get();
	if (type == type_integer)
		return int(Angle::deg((*angle_)(t).get(Angle())).get());

	assert(0);
	throw std::runtime_error(get_local_name() + _(":Bad type ") + get_type().description.local_name);
}

bool
ValueNode_Angle::check_type(Type &type)
{
	return type == type_real || type == type_integer;
}

bool
ValueNode_Angle::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(angle_, type_angle);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Angle::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return angle_;

	return 0;
}

LinkableValueNode::Vocab
ValueNode_Angle::get_children_vocab_vfunc()const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("link")
		.set_local_name(_("Link"))
		.set_description(_("Angle value in degrees"))
	);

	return ret;
}
