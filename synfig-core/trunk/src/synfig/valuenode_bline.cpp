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

inline Vector
linear_interpolation(const Vector& a, const Vector& b, float c)
{ return (b-a)*c+a; }

inline Vector
radial_interpolation(const Vector& a, const Vector& b, float c)
{
	affine_combo<Real,float> mag_combo;
	affine_combo<Angle,float> ang_combo;

	Real mag(mag_combo(a.mag(),b.mag(),c));
	Angle ang(ang_combo(Angle::tan(a[1],a[0]),Angle::tan(b[1],b[0]),c));

	return Point( mag*Angle::cos(ang).get(),mag*Angle::sin(ang).get() );
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

			BLinePoint curr;
			BLinePoint begin;	// begin of dynamic group
			BLinePoint end;		// End of dynamic group
			Time blend_time;
			int dist_from_begin(0), dist_from_end(0);
			BLinePoint ret;

			Time begin_time;
			Time end_time;

			if(!rising)
			{
				try{ end_time=iter->find_prev(t)->get_time(); }
				catch(...) { end_time=Time::begin(); }
				try{ begin_time=iter->find_next(t)->get_time(); }
				catch(...) { begin_time=Time::end(); }
			}
			else
			{
				try{ begin_time=iter->find_prev(t)->get_time(); }
				catch(...) { begin_time=Time::begin(); }
				try{ end_time=iter->find_next(t)->get_time(); }
				catch(...) { end_time=Time::end(); }
			}
			blend_time=begin_time;
			curr=(*iter->value_node)(end_time).get(curr);

//			curr=(*iter->value_node)(t).get(curr);

			// Find "end" of dynamic group
			end_iter=iter;
//			for(++end_iter;begin_iter!=list.end();++end_iter)
			for(++end_iter;end_iter!=list.end();++end_iter)
				if(end_iter->amount_at_time(t)>amount)
				{
					end=(*end_iter->value_node)(blend_time).get(prev);
					break;
				}

			// If we did not find an end of the dynamic group...
			if(end_iter==list.end())
			{
				if(get_loop())
				{
					end_iter=first_iter;
					end=(*end_iter->value_node)(blend_time).get(prev);
//					end=first;
				}
				else
				{
					// Writeme!
					end_iter=first_iter;
					end=(*end_iter->value_node)(blend_time).get(prev);
//					end=first;
				}
			}

			// Find "begin" of dynamic group
			begin_iter=iter;
			begin.set_origin(100.0f); // set the origin to 100 (which is crazy) so that we can check to see if it was found
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
					begin=(*begin_iter->value_node)(blend_time).get(prev);
					break;
				}
			}while(begin_iter!=iter);

			// If we did not find a begin
			if(begin.get_origin()==100.0f)
			{
				if(get_loop())
				{
					begin_iter=first_iter;
					begin=(*begin_iter->value_node)(blend_time).get(prev);
//					begin=first;
				}
				else
				{
					// Writeme!
					begin_iter=first_iter;
					begin=(*begin_iter->value_node)(blend_time).get(prev);
//					begin=first;
				}
			}

			etl::hermite<Vector> curve(begin.get_vertex(),end.get_vertex(),begin.get_tangent2(),end.get_tangent1());
			etl::derivative< etl::hermite<Vector> > deriv(curve);

			ret.set_vertex(curve(curr.get_origin()));

			ret.set_width((end.get_width()-begin.get_width())*curr.get_origin()+begin.get_width());

			ret.set_tangent1(deriv(curr.get_origin()));
			ret.set_tangent2(deriv(curr.get_origin()));

			float prev_tangent_scalar(1.0f);
			float next_tangent_scalar(1.0f);

			//synfig::info("index_%d:dist_from_begin=%d",index,dist_from_begin);
			//synfig::info("index_%d:dist_from_end=%d",index,dist_from_end);

			// If we are the next to the begin
			if(begin_iter==--std::vector<ListEntry>::const_iterator(iter) || dist_from_begin==1)
			{
				prev_tangent_scalar=(1.0f-curr.get_origin())*amount+curr.get_origin();
			}
			else
			{
				float origin=curr.get_origin()-prev.get_origin();
				prev_tangent_scalar=(1.0f-origin)*amount+origin;
			}

			// If we are the next to the end
			if(end_iter==++std::vector<ListEntry>::const_iterator(iter) || dist_from_end==1)
			{
				float origin=1.0-curr.get_origin();
				next_tangent_scalar=(1.0f-origin)*amount+origin;
			}
			else
			if(list.end()!=++std::vector<ListEntry>::const_iterator(iter))
			{
				BLinePoint next;
				next=((*(++std::vector<ListEntry>::const_iterator(iter))->value_node)(t).get(prev));
				float origin=next.get_origin()-curr.get_origin();
				next_tangent_scalar=(1.0f-origin)*amount+origin;
			}
			next_scale=next_tangent_scalar;

			//ret.set_vertex((curr.get_vertex()-ret.get_vertex())*amount+ret.get_vertex());
			if(false)
			{
				// My first try
				Point ref_point_begin(
					(
						(*begin_iter->value_node)(begin_time).get(prev).get_vertex() +
						(*end_iter->value_node)(begin_time).get(prev).get_vertex()
					) * 0.5
				);
				Point ref_point_end(
					(
						(*begin_iter->value_node)(end_time).get(prev).get_vertex() +
						(*end_iter->value_node)(end_time).get(prev).get_vertex()
					) * 0.5
				);
				Point ref_point_now(
					(
						(*begin_iter->value_node)(t).get(prev).get_vertex() +
						(*end_iter->value_node)(t).get(prev).get_vertex()
					) * 0.5
				);
				Point ref_point_linear((ref_point_end-ref_point_begin)*amount+ref_point_begin);

				ret.set_vertex(
					(curr.get_vertex()-ret.get_vertex())*amount+ret.get_vertex() +
					(ref_point_now-ref_point_linear)
				);
				ret.set_tangent1((curr.get_tangent1()-ret.get_tangent1())*amount+ret.get_tangent1());
				ret.set_split_tangent_flag(curr.get_split_tangent_flag());
				if(ret.get_split_tangent_flag())
					ret.set_tangent2((curr.get_tangent2()-ret.get_tangent2())*amount+ret.get_tangent2());
			}
			else
			{
				// My second try
				Point begin_cord_sys[2], begin_cord_origin;
				Point end_cord_sys[2], end_cord_origin;
				Point curr_cord_sys[2], curr_cord_origin;

				{
					const Point a((*end_iter->value_node)(begin_time).get(prev).get_vertex());
					const Point b((*begin_iter->value_node)(begin_time).get(prev).get_vertex());
					begin_cord_origin=(a+b)/2;
					begin_cord_sys[0]=( b - a ).norm();
					begin_cord_sys[1]=begin_cord_sys[0].perp();
				}
				{
					const Point a((*end_iter->value_node)(end_time).get(prev).get_vertex());
					const Point b((*begin_iter->value_node)(end_time).get(prev).get_vertex());
					end_cord_origin=(a+b)/2;
					end_cord_sys[0]=( b - a ).norm();
					end_cord_sys[1]=end_cord_sys[0].perp();
				}
				{
					const Point a((*end_iter->value_node)(t).get(prev).get_vertex());
					const Point b((*begin_iter->value_node)(t).get(prev).get_vertex());
					curr_cord_origin=(a+b)/2;
					curr_cord_sys[0]=( b - a ).norm();
					curr_cord_sys[1]=curr_cord_sys[0].perp();
				}

				/*
				end_cord_origin=(*end_iter->value_node)(end_time).get(prev).get_vertex();
				end_cord_sys[0]=(
					(*begin_iter->value_node)(end_time).get(prev).get_vertex() -
					end_cord_origin
				).norm();
				end_cord_sys[1]=end_cord_sys[0].perp();

				curr_cord_origin=(*end_iter->value_node)(t).get(prev).get_vertex();
				curr_cord_sys[0]=(
					(*begin_iter->value_node)(t).get(prev).get_vertex() -
					curr_cord_origin
				).norm();
				curr_cord_sys[1]=curr_cord_sys[0].perp();
				*/

				// Convert start point
				Point a;
				Vector at1,at2;
				{
					Point tmp(ret.get_vertex()-begin_cord_origin);
					a[0]=tmp*begin_cord_sys[0];
					a[1]=tmp*begin_cord_sys[1];
#define COORD_SYS_RADIAL_TAN_INTERP 1

#ifdef COORD_SYS_RADIAL_TAN_INTERP
					tmp=ret.get_tangent1()+ret.get_vertex()-begin_cord_origin;
					at1[0]=tmp*begin_cord_sys[0];
					at1[1]=tmp*begin_cord_sys[1];

					if(curr.get_split_tangent_flag())
					{
						tmp=ret.get_tangent2()+ret.get_vertex()-begin_cord_origin;
						at2[0]=tmp*begin_cord_sys[0];
						at2[1]=tmp*begin_cord_sys[1];
					}
#endif
				}

				// Convert finish point
				Point b;
				Vector bt1,bt2;
				{
					Point tmp(curr.get_vertex()-end_cord_origin);
					b[0]=tmp*end_cord_sys[0];
					b[1]=tmp*end_cord_sys[1];

#ifdef COORD_SYS_RADIAL_TAN_INTERP
					tmp=curr.get_tangent1()+curr.get_vertex()-end_cord_origin;
					bt1[0]=tmp*end_cord_sys[0];
					bt1[1]=tmp*end_cord_sys[1];

					if(curr.get_split_tangent_flag())
					{
						tmp=curr.get_tangent2()+curr.get_vertex()-end_cord_origin;
						bt2[0]=tmp*end_cord_sys[0];
						bt2[1]=tmp*end_cord_sys[1];
					}
#endif
				}

				// Convert current point
				Point c;
				Vector ct1,ct2;
				{
					// Transpose (invert)
					swap(curr_cord_sys[0][1],curr_cord_sys[1][0]);

					Point tmp((b-a)*amount+a);
					c[0]=tmp*curr_cord_sys[0];
					c[1]=tmp*curr_cord_sys[1];
					c+=curr_cord_origin;

#define INTERP_FUNCTION		radial_interpolation
//#define INTERP_FUNCTION		linear_interpolation

#ifdef COORD_SYS_RADIAL_TAN_INTERP
					tmp=INTERP_FUNCTION(at1,bt1,amount);
					ct1[0]=tmp*curr_cord_sys[0];
					ct1[1]=tmp*curr_cord_sys[1];
					ct1+=curr_cord_origin;
					ct1-=c;

					if(curr.get_split_tangent_flag())
					{
						tmp=INTERP_FUNCTION(at2,bt2,amount);
						ct2[0]=tmp*curr_cord_sys[0];
						ct2[1]=tmp*curr_cord_sys[1];
						ct2+=curr_cord_origin;
						ct2-=c;
					}
#endif
				}

				ret.set_vertex(c);
#ifndef COORD_SYS_RADIAL_TAN_INTERP
				ret.set_tangent1(radial_interpolation(ret.get_tangent1(),curr.get_tangent1(),amount));
				ret.set_split_tangent_flag(curr.get_split_tangent_flag());
				if(ret.get_split_tangent_flag())
					ret.set_tangent2(radial_interpolation(ret.get_tangent2(),curr.get_tangent2(),amount));
#else
				ret.set_tangent1(ct1);
				ret.set_split_tangent_flag(curr.get_split_tangent_flag());
				if(ret.get_split_tangent_flag())
					ret.set_tangent2(ct2);
#endif
			}

			ret.set_origin(curr.get_origin());
			ret.set_width((curr.get_width()-ret.get_width())*amount+ret.get_width());


			// Handle the case where we are the first vertex
			if(first_flag)
			{
				ret.set_tangent1(ret.get_tangent1()*prev_tangent_scalar);
				first_iter=iter;
				first=prev=ret;
				first_flag=false;
				ret_list.push_back(ret);
				continue;
			}

			ret_list.back().set_split_tangent_flag(true);
			ret_list.back().set_tangent2(prev.get_tangent2()*prev_tangent_scalar);
			ret_list.push_back(ret);
			ret_list.back().set_split_tangent_flag(true);
			//ret_list.back().set_tangent2(ret.get_tangent1());
			ret_list.back().set_tangent1(ret.get_tangent1()*prev_tangent_scalar);

			prev=ret;
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
