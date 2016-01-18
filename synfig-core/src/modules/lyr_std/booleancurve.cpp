/* === S Y N F I G ========================================================= */
/*!	\file booleancurve.cpp
**	\brief Boolean Curve Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "booleancurve.h"

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

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

BooleanCurve::BooleanCurve()
{
}

BooleanCurve::~BooleanCurve()
{
}

bool BooleanCurve::set_param(const String & param, const synfig::ValueBase &value)
{
	if(param=="regions" && value.same_type_as(synfig::ValueBase::List()))
	{
		vector<BLinePoint> bv;
		int size = value.get_list().size();

		const vector<ValueBase> &vlist = value.get_list();

		regions.clear();
		for(int i = 0; i < size; ++i)
		{
			regions.push_back(vector<BLinePoint>(vlist[i].get_list_of(BLinePoint())));
		}
		return true;
	}

	return Layer_Shape::set_param(param,value);
}

ValueBase BooleanCurve::get_param(const String & param)const
{
	if(param=="regions")
	{
		ValueBase ret(regions);
		return ret;
	}
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab BooleanCurve::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("regions")
		.set_local_name(_("Region Set"))
		.set_description(_("Set of regions to combine"))
	);

	return ret;
}

Color BooleanCurve::get_color(Context /*context*/, const Point &/*pos*/)const
{
	Color c(Color::alpha());

	return c;
}

bool BooleanCurve::accelerated_render(Context /*context*/,Surface */*surface*/,int /*quality*/, const RendDesc &/*renddesc*/, ProgressCallback */*cb*/)const
{
	return false;
}
