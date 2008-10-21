/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timestring.cpp
**	\brief Implementation of the "TimeString" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "valuenode_timestring.h"
#include "valuenode_const.h"
#include "canvas.h"
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

ValueNode_TimeString::ValueNode_TimeString(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	switch(value.get_type())
	{
	case ValueBase::TYPE_STRING:
		set_link("time",ValueNode_Const::create(Time(0)));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_TimeString::create_new()const
{
	return new ValueNode_TimeString(get_type());
}

ValueNode_TimeString*
ValueNode_TimeString::create(const ValueBase &x)
{
	return new ValueNode_TimeString(x);
}

ValueNode_TimeString::~ValueNode_TimeString()
{
	unlink_all();
}

ValueBase
ValueNode_TimeString::operator()(Time t)const
{
	Time time((*time_)(t).get(Time()));

	switch (get_type())
	{
	case ValueBase::TYPE_STRING:
		if (get_root_canvas())
			return time.get_string(get_root_canvas()->rend_desc().get_frame_rate());
		else
			return time.get_string();
	default:
		break;
	}

	assert(0);
	return ValueBase();
}

String
ValueNode_TimeString::get_name()const
{
	return "timestring";
}

String
ValueNode_TimeString::get_local_name()const
{
	return _("Time String");
}

bool
ValueNode_TimeString::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(time_, ValueBase::TYPE_TIME);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TimeString::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return time_;
	}

	return 0;
}

int
ValueNode_TimeString::link_count()const
{
	return 1;
}

String
ValueNode_TimeString::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "time";
	}
	return String();
}

String
ValueNode_TimeString::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("Time");
	}
	return String();
}

int
ValueNode_TimeString::get_link_index_from_name(const String &name)const
{
	if (name=="time") return 0;

	throw Exception::BadLinkName(name);
}

bool
ValueNode_TimeString::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_STRING;
}
