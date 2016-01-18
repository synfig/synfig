/* === S Y N F I G ========================================================= */
/*!	\file target_scanline.cpp
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

#include "target_scanline.h"

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "context.h"
#include "render.h"
#include "string.h"
#include "surface.h"
#include "rendering/software/surfacesw.h"
#include "rendering/renderer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

// note that if this isn't defined then the rendering is incorrect for
// the straight blend method since the optimize_layers() function in
// canvas.cpp which makes the straight blend method work correctly
// isn't called.  ie. leave this defined.  to see the problem, draw a
// small circle over a solid background.  set circle to amount 0.99
// and blend method 'straight'.  the background should vanish but doesn't
#define SYNFIG_OPTIMIZE_LAYER_TREE

#define PIXEL_RENDERING_LIMIT 1500000

#define USE_PIXELRENDERING_LIMIT 1

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Scanline::Target_Scanline():
	threads_(2)
{
	curr_frame_=0;
	if (const char *s = getenv("SYNFIG_TARGET_DEFAULT_ENGINE"))
		set_engine(s);
}

int
Target_Scanline::next_frame(Time& time)
{
	return Target::next_frame(time);
}

bool
synfig::Target_Scanline::call_renderer(Context &context, const etl::handle<rendering::SurfaceSW> &surfacesw, int quality, const RendDesc &renddesc, ProgressCallback *cb)
{
	surfacesw->set_size(renddesc.get_w(), renddesc.get_h());
	if (get_engine().empty())
	{
		if(!context.accelerated_render(&surfacesw->get_surface(),quality,renddesc,0))
		{
			// For some reason, the accelerated renderer failed.
			if(cb)cb->error(_("Accelerated Renderer Failure"));
			return false;
		}
	}
	else
	{
		rendering::Task::Handle task = context.build_rendering_task();
		if (task)
		{
			rendering::Renderer::Handle renderer = rendering::Renderer::get_renderer(get_engine());
			if (!renderer)
				throw "Renderer '" + get_engine() + "' not found";

			task->target_surface = surfacesw;
			task->target_surface->create();
			task->init_target_rect(RectInt(VectorInt::zero(), surfacesw->get_size()), renddesc.get_tl(), renddesc.get_br());

			rendering::Task::List list;
			list.push_back(task);
			renderer->run(list);
		}
	}
	return true;
}

bool
synfig::Target_Scanline::render(ProgressCallback *cb)
{
	SuperCallback super_cb;
	int
		frames=0,
		total_frames,
		quality=get_quality(),
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

	//synfig::info("1time_set_to %s",t.get_string().c_str());

	if(total_frames>=1)
	{
		do{
			// Grab the time
			frames=next_frame(t);

			// If we have a callback, and it returns
			// false, go ahead and bail. (it may be a user cancel)
			if(cb && !cb->amount_complete(total_frames-frames,total_frames))
				return false;

			Context context;
			// pass the Render Method to the context
			context=canvas->get_context(context_params);
			context.set_render_method(SOFTWARE);

			// Set the time that we wish to render
			if(!get_avoid_time_sync() || canvas->get_time()!=t)
				canvas->set_time(t);

	#ifdef SYNFIG_OPTIMIZE_LAYER_TREE
			Canvas::Handle op_canvas;
			if (!getenv("SYNFIG_DISABLE_OPTIMIZE_LAYER_TREE"))
			{
				op_canvas = Canvas::create();
				op_canvas->set_file_name(canvas->get_file_name());
				optimize_layers(canvas->get_time(), canvas->get_context(context_params), op_canvas);
				context=op_canvas->get_context(context_params);
			}
			else
				context=canvas->get_context(context_params);
	#else
			context=canvas->get_context(context_params);
	#endif

			// If the quality is set to zero, then we
			// use the parametric scanline-renderer.
			if(quality==0 && get_engine().empty())
			{
				if(threads_<=0)
				{
					if(!synfig::render(context,this,desc,0))
						return false;
				}
				else
				{
					if(!synfig::render_threaded(context,this,desc,0,threads_))
						return false;
				}
			}
			else // If quality is set otherwise, then we use the accelerated renderer
			{
				#if USE_PIXELRENDERING_LIMIT
				if(desc.get_w()*desc.get_h() > PIXEL_RENDERING_LIMIT)
				{
					SurfaceSW::Handle surfacesw(new SurfaceSW());
					Surface &surface = surfacesw->get_surface();

					int rowheight = PIXEL_RENDERING_LIMIT/desc.get_w();
					if (!rowheight) rowheight = 1; // TODO: render partial lines to stay within the limit?
					int rows = desc.get_h()/rowheight;
					int lastrowheight = desc.get_h() - rows*rowheight;

					rows++;

					synfig::info("Render broken up into %d block%s %d pixels tall, and a final block %d pixels tall",
								 rows-1, rows==2?"":"s", rowheight, lastrowheight);

					// loop through all the full rows
					if(!start_frame())
					{
						throw(string("add_frame(): target panic on start_frame()"));
						return false;
					}

					for(int i=0; i < rows; ++i)
					{
						RendDesc	blockrd = desc;

						//render the strip at the normal size unless it's the last one...
						if(i == rows-1)
						{
							if(!lastrowheight) break;
							blockrd.set_subwindow(0,i*rowheight,desc.get_w(),lastrowheight);
						}
						else
						{
							blockrd.set_subwindow(0,i*rowheight,desc.get_w(),rowheight);
						}

						if(!call_renderer(context,surfacesw,quality,blockrd,0))
						{
							if(cb)cb->error(_("Accelerated Renderer Failure"));
							return false;
						}else
						{
							int y;
							int rowspan=sizeof(Color)*surface.get_w();
							Surface::pen pen = surface.begin();

							int yoff = i*rowheight;

							for(y = 0; y < blockrd.get_h(); y++, pen.inc_y())
							{
								Color *colordata= start_scanline(y + yoff);
								if(!colordata)
								{
									throw(string("add_frame(): call to start_scanline(y) returned NULL"));
									return false;
								}

								switch(get_alpha_mode())
								{
									case TARGET_ALPHA_MODE_FILL:
										for(int i = 0; i < surface.get_w(); i++)
											colordata[i] = Color::blend(surface[y][i],desc.get_bg_color(),1.0f);
										break;
									case TARGET_ALPHA_MODE_EXTRACT:
										for(int i = 0; i < surface.get_w(); i++)
										{
											float a=surface[y][i].get_a();
											colordata[i] = Color(a,a,a,a);
										}
										break;
									case TARGET_ALPHA_MODE_REDUCE:
										for(int i = 0; i < surface.get_w(); i++)
											colordata[i] = Color(surface[y][i].get_r(),surface[y][i].get_g(),surface[y][i].get_b(),1.0f);
										break;
									case TARGET_ALPHA_MODE_KEEP:
										memcpy(colordata,surface[y],rowspan);
										break;
								}	

								if(!end_scanline())
								{
									throw(string("add_frame(): target panic on end_scanline()"));
									return false;
								}
							}
						}
					}

					end_frame();

				}else //use normal rendering...
				{
				#endif
					SurfaceSW::Handle surfacesw(new SurfaceSW());
					Surface &surface = surfacesw->get_surface();

					if(!call_renderer(context,surfacesw,quality,desc,0))
					{
						// For some reason, the accelerated renderer failed.
						if(cb)cb->error(_("Accelerated Renderer Failure"));
						return false;
					}

					// Put the surface we renderer
					// onto the target.
					if(!add_frame(&surface))
					{
						if(cb)cb->error(_("Unable to put surface on target"));
						return false;
					}
				#if USE_PIXELRENDERING_LIMIT
				}
				#endif
			}
		}while(frames);
	}
    else
    {
		// Set the time that we wish to render
		if(!get_avoid_time_sync() || canvas->get_time()!=t)
			canvas->set_time(t);
		Context context;

#ifdef SYNFIG_OPTIMIZE_LAYER_TREE
		Canvas::Handle op_canvas;
		if (!getenv("SYNFIG_DISABLE_OPTIMIZE_LAYER_TREE"))
		{
			op_canvas = Canvas::create();
			op_canvas->set_file_name(canvas->get_file_name());
			optimize_layers(canvas->get_time(), canvas->get_context(context_params), op_canvas);
			context=op_canvas->get_context(context_params);
		}
		else
			context=canvas->get_context(context_params);
#else
		context=canvas->get_context(context_params);
#endif

		// If the quality is set to zero, then we
		// use the parametric scanline-renderer.
		if(quality==0 && get_engine().empty())
		{
			if(threads_<=0)
			{
				if(!synfig::render(context,this,desc,cb))
					return false;
			}
			else
			{
				if(!synfig::render_threaded(context,this,desc,cb,threads_))
					return false;
			}
		}
		else // If quality is set otherwise, then we use the accelerated renderer
		{
			#if USE_PIXELRENDERING_LIMIT
			if(desc.get_w()*desc.get_h() > PIXEL_RENDERING_LIMIT)
			{
				SurfaceSW::Handle surfacesw(new SurfaceSW());
				Surface &surface = surfacesw->get_surface();

				int totalheight = desc.get_h();
				int rowheight = PIXEL_RENDERING_LIMIT/desc.get_w();
				if (!rowheight) rowheight = 1; // TODO: render partial lines to stay within the limit?
				int rows = desc.get_h()/rowheight;
				int lastrowheight = desc.get_h() - rows*rowheight;

				rows++;

				synfig::info("Render broken up into %d block%s %d pixels tall, and a final block %d pixels tall",
							 rows-1, rows==2?"":"s", rowheight, lastrowheight);

				// loop through all the full rows
				if(!start_frame())
				{
					throw(string("add_frame(): target panic on start_frame()"));
					return false;
				}

				for(int i=0; i < rows; ++i)
				{
					RendDesc	blockrd = desc;

					//render the strip at the normal size unless it's the last one...
					if(i == rows-1)
					{
						if(!lastrowheight) break;
						blockrd.set_subwindow(0,i*rowheight,desc.get_w(),lastrowheight);
					}
					else
					{
						blockrd.set_subwindow(0,i*rowheight,desc.get_w(),rowheight);
					}

					SuperCallback	sc(cb, i*rowheight, (i+1)*rowheight, totalheight);

					if(!call_renderer(context,surfacesw,quality,blockrd,&sc))
					{
						if(cb)cb->error(_("Accelerated Renderer Failure"));
						return false;
					}

					int y;
					int rowspan=sizeof(Color)*surface.get_w();
					Surface::pen pen = surface.begin();

					int yoff = i*rowheight;

					for(y = 0; y < blockrd.get_h(); y++, pen.inc_y())
					{
						Color *colordata= start_scanline(y + yoff);
						if(!colordata)
						{
							throw(string("add_frame(): call to start_scanline(y) returned NULL"));
							return false;
						}

						switch(get_alpha_mode())
						{
							case TARGET_ALPHA_MODE_FILL:
								for(int i = 0; i < surface.get_w(); i++)
									colordata[i] = Color::blend(surface[y][i],desc.get_bg_color(),1.0f);
								break;
							case TARGET_ALPHA_MODE_EXTRACT:
								for(int i = 0; i < surface.get_w(); i++)
								{
									float a=surface[y][i].get_a();
									colordata[i] = Color(a,a,a,a);
								}
								break;
							case TARGET_ALPHA_MODE_REDUCE:
								for(int i = 0; i < surface.get_w(); i++)
									colordata[i] = Color(surface[y][i].get_r(),surface[y][i].get_g(),surface[y][i].get_b(),1.0f);
								break;
							case TARGET_ALPHA_MODE_KEEP:
								memcpy(colordata,surface[y],rowspan);
								break;
						}

						if(!end_scanline())
						{
							throw(string("add_frame(): target panic on end_scanline()"));
							return false;
						}
					}

					//I'm done with this part
					sc.amount_complete(100,100);
				}

				end_frame();

			}else
			{
			#endif
				SurfaceSW::Handle surfacesw(new SurfaceSW());
				Surface &surface = surfacesw->get_surface();

				if(!call_renderer(context,surfacesw,quality,desc,cb))
				{
					if(cb)cb->error(_("Accelerated Renderer Failure"));
					return false;
				}

				// Put the surface we renderer
				// onto the target.
				if(!add_frame(&surface))
				{
					if(cb)cb->error(_("Unable to put surface on target"));
					return false;
				}
			#if USE_PIXELRENDERING_LIMIT
			}
			#endif
		}
	}

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

bool
Target_Scanline::add_frame(const Surface *surface)
{
	assert(surface);


	int y;
	int rowspan=sizeof(Color)*surface->get_w();
	Surface::const_pen pen=surface->begin();

	if(!start_frame())
	{
		throw(string("add_frame(): target panic on start_frame()"));
		return false;
	}

	for(y=0;y<surface->get_h();y++,pen.inc_y())
	{
		Color *colordata= start_scanline(y);
		if(!colordata)
		{
			throw(string("add_frame(): call to start_scanline(y) returned NULL"));
			return false;
		}

		switch(get_alpha_mode())
		{
			case TARGET_ALPHA_MODE_FILL:
				for(int i=0;i<surface->get_w();i++)
					colordata[i]=Color::blend((*surface)[y][i],desc.get_bg_color(),1.0f);
				break;
			case TARGET_ALPHA_MODE_EXTRACT:
				for(int i=0;i<surface->get_w();i++)
				{
					float a=(*surface)[y][i].get_a();
					colordata[i] = Color(a,a,a,a);
				}
				break;
			case TARGET_ALPHA_MODE_REDUCE:
				for(int i = 0; i < surface->get_w(); i++)
					colordata[i] = Color((*surface)[y][i].get_r(),(*surface)[y][i].get_g(),(*surface)[y][i].get_b(),1.0f);
				break;
			case TARGET_ALPHA_MODE_KEEP:
				memcpy(colordata,(*surface)[y],rowspan);
				break;
		}

		if(!end_scanline())
		{
			throw(string("add_frame(): target panic on end_scanline()"));
			return false;
		}
	}

	end_frame();

	return true;
}
