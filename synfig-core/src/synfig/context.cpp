/* === S Y N F I G ========================================================= */
/*!	\file context.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
#include "layer.h"
#include "string.h"
#include "vector.h"
#include "color.h"
#include "valuenode.h"

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

CairoColor
Context::get_cairocolor(const Point &pos)const
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
	if((context)->empty()) return CairoColor::alpha();
	
	RWLock::ReaderLock lock((*context)->get_rw_lock());
	
	return (*context)->get_cairocolor(context+1, pos);
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

	// If this layer isn't defined, return zero-sized rectangle
	if(context->empty()) return Rect::zero();

	return (*context)->get_full_bounding_rect(context+1);
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
Context::set_time(Time time)const
{
	Context context(*this);
	while(!(context)->empty())
	{
		// If this layer is active, and
		// it either isn't already set to the given time
		//        or it's a stroboscope layer,
		//        or it's a time loop layer,
		// then break out of the loop and set its time
		if((*context)->active() &&
		   (!(*context)->dirty_time_.is_equal(time) ||
			(*context)->get_name() == "stroboscope" ||
			(*context)->get_name() == "timeloop"))
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, just return
	if((context)->empty()) return;

	// Set up a writer lock
	RWLock::WriterLock lock((*context)->get_rw_lock());

	//synfig::info("%s: dirty_time=%f",(*context)->get_name().c_str(),(float)(*context)->dirty_time_);
	//synfig::info("%s: time=%f",(*context)->get_name().c_str(),(float)time);

	{
		Layer::ParamList params;
		Layer::DynamicParamList::const_iterator iter;
		// For each parameter of the layer sets the time by the operator()(time)
		for(iter=(*context)->dynamic_param_list().begin();iter!=(*context)->dynamic_param_list().end();iter++)
			params[iter->first]=(*iter->second)(time);
		// Sets the modified parameter list to the current context layer
		(*context)->set_param_list(params);
		// Calls the set time for the next layer in the context.
		(*context)->set_time(context+1,time);
		// Sets the dirty time the current calling time
		(*context)->dirty_time_=time;

	}
}

void
Context::set_dirty_outlines()
{
	Context context(*this);
	while(!(context)->empty())
	{
		if( (*context)->active() &&
			(
			(*context)->get_name() == "outline" ||
			(*context)->get_name() == "advanced_outline" ||
			(*context)->get_name() == "PasteCanvas"
			)
		  )
			{
				{
					(*context)->dirty_time_=Time::end();
				}
			}
		++context;
	}
}


void
Context::set_time(Time time,const Vector &/*pos*/)const
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
