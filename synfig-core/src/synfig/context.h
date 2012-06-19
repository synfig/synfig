/* === S Y N F I G ========================================================= */
/*!	\file context.h
**	\brief Iterator for the layers behind the current Layer.
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

#ifndef __SYNFIG_CONTEXT_H
#define __SYNFIG_CONTEXT_H

/* === H E A D E R S ======================================================= */

#include "canvasbase.h"
#include "rect.h"
#include "renddesc.h"
#include "surface.h"
#include "layer_composite.h"
#include "general.h"

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

/*!	\class Context
**	\brief Context is a class to warp the iterator for a double queue of layers
* (that is the CanvasBase).
**	\see Layer, Canvas, CanvasBase */
class Context : public CanvasBase::const_iterator
{
public:
	Context() { }

	//! Constructor based on other CanvasBase iterator
	Context(const CanvasBase::const_iterator &x):CanvasBase::const_iterator(x) { }

	//!Assignation operator
	Context operator=(const CanvasBase::const_iterator &x)
	{ return CanvasBase::const_iterator::operator=(x); }

	//!	Returns the color of the context at the Point \pos.
	//! It is the blended color of the context
	Color get_color(const Point &pos)const;

	//!	With a given \quality and a given render description it puts the context
	//! blend result into the painting \surface */
	bool accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const
	{ return accelerated_render_<Surface>(surface, quality, renddesc, cb); }

	bool accelerated_render(CairoSurface */*surface*/,int /*quality*/, const RendDesc &/*renddesc*/, ProgressCallback */*cb*/) const
	{ return true; }
	// accelerated_render_<CairoSurface>(surface, quality, renddesc, cb); }

	template<class S>
	bool accelerated_render_(S *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;

	//! Sets the context to the Time \time. It is done recursively.
	void set_time(Time time)const;

	//!	Sets the context to the Time \time. It is done recursively. Vector \pos is not used
	void set_time(Time time,const Vector &pos)const;

	//! Returns the bounding rectangle of all the context.
	//! It is the union of all the layers's bounding rectangle.
	Rect get_full_bounding_rect()const;

	//! Returns the first context's layer's handle that intesects the given \point */
	etl::handle<Layer> hit_check(const Point &point)const;

	//! Sets dirty (dirty_time_= Time::end()) to all Outline type layers
	void set_dirty_outlines();

}; // END of class Context

template <class S>
bool
Context::accelerated_render_(S *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const
{
#ifdef SYNFIG_PROFILE_LAYERS
	String layer_name(curr_layer);
	//sum the pre-work done by layer above us... (curr_layer is layer above us...)
	if(depth>0)
	{
		time_table[curr_layer]+=profile_timer();
		//if(run_table.count(curr_layer))run_table[curr_layer]++;
		//	else run_table[curr_layer]=1;
	}
#endif	// SYNFIG_PROFILE_LAYERS
	
	const Rect bbox(renddesc.get_rect());
	// this is going to be set to true if this layer contributes
	// nothing, but it's a straight blend with non-zero amount, and so
	// it has an effect anyway
	bool straight_and_empty = false;
	etl::handle<Layer_Composite> composite;
	Context context(*this);
	// Run all layers until context is empty
	for(;!(context)->empty();++context)
	{
		// If we are not active then move on to next layer
		if(!(*context)->active())
			continue;
		const Rect layer_bounds((*context)->get_bounding_rect());
		// Cast current layer to composite
		composite = etl::handle<Layer_Composite>::cast_dynamic(*context);
		// If the box area is less than zero or the boxes do not
		// intersect then move on to next layer, unless the layer is
		// using a straight blend and has a non-zero amount, in which
		// case it will still affect the result
		if(layer_bounds.area() <= 0.0000000000001 || !(layer_bounds && bbox))
		{
			if (composite &&
				S::value_type::is_straight(composite->get_blend_method()) &&
				composite->get_amount() != 0.0f)
			{
				straight_and_empty = true;
				break;
			}
			continue;
		}
		// If this layer has Straight as the blend method and amount
		// is 1.0, and the layer doesn't depend on its context, then
		// we don't want to render the context
		if (composite &&
			composite->get_blend_method() == Color::BLEND_STRAIGHT &&
			composite->get_amount() == 1.0f &&
			!composite->reads_context())
		{
			Layer::Handle layer = *context;
			while (!context->empty()) context++; // skip the context
			return layer->accelerated_render(context,surface,quality,renddesc, cb);
		}
		// Break out of the loop--we have found a good layer
		break;
	}
	// If this layer isn't defined, return alpha
	if (context->empty() || (straight_and_empty && composite->get_amount() == 1.0f))
	{
#ifdef SYNFIG_DEBUG_LAYERS
		synfig::info("Context::accelerated_render(): Hit end of list");
#endif	// SYNFIG_DEBUG_LAYERS
		// resize the surface to the given render description
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		// and clear the surface
		surface->clear();
#ifdef SYNFIG_PROFILE_LAYERS
		profile_timer.reset();
#endif	// SYNFIG_PROFILE_LAYERS
		return true;
	}
	
#ifdef SYNFIG_DEBUG_LAYERS
	synfig::info("Context::accelerated_render(): Descending into %s",(*context)->get_name().c_str());
#endif	// SYNFIG_DEBUG_LAYERS
	
	try {
		// lock the context for reading
		RWLock::ReaderLock lock((*context)->get_rw_lock());
#ifdef SYNFIG_PROFILE_LAYERS
		//go down one layer :P
		depth++;
		curr_layer=(*context)->get_name();	//make sure the layer inside is referring to the correct layer outside
		profile_timer.reset(); 										// +
#endif	// SYNFIG_PROFILE_LAYERS
		bool ret;
		// this layer doesn't draw anything onto the canvas we're
		// rendering, but it uses straight blending, so we need to render
		// the stuff under us and then blit transparent pixels over it
		// using the appropriate 'amount'
		if (straight_and_empty)
		{
			if ((ret = Context((context+1)).accelerated_render(surface,quality,renddesc,cb)))
			{
				S clearsurface;
				clearsurface.set_wh(renddesc.get_w(),renddesc.get_h());
				clearsurface.clear();
				typename S::alpha_pen apen(surface->begin());
				apen.set_alpha(composite->get_amount());
				apen.set_blend_method(composite->get_blend_method());
				
				clearsurface.blit_to(apen);
			}
		}
		else
			ret = (*context)->accelerated_render(context+1,surface,quality,renddesc, cb);
#ifdef SYNFIG_PROFILE_LAYERS
		//post work for the previous layer
		time_table[curr_layer]+=profile_timer();							//-
		if(run_table.count(curr_layer))run_table[curr_layer]++;
		else run_table[curr_layer]=1;
		depth--;
		curr_layer = layer_name; //we are now onto this layer (make sure the post gets recorded correctly...
		//print out the table it we're done...
		if(depth==0) _print_profile_report(),time_table.clear(),run_table.clear();
		profile_timer.reset();												//+
#endif	// SYNFIG_PROFILE_LAYERS
		return ret;
	}
	catch(std::bad_alloc)
	{
		synfig::error("Context::accelerated_render(): Layer \"%s\" threw a bad_alloc exception!",(*context)->get_name().c_str());
#ifdef _DEBUG
		return false;
#else  // _DEBUG
		++context;
		return context.accelerated_render(surface, quality, renddesc, cb);
#endif	// _DEBUG
	}
	catch(...)
	{
		synfig::error("Context::accelerated_render(): Layer \"%s\" threw an exception, rethrowing...",(*context)->get_name().c_str());
		throw;
	}
}

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
