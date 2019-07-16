/* === S Y N F I G ========================================================= */
/*!	\file context.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "context.h"

#include "general.h"
#include <synfig/localization.h>
#include "layer.h"
#include "string.h"
#include "vector.h"
#include "color.h"
#include "valuenode.h"
#include "transformation.h"

#include "layers/layer_pastecanvas.h"

#include "rendering/task.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

// #define SYNFIG_PROFILE_LAYERS
// #define SYNFIG_DEBUG_LAYERS

/* === G L O B A L S ======================================================= */

#ifdef SYNFIG_PROFILE_LAYERS
#include <ETL/clock>
static int depth(0);
static std::map<String,float> time_table;
static std::map<String,int> run_table;
static etl::clock profile_timer;
static String curr_layer;
static void
_print_profile_report()
{
	synfig::info(">>>> Profile Report: (Times are in msecs)");
	std::map<String,float>::iterator iter;
	float total_time(0);
	for(iter=time_table.begin();iter!=time_table.end();++iter)
	{
		String layer(iter->first);
		float time(iter->second);
		int runs(run_table[layer]);
		total_time+=time;
		synfig::info(" Layer \"%s\",\tExecs: %03d, Avg Time: %05.1f, Total Time: %05.1f",layer.c_str(),runs,time/runs*1000,time*1000);
	}
	synfig::info("Total Time: %f seconds", total_time);
	synfig::info("<<<< End of Profile Report");
}
#endif	// SYNFIG_PROFILE_LAYERS

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


void
IndependentContext::set_time(Time time, bool force)const
{
	IndependentContext context(*this);
	while(*context)
	{
		if ( (*context)->active() &&
		    (force || !(*context)->get_time_mark().is_equal(time)) )
			break;
		++context;
	}
	if (!*context) return;

	Layer::Handle layer(*context);
	++context;
	RWLock::WriterLock lock(layer->get_rw_lock());
	layer->set_time(context, time);
}

void
IndependentContext::load_resources(Time time, bool /*force*/)const
{
	IndependentContext context(*this);
	while(*context)
	{
		if ( (*context)->active() )
			break;
		++context;
	}
	if (!*context) return;

	Layer::Handle layer(*context);
	++context;
	//RWLock::WriterLock lock(layer->get_rw_lock());
	layer->load_resources(context, time);
}

void
IndependentContext::set_outline_grow(Real outline_grow)const
{
	IndependentContext context(*this);
	while(*context)
	{
		if ( (*context)->active()
		  && fabs((*context)->get_outline_grow_mark() - outline_grow) > 1e-8 )
			break;
		++context;
	}
	if (!*context) return;

	// Set up a writer lock
	
	Layer::Handle layer(*context);
	++context;
	RWLock::WriterLock lock(layer->get_rw_lock());
	layer->set_outline_grow(context, outline_grow);
}

