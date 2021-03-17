/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timeloop.cpp
**	\brief Implementation of the "Time Loop" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_timeloop.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_TimeLoop, RELEASE_VERSION_0_61_08, "timeloop", "Time Loop")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_TimeLoop::ValueNode_TimeLoop(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_TimeLoop::ValueNode_TimeLoop(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("link", x);
	set_link("link_time",  ValueNode_Const::create(Time(0)));
	set_link("local_time", ValueNode_Const::create(Time(0)));
	set_link("duration",   ValueNode_Const::create(Time(1)));
}

ValueNode_TimeLoop*
ValueNode_TimeLoop::create(const ValueBase &x)
{
	return new ValueNode_TimeLoop(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_TimeLoop::create_new()const
{
	return new ValueNode_TimeLoop(get_type());
}

ValueNode_TimeLoop::~ValueNode_TimeLoop()
{
	unlink_all();
}

bool
ValueNode_TimeLoop::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,       get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(link_time_,  type_time);
	case 2: CHECK_TYPE_AND_SET_VALUE(local_time_, type_time);
	case 3: CHECK_TYPE_AND_SET_VALUE(duration_,   type_time);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TimeLoop::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	if(i==1) return link_time_;
	if(i==2) return local_time_;
	if(i==3) return duration_;

	return 0;
}

ValueBase
ValueNode_TimeLoop::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Time link_time  = (*link_time_) (t).get(Time());
	Time local_time = (*local_time_)(t).get(Time());
	Time duration   = (*duration_)  (t).get(Time());

 	if (duration == 0)
		t = link_time;
	else if (duration > 0)
	{
		t -= local_time;
		t -= floor(t / duration) * duration;
		t  = link_time + t;
	}
	else
	{
		duration = -duration;
		t -= local_time;
		t -= floor(t / duration) * duration;
		t  = link_time - t;
	}

	return (*link_)(t);
}



bool
ValueNode_TimeLoop::check_type(Type &type)
{
	if(type != type_nil)
		return true;
	return false;
}


LinkableValueNode::Vocab
ValueNode_TimeLoop::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("link")
		.set_local_name(_("Link"))
		.set_description(_("The value node to time loop"))
	);

	ret.push_back(ParamDesc("link_time")
		.set_local_name(_("Link Time"))
		.set_description(_("Start time of the loop for the value node Timeline"))
	);

	ret.push_back(ParamDesc("local_time")
		.set_local_name(_("Local Time"))
		.set_description(_("The time when the resulted loop starts"))
	);

	ret.push_back(ParamDesc("duration")
		.set_local_name(_("Duration"))
		.set_description(_("Length of the loop"))
	);
	return ret;
}

LinkableValueNode::InvertibleStatus ValueNode_TimeLoop::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (target_value.get_type() != get_link("link")->get_type())
		return INVERSE_ERROR_BAD_TYPE;
	if (link_index)
		*link_index = get_link_index_from_name("link");
	return INVERSE_OK;
}

ValueBase ValueNode_TimeLoop::get_inverse(const Time& /*t*/, const ValueBase& target_value) const
{
	return target_value;
}
