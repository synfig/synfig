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
#include "valuenode_bline.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <synfig/widthpoint.h>
#include <vector>
#include <list>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_WPList, RELEASE_VERSION_0_63_00, "wplist", "WPList")

/* === P R O C E D U R E S ================================================= */

ValueBase
synfig::convert_bline_to_wplist(const ValueBase& bline)
{
	// returns if the parameter is not a list or if it is a list, it is empty
	if(bline.empty())
		return ValueBase(type_list);
	// returns if the contained type is not blinepoint
	if(bline.get_contained_type()!=type_bline_point)
		return ValueBase(type_list);

	std::vector<WidthPoint> ret;
	std::vector<BLinePoint> list(bline.get_list_of(BLinePoint()));
	std::vector<BLinePoint>::const_iterator iter;
	Real position, totalpoints, i(0);
	totalpoints=(Real)list.size();
	// Inserts all the points at the positions given by the bline
	// positions are 0.0 to 1.0 equally spaced based on the number of blinepoints
	for(iter=list.begin();iter!=list.end();++iter)
	{
		position=i/totalpoints;
		ret.push_back(WidthPoint(position,iter->get_width()));
		i=i+1.0;
	}
	// If the bline is not looped then make the cups type rounded for the
	// first and last width points.
	// In the future when this function is used to convert old blines to
	// new wlines the end and start cups should be read from the outline
	// layer
	if(!bline.get_loop())
	{
		std::vector<WidthPoint>::iterator iter;
		iter=ret.end();
		iter--;
		iter->set_side_type_after(WidthPoint::TYPE_ROUNDED);
		iter=ret.begin();
		iter->set_side_type_before(WidthPoint::TYPE_ROUNDED);
	}

	return ValueBase(ret);
}

	//! synfig::widthpoint_interpolate
	/*!
	 * @param prev previous withpoint
	 * @param next next withpoint
	 * @param p position to calculate the new width between previous position and next position
	 * @param smoothness [0,1] how much linear (0) or smooth (1) the interpolation is
	 * @return the interpolated width
	*/
Real
synfig::widthpoint_interpolate(const WidthPoint& prev, const WidthPoint& next, const Real p, const Real smoothness)
{
	// Smoothness gives linear interpolation  between
	// the result of the linear interpolation between the withpoints
	// and the interpolation based on a 5th degree polynomial that matches
	// following rules:
	// q [0,1]
	// p(0)= 0    p(1)= 1
	// p'(0)= 0   p'(1)= 0
	// p''(0)= 0  p''(1)= 0
	// It is: p(q) = 6*q^5 - 15*q^4 + 10*q^3 = q*q*q*(10+q*(6*q-15)
	WidthPoint::SideType side_int(WidthPoint::TYPE_INTERPOLATE);
	int nsb, nsa, psb, psa;
	Real pp, np;
	Real nw, pw, rw(0.0);
	const Real epsilon(0.0000001f);
	np=next.get_position();
	pp=prev.get_position();
	nw=next.get_width();
	pw=prev.get_width();
	nsb=next.get_side_type_before();
	nsa=next.get_side_type_after();
	psb=prev.get_side_type_before();
	psa=prev.get_side_type_after();

	if(p==np)
		return nw;
	if(p==pp)
		return pw;
	// Normal case: previous position is lower than next position
	if(np > pp)
	{
		if(np > p && p > pp )
		{
			Real q;
			if(nsb != side_int)
				nw=0.0;
			if(psa != side_int)
				pw=0.0;
			if(np-pp < epsilon)
				q=0.5;
			else
				q=(p-pp)/(np-pp);
			rw=pw+(nw-pw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness);
		}
		else if(p < pp)
		{
			if(psb != side_int)
				pw=0.0;
			rw=pw;
		}
		else if(p > np)
		{
			if(nsa != side_int)
				nw=0.0;
			rw=nw;
		}
	}
	// particular case: previous position is higher than next position
	else
	if(p > pp || np > p)
	{
		Real q(0);
		if(nsb != side_int)
			nw=0.0;
		if(psa != side_int)
			pw=0.0;
		if(np+1.0-pp < epsilon)
			q=0.5;
		else
		{
			if(p > pp)
				q=(p-pp)/(np+1.0-pp);
			if(np > p)
				q=(p+1.0-pp)/(np+1.0-pp);
		}
		rw=pw+(nw-pw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness);
	}
	else
	if(p > np && p < pp)
	{
		Real q;
		if(nsa != side_int)
			nw=0.0;
		if(psb != side_int)
			pw=0.0;
		if(pp-np < epsilon)
			q=0.5;
		else
			q=(p-np)/(pp-np);
		rw=nw+(pw-nw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness);
	}
	return rw;
}


/* === M E T H O D S ======================================================= */


ValueNode_WPList::ValueNode_WPList():
	ValueNode_DynamicList(type_width_point)
{
}

ValueNode_WPList::~ValueNode_WPList()
{
}

