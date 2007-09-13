/* === S Y N F I G ========================================================= */
/*!	\file valuenode_repeat_gradient.cpp
**	\brief Template File
**
**	$Id: valuenode_repeat_gradient.cpp 604 2007-09-05 14:29:02Z dooglus $
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

#include "general.h"
#include "valuenode_repeat_gradient.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "gradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Repeat_Gradient::ValueNode_Repeat_Gradient():LinkableValueNode(synfig::ValueBase::TYPE_GRADIENT)
{
	set_link("gradient",ValueNode_Const::create(Gradient()));
	set_link("count",count_=ValueNode_Const::create(int(3)));
	set_link("width",ValueNode_Const::create(0.5));
}

LinkableValueNode*
ValueNode_Repeat_Gradient::create_new()const
{
	return new ValueNode_Repeat_Gradient();
}

ValueNode_Repeat_Gradient*
ValueNode_Repeat_Gradient::create(const ValueBase& x)
{
	ValueBase::Type id(x.get_type());

	if(id!=ValueBase::TYPE_GRADIENT)
	{
		assert(0);
		throw runtime_error("synfig::ValueNode_Repeat_Gradient:Bad type "+ValueBase::type_name(id));
	}

	ValueNode_Repeat_Gradient* value_node=new ValueNode_Repeat_Gradient();
	value_node->set_gradient(ValueNode_Const::create(x.get(Gradient())));

	assert(value_node->get_type()==id);

	return value_node;
}

synfig::ValueNode_Repeat_Gradient::~ValueNode_Repeat_Gradient()
{
	unlink_all();
}

bool
synfig::ValueNode_Repeat_Gradient::set_gradient(ValueNode::Handle a)
{
	if(a->get_type()!=ValueBase::TYPE_GRADIENT)
		return false;

	gradient_=a;

	return true;
}

bool
synfig::ValueNode_Repeat_Gradient::set_width(ValueNode::Handle x)
{
	if(x->get_type()!=ValueBase::TYPE_REAL)
		return false;

	width_=x;

	return true;
}

bool
synfig::ValueNode_Repeat_Gradient::set_count(ValueNode::Handle b)
{
	if(b->get_type()!=ValueBase::TYPE_INTEGER)
		return false;
	count_=b;
	return true;
}

synfig::ValueBase
synfig::ValueNode_Repeat_Gradient::operator()(Time t)const
{
	const int count((*count_)(t).get(int()));
	int i;
	Gradient ret;

	if(count<=0)
		return ret;

	const Gradient gradient((*gradient_)(t).get(Gradient()));
	const float width(max(0.0,min(1.0,(*width_)(t).get(Real()))));

	const float gradient_width_a(width/count);
	const float gradient_width_b((1.0-width)/count);

	Gradient::const_iterator iter;
	Gradient::const_reverse_iterator riter;
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
	return ret;
}

bool
ValueNode_Repeat_Gradient::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:
			if(set_gradient(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 1:
			if(set_count(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 2:
			if(set_width(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
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
	    default: return 0;
	}
}

int
ValueNode_Repeat_Gradient::link_count()const
{
	return 3;
}

String
ValueNode_Repeat_Gradient::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:  return _("Gradient");
		case 1:  return _("Count");
		case 2:  return _("Width");
		default: return String();
	}
}

String
ValueNode_Repeat_Gradient::link_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:  return "gradient";
		case 1:  return "count";
		case 2:  return "width";
		default: return String();
	}
}

int
ValueNode_Repeat_Gradient::get_link_index_from_name(const String &name)const
{
	if(name=="gradient") return 0;
	if(name=="count")    return 1;
	if(name=="width")    return 2;
	throw Exception::BadLinkName(name);
}

String
ValueNode_Repeat_Gradient::get_name()const
{
	return "repeat_gradient";
}

String
ValueNode_Repeat_Gradient::get_local_name()const
{
	return _("Repeat Gradient");
}

bool
ValueNode_Repeat_Gradient::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_GRADIENT;
}
