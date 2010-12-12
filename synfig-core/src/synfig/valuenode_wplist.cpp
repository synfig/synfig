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

// convert_bline_to_width_list(const ValueBase& bline)
// This fun
ValueBase
synfig::convert_bline_to_width_list(const ValueBase& bline)
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

	ValueNode_WPList* value_node(new ValueNode_WPList());

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
				//TODO: Composite should handle WidthPoints
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
ValueNode_WPList::create_list_entry(int index, Time time, Real origin)
{
	ValueNode_WPList::ListEntry ret;

	synfig::WPListPoint prev,next;

	int prev_i,next_i;

	index=index%link_count();

	assert(index>=0);
	ret.index=index;
	ret.set_parent_value_node(this);

	if(!list[index].status_at_time(time))
		next_i=find_next_valid_entry(index,time);
	else
		next_i=index;
	prev_i=find_prev_valid_entry(index,time);

	next=(*list[next_i].value_node)(time);
	prev=(*list[prev_i].value_node)(time);

	etl::hermite<Vector> curve(prev.get_vertex(),next.get_vertex(),prev.get_tangent2(),next.get_tangent1());
	etl::derivative< etl::hermite<Vector> > deriv(curve);

	synfig::WPListPoint WPList_point;
	WPList_point.set_vertex(curve(origin));
	WPList_point.set_width((next.get_width()-prev.get_width())*origin+prev.get_width());
	WPList_point.set_tangent1(deriv(origin)*min(1.0-origin,origin));
	WPList_point.set_tangent2(WPList_point.get_tangent1());
	WPList_point.set_split_tangent_flag(false);
	WPList_point.set_origin(origin);

	ret.value_node=ValueNode_Composite::create(WPList_point);

	return ret;
}


