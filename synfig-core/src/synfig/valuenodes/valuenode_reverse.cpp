/* === S Y N F I G ========================================================= */
/*!	\file ValueNode_Reverse.cpp
**	\brief Implementation of the "Reverse" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
**	Copyright (c) 2013 Konstantin Dmitriev
**	Copyright (c) 2015 Max May
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

#include "valuenode_reverse.h"
#include <synfig/blinepoint.h>

#include "valuenode_bline.h"
#include "valuenode_dilist.h"
#include "valuenode_wplist.h"
#include "valuenode_dynamiclist.h"

#include <synfig/general.h>
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

ValueNode_Reverse::ValueNode_Reverse(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Reverse::ValueNode_Reverse(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	if(x.get_type() != type_list)
	{
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+x.get_type().description.local_name);
	}
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(x.get_contained_type());
	if(type == type_bline_point)
		set_link("link", ValueNode_BLine::create(x));
	else if(type == type_dash_item)
		set_link("link", ValueNode_DIList::create(x));
	else if(type == type_width_point)
		set_link("link", ValueNode_WPList::create(x));
	else
		set_link("link", ValueNode_DynamicList::create_from(x));
}

ValueNode_Reverse*
ValueNode_Reverse::create(const ValueBase &x)
{
	return new ValueNode_Reverse(x);
}

LinkableValueNode*
ValueNode_Reverse::create_new()const
{
	return new ValueNode_Reverse(get_type());
}

ValueNode_Reverse::~ValueNode_Reverse()
{
	unlink_all();
}

bool
ValueNode_Reverse::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_, type_list);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Reverse::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;

	return 0;
}

ValueBase
ValueNode_Reverse::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return (*link_)(t);
}



String
ValueNode_Reverse::get_name()const
{
	return "reverse";
}

String
ValueNode_Reverse::get_local_name()const
{
	return _("Reverse");
}

bool
ValueNode_Reverse::check_type(Type &type __attribute__ ((unused)))
{
	return type == type_list;
}

LinkableValueNode::Vocab
ValueNode_Reverse::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The list to be reversed"))
	);

	return ret;
}
