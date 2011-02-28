/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "advanced_outline.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <ETL/calculus>
#include <ETL/bezier>
#include <ETL/hermite>
#include <vector>

#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_wplist.h>
#include <synfig/valuenode_composite.h>
#include <synfig/valuenode_blinecalcvertex.h>


#endif

using namespace etl;


/* === M A C R O S ========================================================= */
#define SAMPLES		50
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.40)
#define SPIKE_AMOUNT		(4)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.0025)
/* === G L O B A L S ======================================================= */
SYNFIG_LAYER_INIT(Advanced_Outline);
SYNFIG_LAYER_SET_NAME(Advanced_Outline,"advanced_outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Advanced_Outline,N_("Advanced Outline"));
SYNFIG_LAYER_SET_CATEGORY(Advanced_Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Advanced_Outline,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Advanced_Outline,"$Id$");
/* === P R O C E D U R E S ================================================= */
Point line_intersection( const Point& p1, const Vector& t1, const Point& p2, const Vector& t2 );
/* === M E T H O D S ======================================================= */

Advanced_Outline::Advanced_Outline()
{
	round_tip_[0]=true;
	round_tip_[1]=true;
	sharp_cusps_=true;
	width_=1.0f;
	expand_=0;
	homogeneous_width_=true;
	clear();

	vector<BLinePoint> bline_point_list;
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list[0].set_vertex(Point(0,1));
	bline_point_list[1].set_vertex(Point(0,-1));
	bline_point_list[2].set_vertex(Point(1,0));
	bline_point_list[0].set_tangent(bline_point_list[1].get_vertex()-bline_point_list[2].get_vertex()*0.5f);
	bline_point_list[1].set_tangent(bline_point_list[2].get_vertex()-bline_point_list[0].get_vertex()*0.5f);
	bline_point_list[2].set_tangent(bline_point_list[0].get_vertex()-bline_point_list[1].get_vertex()*0.5f);
	bline_point_list[0].set_width(1.0f);
	bline_point_list[1].set_width(1.0f);
	bline_point_list[2].set_width(1.0f);
	bline_=bline_point_list;
	vector<WidthPoint> wpoint_list;
	wpoint_list.push_back(WidthPoint());
	wpoint_list.push_back(WidthPoint());
	wpoint_list[0].set_position(0.0);
	wpoint_list[1].set_position(1.0);
	wpoint_list[0].set_width(0.0);
	wpoint_list[1].set_width(1.0);
	wplist_=wpoint_list;
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}


/*! The Sync() function takes the values
**	and creates a polygon to be rendered
**	with the polygon layer.
*/
void
Advanced_Outline::sync()
{
	clear();
	if (!bline_.get_list().size())
	{
		synfig::warning(string("Advanced_Outline::sync():")+N_("No vertices in bline " + string("\"") + get_description() + string("\"")));
		return;
	}
	try
	{
		const vector<BLinePoint> bline(bline_.get_list().begin(),bline_.get_list().end());
		vector<WidthPoint> wplist(wplist_.get_list().begin(), wplist_.get_list().end());
		const bool blineloop(bline_.get_loop());
		int bline_size(bline.size());
		int wplist_size(wplist.size());
		vector<BLinePoint>::const_iterator biter,bnext(bline.begin());
		vector<WidthPoint>::iterator witer, wnext;
		WidthPoint last_widthpoint, next_widthpoint, bezier_last_widthpoint;
		Vector first_tangent;
		Vector last_tangent;
		Real bezier_size = 1.0/(blineloop?bline_size:(bline_size==1?1.0:(bline_size-1)));
		Real biter_pos(0.0), bnext_pos(bezier_size);
		const vector<BLinePoint>::const_iterator bend(bline.end());
		vector<Point> side_a, side_b;
		// Sort the wplist. It is needed to calculate the first widthpoint
		sort(wplist.begin(),wplist.end());
		// If we are looped, the first bezier to handle starts form the
		// last blinepoint and ends at the first one
		if(blineloop)
			biter=--bline.end();
		else
			biter=bnext++;
		// 				biter	bnext
		//				----	----
		// looped		nth		1st
		// !looped		1st		2nd
		// First tangent is used only to add the start of the bline tip
		first_tangent=bline.front().get_tangent2();
		// Last tangent is only to draw sharp cusps for the first blinepoint
		last_tangent=biter->get_tangent1();
		// if we are looped and drawing sharp cusps, we'll need a value for the incoming tangent
		if (blineloop && sharp_cusps_ && last_tangent.is_equal_to(Vector::zero()))
		{
			hermite<Vector> curve((biter-1)->get_vertex(), biter->get_vertex(), (biter-1)->get_tangent2(), biter->get_tangent1());
			const derivative< hermite<Vector> > deriv(curve);
			last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
		}
		// at start, a last_widthpoint is needed
		if(wplist_size)
		{
			if(!blineloop)
				last_widthpoint=wplist.front();
			else
				last_widthpoint=wplist.back();
		}
		else // use the global width
			last_widthpoint=WidthPoint(0.0, get_param("width"));
		// `first' is for making the cusps; don't do that for the first point if we're not looped
		// For all the beziers beween iter and next do:
		for(bool first=!blineloop; bnext!=bend; biter=bnext++)
		{
			Vector prev_t(biter->get_tangent1());
			Vector iter_t(biter->get_tangent2());
			Vector next_t(bnext->get_tangent1());
			bool split_flag(biter->get_split_tangent_flag());
			// if iter.t2 == 0 and next.t1 == 0, this is a straight line
			if(iter_t.is_equal_to(Vector::zero()) && next_t.is_equal_to(Vector::zero()))
			{
				iter_t=next_t=bnext->get_vertex()-biter->get_vertex();
				// split_flag=true;
				// if the two points are on top of each other, ignore this segment
				// leave `first' true if was before
				if (iter_t.is_equal_to(Vector::zero()))
					continue;
			}
			// Setup the curve
			hermite<Vector> curve(
				biter->get_vertex(),
				bnext->get_vertex(),
				iter_t,
				next_t
			);
			// List of widthpoints in the current bezier
			vector<WidthPoint> bwpoints;
			Real biter_width, bnext_width;
			// Find all the widthpoints on the bezier and also find the first
			// widthpoint of the next bezier (the called 'next_widthpoint')
			bool found_bezier_last_widthpoint=false;
			if(wplist_size)
			{
				vector<WidthPoint>::iterator wnext_found(wplist.begin());
				Real witer_pos, wnext_pos;
				for(wnext=witer=wplist.begin(); witer!=wplist.end();witer++)
				{
					witer_pos=witer->get_norm_position();
					wnext_pos=wnext->get_norm_position();
					// if this widhtpoint is on the bezier
					if(witer_pos <= bnext_pos && witer_pos >= biter_pos)
						// store it on the list
						bwpoints.push_back(*witer);
					// if we haven't found any 'next_widthpoint' beyond
					// current bezier edge
					if(witer_pos > wnext_pos && wnext_pos < bnext_pos)
					{
						// then keep track of the current widthpoint
						wnext=witer;
					}
					// else we have one 'next_widthpoint' found at last
					else if(witer_pos > wnext_pos && wnext_pos >= bnext_pos)
					{
						// if current widthpoint is closer to the
						// bezier boundary than the 'next_widthpoint'
						// then keep track of it
						if(witer_pos - bnext_pos < wnext_pos - bnext_pos)
							wnext=witer;
					}
				}
				// if no wfirst is found higher to bnext and we are looped
				// then use the front widthpoint as 'next_widthpoint'
				if(blineloop && wnext->get_norm_position() < bnext_pos)
					wnext=wplist.begin();
				next_widthpoint=*wnext;
			}
			else // use the global width
				next_widthpoint=WidthPoint(1.0,  get_param("width"));
			// Sort the list of collected widthpoints
			sort(bwpoints.begin(), bwpoints.end());
			// keep track of the last widthpoint from the collected
			// It will be used later to be passed to the 'last_widthpoint'
			// if no widthpoint is found at the bezier, the 'last_widthpoint'
			// will remain the same
			if(bwpoints.size())
			{
				bezier_last_widthpoint=bwpoints.back();
				found_bezier_last_widthpoint=true;
			}
			// Now insert the biter withpoint at the bwpoints vector if there
			// is not any widthpoint exactly at biter_pos
			if(bwpoints.size())
			{
				// Calculate the interpolated width on the biter blinepoint.
				biter_width=synfig::widthpoint_interpolate(last_widthpoint, bwpoints.front(), biter_pos);
				bnext_width=synfig::widthpoint_interpolate(bwpoints.back(), next_widthpoint, bnext_pos);
				// Insert a fake widthpoint at the biter_pos and bnext_pos
				// if there are not widthpoint there
				if(bwpoints.front().get_norm_position()!=biter_pos)
					bwpoints.insert(bwpoints.begin(), WidthPoint(biter_pos, biter_width));
				if(bwpoints.back().get_norm_position()!=bnext_pos)
					bwpoints.push_back(WidthPoint(bnext_pos, bnext_width));
			}
			else // if no widthpoint was collected when use the last and next to interpolate
			{
				biter_width=synfig::widthpoint_interpolate(last_widthpoint, next_widthpoint, biter_pos);
				bnext_width=synfig::widthpoint_interpolate(last_widthpoint, next_widthpoint, bnext_pos);
				bwpoints.push_back(WidthPoint(biter_pos, biter_width));
				bwpoints.push_back(WidthPoint(bnext_pos, bnext_width));
			}
			// width points
			const float biter_w(width_*0.5*biter_width);
			const derivative< hermite<Vector> > deriv(curve);
			//if (first)
				//first_tangent = deriv(CUSP_TANGENT_ADJUST);
			// Make cusps as necessary
			if(!first && sharp_cusps_ && split_flag && (!prev_t.is_equal_to(iter_t) || iter_t.is_equal_to(Vector::zero())) && !last_tangent.is_equal_to(Vector::zero()))
			{
				Vector curr_tangent(deriv(CUSP_TANGENT_ADJUST));
				add_cusp(side_a, side_b, biter->get_vertex(), curr_tangent, last_tangent, biter_w);
			}
			// Make the outline
			wnext=bwpoints.begin();
			wnext++;
			for(witer=bwpoints.begin(); wnext!=bwpoints.end(); witer++, wnext++)
			{
				Real s(witer->get_norm_position());
				Real e(wnext->get_norm_position());
				Real start(bline_to_bezier(s, biter_pos, bezier_size));
				Real end(bline_to_bezier(e, biter_pos, bezier_size));
				Real distance=end-start;
				Real increase=distance/SAMPLES;
				if(increase < EPSILON) continue;
				for(Real n=start;n<=end;n+=increase)
				{
					const Vector d(deriv(n).perp().norm());
					const Vector p(curve(n));
					const float w(width_*0.5*synfig::widthpoint_interpolate(*witer, *wnext, bezier_to_bline(n, biter_pos, bezier_size)));
					side_a.push_back(p+d*w);
					side_b.push_back(p-d*w);
				}
			}
			// Insert the last two sides evaluated at end of curve (bezier)
			last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
			side_a.push_back(curve(1.0)+last_tangent.perp().norm()*witer->get_width()*width_*0.5);
			side_b.push_back(curve(1.0)-last_tangent.perp().norm()*witer->get_width()*width_*0.5);
			// make first false as we have done the first bezier
			first=false;
			// update the iter and next positions adding the bezier size.
			biter_pos = bnext_pos;
			bnext_pos+=bezier_size;
			// update the last width point with the last of this group if any
			if(found_bezier_last_widthpoint)
				last_widthpoint=bezier_last_widthpoint;
		} // end of loop through beziers of the bline

		if(blineloop)
		{
			reverse(side_b.begin(),side_b.end());
			add_polygon(side_a);
			add_polygon(side_b);
			return;
		}
		// Insert code for adding end and start tip
		const Point vertex_end(bline.back().get_vertex());
		const Vector tangent_end(last_tangent.norm());
		const WidthPoint wp_end(1.0, 1.0 , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_ROUNDED);
		const Point vertex_start(bline.front().get_vertex());
		const Vector tangent_start(first_tangent.norm());
		const WidthPoint wp_start(0.0, 1.0, WidthPoint::TYPE_ROUNDED, WidthPoint::TYPE_INTERPOLATE);
		if(round_tip_[1] && !blineloop)
			add_tip(side_a, side_b, vertex_end, tangent_end, wp_end);
		if(round_tip_[0] && !blineloop)
		{
			reverse(side_a.begin(), side_a.end());
			reverse(side_b.begin(), side_b.end());
			add_tip(side_a, side_b, vertex_start, tangent_start, wp_start);
		}
		// concatenate sides before add to polygon
		for(;!side_b.empty();side_b.pop_back())
			side_a.push_back(side_b.back());
		add_polygon(side_a);
	}
	catch (...) { synfig::error("Advanced Outline::sync(): Exception thrown"); throw; }
}

bool
Advanced_Outline::set_param(const String & param, const ValueBase &value)
{
	if(param=="bline" && value.get_type()==ValueBase::TYPE_LIST)
	{
		bline_=value;
		return true;
	}
	IMPORT_AS(round_tip_[0],"round_tip[0]");
	IMPORT_AS(round_tip_[1], "round_tip[1]");
	IMPORT_AS(sharp_cusps_, "sharp_cusps");
	IMPORT_AS(width_,"width");
	IMPORT_AS(expand_, "expand");
	IMPORT_AS(homogeneous_width_, "homogeneous_width");
	if(param=="wplist" && value.get_type()==ValueBase::TYPE_LIST)
	{
		wplist_=value;
		return true;
	}
	if(param=="vector_list")
		return false;
	return Layer_Polygon::set_param(param,value);
}

void
Advanced_Outline::set_time(Context context, Time time)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time);
}

