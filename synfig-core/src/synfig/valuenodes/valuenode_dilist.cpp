/* === S Y N F I G ========================================================= */
/*!	\file valuenode_DIList.cpp
**	\brief Implementation of the "Dash Item List" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include "valuenode_dilist.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "valuenode_bline.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <synfig/dashitem.h>
#include <vector>
#include <list>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_DIList, RELEASE_VERSION_0_63_01, "dilist", "DIList")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


ValueNode_DIList::ValueNode_DIList():
	ValueNode_DynamicList(type_dash_item)
{
}

ValueNode_DIList::~ValueNode_DIList()
{
}

ValueNode_DIList*
ValueNode_DIList::create(const ValueBase &value)
{
	// if the parameter is not a list type, return null
	if(value.get_type()!=type_list)
		return NULL;
	// create an empty list
	ValueNode_DIList* value_node(new ValueNode_DIList());
	// If the value parameter is not empty
	if(!value.empty())
	{
		Type &type(value.get_contained_type());
		if (type == type_dash_item)
		{
			std::vector<DashItem> list(value.get_list_of(DashItem()));
			std::vector<DashItem>::const_iterator iter;

			for(iter=list.begin();iter!=list.end();iter++)
			{
				value_node->add(ValueNode::Handle(ValueNode_Composite::create(*iter)));
			}
			value_node->set_loop(value.get_loop());
		}
		else
		{
			// We got a list of who-knows-what. We don't have any idea
			// what to do with it.
			return NULL;
		}
	}

	return value_node;
}

ValueNode_DIList::ListEntry
ValueNode_DIList::create_list_entry(int index, Time time, Real /*origin*/)
{
	ValueNode_DIList::ListEntry ret;
	synfig::DashItem inserted;
	int new_index;
	if(link_count())
	{
		new_index=find_prev_valid_entry(index, time);
		ret.index=new_index;
	}
	else
	{
		ret.index=index;
	}
	ret.set_parent_value_node(this);
	ret.value_node=ValueNode_Composite::create(inserted);
	ret.value_node->set_parent_canvas(get_parent_canvas());
	return ret;
}

ValueBase
ValueNode_DIList::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<DashItem> ret_list;

	std::vector<ListEntry>::const_iterator iter;
	bool rising;

	DashItem curr;

	// go through all the list's entries
	for(iter=list.begin();iter!=list.end();++iter)
	{
		// how 'on' is this dashitem?
		float amount(iter->amount_at_time(t,&rising));
		assert(amount>=0.0f);
		assert(amount<=1.0f);
		// we store the current dash item
		curr=(*iter->value_node)(t).get(curr);
		// it's fully on
		if (amount > 1.0f - 0.0000001f)
		{
			// push back to the returning list
			ret_list.push_back(curr);
		}
	}
	if(list.empty())
		synfig::warning(string("ValueNode_DIList::operator()():")+_("No entries in list"));
	else
	if(ret_list.empty())
		synfig::warning(string("ValueNode_DIList::operator()():")+_("No entries in ret_list"));

	return ValueBase(ret_list,get_loop());
}

String
ValueNode_DIList::link_local_name(int i)const
{
	assert(i>=0 && (unsigned)i<list.size());
	return etl::strprintf(_("DashItem %03d"),i+1);
}



LinkableValueNode*
ValueNode_DIList::create_new()const
{
	return new ValueNode_DIList();
}

bool
ValueNode_DIList::check_type(Type &type)
{
	return type==type_list;
}

ValueNode::LooseHandle
ValueNode_DIList::get_bline()const
{
	return bline_;
}

void
ValueNode_DIList::set_bline(ValueNode::Handle b)
{
	bline_=b;
}

