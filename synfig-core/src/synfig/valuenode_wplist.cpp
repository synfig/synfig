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
#include <ETL/hermite>
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

ValueBase
synfig::convert_bline_to_wplist(const ValueBase& bline)
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
		ret.push_back(WidthPoint(position,iter->get_width()));
		i=i+1.0;
	}
	// If the bline is not looped then make the cups type rounded for the
	// first and last width points.
	// In the future when this function is used to convert old blines to
	// new wlines the end and start cups should be readed from the oultline
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

Real
synfig::widthpoint_interpolate(const WidthPoint& prev, const WidthPoint& next, const Real p, const Real smoothness)
{
	WidthPoint::SideType side_int(WidthPoint::TYPE_INTERPOLATE);
	int nsb, nsa, psb, psa;
	Real pp, np;
	Real nw, pw, rw(0.0);
	const Real epsilon(0.0000001f);
	np=next.get_norm_position();
	pp=prev.get_norm_position();
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
			hermite<Vector> curve(
			Vector(pp, pw),
			Vector(np, nw),
			Vector(0.0,0.0),
			Vector(0.0,0.0)
			);
			rw=(pw+(nw-pw)*q)*(1.0-smoothness)+curve(q)[1]*smoothness;
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
		Real q;
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
		hermite<Vector> curve(
		Vector(pp, pw),
		Vector(np, nw),
		Vector(0.0,0.0),
		Vector(0.0,0.0)
		);
		rw=(pw+(nw-pw)*q)*(1.0-smoothness)+curve(q)[1]*smoothness;
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
		hermite<Vector> curve(
		Vector(np, nw),
		Vector(pp, pw),
		Vector(0.0,0.0),
		Vector(0.0,0.0)
		);
		rw=(nw+(pw-nw)*q)*(1.0-smoothness)+curve(q)[1]*smoothness;
	}
	return rw;
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
			std::vector<WidthPoint>::const_iterator iter;

			for(iter=list.begin();iter!=list.end();iter++)
			{
				value_node->add(ValueNode::Handle(ValueNode_Composite::create(*iter)));
			}
			value_node->set_loop(value.get_loop());
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
	WidthPoint wpoint(position, interpolated_width(position, time));
	// as width points are unsorted in terms of position,
	// we always insert the width point at the begining of the list
	ret.index=0;
	ret.set_parent_value_node(this);
	// Note: before and after interpolations are INTERPOLATE by default.
	// not need to set it up here.
	ret.value_node=ValueNode_Composite::create(wpoint);
	ret.value_node->set_parent_canvas(get_parent_canvas());
	return ret;
}

ValueNode_WPList::ListEntry
ValueNode_WPList::create_list_entry(int /*index*/, Time /*time*/, Real /*origin*/)
{
	// Initially all the width points are created at 0.0
	// TODO: take the decision on what to call on valuenodedynamiclistinsert action
	return create_list_entry(0.0);
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
			Real i_width(interpolated_width(curr.get_norm_position(), t));
			Real curr_width(curr.get_width());
			// linear interpolation by amount
			curr.set_width(i_width*(1.0-amount)+(curr_width)*amount);
			// now insert the calculated width point into the widht list
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

String
ValueNode_WPList::get_name()const
{
	return "wplist";
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

synfig::WidthPoint
ValueNode_WPList::find_next_valid_entry_by_position(Real position, Time time)const
{
	std::vector<ListEntry>::const_iterator iter;
	Real next_pos(1.0);
	synfig::WidthPoint curr, next_ret(next_pos, 0.0);
	for(iter=list.begin();iter!=list.end();++iter)
	{
		curr=(*iter->value_node)(time).get(curr);
		Real curr_pos(curr.get_norm_position());
		bool status((*iter).status_at_time(time));
		if((curr_pos >= position) && (curr_pos <= next_pos) && status)
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
	Real prev_pos(-0.0);
	synfig::WidthPoint curr, prev_ret(prev_pos, 0.0);
	for(iter=list.begin();iter!=list.end();++iter)
	{
		curr=(*iter->value_node)(time).get(curr);
		Real curr_pos(curr.get_norm_position());
		bool status((*iter).status_at_time(time));
		if((curr_pos <= position) && (curr_pos >= prev_pos) && status)
		{
			prev_pos=curr_pos;
			prev_ret=curr;
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
