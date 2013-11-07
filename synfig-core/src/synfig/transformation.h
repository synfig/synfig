/* === S Y N F I G ========================================================= */
/*!	\file transformation.h
**	\brief Affine Transformation
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRANSFORMATION_H
#define __SYNFIG_TRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Transformation
**	\todo writeme
*/
class Transformation
{
public:
	Vector offset;
	Angle angle;
	Vector scale;

	Transformation(): angle(Angle::rad(0.f)), scale(1.0, 1.0) { };
	Transformation(const Vector &offset, const Angle &angle, const Vector &scale):
		offset(offset), angle(angle), scale(scale) { }

	bool is_valid()const
		{ return offset.is_valid() && !isnan(Angle::rad(angle).get()) && scale.is_valid(); }

	bool
	operator==(const Transformation &rhs)const
		{ return offset==rhs.offset && angle==rhs.angle && scale==rhs.scale; }

	bool
	operator!=(const Transformation &rhs)const
		{ return offset!=rhs.offset || angle!=rhs.angle || scale!=rhs.scale; }

	bool is_equal_to(const Transformation& rhs)const
	{
		static const Angle::rad epsilon_angle(0.0000000000001);
		Angle::rad a = angle - rhs.angle;
		return offset.is_equal_to(rhs.offset)
		    && a < epsilon_angle
		    && a > -epsilon_angle
		    && scale.is_equal_to(rhs.scale);
	}

	static const Transformation identity() { return Transformation(); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
