/* === S Y N F I G ========================================================= */
/*!	\file valuenode_twotone.cpp
**	\brief Implementation of the "Two-Tone" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_twotone.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/color.h>
#include <synfig/gradient.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_TwoTone, RELEASE_VERSION_0_61_06, "twotone", "Two-Tone")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_TwoTone::ValueNode_TwoTone(const ValueBase &value):LinkableValueNode(synfig::type_gradient)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_gradient)
	{
		set_link("color1",ValueNode_Const::create(value.get(Gradient())(0)));
		set_link("color2",ValueNode_Const::create(value.get(Gradient())(1)));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_TwoTone::create_new()const
{
	return new ValueNode_TwoTone(get_type());
}

ValueNode_TwoTone*
ValueNode_TwoTone::create(const ValueBase& x)
{
	return new ValueNode_TwoTone(x);
}

synfig::ValueNode_TwoTone::~ValueNode_TwoTone()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_TwoTone::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return Gradient((*ref_a)(t).get(Color()),(*ref_b)(t).get(Color()));
}

bool
ValueNode_TwoTone::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_a, type_color);
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_b, type_color);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TwoTone::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return ref_a;
		case 1:
			return ref_b;
	}
	return 0;
}



bool
ValueNode_TwoTone::check_type(Type &type)
{
	return type==type_gradient;
}

LinkableValueNode::Vocab
ValueNode_TwoTone::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"color1")
		.set_local_name(_("Color 1"))
		.set_description(_("The start color of the gradient"))
	);

	ret.push_back(ParamDesc(ValueBase(),"color2")
		.set_local_name(_("Color 2"))
		.set_description(_("The end color of the gradient"))
	);

	return ret;
}
