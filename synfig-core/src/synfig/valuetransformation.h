
/* === S Y N F I G ========================================================= */
/*!	\file value_transformation.h
**	\brief Affine Transformation of ValueBase class
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

#ifndef __SYNFIG_VALUE_TRANSFORMATION_H
#define __SYNFIG_VALUE_TRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "value.h"
#include "transformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ValueTransformation
**	\todo writeme
*/
class ValueTransformation
{
private:
	//! it's static class
	ValueTransformation() { }

public:
	static bool check_type(Type &type) {
		return type == type_angle
			|| type == type_bline_point
			|| type == type_matrix
			|| type == type_segment
			|| type == type_transformation
			|| type == type_vector
			|| type == type_width_point;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase transform(const Transformation &transformation, const ValueBase &value) {
		Type &type(value.get_type());
		if (type == type_angle)
			return value.get(Angle()) + transformation.angle;
		else
		if (type == type_bline_point)
		{
			BLinePoint bp(value.get(BLinePoint()));
			bp.set_vertex( transformation.transform(bp.get_vertex()) );
			bp.set_tangent1( transformation.transform(bp.get_tangent1(), false) );
			bp.set_tangent2( transformation.transform(bp.get_tangent2(), false) );
			return bp;
		}
		else
		if (type == type_matrix)
			return transformation.transform(value.get(Matrix()));
		else
		if (type == type_segment)
		{
			Segment s(value.get(Segment()));
			s.p1 = transformation.transform(s.p1);
			s.t1 = transformation.transform(s.t1, false);
			s.p2 = transformation.transform(s.p2);
			s.t2 = transformation.transform(s.t2, false);
			return s;
		}
		else
		if (type == type_transformation)
			return transformation.transform(value.get(Transformation()));
		else
		if (type == type_vector)
			return transformation.transform(value.get(Vector()));
		else
		if (type == type_width_point)
		{
			WidthPoint wp(value.get(WidthPoint()));
			wp.set_width( wp.get_width()*transformation.scale[1] );
			return wp;
		}

		return value;
	}

	static ValueBase back_transform(const Transformation &transformation, const ValueBase &value)
		{ return transform(transformation.get_back_transformation(), value); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
