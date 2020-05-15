/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/transformation.h
**	\brief Transformation Header
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

#ifndef __SYNFIG_RENDERING_TRANSFORMATION_H
#define __SYNFIG_RENDERING_TRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "mesh.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


class Transformation: public etl::shared_object
{
public:
	typedef etl::handle<Transformation> Handle;
	
	struct Bounds {
		Rect rect;
		Vector resolution;
		
		explicit Bounds(const Rect &rect = Rect(), const Vector &resolution = Vector(1.0, 1.0)):
			rect(rect), resolution(resolution) { }
		
		inline bool is_valid() const {
			return rect.is_valid()
				&& !rect.is_nan_or_inf()
				&& resolution.is_valid()
				&& !resolution.is_nan_or_inf()
				&& approximate_greater(resolution[0], 0.0)
				&& approximate_greater(resolution[1], 0.0);
		}
	};
	
	struct DiscreteBounds {
		Rect rect;
		VectorInt size;
		explicit DiscreteBounds(const Rect &rect = Rect(), const VectorInt &size = VectorInt()):
			rect(rect), size(size) { }
		
		inline bool is_valid() const {
			return rect.is_valid()
				&& !rect.is_nan_or_inf()
				&& size[0] > 0
				&& size[1] > 0;
		}
	};
	
 	static DiscreteBounds make_discrete_bounds(const Bounds &bounds);
	
protected:
	virtual Transformation* clone_vfunc() const
		{ return 0; }
	virtual Transformation* create_inverted_vfunc() const
		{ return 0; }
	
	virtual Point transform_vfunc(const Point &x) const = 0;
	virtual Matrix2 derivative_vfunc(const Point &x) const;
	virtual Bounds transform_bounds_vfunc(const Bounds &bounds) const = 0;
	
	virtual Mesh::Handle build_mesh_vfunc(const Rect &target_rect, const Vector &precision) const;
	
	virtual bool can_merge_outer_vfunc(const Transformation &other) const;
	virtual bool can_merge_inner_vfunc(const Transformation &other) const;
	virtual void merge_outer_vfunc(const Transformation &other);
	virtual void merge_inner_vfunc(const Transformation &other);
	
public:
	virtual ~Transformation() { }
	
	Transformation* clone() const
		{ return clone_vfunc(); }
	Transformation* create_inverted() const
		{ return create_inverted_vfunc(); }
	Transformation* create_merged(const Transformation& other) const;
	
	Point transform(const Point &x) const
		{ return transform_vfunc(x); }
	Matrix2 derivative(const Point &x) const
		{ return derivative_vfunc(x); }
	Bounds transform_bounds(const Bounds &bounds) const
		{ return transform_bounds_vfunc(bounds); }
	Bounds transform_bounds(const Rect &bounds) const
		{ return transform_bounds_vfunc(Bounds(bounds)); }
	Bounds transform_bounds(const Rect &bounds, const Vector &resolution) const
		{ return transform_bounds_vfunc(Bounds(bounds, resolution)); }
	
	bool can_merge_outer(const Transformation& other) const;
	bool can_merge_inner(const Transformation& other) const;
	bool merge_outer(const Transformation& other); //!< current is child (inner), other is parent (outer)
	bool merge_inner(const Transformation& other); //!< current is parent (outer), other is child (inner)
	
	bool can_merge_outer(const Transformation::Handle& other) const
		{ return other && can_merge_outer(*other); }
	bool can_merge_inner(const Transformation::Handle& other) const
		{ return other && can_merge_inner(*other); }
	
	bool merge_outer(const Transformation::Handle& other)
		{ return other && merge_outer(*other); }
	bool merge_inner(const Transformation::Handle& other)
		{ return other && merge_inner(*other); }
	
	Mesh::Handle build_mesh(const Rect &target_rect, const Vector &precision) const;
	Mesh::Handle build_mesh(const Point &target_rect_lt, const Point &target_rect_rb, const Vector &precision) const;
};


//! Transforms all points to NaN
class TransformationVoid: public Transformation
{
public:
	typedef etl::handle<TransformationVoid> Handle;
protected:
	virtual Transformation* clone_vfunc() const
		{ return new TransformationVoid(); }
	virtual Point transform_vfunc(const Point& /* x */) const
		{ return Point::nan(); }
	virtual Matrix2 derivative_vfunc(const Point& /* x */) const
		{ return Matrix2(real_nan<Real>(), real_nan<Real>(), real_nan<Real>(), real_nan<Real>()); }
	virtual Bounds transform_bounds_vfunc(const Bounds& /* bounds */) const
		{ return Bounds(); }
	virtual bool can_merge_outer_vfunc(const Transformation& /* other */) const
		{ return true; }
	virtual bool can_merge_inner_vfunc(const Transformation& /* other */) const
		{ return true; }
};


//! Keeps points unchanged while transformation
class TransformationNone: public Transformation
{
public:
	typedef etl::handle<TransformationNone> Handle;
protected:
	virtual Transformation* clone_vfunc() const
		{ return new TransformationNone(); }
	virtual Transformation* create_inverted_vfunc() const
		{ return clone_vfunc(); }
	virtual Point transform_vfunc(const Point &x) const
		{ return x; }
	virtual Matrix2 derivative_vfunc(const Point& /* x */) const
		{ return Matrix2(); }
	virtual Bounds transform_bounds_vfunc(const Bounds &bounds) const
		{ return bounds; }
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
