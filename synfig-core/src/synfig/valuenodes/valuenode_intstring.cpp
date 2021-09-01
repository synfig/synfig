/* === S Y N F I G ========================================================= */
/*!	\file valuenode_intstring.cpp
**	\brief Implementation of the "IntString" valuenode conversion.
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

#include "valuenode_intstring.h"
#include "valuenode_const.h"
#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_IntString, RELEASE_VERSION_0_61_09, "intstring", "Int String")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_IntString::ValueNode_IntString(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_string)
	{
		set_link("int",ValueNode_Const::create(int(0)));
		set_link("width",ValueNode_Const::create(int(0)));
		set_link("zero_pad",ValueNode_Const::create(bool(false)));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_IntString::create_new()const
{
	return new ValueNode_IntString(get_type());
}

ValueNode_IntString*
ValueNode_IntString::create(const ValueBase &x)
{
	return new ValueNode_IntString(x);
}

ValueNode_IntString::~ValueNode_IntString()
{
	unlink_all();
}

ValueBase
ValueNode_IntString::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	int integer((*int_)(t).get(int()));
	int width((*width_)(t).get(int()));
	int zero_pad((*zero_pad_)(t).get(bool()));

	if (get_type() == type_string)
		return strprintf(strprintf("%%%s%dd",
								   zero_pad ? "0" : "",
								   width).c_str(), integer);

	assert(0);
	return ValueBase();
}



bool
ValueNode_IntString::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(int_, type_integer);
	case 1: CHECK_TYPE_AND_SET_VALUE(width_, type_integer);
	case 2: CHECK_TYPE_AND_SET_VALUE(zero_pad_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_IntString::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return int_;
	case 1: return width_;
	case 2: return zero_pad_;
	}

	return 0;
}

bool
ValueNode_IntString::check_type(Type &type)
{
	return
		type==type_string;
}

LinkableValueNode::Vocab
ValueNode_IntString::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"int")
		.set_local_name(_("Int"))
		.set_description(_("Value to convert to string"))
	);

	ret.push_back(ParamDesc(ValueBase(),"width")
		.set_local_name(_("Width"))
		.set_description(_("Width of the string"))
	);

	ret.push_back(ParamDesc(ValueBase(),"zero_pad")
		.set_local_name(_("Zero Padded"))
		.set_description(_("When checked, the string is left filled with zeros to match the width"))
	);

	return ret;
}
