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
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <ETL/misc>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Step, RELEASE_VERSION_0_61_08, "step", "Step")

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

	Type &type(get_type());
	if (type == type_angle)
		set_link("link",ValueNode_Const::create(value.get(Angle())));
	else
	if (type == type_color)
		set_link("link",ValueNode_Const::create(value.get(Color())));
	else
	if (type == type_integer)
		set_link("link",ValueNode_Const::create(value.get(int())));
	else
	if (type == type_real)
		set_link("link",ValueNode_Const::create(value.get(Real())));
	else
	if (type == type_time)
		set_link("link",ValueNode_Const::create(value.get(Time())));
	else
	if (type == type_vector)
		set_link("link",ValueNode_Const::create(value.get(Vector())));
	else
		throw Exception::BadType(get_type().description.local_name);
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

	Type &type(get_type());
	if (type == type_angle)   return (*link_)(t).get( Angle());
	if (type == type_color)   return (*link_)(t).get( Color());
	if (type == type_integer) return (*link_)(t).get(   int());
	if (type == type_real)    return (*link_)(t).get(  Real());
	if (type == type_time)    return (*link_)(t).get(  Time());
	if (type == type_vector)  return (*link_)(t).get(Vector());

	assert(0);
	return ValueBase();
}




bool
ValueNode_Step::check_type(Type &type)
{
	return
		type==type_angle	||
		type==type_color	||
		type==type_integer	||
		type==type_real		||
		type==type_time		||
		type==type_vector	;
}

bool
ValueNode_Step::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,         get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(duration_,     type_time);
	case 2: CHECK_TYPE_AND_SET_VALUE(start_time_,   type_time);
	case 3: CHECK_TYPE_AND_SET_VALUE(intersection_, type_real);
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