Color
Context::get_color(const Point &pos)const
{
	Context context(*this);

	while(!context->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if(context.active() && context.in_z_range())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return alpha
	if((context)->empty()) return Color::alpha();

	RWLock::ReaderLock lock((*context)->get_rw_lock());

	return (*context)->get_color(context.get_next(), pos);
}

CairoColor
Context::get_cairocolor(const Point &pos)const
{
	Context context(*this);
	
	while(!context->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if(context.active() && context.in_z_range())
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}
	
	// If this layer isn't defined, return alpha
	if((context)->empty()) return CairoColor::alpha();
	
	RWLock::ReaderLock lock((*context)->get_rw_lock());
	
	return (*context)->get_cairocolor(context.get_next(), pos);
}


Rect
Context::get_full_bounding_rect()const
{
	Context context(*this);

	while(!context->empty())
	{
		// If this layer is active and visible in z_depth range,
		// then go ahead and break out of the loop
		if(context.active() && context.in_z_range())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return zero-sized rectangle
	if(context->empty()) return Rect::zero();

	return (*context)->get_full_bounding_rect(context.get_next());
}


/* Profiling will go like this:
	Profile start = +, stop = -

	+
	-

	time diff is recorded

	to get the independent times we need to break at the one inside and record etc...
	so it looks more like this:

	+
	  -
	  +
		-
		+
			...
		-
		+
	  -
	  +
	-

	at each minus we must record all the info for that which we are worried about...
	each layer can do work before or after the other work is done... so both values must be recorded...
*/


void
Context::set_render_method(RenderMethod x)
{
	Context context(*this);

	if((context)->empty()) return;
	
	(*context)->set_render_method(context.get_next(), x);
}

etl::handle<Layer>
Context::hit_check(const Point &pos)const
{
	Context context(*this);

	while(!context->empty() && context.in_z_range())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if(context.active())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return an empty handle
	if((context)->empty()) return 0;

	return (*context)->hit_check(context.get_next(), pos);
}


bool
Context::accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const
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
	const Matrix &transfromation_matrix(renddesc.get_transformation_matrix());
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
		if(!context.active())
			continue;
		const Rect layer_bounds(Transformation::transform_bounds(transfromation_matrix, (*context)->get_bounding_rect()));
		// Cast current layer to composite
		composite = etl::handle<Layer_Composite>::cast_dynamic(*context);
		// If the box area is less than zero or the boxes do not
		// intersect then move on to next layer, unless the layer is
		// using a straight blend and has a non-zero amount, in which
		// case it will still affect the result
		if(layer_bounds.area() <= 0.0000000000001 || !(layer_bounds && bbox))
		{
			if (composite &&
				Surface::value_type::is_straight(composite->get_blend_method()) &&
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
			if ((ret = Context((context.get_next())).accelerated_render(surface,quality,renddesc,cb)))
			{
				Surface clearsurface;
				clearsurface.set_wh(renddesc.get_w(),renddesc.get_h());
				clearsurface.clear();
				Surface::alpha_pen apen(surface->begin());
				apen.set_alpha(composite->get_amount());
				apen.set_blend_method(composite->get_blend_method());
				
				clearsurface.blit_to(apen);
			}
		}
		else
			ret = (*context)->accelerated_render(context.get_next(),surface,quality,renddesc, cb);
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
	catch(std::bad_alloc&)
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


bool
Context::accelerated_cairorender(cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb) const
{
#ifdef SYNFIG_PROFILE_LAYERS
	String layer_name(curr_layer);
	//sum the pre-work done by layer above us... (curr_layer is layer above us...)
	if(depth>0)
	{
		time_table[curr_layer]+=profile_timer();
	}
#endif	// SYNFIG_PROFILE_LAYERS
	
	Context context(*this);
	// Run all layers until context is empty
	for(;!(context)->empty();++context)
	{
		// If we are not active then move on to next layer
		if(!context.active())
			continue;
		// Found one good layer
		break;
	}
	// If this layer isn't defined, return alpha
	if (context->empty())
	{
#ifdef SYNFIG_DEBUG_LAYERS
		synfig::info("Context::accelerated_cairorender(): Hit end of list");
#endif	// SYNFIG_DEBUG_LAYERS
		// clear the surface
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
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
		ret = (*context)->accelerated_cairorender(context.get_next(),cr,quality,renddesc, cb);
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
	catch(std::bad_alloc&)
	{
		synfig::error("Context::accelerated_cairorender(): Layer \"%s\" threw a bad_alloc exception!",(*context)->get_name().c_str());
#ifdef _DEBUG
		return false;
#else  // _DEBUG
		++context;
		return context.accelerated_cairorender(cr, quality, renddesc, cb);
#endif	// _DEBUG
	}
	catch(...)
	{
		synfig::error("Context::accelerated_cairorender(): Layer \"%s\" threw an exception, rethrowing...",(*context)->get_name().c_str());
		throw;
	}
}

//!	Make rendering task using ContextParams
rendering::Task::Handle
Context::build_rendering_task() const
{
	Context context = *this;
	while ( *context
		 && ( !context.active()
		   || ( !get_params().render_excluded_contexts
			 && (*context)->get_exclude_from_rendering() )))
		++context;

	// TODO: apply z_range and z_blur (now applies in Canvas::optimize_layers)

	return *context
		 ? (*context)->build_rendering_task(context.get_next())
		 : rendering::Task::Handle();
}

