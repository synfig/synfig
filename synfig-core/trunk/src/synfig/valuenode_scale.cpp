/* === S I N F G =========================================================== */
/*!	\file valuenode_scale.cpp
**	\brief Template File
**
**	$Id: valuenode_scale.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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
#include "valuenode_scale.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <cassert>
#include "color.h"
#include "vector.h"
#include "time.h"
#include "angle.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Scale::ValueNode_Scale():LinkableValueNode(sinfg::ValueBase::TYPE_NIL)
{
	set_scalar(1.0);
}

ValueNode_Scale*
ValueNode_Scale::create(const ValueBase& x)
{
	ValueNode_Scale* value_node;
	switch(x.get_type())
	{
	case ValueBase::TYPE_VECTOR:
	case ValueBase::TYPE_REAL:
	case ValueBase::TYPE_TIME:
	case ValueBase::TYPE_INTEGER:
	case ValueBase::TYPE_ANGLE:
	case ValueBase::TYPE_COLOR:
		value_node=new ValueNode_Scale();
		if(!value_node->set_value_node(ValueNode_Const::create(x)))
			return 0;
		assert(value_node->get_value_node()->get_type()==x.get_type());
		break;
	default:
		assert(0);
		throw runtime_error("sinfg::ValueNode_Scale:Bad type "+ValueBase::type_name(x.get_type()));			
	}
	assert(value_node);
	assert(value_node->get_type()==x.get_type());
	
	return value_node;
}

LinkableValueNode*
ValueNode_Scale::create_new()const
{
	return new ValueNode_Scale();
}

sinfg::ValueNode_Scale::~ValueNode_Scale()
{
	unlink_all();
}

void
ValueNode_Scale::set_scalar(Real x)
{
	set_link("scalar",ValueNode::Handle(ValueNode_Const::create(x)));
}

bool
ValueNode_Scale::set_scalar(const ValueNode::Handle &x)
{
	if(!x
		|| x->get_type()!=ValueBase::TYPE_REAL
		&& !PlaceholderValueNode::Handle::cast_dynamic(x)
	)
		return false;
	scalar=x;
	return true;
}

ValueNode::Handle
ValueNode_Scale::get_scalar()const
{
	return scalar;
}

bool
ValueNode_Scale::set_value_node(const ValueNode::Handle &x)
{
	if(!x
		|| ( get_type()==ValueBase::TYPE_NIL
			&& !check_type(x->get_type()) )
		|| ( get_type()!=ValueBase::TYPE_NIL
			&& x->get_type()!=get_type() ) &&
		!PlaceholderValueNode::Handle::cast_dynamic(x)
	)
		return false;

	assert(!(PlaceholderValueNode::Handle::cast_dynamic(x) && !get_type()));

	value_node=x;

	if(!get_type())
		set_type(x->get_type());

	return true;
}

ValueNode::Handle
ValueNode_Scale::get_value_node()const
{
	return value_node;
}


sinfg::ValueBase
sinfg::ValueNode_Scale::operator()(Time t)const
{
	if(!value_node || !scalar)
		throw runtime_error(strprintf("ValueNode_Scale: %s",_("One or both of my parameters aren't set!")));
	else
	if(get_type()==ValueBase::TYPE_VECTOR)
		return (*value_node)(t).get(Vector())*(*scalar)(t).get(Real());
	else
	if(get_type()==ValueBase::TYPE_REAL)
		return (*value_node)(t).get(Real())*(*scalar)(t).get(Real());
	else
	if(get_type()==ValueBase::TYPE_TIME)
		return (*value_node)(t).get(Time())*(*scalar)(t).get(Time());
	else
	if(get_type()==ValueBase::TYPE_INTEGER)
		return (*value_node)(t).get(int())*(*scalar)(t).get(Real());
	else
	if(get_type()==ValueBase::TYPE_ANGLE)
		return (*value_node)(t).get(Angle())*(*scalar)(t).get(Real());
	else
	if(get_type()==ValueBase::TYPE_COLOR)
	{
		Color ret((*value_node)(t).get(Color()));
		Real s((*scalar)(t).get(Real()));
		ret.set_r(ret.get_r()*s);
		ret.set_g(ret.get_r()*s);
		ret.set_b(ret.get_r()*s);
		return ret;
	}

	assert(0);
	return ValueBase();
}


bool
ValueNode_Scale::set_link_vfunc(int i,ValueNode::Handle x)
{
	if(!(i==0 || i==1))
		return false;
	
	if(i==0 && !set_value_node(x))
		return false;
	else
	if(i==1 && !set_scalar(x))
		return false;
	
	signal_child_changed()(i);signal_value_changed()();

	return true;
}

ValueNode::LooseHandle
ValueNode_Scale::get_link_vfunc(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return value_node;
	else if(i==1)
		return scalar;
	return 0;
}

int
ValueNode_Scale::link_count()const
{
	return 2;
}

String
ValueNode_Scale::link_local_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return _("Link");
	else if(i==1)
		return _("Scalar");
	return String();
}	

String
ValueNode_Scale::link_name(int i)const
{
	assert(i==0 || i==1);
	if(i==0)
		return "link";
	else if(i==1)
		return "scalar";
	return String();
}

int
ValueNode_Scale::get_link_index_from_name(const String &name)const
{
	if(name=="link")
		return 0;
	if(name=="scalar")
		return 1;
	
	throw Exception::BadLinkName(name);
}

String
ValueNode_Scale::get_name()const
{
	return "scale";
}

String
ValueNode_Scale::get_local_name()const
{
	return _("Scale");
}

bool
ValueNode_Scale::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_VECTOR ||
		type==ValueBase::TYPE_REAL ||
		type==ValueBase::TYPE_INTEGER ||
		type==ValueBase::TYPE_COLOR ||
		type==ValueBase::TYPE_ANGLE;
}
