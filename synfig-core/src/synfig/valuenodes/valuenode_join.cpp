/* === S Y N F I G ========================================================= */
/*!	\file valuenode_join.cpp
**	\brief Implementation of the "Join" valuenode conversion.
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

#include "valuenode_join.h"
#include "valuenode_const.h"
#include "valuenode_dynamiclist.h"
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

REGISTER_VALUENODE(ValueNode_Join, RELEASE_VERSION_0_61_09, "join", "Joined List")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Join::ValueNode_Join(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	if (value.get_type() == type_string)
	{
		vector<ValueBase> v(1, value.get(String()));

		// "insert item (smart)" inserts before the selected entry, making it hard to append to the end
		// add an extra element at the end to allow the easy insertion of text after the given value's string
		v.push_back(String());

		set_link("strings",ValueNode_DynamicList::create(v));
		set_link("before",ValueNode_Const::create(String("")));
		set_link("separator",ValueNode_Const::create(String("")));
		set_link("after",ValueNode_Const::create(String("")));
	}
	else
	{
		throw Exception::BadType(value.get_type().description.local_name);
	}
}

LinkableValueNode*
ValueNode_Join::create_new()const
{
	return new ValueNode_Join(get_type());
}

ValueNode_Join*
ValueNode_Join::create(const ValueBase &x)
{
	return new ValueNode_Join(x);
}

ValueNode_Join::~ValueNode_Join()
{
	unlink_all();
}

ValueBase
ValueNode_Join::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const std::vector<ValueBase> strings((*strings_)(t).get_list());
	const String before((*before_)(t).get(String()));
	const String separator((*separator_)(t).get(String()));
	const String after((*after_)(t).get(String()));

	if (get_type() == type_string)
	{
		bool first = true;
		String ret(before);
		for (std::vector<ValueBase>::const_iterator iter = strings.begin(); iter != strings.end(); iter++)
		{
			if (first)
				first = false;
			else
				ret += separator;
			ret += iter->get(String());
		}
		ret += after;
		return ret;
	}

	assert(0);
	return ValueBase();
}



bool
ValueNode_Join::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(strings_, type_list);
	case 1: CHECK_TYPE_AND_SET_VALUE(before_, type_string);
	case 2: CHECK_TYPE_AND_SET_VALUE(separator_, type_string);
	case 3: CHECK_TYPE_AND_SET_VALUE(after_, type_string);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Join::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return strings_;
	case 1: return before_;
	case 2: return separator_;
	case 3: return after_;
	}

	return 0;
}

bool
ValueNode_Join::check_type(Type &type)
{
	return
		type==type_string;
}

LinkableValueNode::Vocab
ValueNode_Join::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"strings")
		.set_local_name(_("Strings"))
		.set_description(_("The List of strings to join"))
	);

	ret.push_back(ParamDesc(ValueBase(),"before")
		.set_local_name(_("Before"))
		.set_description(_("The string to place before the joined strings"))
	);

	ret.push_back(ParamDesc(ValueBase(),"separator")
		.set_local_name(_("Separator"))
		.set_description(_("The string to place between each string joined"))
	);

	ret.push_back(ParamDesc(ValueBase(),"after")
		.set_local_name(_("After"))
		.set_description(_("The string to place after the joined strings"))
	);

	return ret;
}
