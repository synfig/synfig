/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2018 Ivan Mahonin
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

//! \note This whole file should be rewritten at some point (darco)

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include "outline.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>

#include <ETL/calculus>
#include <ETL/bezier>
#include <ETL/hermite>
#include <vector>

#include <synfig/valuenodes/valuenode_bline.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

#define SAMPLES		50
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.40)
#define SPIKE_AMOUNT		(4)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.025)

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Outline);
SYNFIG_LAYER_SET_NAME(Outline,"outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Outline,N_("Outline"));
SYNFIG_LAYER_SET_CATEGORY(Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Outline,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Outline,"$Id$");

/* === P R O C E D U R E S ================================================= */

// This function was adapted from what was
// described on http://www.whisqu.se/per/docs/math28.htm
Point line_intersection(
	const Point& p1,
	const Vector& t1,
	const Point& p2,
	const Vector& t2
)
{
	const float& x0(p1[0]);
	const float& y0(p1[1]);

	const float x1(p1[0]+t1[0]);
	const float y1(p1[1]+t1[1]);

	const float& x2(p2[0]);
	const float& y2(p2[1]);

	const float x3(p2[0]+t2[0]);
	const float y3(p2[1]+t2[1]);

	const float near_infinity((float)1e+10);

	float m1,m2;    // the slopes of each line

	// compute slopes, note the kluge for infinity, however, this will
	// be close enough

	if ((x1-x0)!=0)
	   m1 = (y1-y0)/(x1-x0);
	else
	   m1 = near_infinity;

	if ((x3-x2)!=0)
	   m2 = (y3-y2)/(x3-x2);
	else
	   m2 = near_infinity;

	// compute constants
	const float& a1(m1);
	const float& a2(m2);
	const float b1(-1.0f);
	const float b2(-1.0f);
	const float c1(y0-m1*x0);
	const float c2(y2-m2*x2);

	// compute the inverse of the determinate
	const float det_inv(1.0f/(a1*b2 - a2*b1));

	// use Kramers rule to compute the intersection
	return Point(
		((b1*c2 - b2*c1)*det_inv),
		((a2*c1 - a1*c2)*det_inv)
	);
} // end Intersect_Lines

/* === M E T H O D S ======================================================= */


Outline::Outline()
{
	old_version=false;
	param_round_tip[0]=ValueBase(true);
	param_round_tip[1]=ValueBase(true);
	param_sharp_cusps=ValueBase(true);
	param_width=ValueBase(Real(1.0f));
	param_expand=ValueBase(Real(0));
	param_homogeneous_width=ValueBase(true);
	clear();

	std::vector<BLinePoint> bline_point_list;
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
	param_bline.set_list_of(bline_point_list);

	needs_sync=true;
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}


/*! The Sync() function takes the values
**	and creates a polygon to be rendered
**	with the polygon layer.
*/
void
Outline::sync_vfunc()
{
	const BLinePoint blank;

	bool round_tip[2];
	round_tip[0]     = param_round_tip[0].get(bool());
	round_tip[1]     = param_round_tip[1].get(bool());
	bool sharp_cusps = param_sharp_cusps.get(bool());
	Real width       = param_width.get(Real());
	Real expand      = param_expand.get(Real());
	bool homogeneous_width = param_homogeneous_width.get(bool());
	
	clear();

	if (param_bline.get_list().empty()) {
		synfig::warning(
			std::string("Outline::sync():")
		  + N_("No vertices in outline " + std::string("\"") + get_description() + std::string("\"")) );
		return;
	}

	try {
		const bool loop = param_bline.get_loop();

		// convert from segment
		ValueBase bline_segment;
		if (param_bline.get_contained_type() == type_segment) {
			ValueNode_BLine::Handle bline_valuenode = ValueNode_BLine::create(param_bline);
			bline_segment = (*bline_valuenode)(0);
		}
		const ValueBase::List &bline = bline_segment.is_valid() ? bline_segment.get_list() : param_bline.get_list();

		// Retrieve the parent canvas grow value
		Real gv = exp(get_outline_grow_mark());

		rendering::Contour::ChunkList side_a, side_b;

		// 				iter	next
		//				----	----
		// looped		nth		1st
		// !looped		1st		2nd
		const ValueBase::List::const_iterator end = bline.end();
		ValueBase::List::const_iterator next = bline.begin();
		ValueBase::List::const_iterator iter = loop ? --bline.end(): next++;

		const BLinePoint &first_point = iter->get(blank);
		Vector first_tangent = bline.front().get(blank).get_tangent2();
		Vector last_tangent = first_point.get_tangent1();

		// If we are looped and drawing sharp cusps, we'll need a value
		// for the incoming tangent. This code fails if we have
		// a "degraded" spline with just one vertex, so we avoid such case.
		if (loop && sharp_cusps && last_tangent.is_equal_to(Vector::zero()) && bline.size() > 1) {
			ValueBase::List::const_iterator prev = iter; --prev;
			const BLinePoint &prev_point = prev->get(blank);
			etl::hermite<Vector> curve(
				prev_point.get_vertex(),
				first_point.get_vertex(),
				prev_point.get_tangent2(),
				first_point.get_tangent1() );
			const etl::derivative< etl::hermite<Vector> > deriv(curve);
			last_tangent = deriv(1.0 - CUSP_TANGENT_ADJUST);
		}

		// `first' is for making the cusps; don't do that for the first point if we're not looped
		for(bool first=!loop; next!=end; iter=next++) {
			const BLinePoint &bp1 = iter->get(blank);
			const BLinePoint &bp2 = next->get(blank);

			Vector prev_t(bp1.get_tangent1());
			Vector iter_t(bp1.get_tangent2());
			Vector next_t(bp2.get_tangent1());

			bool split_flag(bp1.get_split_tangent_angle() || bp1.get_split_tangent_radius());

			// if iter.t2 == 0 and next.t1 == 0, this is a straight line
			if(iter_t.is_equal_to(Vector::zero()) && next_t.is_equal_to(Vector::zero())) {
				iter_t = next_t = bp2.get_vertex() - bp1.get_vertex();

				// if the two points are on top of each other, ignore this segment
				// leave `first' true if was before
				if (iter_t.is_equal_to(Vector::zero()))
					continue;
			}

			// Setup the curve
			etl::hermite<Vector> curve(
				bp1.get_vertex(),
				bp2.get_vertex(),
				iter_t,
				next_t
			);

			const Real iter_w = gv*(bp1.get_width()*width*0.5 + expand);
			const Real next_w = gv*(bp2.get_width()*width*0.5 + expand);

			const etl::derivative< etl::hermite<Vector> > deriv(curve);

			if (first)
				first_tangent = deriv(CUSP_TANGENT_ADJUST);

			// Make cusps as necessary
			if ( !first
			  && sharp_cusps
			  && split_flag
			  && (!prev_t.is_equal_to(iter_t) || iter_t.is_equal_to(Vector::zero()))
			  && !last_tangent.is_equal_to(Vector::zero()))
			{
				Vector curr_tangent(deriv(CUSP_TANGENT_ADJUST));

				const Vector t1(last_tangent.perp().norm());
				const Vector t2(curr_tangent.perp().norm());

				Real cross(t1*t2.perp());
				Real perp((t1-t2).mag());
				if (cross > CUSP_THRESHOLD) {
					const Point p1(bp1.get_vertex() + t1*iter_w);
					const Point p2(bp1.get_vertex() + t2*iter_w);
					side_a.push_back( rendering::Contour::Chunk(
						line_intersection(p1, last_tangent, p2, curr_tangent) ));
				} else
				if (cross < -CUSP_THRESHOLD) {
					const Point p1(bp1.get_vertex() - t1*iter_w);
					const Point p2(bp1.get_vertex() - t2*iter_w);
					side_b.push_back( rendering::Contour::Chunk(
						line_intersection(p1, last_tangent, p2, curr_tangent) ));
				} else
				if (cross > 0.0 && perp > 1.0) {
					Real amount = std::max(Real(0.0), cross/CUSP_THRESHOLD)*(SPIKE_AMOUNT - 1.0) + 1.0;
					side_a.push_back( rendering::Contour::Chunk(
						bp1.get_vertex() + (t1 + t2).norm()*iter_w*amount ));
				} else
				if(cross<0 && perp>1) {
					Real amount = std::max(Real(0.0), -cross/CUSP_THRESHOLD)*(SPIKE_AMOUNT - 1.0) + 1.0;
					side_b.push_back( rendering::Contour::Chunk(
						bp1.get_vertex() - (t1 + t2).norm()*iter_w*amount ));
				}
			}

			// Precalculate positions and coefficients
			Real length = 0.0;
			Vector points[SAMPLES+2];
			Real dists[SAMPLES+2];
			Vector *p = points;
			for(Real n = 0.0, *ds = dists; n < 1.000001; n += 1.0/SAMPLES, ++p, ++ds) {
				*p = curve(n);
				*ds = n ? (length += (*p - *(p-1)).mag()) : 0.0;
			}
			length += (curve(1.0) - *(p-1)).mag();
			const Real div_length = length > EPSILON ? 1.0/length : 1.0;

			// Make the outline
			p = points;
			Vector pt = deriv(CUSP_TANGENT_ADJUST)/3.0;
			for(Real n = 0.0, *ds = dists; n < 1.000001; n += 1.0/SAMPLES, ++p, ++ds) {
				const Vector t = deriv(std::min(std::max(n, CUSP_TANGENT_ADJUST), 1.0 - CUSP_TANGENT_ADJUST))/3.0;
				const Vector d = t.perp().norm();
				const Real k = homogeneous_width ? (*ds)*div_length : n;
				const Real w = (next_w - iter_w)*k + iter_w;
				if (false && n) {
					// create curve
					Vector a = *(p-1) + d*w;
					Vector b = *p + d*w;
					Real tk = (b - a).mag()*div_length;
					side_a.push_back( rendering::Contour::Chunk(b, a + pt*tk, b - t*tk) );

					a = *(p-1) - d*w;
					b = *p - d*w;
					tk = (b - a).mag()*div_length;
					side_b.push_back( rendering::Contour::Chunk(b, a + pt*tk, b - t*tk) );
				} else {
					side_a.push_back( rendering::Contour::Chunk(*p + d*w) );
					side_b.push_back( rendering::Contour::Chunk(*p - d*w) );
				}
				pt = t;
			}

			last_tangent = deriv(1.0 - CUSP_TANGENT_ADJUST);
			side_a.push_back( rendering::Contour::Chunk(
				curve(1.0) + last_tangent.perp().norm()*next_w ));
			side_b.push_back( rendering::Contour::Chunk(
				curve(1.0) - last_tangent.perp().norm()*next_w ));

			first = false;
		}

		if (side_a.size() < 2 || side_b.size() < 2) return;

		move_to(side_a.front().p1);

		if (loop) {
			add(side_a);
			add_reverse(side_b);
		} else {
			// Insert code for adding end tip
			if (round_tip[1]) {
				const BLinePoint &bp = bline.back().get(blank);
				const Point vertex = bp.get_vertex();
				const Vector tangent = last_tangent.norm();
				const Real w = gv*(bp.get_width()*width*0.5 + expand);

				Vector a = vertex + tangent.perp()*w;
				Vector b = vertex - tangent.perp()*w;
				Vector p1 = a + tangent*w*(ROUND_END_FACTOR/3.0);
				Vector p2 = b + tangent*w*(ROUND_END_FACTOR/3.0);

				// replace the last point
				side_a.back() = rendering::Contour::Chunk(a);
				add(side_a);
				add(rendering::Contour::Chunk(b, p1, p2));
			} else add(side_a);

			// Insert code for adding begin tip
			if (round_tip[0]) {
				const BLinePoint &bp = bline.front().get(blank);
				const Point &vertex = bp.get_vertex();
				const Vector tangent = first_tangent.norm();
				const Real w = gv*(bp.get_width()*width*0.5 + expand);

				Vector a = vertex - tangent.perp()*w;
				Vector b = vertex + tangent.perp()*w;
				Vector p1 = a - tangent*w*(ROUND_END_FACTOR/3.0);
				Vector p2 = b - tangent*w*(ROUND_END_FACTOR/3.0);

				// replace the first point
				side_b.front() = rendering::Contour::Chunk(a);
				add_reverse(side_b);
				add(rendering::Contour::Chunk(b, p1, p2));
			} else add_reverse(side_b);
		}
	} catch (...) { synfig::error("Outline::sync(): Exception thrown"); throw; }

	close();
}

bool
Outline::set_shape_param(const String & param, const ValueBase &value)
{
	if(param=="segment_list")
	{
		if(dynamic_param_list().count("segment_list"))
		{
			connect_dynamic_param("bline",dynamic_param_list().find("segment_list")->second);
			disconnect_dynamic_param("segment_list");
			synfig::warning("Outline::set_param(): Updated valuenode connection to use the new \"bline\" parameter.");
		}
		else
			synfig::warning("Outline::set_param(): The parameter \"segment_list\" is deprecated. Use \"bline\" instead.");
	}

	if (param=="segment_list" || param=="bline")
	{
		if (value.get_type() != type_list)
			return false;
		//if (value.get_contained_type() != type_bline_point)
		//	return false;
		param_bline=value;
		return true;
	}

	IMPORT_VALUE(param_round_tip[0]);
	IMPORT_VALUE(param_round_tip[1]);
	IMPORT_VALUE(param_sharp_cusps);
	IMPORT_VALUE_PLUS(param_width,if(old_version){param_width.set(param_width.get(Real())*2.0);});
	IMPORT_VALUE(param_expand);
	IMPORT_VALUE(param_homogeneous_width);

	return Layer_Shape::set_shape_param(param,value);
}

ValueBase
Outline::get_param(const String& param)const
{
	EXPORT_VALUE(param_bline);
	EXPORT_VALUE(param_expand);
	EXPORT_VALUE(param_homogeneous_width);
	EXPORT_VALUE(param_round_tip[0]);
	EXPORT_VALUE(param_round_tip[1]);
	EXPORT_VALUE(param_sharp_cusps);
	EXPORT_VALUE(param_width);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Outline::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_hint("width")
		.set_description(_("A list of spline points"))
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

	return ret;
}

