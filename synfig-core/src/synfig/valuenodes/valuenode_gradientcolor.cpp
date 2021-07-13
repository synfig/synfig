/* === S Y N F I G ========================================================= */
/*!	\file valuenode_gradientcolor.cpp
**	\brief Implementation of the "Gradient Color" valuenode conversion.
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

#include "valuenode_gradientcolor.h"
#include "valuenode_const.h"
#include <synfig/gradient.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_GradientColor, RELEASE_VERSION_0_61_09, "gradientcolor", "Gradient Color")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_GradientColor::ValueNode_GradientColor(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() ==  type_color)
	{
		set_link("gradient", ValueNode_Const::create(Gradient(value.get(Color()),value.get(Color()))));
		set_link("index",    ValueNode_Const::create(Real(0.5)));
		set_link("loop",    ValueNode_Const::create(bool(false)));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_GradientColor::create_new()const
{
	return new ValueNode_GradientColor(get_type());
}

ValueNode_GradientColor*
ValueNode_GradientColor::create(const ValueBase &x)
{
	return new ValueNode_GradientColor(x);
}

ValueNode_GradientColor::~ValueNode_GradientColor()
{
	unlink_all();
}

ValueBase
ValueNode_GradientColor::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Real index((*index_)(t).get(Real()));
	bool loop((*loop_)(t).get(bool()));
	if (loop) index -= floor(index);
	return (*gradient_)(t).get(Gradient())(index);
}


bool
ValueNode_GradientColor::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(gradient_,	type_gradient);
	case 1: CHECK_TYPE_AND_SET_VALUE(index_,	type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(loop_,		type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_GradientColor::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return gradient_;
		case 1: return index_;
		case 2: return loop_;
	}

	return 0;
}



bool
ValueNode_GradientColor::check_type(Type &type)
{
	return type==type_color;
}

LinkableValueNode::Vocab
ValueNode_GradientColor::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("The gradient where the color is picked from"))
	);

	ret.push_back(ParamDesc(ValueBase(),"index")
		.set_local_name(_("Index"))
		.set_description(_("The position of the color at the gradient (0,1]"))
	);

	ret.push_back(ParamDesc(ValueBase(),"loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked, the index would loop"))
	);

	return ret;
}
