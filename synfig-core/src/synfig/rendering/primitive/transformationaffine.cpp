/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/affinetransformation.cpp
**	\brief AffineTransformation
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include "transformationaffine.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Transformation*
TransformationAffine::clone_vfunc() const
	{ return new TransformationAffine(matrix); }

Transformation*
TransformationAffine::create_inverted_vfunc() const
	{ return new TransformationAffine(Matrix(matrix).invert()); }

Point
TransformationAffine::transform_vfunc(const Point &x) const
	{ return matrix.get_transformed(x); }

Matrix2
TransformationAffine::derivative_vfunc(const Point&) const
	{ return Matrix2(matrix.m00, matrix.m01, matrix.m10, matrix.m11); }

Vector
TransformationAffine::calc_optimal_resolution(const Matrix2 &matrix) {
	const Real max_overscale_sqr = 1.0*4.0;
	const Real real_precision_sqr = real_precision<Real>() * real_precision<Real>();
	const Real real_precision_sqrsqr = real_precision_sqr * real_precision_sqr;
	
	const Real a = matrix.m00 * matrix.m00;
	const Real b = matrix.m01 * matrix.m01;
	const Real c = matrix.m10 * matrix.m10;
	const Real d = matrix.m11 * matrix.m11;
	Real e = fabs(matrix.det());
	if (e < real_precision_sqr)
		return Vector();
	e = 1.0/e;
	
	const Real sum = a*d + b*c;
	const Real ab2 = 2*a*b + real_precision_sqrsqr;
	const Real cd2 = 2*c*d + real_precision_sqrsqr;
	bool abgt = (ab2 >= sum);
	bool cdgt = (cd2 >= sum);
	if (abgt && cdgt) // when both is greater than sum select only lower of them
		(ab2 > cd2 ? abgt : cdgt) = false;
	
	Vector scale;
	if (abgt) {
		scale[0] = sqrt(2*b)*e;
		scale[1] = sqrt(2*a)*e;
	} else
	if (cdgt) {
		scale[0] = sqrt(2*d)*e;
		scale[1] = sqrt(2*c)*e;
	} else {
		const Real dif = a*d - b*c;
		scale[0] = sqrt(dif/(a - c))*e;
		scale[1] = sqrt(dif/(d - b))*e;
	}
	
	const Real sqr = scale[0]*scale[1]*(matrix.m00 + matrix.m10)*(matrix.m01 + matrix.m11);
	if (sqr > max_overscale_sqr)
		scale *= sqrt(max_overscale_sqr/sqr);
	
	return scale[0] <= real_precision<Real>() || scale[1] <= real_precision<Real>()
		 ? Vector() : scale;
}

Transformation::Bounds
TransformationAffine::transform_bounds_affine(const Matrix &matrix, const Bounds &bounds)
{
	if (!bounds.is_valid())
		return Bounds();
	
	const Real kx = 1/bounds.resolution[0];
	const Real ky = 1/bounds.resolution[1];
	
	return Bounds(
		   Rect( matrix.get_transformed( Vector(bounds.rect.minx, bounds.rect.miny) ) )
		.expand( matrix.get_transformed( Vector(bounds.rect.minx, bounds.rect.maxy) ) )
		.expand( matrix.get_transformed( Vector(bounds.rect.maxx, bounds.rect.miny) ) )
		.expand( matrix.get_transformed( Vector(bounds.rect.maxx, bounds.rect.maxy) ) ),
		calc_optimal_resolution(Matrix2(
			matrix.m00*kx, matrix.m01*kx,
			matrix.m10*ky, matrix.m11*ky )) );
}

Transformation::Bounds
TransformationAffine::transform_bounds_vfunc(const Bounds &bounds) const
	{ return transform_bounds_affine(matrix, bounds); }

bool
TransformationAffine::can_merge_outer_vfunc(const Transformation &other) const
	{ return (bool)dynamic_cast<const TransformationAffine*>(&other); }

bool
TransformationAffine::can_merge_inner_vfunc(const Transformation &other) const
	{ return can_merge_outer_vfunc(other); }

void
TransformationAffine::merge_outer_vfunc(const Transformation &other)
	{ matrix = dynamic_cast<const TransformationAffine*>(&other)->matrix * matrix; }

void
TransformationAffine::merge_inner_vfunc(const Transformation &other)
	{ matrix *= dynamic_cast<const TransformationAffine*>(&other)->matrix; }


/* === E N T R Y P O I N T ================================================= */
