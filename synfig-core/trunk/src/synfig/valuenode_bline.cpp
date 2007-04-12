/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "general.h"
#include "exception.h"
#include "blinepoint.h"
#include <vector>
#include <list>
#include <algorithm>
#include <ETL/hermite>
#include <ETL/calculus>
#include "segment.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

inline float
linear_interpolation(const float& a, const float& b, float c)
{ return (b-a)*c+a; }

inline Vector
linear_interpolation(const Vector& a, const Vector& b, float c)
{ return (b-a)*c+a; }

inline Vector
radial_interpolation(const Vector& a, const Vector& b, float c)
{
	// if either extreme is zero then use linear interpolation instead
	if (a.is_equal_to(Vector::zero()) || b.is_equal_to(Vector::zero()))
		return linear_interpolation(a, b, c);

	affine_combo<Real,float> mag_combo;
	affine_combo<Angle,float> ang_combo;

	Real mag(mag_combo(a.mag(),b.mag(),c));
	Angle ang(ang_combo(Angle::tan(a[1],a[0]),Angle::tan(b[1],b[0]),c));

	return Point( mag*Angle::cos(ang).get(),mag*Angle::sin(ang).get() );
}

inline void
transform(Vector in, Vector& out, Point& coord_origin, Point *coord_sys)
{
	in -= coord_origin;
	out[0] = in * coord_sys[0];
	out[1] = in * coord_sys[1];
}

inline void
untransform(const Vector& in, Vector& out, Point& coord_origin, Point *coord_sys)
{
	out[0] = in * coord_sys[0];
	out[1] = in * coord_sys[1];
	out += coord_origin;
}

ValueBase
synfig::convert_bline_to_segment_list(const ValueBase& bline)
{
	std::vector<Segment> ret;

//	std::vector<BLinePoint> list(bline.operator std::vector<BLinePoint>());
	//std::vector<BLinePoint> list(bline);
	std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	std::vector<BLinePoint>::const_iterator	iter;

	BLinePoint prev,first;

	//start with prev = first and iter on the second...

	if(list.empty()) return ValueBase(ret,bline.get_loop());
	first = prev = list.front();

	for(iter=++list.begin();iter!=list.end();++iter)
	{
		ret.push_back(
			Segment(
				prev.get_vertex(),
				prev.get_tangent2(),
				iter->get_vertex(),
				iter->get_tangent1()
			)
		);
		prev=*iter;
	}
	if(bline.get_loop())
	{
		ret.push_back(
			Segment(
				prev.get_vertex(),
				prev.get_tangent2(),
				first.get_vertex(),
				first.get_tangent1()
			)
		);
	}
	return ValueBase(ret,bline.get_loop());
}

ValueBase
synfig::convert_bline_to_width_list(const ValueBase& bline)
{
	std::vector<Real> ret;
//	std::vector<BLinePoint> list(bline.operator std::vector<BLinePoint>());
	//std::vector<BLinePoint> list(bline);
	std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	std::vector<BLinePoint>::const_iterator	iter;

	if(bline.empty())
		return ValueBase(ValueBase::TYPE_LIST);

	for(iter=list.begin();iter!=list.end();++iter)
		ret.push_back(iter->get_width());

	if(bline.get_loop())
		ret.push_back(list.front().get_width());

	return ValueBase(ret,bline.get_loop());
}


/* === M E T H O D S ======================================================= */


ValueNode_BLine::ValueNode_BLine():
	ValueNode_DynamicList(ValueBase::TYPE_BLINEPOINT)
{
}

ValueNode_BLine::~ValueNode_BLine()
{
}

