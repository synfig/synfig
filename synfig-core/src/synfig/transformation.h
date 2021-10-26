/* === S Y N F I G ========================================================= */
/*!	\file transformation.h
**	\brief Affine Transformation
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRANSFORMATION_H
#define __SYNFIG_TRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "vector.h"
#include "matrix.h"
#include "rect.h"

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
	Angle skew_angle;
	Vector scale;

	Transformation():
		offset(0.0, 0.0),
		angle(Angle::rad(0.0)),
		skew_angle(Angle::rad(0.0)),
		scale(1.0, 1.0)
	{ }

	Transformation(
		const Vector &offset,
		const Angle &angle = Angle::rad(0.0),
		const Angle &skew_angle = Angle::rad(0.0),
		const Vector &scale = Vector(1.0, 1.0)
	):
		offset(offset),
		angle(angle),
		skew_angle(skew_angle),
		scale(scale)
	{ }

	bool is_valid()const
	{
		return offset.is_valid()
		    && !std::isnan(Angle::rad(angle).get())
		    && !std::isnan(Angle::rad(skew_angle).get())
		    && scale.is_valid();
	}

	bool
	operator<(const Transformation &rhs)const
	{
		return offset<rhs.offset         ? true : rhs.offset<offset         ? false
			 : angle<rhs.angle           ? true : rhs.angle<angle           ? false
			 : skew_angle<rhs.skew_angle ? true : rhs.skew_angle<skew_angle ? false
			 : scale<rhs.scale;
	}

	bool
	operator==(const Transformation &rhs)const
	{
		return offset==rhs.offset
			&& angle==rhs.angle
			&& skew_angle==rhs.skew_angle
			&& scale==rhs.scale;
	}

	bool
	operator!=(const Transformation &rhs)const
	{
		return offset!=rhs.offset
			|| angle!=rhs.angle
			|| skew_angle!=rhs.skew_angle
			|| scale!=rhs.scale;
	}

	bool is_equal_to(const Transformation& rhs)const
	{
		static const Angle::rad epsilon_angle(0.0000000000001);
		Angle::rad a = angle - rhs.angle;
		Angle::rad sa = skew_angle - rhs.skew_angle;
		return offset.is_equal_to(rhs.offset)
		    && a < epsilon_angle && a > -epsilon_angle
		    && sa < epsilon_angle && sa > -epsilon_angle
		    && scale.is_equal_to(rhs.scale);
	}

	bool is_identity()const
	{
		return is_equal_to(Transformation());
	}

	Matrix get_matrix() const
	{
		if (is_identity()) return Matrix();
		Vector axis_x(scale[0], angle);
		Vector axis_y(scale[1], angle + skew_angle + Angle::deg(90.0));
		return Matrix(axis_x, axis_y, offset);
	}

	void set_matrix(const Matrix &matrix)
	{
		if (matrix.is_identity()) *this = Transformation();
		Vector axis_x(matrix.axis_x());
		Vector axis_y(matrix.axis_y());
		angle = axis_x.angle();
		skew_angle = axis_y.angle() - angle - Angle::deg(90.0);
		scale[0] = axis_x.mag();
		scale[1] = axis_y.mag();
		offset = matrix.offset();
	}

	explicit Transformation(const Matrix &matrix)
		{ set_matrix(matrix); }

	Matrix get_inverted_matrix() const
		{ return get_matrix().invert(); }

	Transformation get_back_transformation() const
		{ return Transformation(get_inverted_matrix()); }

	static Rect transform_bounds(const Matrix &matrix, const Rect &bounds)
	{
		if (std::isnan(bounds.minx) || std::isinf(bounds.minx)
		 || std::isnan(bounds.maxx) || std::isinf(bounds.maxx)
		 || std::isnan(bounds.miny) || std::isinf(bounds.miny)
		 || std::isnan(bounds.maxy) || std::isinf(bounds.maxy))
			return Rect::infinite();

		Rect transformed_bounds(
			matrix.get_transformed(
				Vector(bounds.minx, bounds.miny) ));
		transformed_bounds.expand(
			matrix.get_transformed(
				Vector(bounds.minx, bounds.maxy) ));
		transformed_bounds.expand(
			matrix.get_transformed(
				Vector(bounds.maxx, bounds.miny) ));
		transformed_bounds.expand(
			matrix.get_transformed(
				Vector(bounds.maxx, bounds.maxy) ));
		return transformed_bounds;
	}

	Vector transform(const Vector &v, bool translate = true) const
		{ return get_matrix().get_transformed(v, translate); }
	Transformation transform(const Transformation &transformation) const
		{ return Transformation( get_matrix()*transformation.get_matrix() ); }
	Matrix transform(const Matrix &matrix) const
		{ return get_matrix()*matrix; }
	Rect transform_bounds(const Rect &bounds) const
		{ return transform_bounds(get_matrix(), bounds); }

	Vector back_transform(const Vector &v, bool translate = true) const
		{ return get_inverted_matrix().get_transformed(v, translate); }
	Transformation back_transform(const Transformation &transformation) const
		{ return Transformation( get_inverted_matrix()*transformation.get_matrix() ); }
	Matrix back_transform(const Matrix &matrix) const
		{ return get_inverted_matrix()*matrix; }
	Rect back_transform_bounds(const Rect &bounds) const
		{ return transform_bounds(get_inverted_matrix(), bounds); }

	static const Transformation identity() { return Transformation(); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