void
Advanced_Outline::set_time(Context context, Time time, Vector pos)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time,pos);
}

ValueBase
Advanced_Outline::get_param(const String& param)const
{
	EXPORT_AS(bline_, "bline");
	EXPORT_AS(expand_, "expand");
	EXPORT_AS(homogeneous_width_, "homogeneous_width");
	EXPORT_AS(round_tip_[0], "round_tip[0]");
	EXPORT_AS(round_tip_[1], "round_tip[1]");
	EXPORT_AS(sharp_cusps_, "sharp_cusps");
	EXPORT_AS(width_, "width");
	EXPORT_AS(wplist_, "wplist");
	EXPORT_NAME();
	EXPORT_VERSION();
	if(param=="vector_list")
		return ValueBase();
	return Layer_Polygon::get_param(param);
}

Layer::Vocab
Advanced_Outline::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Polygon::get_param_vocab());
	// Pop off the polygon parameter from the polygon vocab
	ret.pop_back();
	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_description(_("A list of BLine Points"))
	);
	ret.push_back(ParamDesc("width")
		.set_is_distance()
		.set_local_name(_("Outline Width"))
		.set_description(_("Global width of the outline"))
	);
	ret.push_back(ParamDesc("expand")
		.set_is_distance()
		.set_local_name(_("Expand"))
		.set_description(_("Value to add to the global width"))
	);
	ret.push_back(ParamDesc("sharp_cusps")
		.set_local_name(_("Sharp Cusps"))
		.set_description(_("Determines cusp type"))
	);
	ret.push_back(ParamDesc("round_tip[0]")
		.set_local_name(_("Rounded Begin"))
		.set_description(_("Round off the tip"))
	);
	ret.push_back(ParamDesc("round_tip[1]")
		.set_local_name(_("Rounded End"))
		.set_description(_("Round off the tip"))
	);
	ret.push_back(ParamDesc("homogeneous_width")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked the width takes the length of the spline to interpolate"))
	);
	ret.push_back(ParamDesc("wplist")
		.set_local_name(_("Width Point List"))
		.set_hint("width")
		.set_origin("origin")
		.set_description(_("List of width Points that defines the variable width"))
	);
	return ret;
}