ValueBase
ValueNode_WPList::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<WPListPoint> ret_list;

	std::vector<ListEntry>::const_iterator iter,first_iter;
	bool first_flag(true);
	bool rising;
	int index(0);
	float next_scale(1.0f);

	WPListPoint prev,first;
	first.set_origin(100.0f);

	// loop through all the list's entries
	for(iter=list.begin();iter!=list.end();++iter,index++)
	{
		// how 'on' is this vertex?
		float amount(iter->amount_at_time(t,&rising));

		assert(amount>=0.0f);
		assert(amount<=1.0f);

		// it's fully on
		if (amount > 1.0f - EPSILON)
		{
			if(first_flag)
			{
				first_iter=iter;
				first=prev=(*iter->value_node)(t).get(prev);
				first_flag=false;
				ret_list.push_back(first);
				continue;
			}

			WPListPoint curr;
			curr=(*iter->value_node)(t).get(prev);

			if(next_scale!=1.0f)
			{
				ret_list.back().set_split_tangent_flag(true);
				ret_list.back().set_tangent2(prev.get_tangent2()*next_scale);

				ret_list.push_back(curr);

				ret_list.back().set_split_tangent_flag(true);
				ret_list.back().set_tangent2(curr.get_tangent2());
				ret_list.back().set_tangent1(curr.get_tangent1()*next_scale);

				next_scale=1.0f;
			}
			else
			{
				ret_list.push_back(curr);
			}

			prev=curr;
		}
		// it's partly on
		else if(amount>0.0f)
		{
			std::vector<ListEntry>::const_iterator begin_iter,end_iter;

			// This is where the interesting stuff happens
			// We need to seek forward in the list to see what the next
			// active point is

			WPListPoint blp_here_on;  // the current vertex, when fully on
			WPListPoint blp_here_off; // the current vertex, when fully off
			WPListPoint blp_here_now; // the current vertex, right now (between on and off)
			WPListPoint blp_prev_off; // the beginning of dynamic group when fully off
			WPListPoint blp_next_off; // the end of the dynamic group when fully off

			int dist_from_begin(0), dist_from_end(0);
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

			blp_here_on=(*iter->value_node)(on_time).get(blp_here_on);
//			blp_here_on=(*iter->value_node)(t).get(blp_here_on);

			// Find "end" of dynamic group - ie. search forward along
			// the WPList from the current point until we find a point
			// which is more 'on' than the current one
			end_iter=iter;
//			for(++end_iter;begin_iter!=list.end();++end_iter)
			for(++end_iter;end_iter!=list.end();++end_iter)
				if(end_iter->amount_at_time(t)>amount)
					break;

			// If we did not find an end of the dynamic group...
			// Writeme!  at least now it doesn't crash if first_iter
			// isn't set yet
			if(end_iter==list.end())
			{
				if(get_loop() && !first_flag)
					end_iter=first_iter;
				else
					end_iter=--list.end();
			}

			blp_next_off=(*end_iter->value_node)(off_time).get(prev);

			// Find "begin" of dynamic group
			begin_iter=iter;
			blp_prev_off.set_origin(100.0f); // set the origin to 100 (which is crazy) so that we can check to see if it was found
			do
			{
				if(begin_iter==list.begin())
				{
					if(get_loop())
						begin_iter=list.end();
					else
						break;
				}

				--begin_iter;
				dist_from_begin++;

				// if we've gone all around the loop, give up
				if(begin_iter==iter)
					break;

				if(begin_iter->amount_at_time(t)>amount)
				{
					blp_prev_off=(*begin_iter->value_node)(off_time).get(prev);
					break;
				}
			}while(true);

			// If we did not find a begin
			if(blp_prev_off.get_origin()==100.0f)
			{
				// Writeme! - this needs work, but at least now it
				// doesn't crash
				if(first_flag)
					begin_iter=list.begin();
				else
					begin_iter=first_iter;
				blp_prev_off=(*begin_iter->value_node)(off_time).get(prev);
			}

			// this is how the curve looks when we have completely vanished
			etl::hermite<Vector> curve(blp_prev_off.get_vertex(),   blp_next_off.get_vertex(),
									   blp_prev_off.get_tangent2(), blp_next_off.get_tangent1());
			etl::derivative< etl::hermite<Vector> > deriv(curve);

			// where would we be on this curve, how wide will we be, and
			// where will our tangents point (all assuming that we hadn't vanished)
			blp_here_off.set_vertex(curve(blp_here_on.get_origin()));
			blp_here_off.set_width((blp_next_off.get_width()-blp_prev_off.get_width())*blp_here_on.get_origin()+blp_prev_off.get_width());
			blp_here_off.set_tangent1(deriv(blp_here_on.get_origin()));
			blp_here_off.set_tangent2(deriv(blp_here_on.get_origin()));

			float prev_tangent_scalar(1.0f);
			float next_tangent_scalar(1.0f);

			//synfig::info("index_%d:dist_from_begin=%d",index,dist_from_begin);
			//synfig::info("index_%d:dist_from_end=%d",index,dist_from_end);

			// If we are the next to the begin
			if(begin_iter==--std::vector<ListEntry>::const_iterator(iter) || dist_from_begin==1)
				prev_tangent_scalar=linear_interpolation(blp_here_on.get_origin(), 1.0f, amount);
			else
				prev_tangent_scalar=linear_interpolation(blp_here_on.get_origin()-prev.get_origin(), 1.0f, amount);

			// If we are the next to the end
			if(end_iter==++std::vector<ListEntry>::const_iterator(iter) || dist_from_end==1)
				next_tangent_scalar=linear_interpolation(1.0-blp_here_on.get_origin(), 1.0f, amount);
			else if(list.end()!=++std::vector<ListEntry>::const_iterator(iter))
			{
				WPListPoint next;
				next=((*(++std::vector<ListEntry>::const_iterator(iter))->value_node)(t).get(prev));
				next_tangent_scalar=linear_interpolation(next.get_origin()-blp_here_on.get_origin(), 1.0f, amount);
			}
			else
				//! \todo this isn't quite right; we should handle looped WPLists identically no matter where the loop happens
				//! and we currently don't.  this at least makes it a lot better than it was before
				next_tangent_scalar=linear_interpolation(blp_next_off.get_origin()-blp_here_on.get_origin(), 1.0f, amount);
			next_scale=next_tangent_scalar;

			//blp_here_now.set_vertex(linear_interpolation(blp_here_off.get_vertex(), blp_here_on.get_vertex(), amount));
			// if(false)
			// {
			// 	// My first try
			// 	Point ref_point_begin(((*begin_iter->value_node)(off_time).get(prev).get_vertex() +
			// 						   (*end_iter->value_node)(off_time).get(prev).get_vertex()) * 0.5);
			// 	Point ref_point_end(((*begin_iter->value_node)(on_time).get(prev).get_vertex() +
			// 						 (*end_iter->value_node)(on_time).get(prev).get_vertex()) * 0.5);
			// 	Point ref_point_now(((*begin_iter->value_node)(t).get(prev).get_vertex() +
			// 						 (*end_iter->value_node)(t).get(prev).get_vertex()) * 0.5);
			// 	Point ref_point_linear(linear_interpolation(ref_point_begin, ref_point_end, amount));
			//
			// 	blp_here_now.set_vertex(linear_interpolation(blp_here_off.get_vertex(), blp_here_on.get_vertex(), amount) +
			// 							(ref_point_now-ref_point_linear));
			// 	blp_here_now.set_tangent1(linear_interpolation(blp_here_off.get_tangent1(), blp_here_on.get_tangent1(), amount));
			// 	blp_here_now.set_split_tangent_flag(blp_here_on.get_split_tangent_flag());
			// 	if(blp_here_now.get_split_tangent_flag())
			// 		blp_here_now.set_tangent2(linear_interpolation(blp_here_off.get_tangent2(), blp_here_on.get_tangent2(), amount));
			// }
			// else
			{
				// My second try

				// define 3 coordinate systems:
				Point off_coord_sys[2],   off_coord_origin; // when the current vertex is completely off
				Point on_coord_sys[2] ,    on_coord_origin; // when the current vertex is completely on
				Point curr_coord_sys[2], curr_coord_origin; // the current state - somewhere in between

				// for each of the 3 systems, the origin is half way between the previous and next active point
				// and the axes are based on a vector from the next active point to the previous
				{
					const Point   end_pos_at_off_time((  *end_iter->value_node)(off_time).get(prev).get_vertex());
					const Point begin_pos_at_off_time((*begin_iter->value_node)(off_time).get(prev).get_vertex());
					off_coord_origin=(begin_pos_at_off_time + end_pos_at_off_time)/2;
					off_coord_sys[0]=(begin_pos_at_off_time - end_pos_at_off_time).norm();
					off_coord_sys[1]=off_coord_sys[0].perp();

					const Point   end_pos_at_on_time((  *end_iter->value_node)(on_time).get(prev).get_vertex());
					const Point begin_pos_at_on_time((*begin_iter->value_node)(on_time).get(prev).get_vertex());
					on_coord_origin=(begin_pos_at_on_time + end_pos_at_on_time)/2;
					on_coord_sys[0]=(begin_pos_at_on_time - end_pos_at_on_time).norm();
					on_coord_sys[1]=on_coord_sys[0].perp();

					const Point   end_pos_at_current_time((  *end_iter->value_node)(t).get(prev).get_vertex());
					const Point begin_pos_at_current_time((*begin_iter->value_node)(t).get(prev).get_vertex());
					curr_coord_origin=(begin_pos_at_current_time + end_pos_at_current_time)/2;
					curr_coord_sys[0]=(begin_pos_at_current_time - end_pos_at_current_time).norm();
					curr_coord_sys[1]=curr_coord_sys[0].perp();

					// Invert (transpose) the last of these matrices, since we use it for transform back
					swap(curr_coord_sys[0][1],curr_coord_sys[1][0]);
				}

				/* The code that was here before used just end_iter as the origin, rather than the mid-point */

				// We know our location and tangent(s) when fully on and fully off
				// Transform each of these into their corresponding coordinate system
				Point trans_on_point, trans_off_point;
				Vector trans_on_t1, trans_on_t2, trans_off_t1, trans_off_t2;

				transform_coords(blp_here_on.get_vertex(),  trans_on_point,  on_coord_origin,  on_coord_sys);
				transform_coords(blp_here_off.get_vertex(), trans_off_point, off_coord_origin, off_coord_sys);

#define COORD_SYS_RADIAL_TAN_INTERP 1

#ifdef COORD_SYS_RADIAL_TAN_INTERP
				transform_coords(blp_here_on.get_tangent1(),  trans_on_t1,  Point::zero(), on_coord_sys);
				transform_coords(blp_here_off.get_tangent1(), trans_off_t1, Point::zero(), off_coord_sys);

				if(blp_here_on.get_split_tangent_flag())
				{
					transform_coords(blp_here_on.get_tangent2(),  trans_on_t2,  Point::zero(), on_coord_sys);
					transform_coords(blp_here_off.get_tangent2(), trans_off_t2, Point::zero(), off_coord_sys);
				}
#endif

				{
					// Interpolate between the 'on' point and the 'off' point and untransform to get our point's location
					Point tmp;
					untransform_coords(linear_interpolation(trans_off_point, trans_on_point, amount),
									   tmp, curr_coord_origin, curr_coord_sys);
					blp_here_now.set_vertex(tmp);
				}

#define INTERP_FUNCTION		radial_interpolation
//#define INTERP_FUNCTION	linear_interpolation

#ifdef COORD_SYS_RADIAL_TAN_INTERP
				{
					Vector tmp;
					untransform_coords(INTERP_FUNCTION(trans_off_t1,trans_on_t1,amount), tmp, Point::zero(), curr_coord_sys);
					blp_here_now.set_tangent1(tmp);
				}
#else
				blp_here_now.set_tangent1(radial_interpolation(blp_here_off.get_tangent1(),blp_here_on.get_tangent1(),amount));
#endif

				if (blp_here_on.get_split_tangent_flag())
				{
					blp_here_now.set_split_tangent_flag(true);
#ifdef COORD_SYS_RADIAL_TAN_INTERP
					{
						Vector tmp;
						untransform_coords(INTERP_FUNCTION(trans_off_t2,trans_on_t2,amount), tmp, Point::zero(), curr_coord_sys);
						blp_here_now.set_tangent2(tmp);
					}
#else
					blp_here_now.set_tangent2(radial_interpolation(blp_here_off.get_tangent2(),blp_here_on.get_tangent2(),amount));
#endif
				}
				else
					blp_here_now.set_split_tangent_flag(false);
			}

			blp_here_now.set_origin(blp_here_on.get_origin());
			blp_here_now.set_width(linear_interpolation(blp_here_off.get_width(), blp_here_on.get_width(), amount));

			// Handle the case where we are the first vertex
			if(first_flag)
			{
				blp_here_now.set_tangent1(blp_here_now.get_tangent1()*prev_tangent_scalar);
				first_iter=iter;
				first=prev=blp_here_now;
				first_flag=false;
				ret_list.push_back(blp_here_now);
				continue;
			}

			ret_list.back().set_split_tangent_flag(true);
			ret_list.back().set_tangent2(prev.get_tangent2()*prev_tangent_scalar);
			ret_list.push_back(blp_here_now);
			ret_list.back().set_split_tangent_flag(true);
			//ret_list.back().set_tangent2(blp_here_now.get_tangent1());
			ret_list.back().set_tangent1(blp_here_now.get_tangent1()*prev_tangent_scalar);

			prev=blp_here_now;
		}
	}

	if(next_scale!=1.0f)
	{
		ret_list.back().set_split_tangent_flag(true);
		ret_list.back().set_tangent2(prev.get_tangent2()*next_scale);
	}

/*
	if(get_loop() && !first_flag)
	{
		ret_list.push_back(
			Segment(
			prev.get_vertex(),
			prev.get_tangent2(),
			first.get_vertex(),
			first.get_tangent1()
			)
		);
	}
*/

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
