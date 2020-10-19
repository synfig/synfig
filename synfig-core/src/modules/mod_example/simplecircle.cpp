/* === S Y N F I G ========================================================= */
/*!	\file simplecircle.cpp
**	\brief Implementation of the "Simple Circle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "simplecircle.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SimpleCircle);
SYNFIG_LAYER_SET_NAME(SimpleCircle,"simple_circle");
SYNFIG_LAYER_SET_LOCAL_NAME(SimpleCircle,N_("Simple Circle"));
SYNFIG_LAYER_SET_CATEGORY(SimpleCircle,N_("Example"));
SYNFIG_LAYER_SET_VERSION(SimpleCircle,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

SimpleCircle::SimpleCircle():
	param_radius(ValueBase(Real(0.5)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
SimpleCircle::sync_vfunc()
{
	const int num_splines = 8;
	const Angle::deg angle(180.0/(Real)num_splines);
	const Real k = 1.0 / Angle::cos(angle).get();

	Real radius = fabs(param_radius.get(Real()));

	Matrix2 matrix;
	matrix.set_rotate(angle);

	Vector p0, p1, p2(radius, 0.0);
	clear();
	move_to(p2[0], p2[1]);
	for(int i = 0; i < num_splines; ++i)
	{
		Vector p0 = p2;
		p1 = matrix.get_transformed(p0);
		p2 = matrix.get_transformed(p1);
		conic_to(p2[0], p2[1], k*p1[0], k*p1[1]);
	}
	close();
}

bool
SimpleCircle::set_shape_param(const synfig::String & param, const synfig::ValueBase &value)
{
	IMPORT_VALUE(param_radius);
	return false;
}

bool
SimpleCircle::set_param(const String & param, const ValueBase &value)
{
	if (set_shape_param(param, value))
		{ force_sync(); return true; }

	if ( param == "color" )
		return Layer_Shape::set_param(param, value);

	if ( param == "center" )
		return Layer_Shape::set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
SimpleCircle::get_param(const String &param)const
{
	EXPORT_VALUE(param_radius);

	EXPORT_NAME();
	EXPORT_VERSION();

	if ( param == "color" )
		return Layer_Shape::get_param(param);

	if ( param == "center" )
		return Layer_Shape::get_param("origin");

	return Layer_Composite::get_param(param);
}

Layer::Vocab
SimpleCircle::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());
	Layer::Vocab shape(Layer_Shape::get_param_vocab());

	ret.push_back(shape["color"]);
	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Center of the circle"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("Radius of the circle"))
		.set_origin("center")
		.set_is_distance()
	);

	return ret;
}
