/* === S Y N F I G ========================================================= */
/*!	\file region.cpp
**	\brief Implementation of the "Region" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "region.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/valuenodes/valuenode_bline.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

#define SAMPLES		75

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Region);
SYNFIG_LAYER_SET_NAME(Region,"region");
SYNFIG_LAYER_SET_LOCAL_NAME(Region,N_("Region"));
SYNFIG_LAYER_SET_CATEGORY(Region,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Region,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Region::Region()
{
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

void
Region::sync_vfunc()
{
	clear();

	const Real k = 1.0/3.0;
	const BLinePoint blank_point;
	const Segment blank_segment;

	// build splines
	bool first = true;
	bool first_warning = true;
	Vector prev;
	Vector prev_tangent;
	const ValueBase::List &list = param_bline.get_list();
	for(ValueBase::List::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (i->get_type() == type_bline_point) {
			// process BLinePoint
			const BLinePoint &point = i->get(blank_point);
			const Vector &p = point.get_vertex();

			if (first)
				{ first = false; move_to(p); }
			else
			if (prev_tangent.is_equal_to(Vector::zero()) && point.get_tangent1().is_equal_to(Vector::zero()))
				line_to(p);
			else
				cubic_to(p, prev + prev_tangent*k, p - point.get_tangent1()*k);

			prev = point.get_vertex();
			prev_tangent = point.get_tangent2();
		} else
		if (i->get_type() == type_segment) {
			// process Segment
			const Segment &segment = i->get(blank_segment);

			if (first)
				{ first = false; move_to(segment.p1); }
			else
			if (!segment.p1.is_equal_to(prev))
				line_to(segment.p1);

			if (segment.t1.is_equal_to(Vector::zero()) && segment.t2.is_equal_to(Vector::zero()))
				line_to(segment.p2);
			else
				cubic_to(segment.p2, segment.p1 + segment.t1*k, segment.p2 - segment.t2*k);

			prev = segment.p2;
			prev_tangent = Vector();
		} else
		if (first_warning) {
			// warning
			first_warning = false;
			synfig::warning("Region: incorrect type on bline");
		}
	}

	// close loop
	if ( !first
	  && param_bline.get_loop()
	  && list.front().get_type() == type_bline_point )
	{
		const BLinePoint &point = list.front().get(blank_point);
		const Vector &p = point.get_vertex();
		if ( !prev_tangent.is_equal_to(Vector::zero())
		  || !point.get_tangent1().is_equal_to(Vector::zero()) )
			cubic_to(p, prev + prev_tangent*k, p - point.get_tangent1()*k);
	}

	close();
}

bool
Region::set_shape_param(const String & param, const ValueBase &value)
{
	// TODO: move this backward compatibility code to load_canvas
	if(param=="segment_list")
	{
		if(dynamic_param_list().count("segment_list"))
		{
			connect_dynamic_param("bline",dynamic_param_list().find("segment_list")->second);
			disconnect_dynamic_param("segment_list");
			synfig::warning("Region::set_param(): Updated valuenode connection to use the new \"bline\" parameter.");
		}
		else
			synfig::warning("Region::set_param(): The parameter \"segment_list\" is deprecated. Use \"bline\" instead.");
	}

	if (param=="segment_list" || param=="bline")
	{
		if (value.get_type() != type_list)
			return false;
		//if (value.get_contained_type()!=type_bline_point)
		//	return false;
		param_bline=value;
		return true;
	}

	return Layer_Shape::set_shape_param(param, value);
}

ValueBase
Region::get_param(const String& param)const
{
	EXPORT_VALUE(param_bline);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Region::get_param_vocab()const
{
	// Skip polygon parameters
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_description(_("A list of spline points"))
	);

	return ret;
}
