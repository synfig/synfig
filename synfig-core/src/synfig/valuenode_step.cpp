/* === S Y N F I G ========================================================= */
/*!	\file valuenode_step.cpp
**	\brief Implementation of the "Step" valuenode conversion.
**
**	$Id$
**
**	\legal
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

#include "valuenode_step.h"
#include "valuenode_const.h"
#include "general.h"
#include "color.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Step::ValueNode_Step(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("duration",     ValueNode_Const::create(Time(1)));
	set_link("start_time",   ValueNode_Const::create(Time(0)));
	set_link("intersection", ValueNode_Const::create(Real(0.5)));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("link",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("link",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("link",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("link",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("link",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("link",ValueNode_Const::create(value.get(Vector())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(get_type()));
	}
}

LinkableValueNode*
ValueNode_Step::create_new()const
{
	return new ValueNode_Step(get_type());
}

ValueNode_Step*
ValueNode_Step::create(const ValueBase &x)
{
	return new ValueNode_Step(x);
}

ValueNode_Step::~ValueNode_Step()
{
	unlink_all();
}

ValueBase
ValueNode_Step::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Time duration    ((*duration_    )(t).get(Time()));
	Time start_time  ((*start_time_  )(t).get(Time()));
	Real intersection((*intersection_)(t).get(Real()));

	t = (floor((t - start_time) / duration) + intersection) * duration + start_time;

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:   return (*link_)(t).get( Angle());
	case ValueBase::TYPE_COLOR:   return (*link_)(t).get( Color());
	case ValueBase::TYPE_INTEGER: return (*link_)(t).get(   int());
	case ValueBase::TYPE_REAL:    return (*link_)(t).get(  Real());
	case ValueBase::TYPE_TIME:    return (*link_)(t).get(  Time());
	case ValueBase::TYPE_VECTOR:  return (*link_)(t).get(Vector());
	default:
		assert(0);
		return ValueBase();
	}
}


String
ValueNode_Step::get_name()const
{
	return "step";
}

String
ValueNode_Step::get_local_name()const
{
	return _("Step");
}

bool
ValueNode_Step::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_ANGLE		||
		type==ValueBase::TYPE_COLOR		||
		type==ValueBase::TYPE_INTEGER	||
		type==ValueBase::TYPE_REAL		||
		type==ValueBase::TYPE_TIME		||
		type==ValueBase::TYPE_VECTOR	;
}

bool
ValueNode_Step::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,         get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(duration_,     ValueBase::TYPE_TIME);
	case 2: CHECK_TYPE_AND_SET_VALUE(start_time_,   ValueBase::TYPE_TIME);
	case 3: CHECK_TYPE_AND_SET_VALUE(intersection_, ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Step::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_;
	case 1: return duration_;
	case 2: return start_time_;
	case 3: return intersection_;
	default:
		return 0;
	}
}

LinkableValueNode::Vocab
ValueNode_Step::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node used to make the step"))
	);

	ret.push_back(ParamDesc(ValueBase(),"duration")
		.set_local_name(_("Duration"))
		.set_description(_("The duration of the step"))
	);

	ret.push_back(ParamDesc(ValueBase(),"start_time")
		.set_local_name(_("Start Time"))
		.set_description(_("The time when the step conversion starts"))
	);

		ret.push_back(ParamDesc(ValueBase(),"intersection")
		.set_local_name(_("Intersection"))
		.set_description(_("Value that define whether the step is centered on the value [0,1]"))
	);

	return ret;
}
