/* === S Y N F I G ========================================================= */
/*!	\file star.cpp
**	\brief Implementation of the "Star" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include "star.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/segment.h>

#endif

using namespace etl;

/* === M A C R O S ========================================================= */

#define SAMPLES		75

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Star);
SYNFIG_LAYER_SET_NAME(Star,"star");
SYNFIG_LAYER_SET_LOCAL_NAME(Star,N_("Star"));
SYNFIG_LAYER_SET_CATEGORY(Star,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Star,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Star::Star():
	param_radius1(ValueBase(Real(1.0))),
	param_radius2(ValueBase(Real(0.38))),
	param_points(ValueBase(int(5))),
	param_angle(ValueBase(Angle::deg(90))),
	param_regular_polygon(ValueBase(bool(false)))
{
	sync();
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Star::sync_vfunc()
{
	Angle angle = param_angle.get(Angle());
	int points = param_points.get(int(0));
	Real radius1 = param_radius1.get(Real());
	Real radius2 = param_radius2.get(Real());
	bool regular_polygon = param_regular_polygon.get(bool(true));
	
	Angle dist_between_points(Angle::rot(1)/float(points));
	std::vector<Point> vector_list;

	int i;
	for(i=0;i<points;i++)
	{
		Angle dist1(dist_between_points*i+angle);
		Angle dist2(dist_between_points*i+dist_between_points/2+angle);
		vector_list.push_back(Point(Angle::cos(dist1).get()*radius1,Angle::sin(dist1).get()*radius1));
		if (!regular_polygon)
			vector_list.push_back(Point(Angle::cos(dist2).get()*radius2,Angle::sin(dist2).get()*radius2));
	}
	set_stored_polygon(vector_list);
}

bool
Star::set_shape_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_radius1);
	IMPORT_VALUE(param_radius2);
	IMPORT_VALUE_PLUS(param_points,
		  {
			  int points(param_points.get(int(0)));
			  if(points<2)points=2;
			  param_points.set(points);
		  }
	);
	IMPORT_VALUE(param_angle);
	IMPORT_VALUE(param_regular_polygon);

	// Skip polygon parameters
	return Layer_Shape::set_shape_param(param,value);
}

bool
Star::set_param(const String & param, const ValueBase &value)
{
	// Skip polygon parameters
	return Layer_Shape::set_param(param,value);
}

ValueBase
Star::get_param(const String& param)const
{
	EXPORT_VALUE(param_radius1);
	EXPORT_VALUE(param_radius2);
	EXPORT_VALUE(param_points);
	EXPORT_VALUE(param_angle);
	EXPORT_VALUE(param_regular_polygon);

	EXPORT_NAME();
	EXPORT_VERSION();

	// Skip polygon parameters
	return Layer_Shape::get_param(param);
}

Layer::Vocab
Star::get_param_vocab()const
{
	// Skip polygon parameters
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("radius1")
		.set_local_name(_("Outer Radius"))
		.set_description(_("The radius of the outer points in the star"))
		.set_is_distance()
		.set_origin("origin")
	);

	ret.push_back(ParamDesc("radius2")
		.set_local_name(_("Inner Radius"))
		.set_description(_("The radius of the inner points in the star"))
		.set_is_distance()
		.set_origin("origin")
	);

	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Angle"))
		.set_description(_("The orientation of the star"))
		.set_origin("origin")
	);

	ret.push_back(ParamDesc("points")
		.set_local_name(_("Points"))
		.set_description(_("The number of points in the star"))
	);

	ret.push_back(ParamDesc("regular_polygon")
		.set_local_name(_("Regular Polygon"))
		.set_description(_("When checked, draws a regular polygon instead of a star"))
	);

	return ret;
}