bool
Advanced_Outline::connect_dynamic_param(const String& param, etl::loose_handle<ValueNode> x)
{
	synfig::info("attempting to connect %s", param.c_str());
	if(param=="bline")
	{
		if(!connect_bline_to_wplist(x))
			synfig::warning("Advanced Outline: WPList doesn't accept new bline");
	}
	if(param=="wplist")
	{
		if(Layer::connect_dynamic_param(param, x))
		{
			DynamicParamList::const_iterator iter(dynamic_param_list().find("bline"));
			if(iter==dynamic_param_list().end())
				{
					synfig::warning("BLine doesn't exists yet!!");
					return false;
				}
			else if(!connect_bline_to_wplist(iter->second))
			{
				synfig::warning("Advanced Outline: WPList doesn't accept new bline");
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}
	return Layer::connect_dynamic_param(param, x);
}

bool
Advanced_Outline::connect_bline_to_wplist(etl::loose_handle<ValueNode> x)
{
	if(x->get_type() != ValueBase::TYPE_LIST)
	{
		synfig::info("Not a list");
		return false;
	}
	if((*x)(Time(0)).get_list().front().get_type() != ValueBase::TYPE_BLINEPOINT)
	{
		synfig::info("No blinepoints!");
		return false;
	}
	ValueNode::LooseHandle vnode;
	DynamicParamList::const_iterator iter(dynamic_param_list().find("wplist"));
	if(iter==dynamic_param_list().end())
	{
		synfig::warning("WPList doesn't exists yet");
		return false;
	}
	ValueNode_WPList::Handle wplist(ValueNode_WPList::Handle::cast_dynamic(iter->second));
	if(!wplist)
	{
		synfig::info("WPList is not ready: NULL");
		return false;
	}
	if(!wplist->link_count())
		synfig::warning("Advanced_Outline::connect_bline_to_wplist: WPList::link_count()=0");
	wplist->set_bline(ValueNode::Handle(x));
	synfig::info("set bline success");
	return true;
}


Real
Advanced_Outline::bline_to_bezier(Real bline_pos, Real origin, Real bezier_size)
{
	if(bezier_size)
		return (bline_pos-origin)/bezier_size;
	return bline_pos;
}
Real
Advanced_Outline::bezier_to_bline(Real bezier_pos, Real origin, Real bezier_size)
{
	return origin+bezier_pos*bezier_size;
}

void
Advanced_Outline::add_tip(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector tangent, const WidthPoint wp)
{
	Real w(width_*0.5*wp.get_width());
	// Side Before
	switch (wp.get_side_type_before())
	{
		case WidthPoint::TYPE_ROUNDED:
			hermite<Vector> curve(
				vertex-tangent.perp()*w,
				vertex+tangent.perp()*w,
				-tangent*w*ROUND_END_FACTOR,
				tangent*w*ROUND_END_FACTOR
			);
			for(float n=0.0f;n<0.499999f;n+=2.0f/SAMPLES)
			{
				side_a.push_back(curve(0.5+n));
				side_b.push_back(curve(0.5-n));
			}
			side_a.push_back(curve(1.0));
			side_a.push_back(curve(0.0));
	}
	// Side After
	switch (wp.get_side_type_after())
	{
		case WidthPoint::TYPE_ROUNDED:
			hermite<Vector> curve(
				vertex+tangent.perp()*w,
				vertex-tangent.perp()*w,
				tangent*w*ROUND_END_FACTOR,
				-tangent*w*ROUND_END_FACTOR
			);
			for(float n=0.0f;n<0.499999f;n+=2.0f/SAMPLES)
			{
				side_a.push_back(curve(n));
				side_b.push_back(curve(1-n));
			}
			side_a.push_back(curve(0.5));
			side_a.push_back(curve(0.5));
	}
}
void
Advanced_Outline::add_cusp(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector curr, const Vector last, Real w)
{
	const Vector t1(last.perp().norm());
	const Vector t2(curr.perp().norm());
	Real cross(t1*t2.perp());
	Real perp((t1-t2).mag());
	if(cross>CUSP_THRESHOLD)
	{
		const Point p1(vertex+t1*w);
		const Point p2(vertex+t2*w);
		side_a.push_back(line_intersection(p1,last,p2,curr));
	}
	else if(cross<-CUSP_THRESHOLD)
	{
		const Point p1(vertex-t1*w);
		const Point p2(vertex-t2*w);
		side_b.push_back(line_intersection(p1,last,p2,curr));
	}
	else if(cross>0 && perp>1)
	{
		float amount(max(0.0f,(float)(cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);
		side_a.push_back(vertex+(t1+t2).norm()*w*amount);
	}
	else if(cross<0 && perp>1)
	{
		float amount(max(0.0f,(float)(-cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);
		side_b.push_back(vertex-(t1+t2).norm()*w*amount);
	}
}
