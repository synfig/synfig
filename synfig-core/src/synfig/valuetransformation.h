
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
	static bool check_type(ValueBase::TypeId type) {
		switch(type) {
		case ValueBase::TYPE_ANGLE:
		case ValueBase::TYPE_BLINEPOINT:
		case ValueBase::TYPE_MATRIX:
		case ValueBase::TYPE_SEGMENT:
		case ValueBase::TYPE_TRANSFORMATION:
		case ValueBase::TYPE_VECTOR:
		case ValueBase::TYPE_WIDTHPOINT:
			return true;
		default:
			break;
		}
		return false;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase transform(const Transformation &transformation, const ValueBase &value) {
		switch(value.get_type()) {
		case ValueBase::TYPE_ANGLE:
			return value.get(Angle()) + transformation.angle;
		case ValueBase::TYPE_BLINEPOINT:
			{
				BLinePoint bp(value.get(BLinePoint()));
				bp.set_vertex( transformation.transform(bp.get_vertex()) );
				bp.set_tangent1( transformation.transform(bp.get_tangent1(), false) );
				bp.set_tangent2( transformation.transform(bp.get_tangent2(), false) );
				return bp;
			}
			break;
		case ValueBase::TYPE_MATRIX:
			return transformation.transform(value.get(Matrix()));
		case ValueBase::TYPE_SEGMENT:
			{
				Segment s(value.get(Segment()));
				s.p1 = transformation.transform(s.p1);
				s.t1 = transformation.transform(s.t1, false);
				s.p2 = transformation.transform(s.p2);
				s.t2 = transformation.transform(s.t2, false);
				return s;
			}
			break;
		case ValueBase::TYPE_TRANSFORMATION:
			return transformation.transform(value.get(Transformation()));
		case ValueBase::TYPE_VECTOR:
			return transformation.transform(value.get(Vector()));
		case ValueBase::TYPE_WIDTHPOINT:
			{
				WidthPoint wp(value.get(WidthPoint()));
				wp.set_width( wp.get_width()*transformation.scale[1] );
				return wp;
			}
			break;
		default:
			break;
		}
		return value;
	}

	static ValueBase back_transform(const Transformation &transformation, const ValueBase &value)
		{ return transform(transformation.get_back_transformation(), value); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
