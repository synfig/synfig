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
	//internal caching
	struct Intersector;
	Intersector	*edge_table;
	rendering::Contour::Handle contour;

protected:
	Layer_Shape(const Real &a = 1.0, const Color::BlendMethod m = Color::BLEND_COMPOSITE);

public:

	~Layer_Shape();

	//! Clears out any data
	/*!	Also clears out the Intersector
	*/
	void clear();
	void move_to(Real x, Real y);
	void line_to(Real x, Real y);
	void conic_to(Real x, Real y, Real x1, Real y1);
	void cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2);
	void close();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context, Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual Rect get_bounding_rect()const;
	//This function translates Shape primitives to Cairo primitives. Currently only supported move_to and line_to.
	bool shape_to_cairo(cairo_t* cr)const;
	bool feather_cairo_surface(cairo_surface_t* surface, RendDesc renddesc, int quality)const;

protected:
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;

private:
	bool render_shape(Surface *surface, bool useblend, const RendDesc &renddesc) const;
}; // END of Layer_Shape

}; // END of namespace synfig
/* === E N D =============================================================== */

#endif
