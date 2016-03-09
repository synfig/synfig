/* === S Y N F I G ========================================================= */
/*!	\file filledrect.cpp
**	\brief Implementation of the "Rectangle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include "filledrect.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(FilledRect);
SYNFIG_LAYER_SET_NAME(FilledRect,"filled_rectangle");
SYNFIG_LAYER_SET_LOCAL_NAME(FilledRect,N_("Filled Rectangle"));
SYNFIG_LAYER_SET_CATEGORY(FilledRect,N_("Example"));
SYNFIG_LAYER_SET_VERSION(FilledRect,"0.1");
SYNFIG_LAYER_SET_CVS_ID(FilledRect,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

FilledRect::FilledRect():
	Layer_Polygon(),
	param_point1(ValueBase(Vector(0,0))),
	param_point2(ValueBase(Vector(1,1))),
	param_feather_x(ValueBase(Real(0))),
	param_feather_y(ValueBase(Real(0))),
	param_bevel(ValueBase(Real(0))),
	param_bevCircle(ValueBase(bool(false)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void FilledRect::sync_vfunc()
{
	Point p0 = param_point1.get(Point());
	Point p1 = param_point2.get(Point());
	Real bevel = fabs(param_bevel.get(Real()));
	bool bev_circle = param_bevCircle.get(bool());

	if (p1[0] < p0[0]) swap(p0[0], p1[0]);
	if (p1[1] < p0[1]) swap(p0[1], p1[1]);

	Real w = p1[0] - p0[0];
	Real h = p1[1] - p0[1];
	Real bev = (bevel > 1) ? 1 : bevel;
	Real bevx = bev_circle ? min(w*bev/2.0, h*bev/2.0) : w*bev/2.0;
	Real bevy = bev_circle ? min(w*bev/2.0, h*bev/2.0) : h*bev/2.0;;

	clear();
	if (approximate_equal(bevel, 0.0))
	{
		move_to(p0[0], p0[1]);
		line_to(p1[0], p0[1]);
		line_to(p1[0], p1[1]);
		line_to(p0[0], p1[1]);
		close();
	}
	else
	{
		move_to (p1[0]-bevx, p0[1]);
		conic_to(p1[0], p0[1]+bevy, p1[0], p0[1]);
		line_to (p1[0], p1[1]-bevy);
		conic_to(p1[0]-bevx, p1[1], p1[0], p1[1]);
		line_to (p0[0]+bevx, p1[1]);
		conic_to(p0[0], p1[1]-bevy, p0[0], p1[1]);
		line_to (p0[0], p0[1]+bevy);
		conic_to(p0[0]+bevx, p0[1], p0[0], p0[1]);
		close();
	}
}

bool
FilledRect::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_point1, force_sync());
	IMPORT_VALUE_PLUS(param_point2, force_sync());
	IMPORT_VALUE_PLUS(param_feather_x,
		{
			Real feather_x=param_feather_x.get(Real());
			if(feather_x<0) feather_x=0;
			param_feather_x.set(feather_x);
			set_feather(Vector(feather_x, get_feather()[1]));
		});
	IMPORT_VALUE_PLUS(param_feather_y,
		  {
			  Real feather_y=param_feather_y.get(Real());
			  if(feather_y<0) feather_y=0;
			  param_feather_y.set(feather_y);
			  set_feather(Vector(get_feather()[0], feather_y));
		  });
	IMPORT_VALUE_PLUS(param_bevel, force_sync());
	IMPORT_VALUE_PLUS(param_bevCircle, force_sync());

	if (param == "color")
		return Layer_Polygon::set_param(param, value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
FilledRect::get_param(const String &param)const
{
	EXPORT_VALUE(param_point1);
	EXPORT_VALUE(param_point2);
	EXPORT_VALUE(param_feather_x);
	EXPORT_VALUE(param_feather_y);
	EXPORT_VALUE(param_bevel);
	EXPORT_VALUE(param_bevCircle);

	EXPORT_NAME();
	EXPORT_VERSION();

	if (param == "color")
		return Layer_Polygon::get_param(param);

	return Layer_Composite::get_param(param);
}

Layer::Vocab
FilledRect::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Fill color of the layer"))
	);

	ret.push_back(ParamDesc("point1")
		.set_local_name(_("Point 1"))
		.set_description(_("First corner of the rectangle"))
		.set_box("point2")
	);

	ret.push_back(ParamDesc("point2")
		.set_local_name(_("Point 2"))
		.set_description(_("Second corner of the rectangle"))
	);

	ret.push_back(ParamDesc("feather_x")
		.set_local_name(_("Feather X"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("feather_y")
		.set_local_name(_("Feather Y"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("bevel")
		.set_local_name(_("Bevel"))
		.set_description(_("Use Bevel for the corners"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("bevCircle")
		.set_local_name(_("Keep Bevel Circular"))
		.set_description(_("When checked the bevel is circular"))
	);

	return ret;
}
