/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bline.cpp
**	\brief Implementation of the "BLine" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
#include "curve_helper.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define EPSILON 0.0000001f

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
	Angle angle_a(Angle::tan(a[1],a[0]));
	Angle angle_b(Angle::tan(b[1],b[0]));
	float diff = Angle::deg(angle_b - angle_a).get();
	if (diff < -180) angle_b += Angle::deg(360);
	else if (diff > 180) angle_a += Angle::deg(360);
	Angle ang(ang_combo(angle_a, angle_b, c));

	return Point( mag*Angle::cos(ang).get(),mag*Angle::sin(ang).get() );
}

inline void
transform_coords(Vector in, Vector& out, const Point& coord_origin, const Point *coord_sys)
{
	in -= coord_origin;
	out[0] = in * coord_sys[0];
	out[1] = in * coord_sys[1];
}

inline void
untransform_coords(const Vector& in, Vector& out, const Point& coord_origin, const Point *coord_sys)
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

Real
synfig::find_closest_point(const ValueBase &bline, const Point &pos, Real &radius, bool loop, Point *out_point)
{
	Real d,step;
	float time = 0;
	float best_time = 0;
	int best_index = -1;
	synfig::Point best_point;

	if(radius==0)radius=10000000;
	Real closest(10000000);

	int i=0;
	std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	typedef std::vector<BLinePoint>::const_iterator iterT;
	iterT iter, prev, first;
	for(iter=list.begin(); iter!=list.end(); ++i, ++iter)
	{
		if( first == iterT() )
			first = iter;

		if( prev != iterT() )
		{
			bezier<Point>	curve;

			curve[0] = (*prev).get_vertex();
			curve[1] = curve[0] + (*prev).get_tangent2()/3;
			curve[3] = (*iter).get_vertex();
			curve[2] = curve[3] - (*iter).get_tangent1()/3;
			curve.sync();

			#if 0
			// I don't know why this doesn't work
			time=curve.find_closest(pos,6);
			d=((curve(time)-pos).mag_squared());

			#else
			//set the step size based on the size of the picture
			d = (curve[1] - curve[0]).mag() + (curve[2]-curve[1]).mag()	+ (curve[3]-curve[2]).mag();

			step = d/(2*radius); //want to make the distance between lines happy

			step = max(step,0.01); //100 samples should be plenty
			step = min(step,0.1); //10 is minimum

			d = find_closest(curve,pos,step,&closest,&time);
			#endif

			if(d < closest)
			{
				closest = d;
				best_time = time;
				best_index = i;
				best_point = curve(best_time);
			}

		}

		prev = iter;
	}

	// Loop if necessary
	if( loop && ( first != iterT() ) && ( prev != iterT() ) )
	{
		bezier<Point>	curve;

		curve[0] = (*prev).get_vertex();
		curve[1] = curve[0] + (*prev).get_tangent2()/3;
		curve[3] = (*first).get_vertex();
		curve[2] = curve[3] - (*first).get_tangent1()/3;
		curve.sync();

		#if 0
		// I don't know why this doesn't work
		time=curve.find_closest(pos,6);
		d=((curve(time)-pos).mag_squared());

		#else
		//set the step size based on the size of the picture
		d = (curve[1] - curve[0]).mag() + (curve[2]-curve[1]).mag()	+ (curve[3]-curve[2]).mag();

		step = d/(2*radius); //want to make the distance between lines happy

		step = max(step,0.01); //100 samples should be plenty
		step = min(step,0.1); //10 is minimum

			d = find_closest(curve,pos,step,&closest,&time);
		#endif

		if(d < closest)
		{
			closest = d;
			best_time = time;
			best_index = 0;
			best_point = curve(best_time);
		}
	}

	if(best_index != -1)
	{
		if(out_point)
			*out_point = best_point;

		int loop_adjust(loop ? 0 : -1);
		int size = list.size();
		Real amount = (best_index + best_time + loop_adjust) / (size + loop_adjust);
		return amount;
	}

	return 0.0;

}