ValueNode_BLine*
ValueNode_BLine::create(const ValueBase &value)
{
	if(value.get_type()!=ValueBase::TYPE_LIST)
		return 0;

	ValueNode_BLine* value_node(new ValueNode_BLine());

	if(!value.empty())
	{
		switch(value.get_contained_type())
		{
		case ValueBase::TYPE_BLINEPOINT:
		{
//			std::vector<BLinePoint> bline_points(value.operator std::vector<BLinePoint>());
			//std::vector<BLinePoint> bline_points(value);
			std::vector<BLinePoint> bline_points(value.get_list().begin(),value.get_list().end());
			std::vector<BLinePoint>::const_iterator iter;

			for(iter=bline_points.begin();iter!=bline_points.end();iter++)
			{
				value_node->add(ValueNode::Handle(ValueNode_Composite::create(*iter)));
			}
			value_node->set_loop(value.get_loop());
		}
			break;
		case ValueBase::TYPE_SEGMENT:
		{
			// Here, we want to convert a list of segments
			// into a list of BLinePoints. We make an assumption
			// that the segment list is continuous(sp), but not necessarily
			// smooth.

			value_node->set_loop(false);
//			std::vector<Segment> segments(value.operator std::vector<Segment>());
//			std::vector<Segment> segments(value);
			std::vector<Segment> segments(value.get_list().begin(),value.get_list().end());
			std::vector<Segment>::const_iterator iter,last(segments.end());
			--last;
			ValueNode_Const::Handle prev,first;

			for(iter=segments.begin();iter!=segments.end();iter++)
			{
#define PREV_POINT	prev->get_value().get(BLinePoint())
#define FIRST_POINT	first->get_value().get(BLinePoint())
#define CURR_POINT	curr->get_value().get(BLinePoint())
				if(iter==segments.begin())
				{
					prev=ValueNode_Const::create(ValueBase::TYPE_BLINEPOINT);
					{
						BLinePoint prev_point(PREV_POINT);
						prev_point.set_vertex(iter->p1);
						prev_point.set_tangent1(iter->t1);
						prev_point.set_width(0.01);
						prev_point.set_origin(0.5);
						prev_point.set_split_tangent_flag(false);
						prev->set_value(prev_point);
					}
					first=prev;
					value_node->add(ValueNode::Handle(prev));

				}
				if(iter==last && iter->p2.is_equal_to(FIRST_POINT.get_vertex()))
				{
					value_node->set_loop(true);
					if(!iter->t2.is_equal_to(FIRST_POINT.get_tangent1()))
					{
						BLinePoint first_point(FIRST_POINT);
						first_point.set_tangent1(iter->t2);
						first->set_value(first_point);
					}
					continue;
				}

				ValueNode_Const::Handle curr;
				curr=ValueNode_Const::create(ValueBase::TYPE_BLINEPOINT);
				{
					BLinePoint curr_point(CURR_POINT);
					curr_point.set_vertex(iter->p2);
					curr_point.set_tangent1(iter->t2);
					curr_point.set_width(0.01);
					curr_point.set_origin(0.5);
					curr_point.set_split_tangent_flag(false);
					curr->set_value(curr_point);
				}
				if(!PREV_POINT.get_tangent1().is_equal_to(iter->t1))
				{
					BLinePoint prev_point(PREV_POINT);
					prev_point.set_split_tangent_flag(true);
					prev_point.set_tangent2(iter->t1);
					prev->set_value(prev_point);
				}
				value_node->add(ValueNode::Handle(curr));
				prev=curr;
			}

		}
			break;
		default:
			// We got a list of who-knows-what. We don't have any idea
			// what to do with it.
			return 0;
			break;
		}
	}


	return value_node;
}

ValueNode_BLine::ListEntry
ValueNode_BLine::create_list_entry(int index, Time time, Real origin)
{
	ValueNode_BLine::ListEntry ret;


	synfig::BLinePoint prev,next;

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

	synfig::info("index=%d, next_i=%d, prev_i=%d",index,next_i,prev_i);

	next=(*list[next_i].value_node)(time);
	prev=(*list[prev_i].value_node)(time);

	etl::hermite<Vector> curve(prev.get_vertex(),next.get_vertex(),prev.get_tangent2(),next.get_tangent1());
	etl::derivative< etl::hermite<Vector> > deriv(curve);

	synfig::BLinePoint bline_point;
	bline_point.set_vertex(curve(origin));
	bline_point.set_width((next.get_width()-prev.get_width())*origin+prev.get_width());
	bline_point.set_tangent1(deriv(origin)*min(1.0-origin,origin));
	bline_point.set_tangent2(bline_point.get_tangent1());
	bline_point.set_split_tangent_flag(false);
	bline_point.set_origin(origin);

	ret.value_node=ValueNode_Composite::create(bline_point);

	return ret;
}

