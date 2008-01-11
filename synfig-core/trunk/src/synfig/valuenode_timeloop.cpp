/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timeloop.cpp
**	\brief Implementation of the "Time Loop" valuenode conversion.
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

#include "valuenode_timeloop.h"
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

ValueNode_TimeLoop::ValueNode_TimeLoop(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_TimeLoop::ValueNode_TimeLoop(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
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
	case 1: CHECK_TYPE_AND_SET_VALUE(link_time_,  ValueBase::TYPE_TIME);
	case 2: CHECK_TYPE_AND_SET_VALUE(local_time_, ValueBase::TYPE_TIME);
	case 3: CHECK_TYPE_AND_SET_VALUE(duration_,   ValueBase::TYPE_TIME);
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

int
ValueNode_TimeLoop::link_count()const
{
	return 4;
}

String
ValueNode_TimeLoop::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return _("Link");
	if(i==1) return _("Link Time");
	if(i==2) return _("Local Time");
	if(i==3) return _("Duration");
	return String();
}

String
ValueNode_TimeLoop::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return "link";
	if(i==1) return "link_time";
	if(i==2) return "local_time";
	if(i==3) return "duration";
	return String();
}

int
ValueNode_TimeLoop::get_link_index_from_name(const String &name)const
{
	if(name=="link")       return 0;
	if(name=="link_time")  return 1;
	if(name=="local_time") return 2;
	if(name=="duration")   return 3;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_TimeLoop::operator()(Time t)const
{
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

String
ValueNode_TimeLoop::get_name()const
{
	return "timeloop";
}

String
ValueNode_TimeLoop::get_local_name()const
{
	return _("Time Loop");
}

bool
ValueNode_TimeLoop::check_type(ValueBase::Type type)
{
	if(type)
		return true;
	return false;
}