ValueNode_WPList*
ValueNode_WPList::create(const ValueBase &value)
{
	// if the parameter is not a list type, return null
	if(value.get_type()!=type_list)
		return NULL;
	// create an empty list
	ValueNode_WPList* value_node(new ValueNode_WPList());
	// If the value parameter is not empty
	if(!value.empty())
	{
		if (value.get_contained_type() == type_width_point)
		{
			std::vector<WidthPoint> list(value.get_list_of(WidthPoint()));
			std::vector<WidthPoint>::const_iterator iter;

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

ValueNode_WPList::ListEntry
ValueNode_WPList::create_list_entry(int index, Time time, Real /*origin*/)
{
	ValueNode_WPList::ListEntry ret;
	synfig::WidthPoint curr, prev, inserted;
	if(link_count())
	{
		curr=(*(list[index].value_node))(time).get(curr);
		Real curr_pos(curr.get_norm_position(get_loop()));
		prev=find_prev_valid_entry_by_position(curr_pos, time);
		Real prev_pos(prev.get_norm_position(get_loop()));
		inserted.set_position((prev_pos+curr_pos)/2);
		Real prev_width(prev.get_width());
		Real curr_width(curr.get_width());
		inserted.set_width((prev_width+curr_width)/2);
	}
	else
	{
		inserted.set_position(0.5);
		inserted.set_width(1.0);
	}
	ret.index=0;
	ret.set_parent_value_node(this);
	ret.value_node=ValueNode_Composite::create(inserted);
	ret.value_node->set_parent_canvas(get_parent_canvas());
	return ret;
}

ValueBase
ValueNode_WPList::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<WidthPoint> ret_list;

	std::vector<ListEntry>::const_iterator iter;
	bool rising;

	WidthPoint curr;

	// go through all the list's entries
	for(iter=list.begin();iter!=list.end();++iter)
	{
		// how 'on' is this widthpoint?
		float amount(iter->amount_at_time(t,&rising));
		assert(amount>=0.0f);
		assert(amount<=1.0f);
		// we store the current width point
		curr=(*iter->value_node)(t).get(curr);
		// it's fully on
		if (amount > 1.0f - 0.0000001f)
		{
			// push back to the returning list
			ret_list.push_back(curr);
		}
		// it's partly on
		else if(amount>0.0f)
		{
			// This is where the interesting stuff happens
			Time off_time, on_time;
			if(!rising)	// if not rising, then we were fully 'on' in the past, and will be fully 'off' in the future
			{
				try{ on_time=iter->find_prev(t)->get_time(); }
				catch(...) { on_time=Time::begin(); }
				try{ off_time=iter->find_next(t)->get_time(); }
				catch(...) { off_time=Time::end(); }
			}
			else // otherwise we were fully 'off' in the past, and will be fully 'on' in the future
			{
				try{ off_time=iter->find_prev(t)->get_time(); }
				catch(...) { off_time=Time::begin(); }
				try{ on_time=iter->find_next(t)->get_time(); }
				catch(...) { on_time=Time::end(); }
			}
			// i_width is the interpolated width at current time given by fully 'on' surrounding width points
			Real i_width(interpolated_width(curr.get_norm_position(get_loop()), t));
			Real curr_width(curr.get_width());
			// linear interpolation by amount
			curr.set_width(i_width*(1.0-amount)+(curr_width)*amount);
			// now insert the calculated width point into the width list
			ret_list.push_back(curr);
		}
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



LinkableValueNode*
ValueNode_WPList::create_new()const
{
	return new ValueNode_WPList();
}

bool
ValueNode_WPList::check_type(Type &type)
{
	return type==type_list;
}

synfig::WidthPoint
ValueNode_WPList::find_next_valid_entry_by_position(Real position, Time time)const
{
	std::vector<ListEntry>::const_iterator iter;
	Real next_pos(1.0);
	synfig::WidthPoint curr, next_ret(next_pos, 0.0);
	for(iter=list.begin();iter!=list.end();++iter)
	{
		curr=(*iter->value_node)(time).get(curr);
		Real curr_pos(curr.get_norm_position(get_loop()));
		bool status((*iter).status_at_time(time));
		if((curr_pos > position) && (curr_pos < next_pos) && status)
		{
			next_pos=curr_pos;
			next_ret=curr;
		}
	}
	return next_ret;
}

synfig::WidthPoint
ValueNode_WPList::find_prev_valid_entry_by_position(Real position, Time time)const
{
	std::vector<ListEntry>::const_iterator iter;
	Real prev_pos(-123456.0);
	synfig::WidthPoint curr, prev_ret(prev_pos, 0.0);
	if(!list.size())
		return prev_ret;
	for(iter=list.begin();iter!=list.end();++iter)
	{
		curr=(*iter->value_node)(time).get(curr);
		Real curr_pos(curr.get_norm_position(get_loop()));
		bool status((*iter).status_at_time(time));
		if((curr_pos < position) && (curr_pos > prev_pos) && status)
		{
			prev_pos=curr_pos;
			prev_ret=curr;
		}
	}
	if(prev_ret.get_position() == -123456.0)
	// This means that no previous has been found.
	// Let's consider if the bline is looped.
	{
		bool blineloop(ValueNode_BLine::Handle::cast_dynamic(get_bline())->get_loop());
		if(blineloop)
			prev_ret =find_prev_valid_entry_by_position(2.0, time);
		else
		{
			prev_ret =find_next_valid_entry_by_position(-1.0, time);
			prev_ret.set_position(0.0);
		}

	}
	return prev_ret;
}

Real
ValueNode_WPList::interpolated_width(Real position, Time time)const
{
	synfig::WidthPoint prev, next;
	prev=find_prev_valid_entry_by_position(position, time);
	next=find_next_valid_entry_by_position(position, time);
	prev.normalize(get_loop());
	next.normalize(get_loop());
	return widthpoint_interpolate(prev, next, position);
}


ValueNode::LooseHandle
ValueNode_WPList::get_bline()const
{
	return bline_;
}

void
ValueNode_WPList::set_bline(ValueNode::Handle b)
{
	bline_=b;
}