Real
synfig::std_to_hom(const ValueBase &bline, Real pos, bool index_loop, bool bline_loop)
{
	BLinePoint blinepoint0, blinepoint1;
	const std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	int size = list.size(), from_vertex;
	// trivial cases
	if(pos == 0.0 || pos == 1.0)
		return pos;
	if(!bline_loop) size--;
	if(size < 1) return Real();
	Real int_pos((int)pos);
	Real one(0.0);
	if (index_loop)
	{
		pos = pos - int_pos;
		if (pos < 0)
		{
			pos++;
			one=1.0;
		}
	}
	else
	{
		if (pos < 0) pos = 0;
		if (pos > 1) pos = 1;
	}
	// Calculate the lengths and the total length
	Real tl=0, pl=0;
	std::vector<Real> lengths;
	vector<BLinePoint>::const_iterator iter, next;
	tl=bline_length(bline, bline_loop, &lengths);
	// If the total length of the bline is zero return pos
	if(tl==0.0) return pos;
	from_vertex = int(pos*size);
	// Calculate the partial length until the bezier that holds the current
	std::vector<Real>::const_iterator liter(lengths.begin());
	for(int i=0;i<from_vertex; i++, liter++)
		pl+=*liter;
	// Calculate the remaining length of the position over current bezier
	// Setup the curve of the current bezier.
	next=list.begin();
	iter = bline_loop ? --list.end() : next++;
	if (from_vertex > size-1) from_vertex = size-1; // if we are at the end of the last bezier
	blinepoint0 = from_vertex ? *(next+from_vertex-1) : *iter;
	blinepoint1 = *(next+from_vertex);
	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	// add the distance on the bezier we are on.
	pl+=curve.find_distance(0.0, pos*size - from_vertex);
	// and return the homogenous position
	return int_pos+pl/tl-one;
}

Real
synfig::hom_to_std(const ValueBase &bline, Real pos, bool index_loop, bool bline_loop)
{
	BLinePoint blinepoint0, blinepoint1;
	const std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	int size = list.size(), from_vertex(0);
	// trivial cases
	if(pos == 0.0 || pos == 1.0)
		return pos;
	if(!bline_loop) size--;
	if(size < 1) return Real();
	Real int_pos=int(pos);
	Real one(0.0);
	if (index_loop)
	{
		pos = pos - int_pos;
		if (pos < 0)
		{
			pos++;
			one=1.0;
		}
	}
	else
	{
		if (pos < 0) pos = 0;
		if (pos > 1) pos = 1;
	}
	// Calculate the lengths and the total length
	Real tl(0), pl(0), mpl, bl;
	std::vector<Real> lengths;
	vector<BLinePoint>::const_iterator iter, next;
	tl=bline_length(bline, bline_loop,&lengths);
	// Calculate the my partial length (the length where pos is)
	mpl=pos*tl;
	next=list.begin();
	iter = bline_loop ? --list.end() : next++;
	std::vector<Real>::const_iterator liter(lengths.begin());
	// Find the previous bezier where we pos is placed and the sum
	// of lengths to it (pl)
	// also remember the bezier's length where we stop
	while(mpl > pl && next!=list.end())
	{
		pl+=*liter;
		bl=*liter;
		iter=next;
		next++;
		liter++;
		from_vertex++;
	}
	// correct the iters and partial length in case we passed over
	if(pl > mpl)
	{
		liter--;
		next--;
		if(next==list.begin())
			iter=--list.end();
		else
			iter--;
		pl-=*liter;
		from_vertex--;
	}
	// set up the cureve
	blinepoint0 = *iter;
	blinepoint1 = *next;
	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	// Find the solution to which is the standard postion which matches the current
	// homogenous position
	// Secant method: http://en.wikipedia.org/wiki/Secant_method
	Real sn(0.0); // the standard position on current bezier
	Real sn1(0.0), sn2(1.0);
	Real t0((mpl-pl)/bl); // the homogenous position on the current bezier
	int iterations=0;
	int max_iterations=100;
	Real max_error(0.00001);
	Real error;
	Real fsn1(t0-curve.find_distance(0.0,sn1)/bl);
	Real fsn2(t0-curve.find_distance(0.0,sn2)/bl);
	Real fsn;
	do
	{
		sn=sn1-fsn1*((sn1-sn2)/(fsn1-fsn2));
		fsn=t0-curve.find_distance(0.0, sn)/bl;
		sn2=sn1;
		sn1=sn;
		fsn2=fsn1;
		fsn1=fsn;
		error=fabs(fsn2-fsn1);
		iterations++;
	}while (error>max_error && max_iterations > iterations);
	// convert the current standard index (s) to the bline's standard index
	// and return it
	return int_pos+Real(from_vertex + sn)/size-one;
}

