/* === S Y N F I G ========================================================= */
/*!	\file valuenode_exp.cpp
**	\brief Implementation of the "Exponential" valuenode conversion.
**
**	$Id$
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

#include "valuenode_exp.h"
#include "valuenode_const.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Exp::ValueNode_Exp(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_REAL:
		set_link("exp",ValueNode_Const::create(Real(1)));
		set_link("scale",ValueNode_Const::create(value.get(Real())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_Exp::create_new()const
{
	return new ValueNode_Exp(get_type());
}

ValueNode_Exp*
ValueNode_Exp::create(const ValueBase &x)
{
	return new ValueNode_Exp(x);
}

ValueNode_Exp::~ValueNode_Exp()
{
	unlink_all();
}

ValueBase
ValueNode_Exp::operator()(Time t)const
{
	return (exp((*exp_)(t).get(Real())) *
			(*scale_)(t).get(Real()));
}

String
ValueNode_Exp::get_name()const
{
	return "exp";
}

String
ValueNode_Exp::get_local_name()const
{
	return _("Exponential");
}

bool
ValueNode_Exp::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());

	if(i==0)
	{
		exp_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	if(i==1)
	{
		scale_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Exp::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return exp_;
	if(i==1)
		return scale_;

	return 0;
}

int
ValueNode_Exp::link_count()const
{
	return 2;
}

String
ValueNode_Exp::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return "exp";
	if(i==1)
		return "scale";
	return String();
}

String
ValueNode_Exp::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return _("Exponent");
	if(i==1)
		return _("Scale");
	return String();
}

int
ValueNode_Exp::get_link_index_from_name(const String &name)const
{
	if(name=="exp")
		return 0;
	if(name=="scale")
		return 1;

	throw Exception::BadLinkName(name);
}

bool
ValueNode_Exp::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_REAL;
}
