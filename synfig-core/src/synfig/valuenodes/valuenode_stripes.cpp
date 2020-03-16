/* === S Y N F I G ========================================================= */
/*!	\file valuenode_stripes.cpp
**	\brief Implementation of the "Stripes" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_stripes.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/color.h>
#include <synfig/gradient.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Stripes, RELEASE_VERSION_0_61_06, "stripes", "Stripes")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Stripes::ValueNode_Stripes():LinkableValueNode(synfig::type_gradient)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("color1",ValueNode_Const::create(Color::alpha()));
	set_link("color2",ValueNode_Const::create(Color::black()));
	set_link("stripes",stripes_=ValueNode_Const::create(int(5)));
	set_link("width",ValueNode_Const::create(0.5));
}

LinkableValueNode*
ValueNode_Stripes::create_new()const
{
	return new ValueNode_Stripes();
}

ValueNode_Stripes*
ValueNode_Stripes::create(const ValueBase& x)
{
	Type &type(x.get_type());

	if(type!=type_gradient)
	{
		assert(0);
		throw runtime_error(String(_("Stripes"))+_(":Bad type ")+type.description.local_name);
	}

	ValueNode_Stripes* value_node=new ValueNode_Stripes();

	assert(value_node->get_type()==type);

	return value_node;
}

synfig::ValueNode_Stripes::~ValueNode_Stripes()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Stripes::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const int total((*stripes_)(t).get(int()));
	int i;
	Gradient ret;

	if(total<=0)
		return ret;

	const Color color1((*color1_)(t).get(Color()));
	const Color color2((*color2_)(t).get(Color()));
	const float width(max(0.0,min(1.0,(*width_)(t).get(Real()))));

	const float stripe_width_a(width/total);
	const float stripe_width_b((1.0-width)/total);

	for(i=0;i<total;i++)
	{
		float pos(float(i)/total+stripe_width_b/2);
		ret.push_back(Gradient::CPoint(pos,color1));
		ret.push_back(Gradient::CPoint(pos,color2));
		pos+=stripe_width_a;
		ret.push_back(Gradient::CPoint(pos,color2));
		ret.push_back(Gradient::CPoint(pos,color1));
	}
	return ret;
}

bool
ValueNode_Stripes::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(color1_,  type_color);
	case 1: CHECK_TYPE_AND_SET_VALUE(color2_,  type_color);
	case 2: CHECK_TYPE_AND_SET_VALUE(stripes_, type_integer);
	case 3: CHECK_TYPE_AND_SET_VALUE(width_,   type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Stripes::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:
			return color1_;
		case 1:
			return color2_;
		case 2:
			return stripes_;
		case 3:
			return width_;
	}
	return 0;
}



bool
ValueNode_Stripes::check_type(Type &type)
{
	return type==type_gradient;
}

LinkableValueNode::Vocab
ValueNode_Stripes::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"color1")
		.set_local_name(_("Color 1"))
		.set_description(_("One color of the gradient stripes"))
	);

	ret.push_back(ParamDesc(ValueBase(),"color2")
		.set_local_name(_("Color 2"))
		.set_description(_("Other color of the gradient stripes"))
	);

		ret.push_back(ParamDesc(ValueBase(),"stripes")
		.set_local_name(_("Stripe Count"))
		.set_description(_("Number of stripes in the gradient"))
	);

		ret.push_back(ParamDesc(ValueBase(),"width")
		.set_local_name(_("Width"))
		.set_description(_("Width of stripes in the gradient between [0,1]"))
	);

	return ret;
}
