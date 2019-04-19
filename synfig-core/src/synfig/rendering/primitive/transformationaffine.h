/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/transformationaffine.h
**	\brief TransformationAffine Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TRANSFORMATIONAFFINE_H
#define __SYNFIG_RENDERING_TRANSFORMATIONAFFINE_H

/* === H E A D E R S ======================================================= */

#include <synfig/matrix.h>

#include "transformation.h"

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

	Matrix matrix;

	TransformationAffine() { }
	explicit TransformationAffine(const Matrix &matrix): matrix(matrix) { }

protected:
	virtual Transformation* clone_vfunc() const;
	virtual Transformation* create_inverted_vfunc() const;
	virtual Point transform_vfunc(const Point &x, bool direction) const;
	virtual Bounds transform_bounds_vfunc(const Bounds &bounds) const;
	virtual bool can_merge_outer_vfunc(const Transformation &other) const;
	virtual bool can_merge_inner_vfunc(const Transformation &other) const;
	virtual void merge_outer_vfunc(const Transformation &other);
	virtual void merge_inner_vfunc(const Transformation &other);
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
