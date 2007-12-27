/* === S Y N F I G ========================================================= */
/*!	\file valuenode_reciprocal.cpp
**	\brief Implementation of the "Reciprocal" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "valuenode_reciprocal.h"
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

ValueNode_Reciprocal::ValueNode_Reciprocal(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Reciprocal::ValueNode_Reciprocal(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	set_link("link", x);
	set_link("epsilon",  ValueNode_Const::create(Real(0.000001)));
	set_link("infinite", ValueNode_Const::create(Real(999999.0)));
}

ValueNode_Reciprocal*
ValueNode_Reciprocal::create(const ValueBase &x)
{
	return new ValueNode_Reciprocal(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_Reciprocal::create_new()const
{
	return new ValueNode_Reciprocal(get_type());
}

ValueNode_Reciprocal::~ValueNode_Reciprocal()
{
	unlink_all();
}

bool
ValueNode_Reciprocal::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i >= 0 && i < link_count());
	switch(i)
	{
	case 0:  link_     = x; break;
	case 1:  epsilon_  = x; break;
	case 2:  infinite_ = x; break;
	default: return false;
	}

	signal_child_changed()(i);
	signal_value_changed()();
	return true;
}

ValueNode::LooseHandle
ValueNode_Reciprocal::get_link_vfunc(int i)const
{
	assert(i >= 0 && i < link_count());
	if(i==0) return link_;
	if(i==1) return epsilon_;
	if(i==2) return infinite_;

	return 0;
}

int
ValueNode_Reciprocal::link_count()const
{
	return 3;
}

String
ValueNode_Reciprocal::link_local_name(int i)const
{
	assert(i >= 0 && i < link_count());
	if(i==0) return _("Link");
	if(i==1) return _("Epsilon");
	if(i==2) return _("Infinite");
	return String();
}

String
ValueNode_Reciprocal::link_name(int i)const
{
	assert(i >= 0 && i < link_count());
	if(i==0) return "link";
	if(i==1) return "epsilon";
	if(i==2) return "infinite";
	return String();
}

int
ValueNode_Reciprocal::get_link_index_from_name(const String &name)const
{
	if(name=="link")     return 0;
	if(name=="epsilon")  return 1;
	if(name=="infinite") return 2;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_Reciprocal::operator()(Time t)const
{
	Real link     = (*link_)    (t).get(Real());
	Real epsilon  = (*epsilon_) (t).get(Real());
	Real infinite = (*infinite_)(t).get(Real());

	if (abs(link) < epsilon)
		if (link < 0)
			return -infinite;
		else
			return infinite;
	else
		return 1.0f / link;
}

String
ValueNode_Reciprocal::get_name()const
{
	return "reciprocal";
}

String
ValueNode_Reciprocal::get_local_name()const
{
	return _("Reciprocal");
}

bool
ValueNode_Reciprocal::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_REAL;
}
