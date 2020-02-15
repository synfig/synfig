/* === S Y N F I G ========================================================= */
/*!	\file valuenode_sine.cpp
**	\brief Implementation of the "Sine" valuenode conversion.
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

#include "valuenode_sine.h"
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

REGISTER_VALUENODE(ValueNode_Sine, RELEASE_VERSION_0_61_06, "sine", _("Sine"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Sine::ValueNode_Sine(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_real)
	{
		set_link("angle",ValueNode_Const::create(Angle::deg(90)));
		set_link("amp",ValueNode_Const::create(value.get(Real())));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_Sine::create_new()const
{
	return new ValueNode_Sine(get_type());
}

ValueNode_Sine*
ValueNode_Sine::create(const ValueBase &x)
{
	return new ValueNode_Sine(x);
}

ValueNode_Sine::~ValueNode_Sine()
{
	unlink_all();
}

ValueBase
ValueNode_Sine::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return
		Angle::sin(
			(*angle_)(t).get(Angle())
		).get() * (*amp_)(t).get(Real())
	;
}




bool
ValueNode_Sine::check_type(Type &type)
{
	return type==type_real;
}

bool
ValueNode_Sine::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(angle_, type_angle);
	case 1: CHECK_TYPE_AND_SET_VALUE(amp_,   type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Sine::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return angle_;
	if(i==1)
		return amp_;

	return 0;
}

LinkableValueNode::Vocab
ValueNode_Sine::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"angle")
		.set_local_name(_("Angle"))
		.set_description(_("The angle where the sine is calculated from"))
	);

	ret.push_back(ParamDesc(ValueBase(),"amp")
		.set_local_name(_("Amplitude"))
		.set_description(_("The value that multiplies the resulting sine"))
	);

	return ret;
}
