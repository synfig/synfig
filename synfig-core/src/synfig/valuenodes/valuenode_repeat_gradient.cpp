/* === S Y N F I G ========================================================= */
/*!	\file valuenode_repeat_gradient.cpp
**	\brief Implementation of the "Repeat Gradient" valuenode conversion.
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

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_repeat_gradient.h"
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

REGISTER_VALUENODE(ValueNode_Repeat_Gradient, RELEASE_VERSION_0_61_07, "repeat_gradient", "Repeat Gradient")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Repeat_Gradient::ValueNode_Repeat_Gradient(const Gradient& x):LinkableValueNode(synfig::type_gradient)
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("gradient",ValueNode_Const::create(x));
	set_link("count",count_=ValueNode_Const::create(int(3)));
	set_link("width",ValueNode_Const::create(0.5));
	set_link("specify_start",ValueNode_Const::create(true));
	set_link("specify_end",ValueNode_Const::create(true));
	set_link("start_color",ValueNode_Const::create(Color::alpha()));
	set_link("end_color",ValueNode_Const::create(Color::alpha()));
}

LinkableValueNode*
ValueNode_Repeat_Gradient::create_new()const
{
	return new ValueNode_Repeat_Gradient(Gradient());
}

ValueNode_Repeat_Gradient*
ValueNode_Repeat_Gradient::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	Type &type(x.get_type());

	if(type!=type_gradient)
	{
		assert(0);
		throw std::runtime_error(String(_("Repeat Gradient"))+_(":Bad type ")+type.description.local_name);
	}

	ValueNode_Repeat_Gradient* value_node=new ValueNode_Repeat_Gradient(x.get(Gradient()));

	assert(value_node->get_type()==type);

	return value_node;
}

synfig::ValueNode_Repeat_Gradient::~ValueNode_Repeat_Gradient()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Repeat_Gradient::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const int count((*count_)(t).get(int()));
	int i;
	Gradient ret;

	if(count<=0)
		return ret;

	const Gradient gradient((*gradient_)(t).get(Gradient()));
	const float width(std::max(0.0, std::min(1.0,(*width_)(t).get(Real()))));
	const bool specify_start((*specify_start_)(t).get(bool()));
	const bool specify_end((*specify_end_)(t).get(bool()));

	const float gradient_width_a(width/count);
	const float gradient_width_b((1.0-width)/count);

	Gradient::const_iterator iter;
	Gradient::const_reverse_iterator riter;
	if (specify_start)
		ret.push_back(Gradient::CPoint(0,(*start_color_)(t).get(Color())));
	for(i=0;i<count;i++)
	{
		float pos(float(i)/count);
		if (width != 0.0)
			for(iter=gradient.begin();iter!=gradient.end();iter++)
				ret.push_back(Gradient::CPoint(pos+gradient_width_a*iter->pos,iter->color));
		pos+=gradient_width_a;
		if (width != 1.0)
			for(riter=gradient.rbegin();riter!=gradient.rend();riter++)
				ret.push_back(Gradient::CPoint(pos+gradient_width_b*(1-(riter->pos)),riter->color));
	}
	if (specify_end)
		ret.push_back(Gradient::CPoint(1,(*end_color_)(t).get(Color())));
	return ret;
}

bool
ValueNode_Repeat_Gradient::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(gradient_,	     get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(count_,		 type_integer);
	case 2: CHECK_TYPE_AND_SET_VALUE(width_,		 type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(specify_start_, type_bool);
	case 4: CHECK_TYPE_AND_SET_VALUE(specify_end_,   type_bool);
	case 5: CHECK_TYPE_AND_SET_VALUE(start_color_,   type_color);
	case 6: CHECK_TYPE_AND_SET_VALUE(end_color_,     type_color);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Repeat_Gradient::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0:  return gradient_;
		case 1:  return count_;
		case 2:  return width_;
		case 3:  return specify_start_;
		case 4:  return specify_end_;
		case 5:  return start_color_;
		case 6:  return end_color_;
	    default: return 0;
	}
}



bool
ValueNode_Repeat_Gradient::check_type(Type &type)
{
	return type==type_gradient;
}

LinkableValueNode::Vocab
ValueNode_Repeat_Gradient::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("The source gradient to repeat"))
	);

	ret.push_back(ParamDesc(ValueBase(),"count")
		.set_local_name(_("Count"))
		.set_description(_("The number of repetition of the gradient"))
	);

	ret.push_back(ParamDesc(ValueBase(),"width")
		.set_local_name(_("Width"))
		.set_description(_("Specifies how much biased is the source gradient in the repetition [0,1]"))
	);

	ret.push_back(ParamDesc(ValueBase(),"specify_start")
		.set_local_name(_("Specify Start"))
		.set_description(_("When checked, 'Start Color' is used as the start of the resulting gradient"))
	);

	ret.push_back(ParamDesc(ValueBase(),"specify_end")
		.set_local_name(_("Specify End"))
		.set_description(_("When checked, 'End Color' is used as the start of the resulting gradient"))
	);

	ret.push_back(ParamDesc(ValueBase(),"start_color")
		.set_local_name(_("Start Color"))
		.set_description(_("Used as the start of the resulting gradient"))
	);

	ret.push_back(ParamDesc(ValueBase(),"end_color")
		.set_local_name(_("End Color"))
		.set_description(_("Used as the end of the resulting gradient"))
	);

	return ret;
}
