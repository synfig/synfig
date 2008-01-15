/* === S Y N F I G ========================================================= */
/*!	\file valuenode_duplicate.cpp
**	\brief Implementation of the "Duplicate" valuenode conversion.
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

#include "valuenode_duplicate.h"
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

ValueNode_Duplicate::ValueNode_Duplicate(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Duplicate::ValueNode_Duplicate(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	set_link("from", ValueNode_Const::create(Real(1.0)));
	set_link("to",   ValueNode_Const::create(x.get(Real())));
	set_link("step", ValueNode_Const::create(Real(1.0)));
	index = 1.0;
}

ValueNode_Duplicate*
ValueNode_Duplicate::create(const ValueBase &x)
{
	return new ValueNode_Duplicate(x);
}

LinkableValueNode*
ValueNode_Duplicate::create_new()const
{
	return new ValueNode_Duplicate(get_type());
}

ValueNode_Duplicate::~ValueNode_Duplicate()
{
	unlink_all();
}

bool
ValueNode_Duplicate::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(from_, ValueBase::TYPE_REAL);
	case 1: CHECK_TYPE_AND_SET_VALUE(to_,   ValueBase::TYPE_REAL);
	case 2: CHECK_TYPE_AND_SET_VALUE(step_, ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Duplicate::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return from_;
	if(i==1) return to_;
	if(i==2) return step_;

	return 0;
}

int
ValueNode_Duplicate::link_count()const
{
	return 3;
}

String
ValueNode_Duplicate::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return _("From");
	if(i==1) return _("To");
	if(i==2) return _("Step");
	return String();
}

String
ValueNode_Duplicate::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return "from";
	if(i==1) return "to";
	if(i==2) return "step";
	return String();
}

int
ValueNode_Duplicate::get_link_index_from_name(const String &name)const
{
	if(name=="from") return 0;
	if(name=="to")   return 1;
	if(name=="step") return 2;

	throw Exception::BadLinkName(name);
}

void
ValueNode_Duplicate::reset_index(Time t)const
{
	Real from = (*from_)(t).get(Real());
	index = from;
}

bool
ValueNode_Duplicate::step(Time t)const
{
	Real from = (*from_)(t).get(Real());
	Real to   = (*to_  )(t).get(Real());
	Real step = (*step_)(t).get(Real());
	Real prev = index;

	if (step == 0) return false;

	step = abs(step);

	if (from < to)
	{
		if ((index += step) <= to) return true;
	}
	else
		if ((index -= step) >= to) return true;

	// at the end of the loop, leave the index at the last value that was used
	index = prev;
	return false;
}

int
ValueNode_Duplicate::count_steps(Time t)const
{
	Real from = (*from_)(t).get(Real());
	Real to   = (*to_  )(t).get(Real());
	Real step = (*step_)(t).get(Real());

	if (step == 0) return 1;

	return abs((from - to) / step) + 1;
}

ValueBase
ValueNode_Duplicate::operator()(Time t)const
{
	return index;
}

String
ValueNode_Duplicate::get_name()const
{
	return "duplicate";
}

String
ValueNode_Duplicate::get_local_name()const
{
	return _("Duplicate");
}

bool
ValueNode_Duplicate::check_type(ValueBase::Type type)
{
	// never offer this as a choice.  it's used automatically by the 'Duplicate' layer.
	return false;
}
