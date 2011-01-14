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

#endif

using namespace etl;

/* === M A C R O S ========================================================= */
#define SAMPLES		50
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.40)
#define SPIKE_AMOUNT		(4)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.025)
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
	old_version_=false;
	round_tip_[0]=true;
	round_tip_[1]=true;
	sharp_cusps_=true;
	width_=1.0f;
	loopyness_=1.0f;
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

	needs_sync=true;

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

	try {

	const bool loop(bline_.get_loop());

	ValueNode_BLine::Handle bline_valuenode;

	const vector<synfig::BLinePoint> bline(bline_.get_list().begin(),bline_.get_list().end());

	vector<BLinePoint>::const_iterator
		iter,
		next(bline.begin());

	const vector<BLinePoint>::const_iterator
		end(bline.end());

	vector<Point>
		side_a,
		side_b;

	if(loop)
		iter=--bline.end();
	else
		iter=next++;

	// 				iter	next
	//				----	----
	// looped		nth		1st
	// !looped		1st		2nd

	Vector first_tangent=bline.front().get_tangent2();
	Vector last_tangent=iter->get_tangent1();

	// if we are looped and drawing sharp cusps, we'll need a value for the incoming tangent
	if (loop && sharp_cusps_ && last_tangent.is_equal_to(Vector::zero()))
	{
		hermite<Vector> curve((iter-1)->get_vertex(), iter->get_vertex(), (iter-1)->get_tangent2(), iter->get_tangent1());
		const derivative< hermite<Vector> > deriv(curve);
		last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
	}

	// `first' is for making the cusps; don't do that for the first point if we're not looped
	for(bool first=!loop; next!=end; iter=next++)
	{
		Vector prev_t(iter->get_tangent1());
		Vector iter_t(iter->get_tangent2());
		Vector next_t(next->get_tangent1());

		bool split_flag(iter->get_split_tangent_flag());

		// if iter.t2 == 0 and next.t1 == 0, this is a straight line
		if(iter_t.is_equal_to(Vector::zero()) && next_t.is_equal_to(Vector::zero()))
		{
			iter_t=next_t=next->get_vertex()-iter->get_vertex();
			// split_flag=true;

			// if the two points are on top of each other, ignore this segment
			// leave `first' true if was before
			if (iter_t.is_equal_to(Vector::zero()))
				continue;
		}

		// Setup the curve
		hermite<Vector> curve(
			iter->get_vertex(),
			next->get_vertex(),
			iter_t,
			next_t
		);

		const float
			iter_w((iter->get_width()*width_)*0.5f+expand_),
			next_w((next->get_width()*width_)*0.5f+expand_);

		const derivative< hermite<Vector> > deriv(curve);

		if (first)
			first_tangent = deriv(CUSP_TANGENT_ADJUST);

		// Make cusps as necessary
		if(!first && sharp_cusps_ && split_flag && (!prev_t.is_equal_to(iter_t) || iter_t.is_equal_to(Vector::zero())) && !last_tangent.is_equal_to(Vector::zero()))
		{
			Vector curr_tangent(deriv(CUSP_TANGENT_ADJUST));

			const Vector t1(last_tangent.perp().norm());
			const Vector t2(curr_tangent.perp().norm());

			Real cross(t1*t2.perp());
			Real perp((t1-t2).mag());
			if(cross>CUSP_THRESHOLD)
			{
				const Point p1(iter->get_vertex()+t1*iter_w);
				const Point p2(iter->get_vertex()+t2*iter_w);

				side_a.push_back(line_intersection(p1,last_tangent,p2,curr_tangent));
			}
			else if(cross<-CUSP_THRESHOLD)
			{
				const Point p1(iter->get_vertex()-t1*iter_w);
				const Point p2(iter->get_vertex()-t2*iter_w);

				side_b.push_back(line_intersection(p1,last_tangent,p2,curr_tangent));
			}
			else if(cross>0 && perp>1)
			{
				float amount(max(0.0f,(float)(cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);

				side_a.push_back(iter->get_vertex()+(t1+t2).norm()*iter_w*amount);
			}
			else if(cross<0 && perp>1)
			{
				float amount(max(0.0f,(float)(-cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);

				side_b.push_back(iter->get_vertex()-(t1+t2).norm()*iter_w*amount);
			}
		}

		// Make the outline
		if(homogeneous_width_)
		{
			const float length(curve.length());
			float dist(0);
			Point lastpoint;
			for(float n=0.0f;n<0.999999f;n+=1.0f/SAMPLES)
			{
				const Vector d(deriv(n>CUSP_TANGENT_ADJUST?n:CUSP_TANGENT_ADJUST).perp().norm());
				const Vector p(curve(n));

				if(n)
					dist+=(p-lastpoint).mag();

				const float w(((next_w-iter_w)*(dist/length)+iter_w));

				side_a.push_back(p+d*w);
				side_b.push_back(p-d*w);

				lastpoint=p;
			}
		}
		else
			for(float n=0.0f;n<0.999999f;n+=1.0f/SAMPLES)
			{
				const Vector d(deriv(n>CUSP_TANGENT_ADJUST?n:CUSP_TANGENT_ADJUST).perp().norm());
				const Vector p(curve(n));
				const float w(((next_w-iter_w)*n+iter_w));

				side_a.push_back(p+d*w);
				side_b.push_back(p-d*w);
			}
		last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
		side_a.push_back(curve(1.0)+last_tangent.perp().norm()*next_w);
		side_b.push_back(curve(1.0)-last_tangent.perp().norm()*next_w);

		first=false;
	}

	if(loop)
	{
		reverse(side_b.begin(),side_b.end());
		add_polygon(side_a);
		add_polygon(side_b);
		return;
	}

	// Insert code for adding end tip
	if(round_tip_[1] && !loop && side_a.size())
	{
		// remove the last point
		side_a.pop_back();

		const Point vertex(bline.back().get_vertex());
		const Vector tangent(last_tangent.norm());
		const float w((bline.back().get_width()*width_)*0.5f+expand_);

		hermite<Vector> curve(
			vertex+tangent.perp()*w,
			vertex-tangent.perp()*w,
			tangent*w*ROUND_END_FACTOR,
			-tangent*w*ROUND_END_FACTOR
		);

		for(float n=0.0f;n<0.999999f;n+=1.0f/SAMPLES)
			side_a.push_back(curve(n));
	}

	for(;!side_b.empty();side_b.pop_back())
		side_a.push_back(side_b.back());

	// Insert code for adding begin tip
	if(round_tip_[0] && !loop && side_a.size())
	{
		// remove the last point
		side_a.pop_back();

		const Point vertex(bline.front().get_vertex());
		const Vector tangent(first_tangent.norm());
		const float w((bline.front().get_width()*width_)*0.5f+expand_);

		hermite<Vector> curve(
			vertex-tangent.perp()*w,
			vertex+tangent.perp()*w,
			-tangent*w*ROUND_END_FACTOR,
			tangent*w*ROUND_END_FACTOR
		);

		for(float n=0.0f;n<0.999999f;n+=1.0f/SAMPLES)
			side_a.push_back(curve(n));
	}

	add_polygon(side_a);

	} catch (...) { synfig::error("Outline::sync(): Exception thrown"); throw; }
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
	IMPORT_AS(round_tip_[1], "round_tip[0]");
	IMPORT_AS(sharp_cusps_, "sharp_cusps");
	if( param=="width" && value.get_type()==ValueBase::get_type(Real()) )
	{
		width_=value;
		if(old_version_)
			width_*=2.0;
		return true;
	}
	IMPORT_AS(loopyness_, "loopyness");
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
	EXPORT_AS(loopyness_, "loopyness");
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
	ret.push_back(ParamDesc("loopyness")
		.set_local_name(_("Loopyness"))
	);
	ret.push_back(ParamDesc("homogeneous_width")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked the width takes the length of the spline to interpolate"))
	);
	ret.push_back(ParamDesc("wplist")
		.set_local_name(_("Width Point List"))
		.set_hint("width")
		.set_description(_("List of width Points that defines the variable width"))
	);

	return ret;
}
