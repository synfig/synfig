/* === S Y N F I G ========================================================= */
/*!	\file valuenode_timestring.cpp
**	\brief Implementation of the "TimeString" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "valuenode_timestring.h"
#include "valuenode_const.h"
#include <synfig/canvas.h>
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

REGISTER_VALUENODE(ValueNode_TimeString, RELEASE_VERSION_0_61_09, "timestring", "Time String")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_TimeString::ValueNode_TimeString(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_string)
		set_link("time",ValueNode_Const::create(Time(0)));
	else
		throw Exception::BadType(value.get_type().description.local_name);
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
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Time time((*time_)(t).get(Time()));

	if (get_type() == type_string)
	{
		if (get_root_canvas())
			return time.get_string(get_root_canvas()->rend_desc().get_frame_rate());
		else
			return time.get_string();
	}

	assert(0);
	return ValueBase();
}



bool
ValueNode_TimeString::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(time_, type_time);
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

bool
ValueNode_TimeString::check_type(Type &type)
{
	return
		type==type_string;
}

LinkableValueNode::Vocab
ValueNode_TimeString::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"time")
		.set_local_name(_("Time"))
		.set_description(_("The time that is converted to string"))
	);

	return ret;
}
