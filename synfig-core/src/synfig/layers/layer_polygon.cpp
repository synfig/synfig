/* === S Y N F I G ========================================================= */
/*!	\file layer_polygon.cpp
**	\brief Implementation of the "Polygon" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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

#include "layer_polygon.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Polygon);
SYNFIG_LAYER_SET_NAME(Layer_Polygon,"polygon");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Polygon,N_("Polygon"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Polygon,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Layer_Polygon,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Polygon,"$Id$");

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_Polygon::Layer_Polygon():
	Layer_Shape(1.0,Color::BLEND_COMPOSITE),
	param_vector_list(ValueBase(std::vector<ValueBase>()))
{
	std::vector<Point> vector_list;
	vector_list.push_back(Point(0,0.5));
	vector_list.push_back(Point(-0.333333,0));
	vector_list.push_back(Point(0.333333,0));
	param_vector_list.set_list_of(vector_list);
	sync();
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_Polygon::~Layer_Polygon()
	{ }

void
Layer_Polygon::add_polygon(const std::vector<Point> &point_list)
{
	int i,pointcount=point_list.size();

	if(pointcount<3)
		return;

	// Build edge table
	move_to(point_list[0][0],point_list[0][1]);

	for(i = 1;i < pointcount; i++)
	{
		if(std::isnan(point_list[i][0]) || std::isnan(point_list[i][1]))
			break;
		line_to(point_list[i][0],point_list[i][1]);
	}
	close();
}

void
Layer_Polygon::set_stored_polygon(const std::vector<Point> &point_list)
{
	ValueBase::List vector_list;
	vector_list.reserve(point_list.size());
	for(std::vector<Point>::const_iterator i = point_list.begin(); i != point_list.end(); ++i)
		vector_list.push_back(*i);
	param_vector_list.set(vector_list);
	Layer_Shape::clear();
	add_polygon(point_list);
}

void
Layer_Polygon::clear_stored_polygon()
{
	Layer_Shape::clear();
	param_vector_list.set(ValueBase::List());
}

bool
Layer_Polygon::set_param(const String & param, const ValueBase &value)
{
	if(	param=="vector_list" && param_vector_list.get_type()==value.get_type())
	{
		param_vector_list=value;
		Layer_Shape::clear();
		add_polygon(value.get_list_of(Vector()));
		return true;
	}

	return Layer_Shape::set_param(param,value);
}

ValueBase
Layer_Polygon::get_param(const String &param)const
{
	EXPORT_VALUE(param_vector_list);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Layer_Polygon::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("vector_list")
		.set_local_name(_("Vertices List"))
		.set_description(_("Define the corners of the polygon"))
		.set_origin("origin")
	);

	return ret;
}

