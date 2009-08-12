/* === S Y N F I G ========================================================= */
/*!	\file valuenode_not.cpp
**	\brief Implementation of the "Not" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#include "valuenode_not.h"
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

ValueNode_Not::ValueNode_Not(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	bool value(x.get(bool()));

	set_link("link",         ValueNode_Const::create(!value));
}

ValueNode_Not*
ValueNode_Not::create(const ValueBase &x)
{
	return new ValueNode_Not(x);
}

LinkableValueNode*
ValueNode_Not::create_new()const
{
	return new ValueNode_Not(get_type());
}

ValueNode_Not::~ValueNode_Not()
{
	unlink_all();
}

bool
ValueNode_Not::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,     ValueBase::TYPE_BOOL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Not::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	return 0;
}

int
ValueNode_Not::link_count()const
{
	return 1;
}

String
ValueNode_Not::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return _("Link");
	return String();
}

String
ValueNode_Not::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return "link";
	return String();
}

int
ValueNode_Not::get_link_index_from_name(const String &name)const
{
	if(name=="link")     return 0;

	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_Not::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	bool link      = (*link_)    (t).get(bool());

	return !link;
}

String
ValueNode_Not::get_name()const
{
	return "not";
}

String
ValueNode_Not::get_local_name()const
{
	return _("NOT");
}

bool
ValueNode_Not::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_BOOL;
}
