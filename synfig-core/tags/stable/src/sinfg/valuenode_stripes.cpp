/* === S I N F G =========================================================== */
/*!	\file valuenode_subtract.cpp
**	\brief Template File
**
**	$Id: valuenode_stripes.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
#include "valuenode_stripes.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "color.h"
#include "gradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

sinfg::ValueNode_Stripes::ValueNode_Stripes():LinkableValueNode(sinfg::ValueBase::TYPE_GRADIENT)
{
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
	ValueBase::Type id(x.get_type());
	
	if(id!=ValueBase::TYPE_GRADIENT)
	{
		assert(0);
		throw runtime_error("sinfg::ValueNode_Stripes:Bad type "+ValueBase::type_name(id));			
	}		

	ValueNode_Stripes* value_node=new ValueNode_Stripes();

	assert(value_node->get_type()==id);
	
	return value_node;
}

sinfg::ValueNode_Stripes::~ValueNode_Stripes()
{
	unlink_all();
}

bool
sinfg::ValueNode_Stripes::set_color1(ValueNode::Handle a)
{
	if(a->get_type()!=ValueBase::TYPE_COLOR)
		return false;

	color1_=a;

	return true;
}

bool
sinfg::ValueNode_Stripes::set_color2(ValueNode::Handle a)
{
	if(a->get_type()!=ValueBase::TYPE_COLOR)
		return false;

	color2_=a;

	return true;
}

bool
sinfg::ValueNode_Stripes::set_width(ValueNode::Handle x)
{
	if(x->get_type()!=ValueBase::TYPE_REAL)
		return false;

	width_=x;

	return true;
}

bool
sinfg::ValueNode_Stripes::set_stripes(ValueNode::Handle b)
{
	if(b->get_type()!=ValueBase::TYPE_INTEGER)
		return false;
	stripes_=b;
	return true;
}

sinfg::ValueBase
sinfg::ValueNode_Stripes::operator()(Time t)const
{
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
ValueNode_Stripes::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:
			if(set_color1(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 1:
			if(set_color2(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 2:
			if(set_stripes(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 3:
			if(set_width(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
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

int
ValueNode_Stripes::link_count()const
{
	return 4;
}

String
ValueNode_Stripes::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:
			return _("Color 1");
		case 1:
			return _("Color 2");
		case 2:
			return _("Stripe Count");
		case 3:
			return _("Width");
	}
	return String();
}	

String
ValueNode_Stripes::link_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
		case 0:
			return "color1";
		case 1:
			return "color2";
		case 2:
			return "stripes";
		case 3:
			return "width";
	}
	return String();
}	

int
ValueNode_Stripes::get_link_index_from_name(const String &name)const
{
	if(name=="color1")
		return 0;
	if(name=="color2")
		return 1;
	if(name=="stripes")
		return 2;
	if(name=="width")
		return 3;
	throw Exception::BadLinkName(name);
}

String
ValueNode_Stripes::get_name()const
{
	return "stripes";
}

String
ValueNode_Stripes::get_local_name()const
{
	return _("Stripes");
}

bool
ValueNode_Stripes::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_GRADIENT;
}
