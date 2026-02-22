/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Outline" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2018-2019 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/rendering/primitive/contour.h>
#include <synfig/rendering/primitive/bend.h>

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
SYNFIG_LAYER_SET_VERSION(Outline,"0.3");

/* === P R O C E D U R E S ================================================= */

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
	const int wire_segments = 64;
	const int contour_segments = 32;

	// Used for tip generation
	const double EPSILON = 1.0e-12;
	const Real ROUND_END_FACTOR = 4.0;
	const int TIP_SAMPLES = 50;

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

		std::vector<BLinePoint> points;
		for (const auto & i : bline)
			points.push_back(i.get(blank));

		for (size_t i = 0; i < points.size(); ++i) {

			// Check if tangent points backwards (opposite to the line segment)
			// Backward tangents cause the mesh to fold and break (artifacts)
			// We ignore the backward direction and force it to point forward
			if (i > 0 || loop) {
				size_t prev_idx = (i == 0) ? points.size() - 1 : i - 1;
				Vector chord = points[i].get_vertex() - points[prev_idx].get_vertex();
				if (points[i].get_tangent1() * chord < 0)
					points[i].set_tangent1(chord.norm() * points[i].get_tangent1().mag());
			}

			if (i < points.size() - 1 || loop) {
				size_t next_idx = (i == points.size() - 1) ? 0 : i + 1;
				Vector chord = points[next_idx].get_vertex() - points[i].get_vertex();
				if (points[i].get_tangent2() * chord < 0)
					points[i].set_tangent2(chord.norm() * points[i].get_tangent2().mag());
			}
		}
		// retrieve the parent canvas grow value
		Real gv = exp(get_outline_grow_mark());
		
		rendering::Bend bend;
		rendering::Contour contour;
		
		Real w = 0, w0 = 0;
		// Draw body only (no tips)
		for (auto i = points.begin(); i != points.end(); ++i) {
			const BLinePoint &point = *i;

			bend.add(
				point.get_vertex(),
				point.get_tangent1(),
				point.get_tangent2(),
				sharp_cusps ? rendering::Bend::CORNER : rendering::Bend::ROUND,
				homogeneous_width,
				wire_segments );
			Real length = bend.length1();
			
			w = gv*(point.get_width()*width*0.5 + expand);

			if (i == points.begin()) {
				w0 = w;
				if (loop) {
					contour.move_to( Vector(-w, 0) );
					contour.line_to( Vector(-w, w) );
				} else {
					contour.move_to( Vector(0, 0) );
					contour.line_to( Vector(0, w) );
				}
			}

			contour.line_to( Vector(length, w) );
		}
		
		if (loop) {
			bend.loop(homogeneous_width, wire_segments);
			Real length = bend.length1();
			contour.line_to( Vector(length, w0) );
			contour.line_to( Vector(length + w0, w0) );
			contour.line_to( Vector(length + w0, 0) );
		} else {
			bend.tails();
			Real length = bend.length1();
			contour.line_to( Vector(length, 0) );
		}
		
		contour.close_mirrored_vert();
		bend.bend(shape_contour(), contour, Matrix(), contour_segments);

		// Append round tips using hermite curves for accuracy
		if (!loop && points.size() >= 2) {
			auto draw_tip = [&](Vector pos, Vector tangent, Real tip_w, bool start_tip) {
				if (tangent.mag_squared() < EPSILON)
					return;
				tangent = tangent.norm();
				if(start_tip)
					tangent = -tangent;

				hermite<Vector> curve(
					pos + tangent.perp() * tip_w, pos - tangent.perp() * tip_w,
					tangent * tip_w * ROUND_END_FACTOR, -tangent * tip_w * ROUND_END_FACTOR);

				shape_contour().move_to(curve(0.0f));

				for (int i = 1; i <= TIP_SAMPLES; ++i) {
					shape_contour().line_to(curve(static_cast<float>(i) / TIP_SAMPLES));
				}

				shape_contour().line_to(curve(1.0f));
				shape_contour().close();
			};

			if (round_tip[0]) {
				Vector tangent = points.front().get_tangent2();
				if (tangent.mag_squared() < EPSILON)
					tangent = points[1].get_vertex() - points.front().get_vertex();
				draw_tip(points.front().get_vertex(), tangent, gv*(points.front().get_width()*width*0.5+expand), true);
			}

			if (round_tip[1]) {
				Vector tangent = points.back().get_tangent1();
				if (tangent.mag_squared() < EPSILON)
					tangent = points.back().get_vertex() - points[points.size()-2].get_vertex();
				draw_tip(points.back().get_vertex(), tangent, gv*(points.back().get_width()*width*0.5+expand), false);
			}

		}

	} catch (...) { synfig::error("Outline::sync(): Exception thrown"); throw; }
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
		.set_description(_("Determines cusps type"))
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
		.set_description(_("When checked, the width takes the length of the spline to interpolate"))
	);

	return ret;
}

