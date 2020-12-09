/* === S Y N F I G ========================================================= */
/*!	\file ducktransform_matrix.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_MATRIX_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_MATRIX_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"
#include <synfig/general.h>
#include <synfig/matrix.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Transform_Matrix : public synfig::Transform
{
private:
	synfig::Matrix transform;
	synfig::Matrix inverse_transform;

public:
	Transform_Matrix(const synfig::GUID& guid, const synfig::Matrix& matrix):
		Transform(guid),
		transform(matrix)
	{
		if (transform.is_invertible())
			(inverse_transform = transform).invert();
		else
			synfig::warning("%s:%d passed a non-invertible matrix", __FILE__, __LINE__);
	}

	synfig::Vector perform(const synfig::Vector& x)const
	{
		return transform.get_transformed(x);
	}
	synfig::Vector unperform(const synfig::Vector& x)const
	{
		return inverse_transform.get_transformed(x);
	}

	synfig::String get_string()const
	{
		return "duck matrix";
	}
};

};

/* === E N D =============================================================== */

#endif
