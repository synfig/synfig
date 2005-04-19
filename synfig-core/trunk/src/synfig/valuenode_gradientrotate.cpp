/* === S I N F G =========================================================== */
/*!	\file valuenode_gradientrotate.cpp
**	\brief Template File
**
**	$Id: valuenode_gradientrotate.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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
#include "valuenode_gradientrotate.h"
#include "valuenode_const.h"
#include <stdexcept>
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

sinfg::ValueNode_GradientRotate::ValueNode_GradientRotate():
	LinkableValueNode(sinfg::ValueBase::TYPE_GRADIENT)
{
	set_link("gradient",ValueNode_Const::create(Gradient()));
	set_link("offset",ValueNode_Const::create(Real(0)));
	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_GradientRotate::create_new()const
{
	return new ValueNode_GradientRotate();
}

ValueNode_GradientRotate*
ValueNode_GradientRotate::create(const ValueBase& x)
{
	ValueBase::Type id(x.get_type());
	if(id!=ValueBase::TYPE_GRADIENT)
	{
		assert(0);
		throw runtime_error("sinfg::ValueNode_GradientRotate:Bad type "+ValueBase::type_name(id));			
	}		

	ValueNode_GradientRotate* value_node=new ValueNode_GradientRotate();
	value_node->set_gradient(ValueNode_Const::create(x.get(Gradient())));

	assert(value_node->get_type()==id);
	
	return value_node;
}

sinfg::ValueNode_GradientRotate::~ValueNode_GradientRotate()
{
	unlink_all();
}

bool
sinfg::ValueNode_GradientRotate::set_gradient(ValueNode::Handle a)
{
	if(a->get_type()!=ValueBase::TYPE_GRADIENT&& !PlaceholderValueNode::Handle::cast_dynamic(a))
		return false;

	ref_gradient=a;

	return true;
}

bool
sinfg::ValueNode_GradientRotate::set_offset(ValueNode::Handle b)
{
	if(b->get_type()!=ValueBase::TYPE_REAL&& !PlaceholderValueNode::Handle::cast_dynamic(b))
		return false;
	ref_offset=b;
	return true;
}

sinfg::ValueBase
sinfg::ValueNode_GradientRotate::operator()(Time t)const
{
	Gradient gradient;
	gradient=(*ref_gradient)(t).get(gradient);
	Real offset((*ref_offset)(t).get(Real()));
	Gradient::iterator iter;
	for(iter=gradient.begin();iter!=gradient.end();++iter)
		iter->pos+=offset;
	
	return gradient;
}

bool
ValueNode_GradientRotate::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			if(set_gradient(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
		case 1:
			if(set_offset(x)) { signal_child_changed()(i);signal_value_changed()(); return true; }
			else { return false; }
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_GradientRotate::get_link_vfunc(int i)const
{
	assert(i>=0 && i<3);
	switch(i)
	{
		case 0:
			return ref_gradient;
		case 1:
			return ref_offset;
	}
	return 0;
}

int
ValueNode_GradientRotate::link_count()const
{
	return 2;
}

String
ValueNode_GradientRotate::link_local_name(int i)const
{
	assert(i>=0 && i<2);
	switch(i)
	{
		case 0:
			return _("Gradient");
		case 1:
			return _("Offset");
	}
	return String();
}	

String
ValueNode_GradientRotate::link_name(int i)const
{
	assert(i>=0 && i<2);
	switch(i)
	{
		case 0:
			return "gradient";
		case 1:
			return "offset";
	}
	return String();
}	

int
ValueNode_GradientRotate::get_link_index_from_name(const String &name)const
{
	if(name=="gradient")
		return 0;
	if(name=="offset")
		return 1;
	throw Exception::BadLinkName(name);
}

String
ValueNode_GradientRotate::get_name()const
{
	return "gradient_rotate";
}

String
ValueNode_GradientRotate::get_local_name()const
{
	return _("Gradient Rotate");
}

bool
ValueNode_GradientRotate::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_GRADIENT;
}
