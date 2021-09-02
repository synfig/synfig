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
#include <synfig/segment.h>
#include <synfig/gradient.h>
#include <synfig/blinepoint.h>

#include "valuenode_bline.h"
#include "valuenode_dilist.h"
#include "valuenode_wplist.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_composite.h"
#include "valuenode_const.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <ETL/misc>

#include <algorithm>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Reverse, RELEASE_VERSION_1_0_2, "reverse", "Reverse")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Reverse::ValueNode_Reverse(Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Reverse::ValueNode_Reverse(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(x.get_type());
	if(x.empty()) {
		set_link("link", ValueNode_Const::create(x));
	}
	else
	if(type == type_list)
	{
		Type &c_type(x.get_contained_type());
		if(c_type == type_bline_point)
			set_link("link", ValueNode_BLine::create(x));
		else if(c_type == type_dash_item)
			set_link("link", ValueNode_DIList::create(x));
		else if(c_type == type_width_point)
			set_link("link", ValueNode_WPList::create(x));
		else
			set_link("link", ValueNode_DynamicList::create(x));
	}
	else
	if(ValueNode_Composite::check_type(type)) {
		set_link("link", ValueNode_Composite::create(x));
	}
	else
	{
		set_link("link", ValueNode_Const::create(x));
	}
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
	case 0: CHECK_TYPE_AND_SET_VALUE(link_, get_type());
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
reverse_value(const ValueBase &value)
{
	Type &type(value.get_type());

	if(type == type_list)
	{
		// We'll be writing to this later, so copy it.
		ValueBase v = value;
		const ValueBase::List &list = v.get_list();
		ValueBase::List out;
		Type &c_type(v.get_contained_type());
		if(ValueNode_Reverse::check_type(c_type))
		{
			// This is a "deep" reversal, so reverse the elements of the list, too.

			out.reserve(list.size());

			if(!v.get_loop())
			{
				for(ValueBase::List::const_reverse_iterator it=list.rbegin(),end=list.rend(); it!=end; ++it)
				{
					out.push_back(reverse_value(*it));
				}
			}
			else
			{
				// The reversal of a looped list is rotated to end with the same value as the original.
				// This makes some things work better, e.g. makes an adv. outline come out looking the same
				// after having both its vertices and its width points reversed.
				for(ValueBase::List::const_reverse_iterator it=++list.rbegin(),end=list.rend(); it!=end; ++it)
				{
					out.push_back(reverse_value(*it));
				}
				out.push_back(reverse_value(list.back()));
			}

			if(c_type == type_dash_item)
			{
				// Dash items need to exchange offsets with their neighbors to work right.
				Real prev = out.back().get(DashItem()).get_offset();
				for(ValueBase::List::iterator it=out.begin(),end=out.end(); it!=end; ++it)
				{
					DashItem di = it->get(DashItem());
					Real tmp = di.get_offset();
					di.set_offset(prev);
					prev = tmp;
					it->set(di);
				}
			}
		}
		else
		{
			// The elements aren't reversible. Just copy them.
			out.resize(list.size());
			std::reverse_copy(list.begin(), list.end(), out.begin());
		}
		v.set(out);
		return v;
	}
	else
	if(type == type_string)
	{
		String out = value.get(String());
		std::reverse(out.begin(), out.end());
		return out;
	}
	else
	if(type == type_segment)
	{
		Segment out = value.get(Segment());
		std::swap(out.p1, out.p2);
		std::swap(out.t1, out.t2);
		return out;
	}
	else
	if(type == type_gradient)
	{
		const Gradient &grad = value.get(Gradient());
		Gradient out;
		for(Gradient::const_reverse_iterator it=grad.rbegin(),end=grad.rend(); it!=end; ++it)
		{
			out.push_back(GradientCPoint(1-it->pos, it->color));
		}
		return out;
	}
	else
	if(type == type_bline_point)
	{
		BLinePoint bp = value.get(BLinePoint());
		bp.reverse();
		return bp;
	}
	else
	if(type == type_width_point)
	{
		WidthPoint wp = value.get(WidthPoint());
		wp.reverse();
		int tmp = wp.get_side_type_before();
		wp.set_side_type_before(wp.get_side_type_after());
		wp.set_side_type_after(tmp);
		return wp;
	}
	else
	if(type == type_dash_item)
	{
		DashItem di = value.get(DashItem());
		int tmp = di.get_side_type_before();
		di.set_side_type_before(di.get_side_type_after());
		di.set_side_type_after(tmp);
		return di;
	}

	assert(0);
	return value;
}

ValueBase
ValueNode_Reverse::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	return reverse_value((*link_)(t));
}





bool
ValueNode_Reverse::check_type(Type &type)
{
	return
		type == type_list ||
		type == type_string ||
		type == type_segment ||
		type == type_gradient ||
		type == type_bline_point ||
		type == type_width_point ||
		type == type_dash_item;
}

LinkableValueNode::Vocab
ValueNode_Reverse::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value to be reversed"))
	);

	return ret;
}