ValueBase
ValueNode_BLine::operator()(Time t)const
{
	std::vector<BLinePoint> ret_list;

	std::vector<ListEntry>::const_iterator iter,first_iter;
	bool first_flag(true);
	bool rising;
	int index(0);
	float next_scale(1.0f);

	BLinePoint prev,first;
	first.set_origin(100.0f);

	for(iter=list.begin();iter!=list.end();++iter,index++)
	{
		float amount(iter->amount_at_time(t,&rising));

		assert(amount>=0.0f);
		assert(amount<=1.0f);

		if(amount==1.0f)
		{
			if(first_flag)
			{
				first_iter=iter;
				first=prev=(*iter->value_node)(t).get(prev);
				first_flag=false;
				ret_list.push_back(first);
				continue;
			}

			BLinePoint curr;
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
		else
		if(amount>0.0f)
		{
			std::vector<ListEntry>::const_iterator begin_iter,end_iter;

			// This is where the interesting stuff happens
			// We need to seek forward in the list to see what the next
			// active point is

			BLinePoint blp_here_on;  // the current vertex, when fully on
			BLinePoint blp_here_off; // the current vertex, when fully off
			BLinePoint blp_here_now; // the current vertex, right now (between on and off)
			BLinePoint blp_prev_off; // the beginning of dynamic group when fully off
			BLinePoint blp_next_off; // end of the dynamic group when fully off

			int dist_from_begin(0), dist_from_end(0);
			Time off_time, on_time;

			if(!rising)
			{
				try{ on_time=iter->find_prev(t)->get_time(); }
				catch(...) { on_time=Time::begin(); }
				try{ off_time=iter->find_next(t)->get_time(); }
				catch(...) { off_time=Time::end(); }
			}
			else
			{
				try{ off_time=iter->find_prev(t)->get_time(); }
				catch(...) { off_time=Time::begin(); }
				try{ on_time=iter->find_next(t)->get_time(); }
				catch(...) { on_time=Time::end(); }
			}

			blp_here_on=(*iter->value_node)(on_time).get(blp_here_on);
//			blp_here_on=(*iter->value_node)(t).get(blp_here_on);

			// Find "end" of dynamic group
			end_iter=iter;
//			for(++end_iter;begin_iter!=list.end();++end_iter)
			for(++end_iter;end_iter!=list.end();++end_iter)
				if(end_iter->amount_at_time(t)>amount)
				{
					blp_next_off=(*end_iter->value_node)(off_time).get(prev);
					break;
				}

			// If we did not find an end of the dynamic group...
			if(end_iter==list.end())
			{
				if(get_loop())
				{
					end_iter=first_iter;
					blp_next_off=(*end_iter->value_node)(off_time).get(prev);
//					end=first;
				}
				else
				{
					// Writeme!
					end_iter=first_iter;
					blp_next_off=(*end_iter->value_node)(off_time).get(prev);
//					end=first;
				}
			}

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

				if(begin_iter==iter)
					break;

				if(begin_iter->amount_at_time(t)>amount)
				{
					blp_prev_off=(*begin_iter->value_node)(off_time).get(prev);
					break;
				}
			}while(begin_iter!=iter);

			// If we did not find a begin
			if(blp_prev_off.get_origin()==100.0f)
			{
				if(get_loop())
				{
					begin_iter=first_iter;
					blp_prev_off=(*begin_iter->value_node)(off_time).get(prev);
//					blp_prev_off=first;
				}
				else
				{
					// Writeme!
					begin_iter=first_iter;
					blp_prev_off=(*begin_iter->value_node)(off_time).get(prev);
//					blp_prev_off=first;
				}
			}

			etl::hermite<Vector> curve(blp_prev_off.get_vertex(),   blp_next_off.get_vertex(),
									   blp_prev_off.get_tangent2(), blp_next_off.get_tangent1());
			etl::derivative< etl::hermite<Vector> > deriv(curve);

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
				BLinePoint next;
				next=((*(++std::vector<ListEntry>::const_iterator(iter))->value_node)(t).get(prev));
				next_tangent_scalar=linear_interpolation(next.get_origin()-blp_here_on.get_origin(), 1.0f, amount);
			}
			else
				//! \todo this isn't quite right; we should handle looped blines identically no matter where the loop happens
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
				}

				/* The code that was here before used just end_iter as the origin, rather than the mid-point */

				// For each of the 3 coordinate systems we've just defined, we convert a point and tangent(s) into that system
				Point trans_on_point, trans_off_point, untrans_curr_point;
				Vector trans_on_t1, trans_on_t2, trans_off_t1, trans_off_t2, untrans_curr_t1, untrans_curr_t2;

				// Convert points where vertex is fully on and fully off
				transform(blp_here_on.get_vertex(),  trans_on_point,  on_coord_origin,  on_coord_sys);
				transform(blp_here_off.get_vertex(), trans_off_point, off_coord_origin, off_coord_sys);

#define COORD_SYS_RADIAL_TAN_INTERP 1

#ifdef COORD_SYS_RADIAL_TAN_INTERP
				// this I don't understand.  why add on this bit ---v
				transform(blp_here_on.get_tangent1()  + blp_here_on.get_vertex(),  trans_on_t1,  on_coord_origin,  on_coord_sys);
				transform(blp_here_off.get_tangent1() + blp_here_off.get_vertex(), trans_off_t1, off_coord_origin, off_coord_sys);

				if(blp_here_on.get_split_tangent_flag())
				{
					transform(blp_here_on.get_tangent2()+blp_here_on.get_vertex(),   trans_on_t2,  on_coord_origin,  on_coord_sys);
					transform(blp_here_off.get_tangent2()+blp_here_off.get_vertex(), trans_off_t2, off_coord_origin, off_coord_sys);
				}
#endif
				// Convert current point
				// Transpose (invert)
				swap(curr_coord_sys[0][1],curr_coord_sys[1][0]);

				// interpolate between the 'on' point and the 'off' point and untransform to get our point's location
				untransform(linear_interpolation(trans_off_point, trans_on_point, amount),
							untrans_curr_point, curr_coord_origin, curr_coord_sys);

#define INTERP_FUNCTION		radial_interpolation
//#define INTERP_FUNCTION	linear_interpolation

#ifdef COORD_SYS_RADIAL_TAN_INTERP
				untransform(INTERP_FUNCTION(trans_off_t1,trans_on_t1,amount),
							untrans_curr_t1, curr_coord_origin, curr_coord_sys);
				untrans_curr_t1 -= untrans_curr_point;

				if(blp_here_on.get_split_tangent_flag())
				{
					untransform(INTERP_FUNCTION(trans_off_t2,trans_on_t2,amount),
								untrans_curr_t2, curr_coord_origin, curr_coord_sys);
					untrans_curr_t2 -= untrans_curr_point;
				}
#endif

				blp_here_now.set_vertex(untrans_curr_point);
#ifndef COORD_SYS_RADIAL_TAN_INTERP
				blp_here_now.set_tangent1(radial_interpolation(blp_here_off.get_tangent1(),blp_here_on.get_tangent1(),amount));
				blp_here_now.set_split_tangent_flag(blp_here_on.get_split_tangent_flag());
				if(blp_here_now.get_split_tangent_flag())
					blp_here_now.set_tangent2(radial_interpolation(blp_here_off.get_tangent2(),blp_here_on.get_tangent2(),amount));
#else
				blp_here_now.set_tangent1(untrans_curr_t1);
				blp_here_now.set_split_tangent_flag(blp_here_on.get_split_tangent_flag());
				if(blp_here_now.get_split_tangent_flag())
					blp_here_now.set_tangent2(untrans_curr_t2);
#endif
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
		synfig::warning(string("ValueNode_BLine::operator()():")+_("No entries in list"));
	else
	if(ret_list.empty())
		synfig::warning(string("ValueNode_BLine::operator()():")+_("No entries in ret_list"));

	return ValueBase(ret_list,get_loop());
}

