/* === S I N F G =========================================================== */
/*!	\file valuenode_subtract.cpp
**	\brief Template File
**
**	$Id: valuenode_twotone.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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
#include "valuenode_twotone.h"
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

sinfg::ValueNode_TwoTone::ValueNode_TwoTone():LinkableValueNode(sinfg::ValueBase::TYPE_GRADIENT)
{
	set_link("color1",ValueNode_Const::create(Color::black()));
	set_link("color2",ValueNode_Const::create(Color::white()));
	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_TwoTone::create_new()const
{
	return new ValueNode_TwoTone();
}

ValueNode_TwoTone*
ValueNode_TwoTone::create(const ValueBase& x)
{
	ValueBase::Type id(x.get_type());
	if(id!=ValueBase::TYPE_GRADIENT)
	{
		assert(0);
		throw runtime_error("sinfg::ValueNode_TwoTone:Bad type "+ValueBase::type_name(id));			
	}		

	ValueNode_TwoTone* value_node=new ValueNode_TwoTone();

	assert(value_node->get_type()==id);
	
	return value_node;
}

sinfg::ValueNode_TwoTone::~ValueNode_TwoTone()
{
	unlink_all();
}

bool
sinfg::ValueNode_TwoTone::set_lhs(ValueNode::Handle a)
{
	if(a->get_type()!=ValueBase::TYPE_COLOR)
		return false;

	ref_a=a;

	return true;
}

bool
sinfg::ValueNode_TwoTone::set_rhs(ValueNode::Handle b)
{
	if(b->get_type()!=ValueBase::TYPE_COLOR)
		return false;
	ref_b=b;
	return true;
}

sinfg::ValueBase
sinfg::ValueNode_TwoTone::operator()(Time t)const
{
	return Gradient((*ref_a)(t).get(Color()),(*ref_b)(t).get(Color()));
}

bool
ValueNode_TwoTone::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			if(set_lhs(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 1:
			if(set_rhs(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_TwoTone::get_link_vfunc(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			return ref_a;
		case 1:
			return ref_b;
	}
	return 0;
}

int
ValueNode_TwoTone::link_count()const
{
	return 2;
}

String
ValueNode_TwoTone::link_local_name(int i)const
{
	assert(i>=0 && i<2);
	switch(i)
	{
		case 0:
			return _("Color1");
		case 1:
			return _("Color2");
	}
	return String();
}	

String
ValueNode_TwoTone::link_name(int i)const
{
	assert(i>=0 && i<2);
	switch(i)
	{
		case 0:
			return "color1";
		case 1:
			return "color2";
	}
	return String();
}	

int
ValueNode_TwoTone::get_link_index_from_name(const String &name)const
{
	if(name=="color1")
		return 0;
	if(name=="color2")
		return 1;
	throw Exception::BadLinkName(name);
}

String
ValueNode_TwoTone::get_name()const
{
	return "twotone";
}

String
ValueNode_TwoTone::get_local_name()const
{
	return _("Two-Tone");
}

bool
ValueNode_TwoTone::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_GRADIENT;
}
