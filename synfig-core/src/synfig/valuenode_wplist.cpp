/* === S Y N F I G ========================================================= */
/*!	\file valuenode_WPList.cpp
**	\brief Implementation of the "Width Point List" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
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

#include "valuenode_wplist.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include "widthpoint.h"
#include <vector>
#include <list>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

//! Converts a ValueNode_BLine into a WidthPoint list
ValueBase
synfig::convert_bline_to_wpoint_list(const ValueBase& bline)
{
	// returns if the parameter is not a list or if it is a list, it is empty
	if(bline.empty())
		return ValueBase(ValueBase::TYPE_LIST);
	// returns if the contained type is not blinepoint
	if(bline.get_contained_type()!=ValueBase::TYPE_BLINEPOINT)
		return ValueBase(ValueBase::TYPE_LIST);

	std::vector<WidthPoint> ret;
	std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	std::vector<BLinePoint>::const_iterator iter;
	Real position, totalpoints, i(0);
	totalpoints=(Real)list.size();
	// Inserts all the points at the positions given by the bline
	// positions are 0.0 to 1.0 equally spaced based on the number of blinepoints
	for(iter=list.begin();iter!=list.end();++iter)
	{
		position=i/totalpoints;
		ret.push_back(WidthPoint(position,iter->get_width());
		i=i+1.0;
	}
	// If the bline is not looped then make the cups type rounded for the
	// first and last width points.
	// In the future when this function is used to convert old blines to
	// new wlines the end and start cups should be readed from the oultline
	// layer
	if(!bline.get_loop())
	{
		iter=ret.back();
		iter->set_cup_type_after(WidthPoint::CUPTYPE_ROUNDED);
		iter=ret.front();
		iter->set_cup_type_before(WidthPoint::CUPTYPE_ROUNDED);
	}

	return ValueBase(ret);
}


/* === M E T H O D S ======================================================= */


ValueNode_WPList::ValueNode_WPList():
	ValueNode_DynamicList(ValueBase::TYPE_WIDTHPOINT)
{
}

ValueNode_WPList::~ValueNode_WPList()
{
}

ValueNode_WPList*
ValueNode_WPList::create(const ValueBase &value)
{
	// if the parameter is not a list type, return null
	if(value.get_type()!=ValueBase::TYPE_LIST)
		return NULL;
	// create an empty list
	ValueNode_WPList* value_node(new ValueNode_WPList());
	// If the value parameter is not empty
	if(!value.empty())
	{
		switch(value.get_contained_type())
		{
		case ValueBase::TYPE_WIDTHPOINT:
		{
			std::vector<WidthPoint> list(value.get_list().begin(),value.get_list().end());
			std::vector<WPListPoint>::const_iterator iter;

			for(iter=list.begin();iter!=list.end();iter++)
			{
				//TODO: Composite should handle WidthPoints ************
				value_node->add(ValueNode::Handle(ValueNode_Composite::create(*iter)));
			}
			value_node->set_loop(value.get_loop());
		}
			break;
		case ValueBase::TYPE_BLINEPOINT:
		{
			// this will create a standard list of width points
			std::vector<WidthPoint> wplist(convert_bline_to_width_list(value));
			// and then let's call again the create method to convert the
			// standard list of widthpoints to a ValueNode_WPList
			return create(wplist);
		}
			break;
		default:
			// We got a list of who-knows-what. We don't have any idea
			// what to do with it.
			return NULL;
			break;
		}
	}

	return value_node;
}