String
ValueNode_BLine::link_local_name(int i)const
{
	assert(i>=0 && (unsigned)i<list.size());
	return etl::strprintf(_("Vertex %03d"),i+1);
}

ValueNode*
ValueNode_BLine::clone(const GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }

	ValueNode_BLine* ret=new ValueNode_BLine();
	ret->set_guid(get_guid()^deriv_guid);

	std::vector<ListEntry>::const_iterator iter;

	for(iter=list.begin();iter!=list.end();++iter)
	{
		if(iter->value_node->is_exported())
			ret->add(*iter);
		else
		{
			ListEntry list_entry(*iter);
			//list_entry.value_node=find_value_node(iter->value_node->get_guid()^deriv_guid).get();
			//if(!list_entry.value_node)
				list_entry.value_node=iter->value_node->clone(deriv_guid);
			ret->add(list_entry);
			//ret->list.back().value_node=iter->value_node.clone();
		}
	}
	ret->set_loop(get_loop());

	return ret;
}

String
ValueNode_BLine::get_name()const
{
	return "bline";
}

String
ValueNode_BLine::get_local_name()const
{
	return _("BLine");
}

LinkableValueNode*
ValueNode_BLine::create_new()const
{
	assert(0);
	return 0;
}

bool
ValueNode_BLine::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_LIST;
}
