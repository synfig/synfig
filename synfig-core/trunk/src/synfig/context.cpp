/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: context.cpp,v 1.4 2005/01/24 05:00:18 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
#include "layer.h"
#include "string.h"
#include "vector.h"
#include "color.h"
#include "surface.h"
#include "renddesc.h"
#include "valuenode.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

//#define SINFG_PROFILE_LAYERS
//#define SINFG_DEBUG_LAYERS

/* === G L O B A L S ======================================================= */

#ifdef SINFG_PROFILE_LAYERS
#include <ETL/clock>
static int depth(0);
static std::map<String,float> time_table;
static std::map<String,int> run_table;
static etl::clock profile_timer;
static String curr_layer;
static void
_print_profile_report()
{
	sinfg::info(">>>> Profile Report: (Times are in msecs)");
	std::map<String,float>::iterator iter;
	float total_time(0);
	for(iter=time_table.begin();iter!=time_table.end();++iter)
	{
		String layer(iter->first);
		float time(iter->second);
		int runs(run_table[layer]);
		total_time+=time;
		sinfg::info(" Layer \"%s\",\tExecs: %03d, Avg Time: %05.1f, Total Time: %05.1f",layer.c_str(),runs,time/runs*1000,time*1000);
	}
	sinfg::info("Total Time: %f seconds", total_time);
	sinfg::info("<<<< End of Profile Report");
}

#endif

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Color
Context::get_color(const Point &pos)const
{
	Context context(*this);
		
	while(!context->empty())
	{	
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return alpha
	if((context)->empty()) return Color::alpha();

	RWLock::ReaderLock lock((*context)->get_rw_lock());

	return (*context)->get_color(context+1, pos);
}

Rect
Context::get_full_bounding_rect()const
{
	Context context(*this);
		
	while(!context->empty())
	{	
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	if(context->empty())
		return Rect::zero();

	return (*context)->get_full_bounding_rect(*this+1);
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

bool
Context::accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const
{
	#ifdef SINFG_PROFILE_LAYERS
	String layer_name(curr_layer);
	
	//sum the pre-work done by layer above us... (curr_layer is layer above us...)
	if(depth>0)
	{
		time_table[curr_layer]+=profile_timer();
		//if(run_table.count(curr_layer))run_table[curr_layer]++;
		//	else run_table[curr_layer]=1;
	}
	#endif

	const Rect bbox(renddesc.get_rect());
	
	Context context(*this);
	for(;!(context)->empty();++context)
	{
		// If we are not active
		// then move on to next layer
		if(!(*context)->active())
			continue;
		
		const Rect layer_bounds((*context)->get_bounding_rect());
		
		// If the box area is less than zero
		// then move on to next layer
		if(layer_bounds.area()<=0.0000000000001)
			continue;
		
		// If the boxes do not intersect
		// then move on to next layer
		if(!(layer_bounds && bbox))
			continue;
		
		// Break out of the loop--we have found a good layer
		break;
	}

	// If this layer isn't defined, return alpha
	if((context)->empty())
	{
#ifdef SINFG_DEBUG_LAYERS
		sinfg::info("Context::accelerated_render(): Hit end of list");
#endif
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();
		#ifdef SINFG_PROFILE_LAYERS
		profile_timer.reset();	
		#endif
		return true;
	}

#ifdef SINFG_DEBUG_LAYERS
	sinfg::info("Context::accelerated_render(): Descending into %s",(*context)->get_name().c_str());
#endif

	try {
		RWLock::ReaderLock lock((*context)->get_rw_lock());
		
	#ifdef SINFG_PROFILE_LAYERS
	
	//go down one layer :P
	depth++;
	curr_layer=(*context)->get_name();	//make sure the layer inside is referring to the correct layer outside
	profile_timer.reset(); 										// +
	bool ret((*context)->accelerated_render(context+1,surface,quality,renddesc, cb));
	
	//post work for the previous layer
	time_table[curr_layer]+=profile_timer();							//-
	if(run_table.count(curr_layer))run_table[curr_layer]++;
		else run_table[curr_layer]=1;

	depth--;
	curr_layer = layer_name; //we are now onto this layer (make sure the post gets recorded correctly...
		
	//print out the table it we're done...
	if(depth==0) _print_profile_report(),time_table.clear(),run_table.clear();
	profile_timer.reset();												//+
	return ret;
	#else
	return (*context)->accelerated_render(context+1,surface,quality,renddesc, cb);
	#endif

	}
	catch(std::bad_alloc)
	{
		sinfg::error("Context::accelerated_render(): Layer \"%s\" threw a bad_alloc exception!",(*context)->get_name().c_str());
#ifdef _DEBUG
		return false;
#else
		++context;
		return context.accelerated_render(surface, quality, renddesc, cb);
#endif
	}
	catch(...)
	{
		sinfg::error("Context::accelerated_render(): Layer \"%s\" threw an exception, rethrowing...",(*context)->get_name().c_str());
		throw;		
	}
}

void
Context::set_time(Time time)const
{
	Context context(*this);
	while(!(context)->empty())
	{	
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active() && !(*context)->dirty_time_.is_equal(time))
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, just return
	if((context)->empty()) return;

	// Set up a wrter lock
	RWLock::WriterLock lock((*context)->get_rw_lock());

	//sinfg::info("%s: dirty_time=%f",(*context)->get_name().c_str(),(float)(*context)->dirty_time_);
	//sinfg::info("%s: time=%f",(*context)->get_name().c_str(),(float)time);

	{
		Layer::ParamList params;
		Layer::DynamicParamList::const_iterator iter;
		
		for(iter=(*context)->dynamic_param_list().begin();iter!=(*context)->dynamic_param_list().end();iter++)
			params[iter->first]=(*iter->second)(time);
		
		(*context)->set_param_list(params);

		(*context)->set_time(context+1,time);
		(*context)->dirty_time_=time;

	}
}

void
Context::set_time(Time time,const Vector &pos)const
{
	set_time(time);
/*
	Context context(*this);
	while(!(context)->empty())
	{	
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, just return
	if((context)->empty()) return;

	else
	{
		Layer::ParamList params;
		Layer::DynamicParamList::const_iterator iter;
		
		for(iter=(*context)->dynamic_param_list().begin();iter!=(*context)->dynamic_param_list().end();iter++)
			params[iter->first]=(*iter->second)(time);
		
		(*context)->set_param_list(params);

		(*context)->set_time(context+1,time,pos);
	}
*/
}

etl::handle<Layer>
Context::hit_check(const Point &pos)const
{
	Context context(*this);
	
	while(!context->empty())
	{	
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;
		
		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return an empty handle
	if((context)->empty()) return 0;

	return (*context)->hit_check(context+1, pos);
}
