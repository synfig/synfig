/* === S Y N F I G ========================================================= */
/*!	\file valuenode_atan2.cpp
**	\brief Implementation of the "aTan2" valuenode conversion.
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

#include "valuenode_atan2.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Atan2, RELEASE_VERSION_0_61_08, "atan2", _("aTan2"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Atan2::ValueNode_Atan2(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_angle)
	{
		set_link("x",ValueNode_Const::create(Angle::cos(value.get(Angle())).get()));
		set_link("y",ValueNode_Const::create(Angle::sin(value.get(Angle())).get()));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_Atan2::create_new()const
{
	return new ValueNode_Atan2(get_type());
}

ValueNode_Atan2*
ValueNode_Atan2::create(const ValueBase &x)
{
	return new ValueNode_Atan2(x);
}

ValueNode_Atan2::~ValueNode_Atan2()
{
	unlink_all();
}

ValueBase
ValueNode_Atan2::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return Angle::tan((*y_)(t).get(Real()),
					  (*x_)(t).get(Real()));
}




bool
ValueNode_Atan2::check_type(Type &type)
{
	return type==type_angle;
}

bool
ValueNode_Atan2::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(x_, type_real);
	case 1: CHECK_TYPE_AND_SET_VALUE(y_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Atan2::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return x_;
	if(i==1)
		return y_;

	return 0;
}

LinkableValueNode::Vocab
ValueNode_Atan2::get_children_vocab_vfunc()const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"x")
		.set_local_name(_("X"))
		.set_description(_("Cosine of the angle"))
	);

	ret.push_back(ParamDesc(ValueBase(),"y")
		.set_local_name(_("Y"))
		.set_description(_("Sine of the angle"))
	);

	return ret;
}
