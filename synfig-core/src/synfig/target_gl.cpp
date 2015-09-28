/* === S Y N F I G ========================================================= */
/*!	\file target_gl.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "target_gl.h"

#include "context.h"
#include "rendering/renderer.h"
#include "rendering/software/surfacesw.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_GL::Target_GL()
{
	curr_frame_=0;
}

int
Target_GL::next_frame(Time& time)
{
	return Target::next_frame(time);
}

bool
Target_GL::render(ProgressCallback *cb)
{
	int
		frames=0,
		total_frames,
		frame_start,
		frame_end;
	Time
		t=0;

	assert(canvas);
	curr_frame_=0;

	if( !init() ){
		if(cb) cb->error(_("Target initialization failure"));
		return false;
	}

	frame_start=desc.get_frame_start();
	frame_end=desc.get_frame_end();

	ContextParams context_params(desc.get_render_excluded_contexts());

	// Calculate the number of frames
	total_frames=frame_end-frame_start+1;
	if(total_frames<=0)total_frames=1;

	try {
		do{
			// Grab the time
			if(total_frames>=1)
				frames=next_frame(t);
			else
				frames=0;

			// If we have a callback, and it returns
			// false, go ahead and bail. (it may be a user cancel)
			if(cb && !cb->amount_complete(total_frames-frames,total_frames))
				return false;
			
			Context context;
			// pass the Render Method to the context
			context=canvas->get_context(context_params);

			// Set the time that we wish to render
			if(!get_avoid_time_sync() || canvas->get_time()!=t)
				canvas->set_time(t);

			rendering::Task::Handle task = context.build_rendering_task();
			if (task)
			{
				rendering::Renderer::Handle renderer = rendering::Renderer::get_renderer("software");
				if (!renderer)
					throw String("Renderer 'gl' not found");

				task->target_surface = new rendering::SurfaceSW();
				task->target_surface->set_size(desc.get_w(), desc.get_h());
				task->rect_lt = desc.get_tl();
				task->rect_rb = desc.get_br();

				rendering::Task::List list;
				list.push_back(task);
				renderer->run(list);

				surfaces[curr_frame_] = task->target_surface;
				end_frame();
			}
		}while(frames);
	}
	catch(String str)
	{
		if(cb)cb->error(_("Caught string :")+str);
		return false;
	}
	catch(std::bad_alloc)
	{
		if(cb)cb->error(_("Ran out of memory (Probably a bug)"));
		return false;
	}
	catch(...)
	{
		if(cb)cb->error(_("Caught unknown error, rethrowing..."));
		throw;
	}
	return true;
}

rendering::Surface::Handle
Target_GL::get_surface(int frame) const
{
	std::map<int, rendering::Surface::Handle>::const_iterator i = surfaces.find(frame);
	return i == surfaces.end() ? rendering::Surface::Handle() : i->second;
}
