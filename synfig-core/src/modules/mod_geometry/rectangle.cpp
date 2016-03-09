/* === S Y N F I G ========================================================= */
/*!	\file rectangle.cpp
**	\brief Implementation of the "Rectangle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <ETL/pen>
#include <ETL/misc>

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "rectangle.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Rectangle);
SYNFIG_LAYER_SET_NAME(Rectangle,"rectangle");
SYNFIG_LAYER_SET_LOCAL_NAME(Rectangle,N_("Rectangle"));
SYNFIG_LAYER_SET_CATEGORY(Rectangle,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Rectangle,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Rectangle,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Rectangle::Rectangle():
	param_point1(ValueBase(Point(0,0))),
	param_point2(ValueBase(Point(1,1))),
	param_expand(ValueBase(Real(0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Rectangle::update_rect()
{
	Real expand = fabs(param_expand.get(Real()));
	Point p0 = param_point1.get(Point());
	Point p1 = param_point2.get(Point());
	if (p1[0] < p0[0]) swap(p0[0], p1[0]);
	if (p1[1] < p0[1]) swap(p0[1], p1[1]);

	std::vector<Point> list(4);

	list[0][0] = list[3][0] = p0[0] - expand;
	list[0][1] = list[1][1] = p0[1] - expand;
	list[2][0] = list[1][0] = p1[0] + expand;
	list[2][1] = list[3][1] = p1[1] + expand;

	set_stored_polygon(list);
}

bool
Rectangle::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_point1, { update_rect(); } );
	IMPORT_VALUE_PLUS(param_point2, { update_rect(); } );
	IMPORT_VALUE_PLUS(param_expand, { update_rect(); } );

	if ( param == "color"
	  || param == "invert" )
		return Layer_Polygon::set_param(param, value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Rectangle::get_param(const String &param)const
{
	EXPORT_VALUE(param_point1);
	EXPORT_VALUE(param_point2);
	EXPORT_VALUE(param_expand);

	EXPORT_NAME();
	EXPORT_VERSION();

	if ( param == "color"
	  || param == "invert" )
		return Layer_Polygon::get_param(param);

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Rectangle::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Fill color of the layer"))
	);

	ret.push_back(ParamDesc("point1")
		.set_local_name(_("Point 1"))
		.set_box("point2")
		.set_description(_("First corner of the rectangle"))
	);

	ret.push_back(ParamDesc("point2")
		.set_local_name(_("Point 2"))
		.set_description(_("Second corner of the rectangle"))
	);

	ret.push_back(ParamDesc("expand")
		.set_is_distance()
		.set_local_name(_("Expand amount"))
	);

	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert the rectangle"))
	);

	return ret;
}