Real
synfig::bline_length(const ValueBase &bline, bool bline_loop, std::vector<Real> *lengths)
{
	BLinePoint blinepoint0, blinepoint1;
	const std::vector<BLinePoint> list(bline.get_list().begin(),bline.get_list().end());
	int size(list.size());
	if(!bline_loop) size--;
	if(size < 1) return Real();
	// Calculate the lengths and the total length
	Real tl(0), l;
	vector<BLinePoint>::const_iterator iter, next(list.begin());
	iter = bline_loop ? --list.end() : next++;
	for(;next!=list.end(); next++)
	{
		blinepoint0 = *iter;
		blinepoint1 = *next;
		etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
		l=curve.length();
		if(lengths) lengths->push_back(l);
		tl+=l;
		iter=next;
	}
	return tl;
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
	synfig::BLinePoint bline_point;
	int prev_i,next_i;

	if(link_count())
	{
		index=index%link_count();
		assert(index>=0);
		if(!list[index].status_at_time(time))
			next_i=find_next_valid_entry(index,time);
		else
			next_i=index;
		prev_i=find_prev_valid_entry(index,time);
		next=(*list[next_i].value_node)(time);
		prev=(*list[prev_i].value_node)(time);
		etl::hermite<Vector> curve(prev.get_vertex(),next.get_vertex(),prev.get_tangent2(),next.get_tangent1());
		etl::derivative< etl::hermite<Vector> > deriv(curve);
		bline_point.set_vertex(curve(origin));
		bline_point.set_width((next.get_width()-prev.get_width())*origin+prev.get_width());
		bline_point.set_tangent1(deriv(origin)*min(1.0-origin,origin));
		bline_point.set_tangent2(bline_point.get_tangent1());
		bline_point.set_split_tangent_flag(false);
		bline_point.set_origin(origin);
	}
	ret.index=index;
	ret.set_parent_value_node(this);
	ret.value_node=ValueNode_Composite::create(bline_point);
	ret.value_node->set_parent_canvas(get_parent_canvas());
	return ret;
}

// Remove this variable because it is not used.
//static int instance_count;

ValueBase
ValueNode_BLine::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<BLinePoint> ret_list;

	std::vector<ListEntry>::const_iterator iter,first_iter;
	bool first_flag(true);
	bool rising;
	int index(0);
	float next_scale(1.0f);

	BLinePoint prev,first;
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
		// it's partly on
		else if(amount>0.0f)
		{
			std::vector<ListEntry>::const_iterator begin_iter,end_iter;

			// This is where the interesting stuff happens
			// We need to seek forward in the list to see what the next
			// active point is

			BLinePoint blp_here_on;  // the current vertex, when fully on
			BLinePoint blp_here_off; // the current vertex, when fully off
			BLinePoint blp_here_now; // the current vertex, right now (between on and off)
			BLinePoint blp_prev_off; // the beginning of dynamic group when fully off
			BLinePoint blp_next_off; // the end of the dynamic group when fully off

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
			// the bline from the current point until we find a point
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

String
ValueNode_BLine::get_name()const
{
	return "bline";
}

String
ValueNode_BLine::get_local_name()const
{
	return _("Spline");
}

LinkableValueNode*
ValueNode_BLine::create_new()const
{
	return new ValueNode_BLine();
}

bool
ValueNode_BLine::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_LIST;
}

LinkableValueNode::Vocab
ValueNode_BLine::get_children_vocab_vfunc()const
{
	LinkableValueNode::Vocab ret;
	for(unsigned int i=0; i<list.size();i++)
	{
		ret.push_back(ParamDesc(ValueBase(),strprintf("item%04d",i))
			.set_local_name(etl::strprintf(_("Vertex %03d"),i+1))
		);
	}

	return ret;
}