ValueNode_WPList::ListEntry
ValueNode_WPList::create_list_entry(Real position, Time time)
{
	ValueNode_WPList::ListEntry ret;

	synfig::WidthPoint prev,next;

	int prev_i,next_i;
	int index(0);

	// as width points are unsorted in terms of position,
	// we always insert the width point at the begining of the list
	ret.index=index;
	ret.set_parent_value_node(this);

	// TODO: define those functions
	// Given a time and a postion, those functions returns a valid withpoint
	// (fully on) before or after that position
	next=find_next_valid_entry_by_postion(position, time);
	prev=find_prev_valid_entry_by_postion(position, time);

	synfig::WidthPoint wpoint;
	Real ppos(prev.get_position());
	Real npos(next.get_position());
	Real pos;
	if(npos-ppos < Real(0.0000001f))
		pos=0.5;
	else
	// linear interpolation
		pos=(position-ppos)/(npos-ppos);
	Real pwid(prev.get_width());
	Real nwid(next.get_width());
	Real wid(pwid+(nwid-pwid)*pos);
	// Setup the position
	wpoint.set_position(position);
	// Setup the width
	// TODO: if in the future there are different interpolations between
	// width points other than linear, use it here.
	wpoint.set_width(wid);
	// Note: before and after interpolations are INTERPOLATE by default.
	// no need to set up here.
	ret.value_node=ValueNode_Composite::create(wpoint);

	return ret;
}


ValueBase
ValueNode_WPList::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<WidthPoint> ret_list;

	std::vector<ListEntry>::const_iterator iter,first_iter;
	bool rising;

	WidthPoint curr;

	// loop through all the list's entries
	for(iter=list.begin();iter!=list.end();++iter)
	{
		// how 'on' is this widthpoint?
		float amount(iter->amount_at_time(t,&rising));

		assert(amount>=0.0f);
		assert(amount<=1.0f);

		// it's fully on
		if (amount > 1.0f - 0.0000001f)
		{
			// we store the current width point
			curr=(*iter->value_node)(t).get(curr);
			// and push back to the returning list
			ret_list.push_back(curr);
		}
		// it's partly on
		else if(amount>0.0f)
		{
			// This is where the interesting stuff happens
			// We need to seek in the list to see what the next and prev
			// active width points are

			WidthPoint wp_on;  // the current widthpoint, when fully on
			WidthPoint wp_off; // the current widthpoint, when fully off
			WidthPoint wp_now; // the current widthpoint, right now (between on and off)
			WidthPoint wp_prev_off; // the previous width point in terms of position when the current one is fully off
			WidthPoint wp_next_off; // the next width point in terms of position when the current one is fully off

			Time off_time, on_time;
			if(!rising)	// if not rising, then we were fully on in the past, and will be fully off in the future
			{
				try{ on_time=iter->find_prev(t)->get_time(); }
				catch(...) { on_time=Time::begin(); }
				try{ off_time=iter->find_next(t)->get_time(); }
				catch(...) { off_time=Time::end(); }
			}
			else // otherwise we were fully off in the past, and will be fully on in the future
			{
				try{ off_time=iter->find_prev(t)->get_time(); }
				catch(...) { off_time=Time::begin(); }
				try{ on_time=iter->find_next(t)->get_time(); }
				catch(...) { on_time=Time::end(); }
			}

			wp_on=(*iter->value_node)(on_time).get(wp_on);
			wp_off=(*iter->value_node)(off_time).get(wp_off);
			wp_now=(*iter->value_node)(t).get(wp_now);
			wp_prev_off=find_prev_valid_entry_by_postion(position, off_time);
			wp_next_off=find_next_valid_entry_by_postion(position, off_time);
			// TODO: interpolate_width member function
			Real off_width(interpolate_width(wp_prev_off, wp_next_off, position));
			Real on_width(wp_on.get_width());
			// linear interpolation
			// TODO: use the wp_now width to scale the result
			wp_now.set_width(off_width+(on_width-off_width)*amount);
		}
	if(list.empty())
		synfig::warning(string("ValueNode_WPList::operator()():")+_("No entries in list"));
	else
	if(ret_list.empty())
		synfig::warning(string("ValueNode_WPList::operator()():")+_("No entries in ret_list"));

	return ValueBase(ret_list,get_loop());
}

String
ValueNode_WPList::link_local_name(int i)const
{
	assert(i>=0 && (unsigned)i<list.size());
	return etl::strprintf(_("WidthPoint %03d"),i+1);
}

String
ValueNode_WPList::get_name()const
{
	return "WPList";
}

String
ValueNode_WPList::get_local_name()const
{
	return _("WPList");
}

LinkableValueNode*
ValueNode_WPList::create_new()const
{
	return new ValueNode_WPList();
}

bool
ValueNode_WPList::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_LIST;
}
