/* === S Y N F I G ========================================================= */
/*!	\file layer_shape.h
**	\brief Header file for implementation of the "Shape" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_LAYER_SHAPE_H
#define __SYNFIG_LAYER_SHAPE_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>

#include <synfig/rendering/primitive/contour.h>

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Shape
**	\brief writeme			*/
class Layer_Shape : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

protected:
	//!Parameter: (Color) Color of the shape
	ValueBase   param_color;
	//!Parameter: (Point) Origin of the shape definition
	ValueBase   param_origin;
	//!Parameter: (bool) Whether the shape is inverted or not
	ValueBase   param_invert;
	//!Parameter: (bool) Whether the shape has antialiased edge
	ValueBase   param_antialias;
	//!Parameter: (Blur::Type) The type of blur used for the feather
	ValueBase   param_blurtype;
	//!Parameter: (Real) Amount of feather of the shape
	ValueBase   param_feather;
	//!Parameter: (WindingStyle) How shape is rendered when crosses it self
	ValueBase	param_winding_style;

private:
	rendering::Contour::Handle contour;
	Vector feather;

	mutable Time last_sync_time;
	mutable Real last_sync_outline_grow;

protected:
	Layer_Shape(const Real &a = 1.0, const Color::BlendMethod m = Color::BLEND_COMPOSITE);

public:

	~Layer_Shape();

protected:
	rendering::Contour& shape_contour()
		{ return *contour; }
	
	void clear();
	void move_to(Real x, Real y);
	void line_to(Real x, Real y);
	void conic_to(Real x, Real y, Real x1, Real y1);
	void cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2);
	void close();

	void move_to(const Vector &p)
		{ move_to(p[0], p[1]); }
	void line_to(const Vector &p)
		{ line_to(p[0], p[1]); }
	void conic_to(const Vector &p, const Vector &p1)
		{ conic_to(p[0], p[1], p1[0], p1[1]); }
	void cubic_to(const Vector &p, const Vector &p1, const Vector &p2)
		{ cubic_to(p[0], p[1], p1[0], p1[1], p2[0], p2[1]); }

	void add(const rendering::Contour::Chunk &chunk);
	void add(const rendering::Contour::ChunkList &chunks);

	//! list will attached as line
	//! curve information for first segment of incoming list will ignored
	void add_reverse(const rendering::Contour::ChunkList &list);

	Vector get_feather() const { return feather; }
	void set_feather(const Vector &x) { feather = x; }

public:
	void sync(bool force = false) const;
	void force_sync() const { sync(true); }

	virtual bool set_shape_param(const String & param, const synfig::ValueBase &value);
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context, Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual Rect get_bounding_rect()const;

protected:
	virtual void sync_vfunc();
	virtual void set_time_vfunc(IndependentContext context, Time time)const;
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;

private:
	bool render_shape(Surface *surface, bool useblend, const RendDesc &renddesc) const;
}; // END of Layer_Shape

}; // END of namespace synfig
/* === E N D =============================================================== */

#endif
