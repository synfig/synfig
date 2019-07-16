/* === S Y N F I G ========================================================= */
/*!	\file context.h
**	\brief Iterator for the layers behind the current Layer.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_CONTEXT_H
#define __SYNFIG_CONTEXT_H

/* === H E A D E R S ======================================================= */

#include "canvas.h"
#include "rect.h"
#include "renddesc.h"
#include "surface.h"
#include "rendering/task.h"

#include <synfig/layers/layer_composite.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Vector;
typedef Vector Point;
class Color;
class Surface;
class CairoSurface;
class RendDesc;
class ProgressCallback;
class Layer;
class Time;
class Rect;

/*!	\class IndependentContext
**	\brief IndependentContext is a class to warp the iterator for a double queue of layers
* (that is the CanvasBase).
**	\see Layer, Canvas, CanvasBase, Context */
class IndependentContext: public CanvasBase::const_iterator
{
public:
	IndependentContext() { }

	//! Constructor based on other CanvasBase iterator
	IndependentContext(const CanvasBase::const_iterator &x):CanvasBase::const_iterator(x) { }

	//! Assignation operator
	IndependentContext operator=(const CanvasBase::const_iterator &x)
	{ return CanvasBase::const_iterator::operator=(x); }

	//! Sets the context to the Time \time. It is done recursively.
	void set_time(Time time, bool force = false) const;
	
	//! Loads resources (frames from external files) for Time \time. It is done recursively.
	void load_resources(Time time, bool force = false) const;

	//! Sets the context outline grow to \outline_grow. It is done recursively.
	void set_outline_grow(Real outline_grow) const;
};


/*!	\class ContextParams
**	\brief ContextParams is a class to store rendering parameters significant for Context.
**	\see Context */
class ContextParams {
public:
	//! When \c true layers with exclude_from_rendering flag should be rendered
	bool render_excluded_contexts;
	//! When \c true layers are visible only in Z_Depth range
	bool z_range;
	//! Defines the starting position to apply Z_Depth visibility
	Real z_range_position;
	//! Defines the depth of the range of the Z_Depth visibility
	Real z_range_depth;
	//! Layers with z_Depth inside transition are partially visible
	Real z_range_blur;

	explicit ContextParams(bool render_excluded_contexts = false):
	render_excluded_contexts(render_excluded_contexts),
	z_range(false),
	z_range_position(0.0),
	z_range_depth(0.0),
	z_range_blur(0.0){ }
};

/*!	\class Context
**	\brief Context is a class to warp the iterator for a double queue of layers
* (that is the CanvasBase) with additional information about rendering parameters.
**	\see Layer, Canvas, CanvasBase, IndependentContext, ContextParams */
class Context : public IndependentContext
{
private:
	//! Rendering parameters significant for Context.
	ContextParams params;

public:

	Context() { }

	//! Constructor based on IndependentContext.
	Context(const IndependentContext &x, const ContextParams &params):
		IndependentContext(x), params(params) { }

	//! Constructor based on IndependentContext and other Context (to get parameters).
	Context(const IndependentContext &x, const Context &context):
		IndependentContext(x), params(context.params) { }

	//! Constructor based on other CanvasBase iterator and other Context (to get parameters).
	Context(const CanvasBase::const_iterator &x, const ContextParams &params):
		IndependentContext(x), params(params) { }

	Context(const CanvasBase::const_iterator &x, const Context &context):
		IndependentContext(x), params(context.params) { }

	//! Returns next iterator.
	Context get_next() const {
		IndependentContext c(*this);
		return Context(++c, params);
	}

	//! Returns previous iterator.
	Context get_previous() const {
		IndependentContext c(*this);
		return Context(--c, params);
	}

	//! Get rendering parameters.
	const ContextParams& get_params()const { return params; }

	//!	Returns the color of the context at the Point \pos.
	//! It is the blended color of the context
	Color get_color(const Point &pos)const;
	CairoColor get_cairocolor(const Point &pos)const;

	//!	With a given \quality and a given render description it puts the context
	//! blend result into the painting \surface */
	bool accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;
	bool accelerated_cairorender(cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;

	//!	Make rendering task
	rendering::Task::Handle build_rendering_task() const;

	//! Returns the bounding rectangle of all the context.
	//! It is the union of all the layers's bounding rectangle.
	Rect get_full_bounding_rect()const;

	//! Returns the first context's layer's handle that intesects the given \point */
	etl::handle<Layer> hit_check(const Point &point)const;

	// Set Render Method. Passes the information of the render method to use to the layers
	void set_render_method(RenderMethod x);

	//! Returns \c true if layer is active with this context_params
	static inline bool active(const ContextParams &context_params, const Layer &layer) {
		return layer.active()
		    && (context_params.render_excluded_contexts
		    || !layer.get_exclude_from_rendering());
	}

	//! Returns a value between 1.0 and 0.0 for layer visibility in z_depth range with this context_params
	static inline float z_depth_visibility(const ContextParams &cp, const Layer &layer)
	{
		if(!cp.z_range)
			return 1.0;
		float z=layer.get_true_z_depth();
		float p=cp.z_range_position;
		float d=cp.z_range_depth;
		float t=cp.z_range_blur;
		// Out of range
		if(z>p+d+t || z<p-t)
			return 0.0;
		else
		// Inside right range
		if(z>p+d)
			return t>0.0?(p+d+t-z)/t:0.0;
		else
		// Inside left range
		if(z<p)
			return t>0.0?(z-p+t)/t:0.0;
		else
		// Full visible
			return 1.0;
	}

	//! Returns \c true if layer is active in this context
	inline bool active(const Layer &layer) {
		return active(params, layer);
	}

	//! Returns \c true if layers is visible in z_depth range in this context
	inline float z_depth_visibility(const Layer &layer) {
		return z_depth_visibility(params, layer);
	}

	//! Returns \c true if layer is active in this context
	inline bool active()const {
		return !(operator*()).empty()
			 && active(params, *(operator*()));
	}

	//! Returns \c true if layer is visible in z_depth range in this context
	inline bool in_z_range()const {
		return !(operator*()).empty()
			 && z_depth_visibility(params, *(operator*()));
	}
	
}; // END of class Context

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
