/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/affinetransformation.cpp
**	\brief AffineTransformation
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "transformationaffine.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Transformation::Handle
TransformationAffine::create_inverted_vfunc() const
	{ return new TransformationAffine(Matrix(matrix).invert()); }

Point
TransformationAffine::transform_vfunc(const Point &x, bool translate) const
	{ return matrix.get_transformed(x, translate); }

Transformation::Bounds
TransformationAffine::transform_bounds_vfunc(const Bounds &bounds) const
{
	if (!bounds.is_valid())
		return Bounds();

	// calculate bounds rect
	Vector corners[] = {
		matrix.get_transformed( bounds.rect.get_min() ),
		matrix.get_transformed( Vector(bounds.rect.minx, bounds.rect.maxy) ),
		matrix.get_transformed( Vector(bounds.rect.maxx, bounds.rect.miny) ),
		matrix.get_transformed( bounds.rect.get_max() ) };

	Rect rect = Rect( corners[0] )
			 .expand( corners[1] )
			 .expand( corners[2] )
			 .expand( corners[3] );

	// calculate units per "pixel"
	Real upp_x0 = fabs(corners[1][0] - corners[0][0]) / (bounds.resolution[0] * bounds.rect.get_width());
	Real upp_x1 = fabs(corners[2][0] - corners[0][0]) / (bounds.resolution[1] * bounds.rect.get_height());
	Real upp_y0 = fabs(corners[1][1] - corners[0][1]) / (bounds.resolution[0] * bounds.rect.get_width());
	Real upp_y1 = fabs(corners[2][1] - corners[0][1]) / (bounds.resolution[1] * bounds.rect.get_height());
	Vector upp(
		std::max(upp_x0, upp_x1),
		std::max(upp_y0, upp_y1) );

	if ( approximate_less_or_equal(upp[0], Real(0.0))
	  && approximate_less_or_equal(upp[1], Real(0.0)) )
		return Bounds();

	return Bounds(rect, Vector(1.0/upp[0], 1.0/upp[1]));
}

/* === E N T R Y P O I N T ================================================= */
