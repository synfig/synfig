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
**	......... ... 2018-2019 Ivan Mahonin
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

#include <vector>

#include <ETL/calculus>
#include <ETL/hermite>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>

#include <synfig/valuenodes/valuenode_bline.h>

#include "outline.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Outline);
SYNFIG_LAYER_SET_NAME(Outline,"outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Outline,N_("Outline"));
SYNFIG_LAYER_SET_CATEGORY(Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Outline,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Outline,"$Id$");

/* === P R O C E D U R E S ================================================= */

static Vector line_intersection(
	const Vector &p0,
	const Vector &t0,
	const Vector &p1,
	const Vector &t1,
	const Vector &failvalue )
{
	const Vector tp = t1.perp();
	Real d = t0*tp;
	if (approximate_zero(d))
		return failvalue; // no intersection
	d = (p1 - p0)*tp/d;
	return p0 + t0*d;
}

static Vector line_intersection(
	const Vector &p0,
	const Vector &t0,
	const Vector &p1,
	const Vector &t1 )
{
	return line_intersection(p0, t0, p1, t1, (p0 + p1)*0.5); // return middle point, when no intersection
}

static void make_arc(
	rendering::Contour::ChunkList &list,
	const Vector &center,
	Real radius,
	const Vector &p1,
	int level )
{
	const Vector p0 = list.back().p1;
	const Vector t0 = list.back().pp1 - p0;
	const Vector t1 = (p1 - center).perp();
	Vector pp = line_intersection(p0, t0, p1, t1, center + t1);
	
	if (!level) {
		list.push_back(
			rendering::Contour::Chunk(
				p1, pp ));
		return;
	}
	
	pp = (pp - center).norm()*radius + center;
	make_arc(list, center, radius, pp, level - 1);
	make_arc(list, center, radius, p1, level - 1);
}

static void make_cusp(
	rendering::Contour::ChunkList &list,
	const Vector &center,
	Real radius,
	bool sharp,
	int arc_level,
	const Vector &p1 )
{
	if (list.empty()) {
		// move to target point
		list.push_back(
			rendering::Contour::Chunk(
				rendering::Contour::MOVE, p1 ));
		return;
	}

	const Vector p0 = list.back().p1;
	const Vector t0 = list.back().pp1 - p0;
	const Vector tp1 = p1 - center;

	// sharp corner or inner corner
	if (sharp || approximate_greater_or_equal(tp1*t0, Real())) {
		list.push_back(
			rendering::Contour::Chunk(
				line_intersection(p0, t0, p1, tp1.perp()) ));
		list.push_back( rendering::Contour::Chunk(p1) );
		return;
	}

	make_arc(list, center, radius, p1, arc_level);
}


/* === M E T H O D S ======================================================= */


Outline::Outline()
{
	old_version = false;
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
	clear();
	if (param_bline.get_list().empty()) return;

	const BLinePoint blank;
	const int arc_level = 2; // make cusps and tips with 4 (2^2) segments
	const int segments = 32; // segmensts for curve
	const Real segment_step = 1.0/(segments - 1);
	const Real derivative_bound = 0.025;
	const Real derivative_step = 0.001;
	
	const Real width  = param_width.get(Real());
	const Real expand = param_expand.get(Real());
	const bool sharp_cusps = param_sharp_cusps.get(bool());
	const bool homogeneous_width = param_homogeneous_width.get(bool());
	const bool round_tip[] = {
		param_round_tip[0].get(bool()),
		param_round_tip[1].get(bool()) };

	try {
		const bool loop = param_bline.get_loop();

		// convert from segment
		ValueBase bline_segment;
		if (param_bline.get_contained_type() == type_segment) {
			ValueNode_BLine::Handle bline_valuenode = ValueNode_BLine::create(param_bline);
			bline_segment = (*bline_valuenode)(0);
		}
		const ValueBase::List &bline = bline_segment.is_valid() ? bline_segment.get_list() : param_bline.get_list();

		// retrieve the parent canvas grow value
		Real gv = exp(get_outline_grow_mark());

		const int max_count = (int)bline.size()*(segments + 4) + 4 + 1; // 4 - arc cusp, 4 - arc tip, 1 - move
		rendering::Contour::ChunkList side_a, side_b;
		side_a.reserve(max_count);
		side_b.reserve(max_count);
		
		// calculation cache
		struct CalcPoint {
			Vector pos;
			Vector tangent;
			Vector pa;
			Vector pb;
			Real n;
			Real length;
			Real w;
			CalcPoint(): n(), length(), w() { }
		} points[segments];
		CalcPoint &last_point = points[segments - 1];
		Real n = 0;
		for(CalcPoint *p = points, *end = points + segments; p < end; ++p, n += segment_step)
			p->n = n;
		last_point.n = 1;
		
		bool started = false;
		Vector first_pos;
		Real first_w = 0;

		for(ValueBase::List::const_iterator i0 = bline.begin(); i0 != bline.end(); ++i0) {
			ValueBase::List::const_iterator i1 = i0; ++i1;
			if (i1 == bline.end()) {
				if (!loop) break;
				i1 = bline.begin();
			}
			
			const BLinePoint &point0 = i0->get(blank);
			const BLinePoint &point1 = i1->get(blank);
			const Real width0 = gv*(point0.get_width()*width*0.5 + expand);
			const Real width1 = gv*(point1.get_width()*width*0.5 + expand);
			const Real dw = width1 - width0;

			if ( point0.get_vertex()  .is_equal_to( point1.get_vertex()   )
			  && point0.get_tangent2().is_equal_to( point1.get_tangent1() ) )
				continue;
			
			const etl::hermite<Vector> curve(
				point0.get_vertex(),
				point1.get_vertex(),
				point0.get_tangent2(),
				point1.get_tangent1() );
			
			// precalculate positions and length
			Real length = 0.0;
			
			for(CalcPoint *prev = 0, *p = points, *end = points + segments; p < end; prev = p++) {
				p->pos = curve(p->n);
				if (prev) length += (p->pos - prev->pos).mag();
				p->length = length;
			}
			Real div_length = approximate_greater(length, Real()) ? 1.0/length : 1.0;
			
			for(CalcPoint *p = points, *end = points + segments; p < end; ++p) {
				const Real wk = homogeneous_width ? p->length * div_length : p->n;
				p->w = dw*wk + width0;
				const Real nn = clamp(p->n, derivative_bound, 1 - derivative_bound);
				p->tangent = curve(nn + derivative_step) - curve(nn - derivative_step);
				const Vector perp = p->tangent.perp().norm() * p->w;
				p->pa = p->pos + perp;
				p->pb = p->pos - perp;
			}
			
			// make cusps
			if (!started) { started = true; first_pos = points->pos; first_w = points->w; }
			make_cusp(side_a, points->pos, points->w, sharp_cusps, arc_level, points->pa);
			make_cusp(side_b, points->pos, points->w, sharp_cusps, arc_level, points->pb);
			
			// make outline segment
			for(CalcPoint *prev = points, *p = points + 1, *end = points + segments; p < end; prev = p++) {
				side_a.push_back(
					rendering::Contour::Chunk(
						p->pa,
						line_intersection(prev->pa, prev->tangent, p->pa, p->tangent) ));
				side_b.push_back(
					rendering::Contour::Chunk(
						p->pb,
						line_intersection(prev->pb, prev->tangent, p->pb, p->tangent) ));
			}
		}

		if (started) {
			// close the loop
			if (loop) {
				make_cusp(side_a, last_point.pos, last_point.w, sharp_cusps, arc_level, side_a.front().p1);
				make_cusp(side_b, last_point.pos, last_point.w, sharp_cusps, arc_level, side_b.front().p1);
			}
			
			// tip at the end
			if (!loop && round_tip[1])
				make_arc(side_a, last_point.pos, last_point.w, side_b.back().p1, arc_level);
			else
				side_a.push_back( rendering::Contour::Chunk(side_b.back().p1) );
			
			// flip side_b
			Vector tmp;
			rendering::Contour::reverse(side_b, tmp);
			side_b.pop_back(); // remove initial move operation
			
			// tip at the beginning
			if (!loop && round_tip[0])
				make_arc(side_b, first_pos, first_w, side_a.front().p1, arc_level);
			else
				side_b.push_back( rendering::Contour::Chunk(side_a.front().p1) );
			
			// add side_b (with the beginning tip)
			add(side_a);
			add(side_b);
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

