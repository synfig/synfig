
/* === S Y N F I G ========================================================= */
/*!	\file value_transformation.h
**	\brief Affine Transformation of ValueBase class
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

	static ValueBase transform(const Transformation &transformation, const ValueBase &value);

	static ValueBase back_transform(const Transformation &transformation, const ValueBase &value)
		{ return transform(transformation.get_back_transformation(), value); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
