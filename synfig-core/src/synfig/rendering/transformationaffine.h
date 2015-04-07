/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/transformationaffine.h
**	\brief TransformationAffine Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TRANSFORMATIONAFFINE_H
#define __SYNFIG_RENDERING_TRANSFORMATIONAFFINE_H

/* === H E A D E R S ======================================================= */

#include "transformation.h"
#include "../matrix.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TransformationAffine: public Transformation
{
public:
	typedef etl::handle<TransformationAffine> Handle;

private:
	Matrix matrix;

protected:
	virtual TransformedPoint transform_vfunc(const Point &x) const;

public:
	TransformationAffine();

	const Matrix& get_matrix() const { return matrix; }
	void set_matrix(const Matrix& x)
		{ if (get_matrix() != x) { matrix = x; changed(); } }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
