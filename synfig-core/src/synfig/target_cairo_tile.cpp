/* === S Y N F I G ========================================================= */
/*!	\file target_cairo_tile.cpp
**	\brief Target Cairo class tile mode
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2013-2013 Carlos López
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

#include "target_cairo_tile.h"

#include "general.h"
#include <synfig/localization.h>

#include "canvas.h"
#include "context.h"
#include "render.h"
#include "string.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */
const unsigned int	DEF_TILE_WIDTH = TILE_SIZE / 2;
const unsigned int	DEF_TILE_HEIGHT= TILE_SIZE / 2;

// note that if this isn't defined then the rendering is incorrect for
// the straight blend method since the optimize_layers() function in
// canvas.cpp which makes the straight blend method work correctly
// isn't called.  ie. leave this defined.  to see the problem, draw a
// small circle over a solid background.  set circle to amount 0.99
// and blend method 'straight'.  the background should vanish but doesn't
#define SYNFIG_OPTIMIZE_LAYER_TREE

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Cairo_Tile::Target_Cairo_Tile():
	threads_(2),
	tile_w_(DEF_TILE_WIDTH),
	tile_h_(DEF_TILE_HEIGHT),
	curr_tile_(0),
	clipping_(true)
{
	curr_frame_=0;
}

int
Target_Cairo_Tile::next_frame(Time& time)
{
	return Target::next_frame(time);
}

int
Target_Cairo_Tile::next_tile(int& x, int& y)
{
	// Width of the image(in tiles)
	int tw(rend_desc().get_w()/tile_w_);
	int th(rend_desc().get_h()/tile_h_);
	
	// Add the last tiles (which will be clipped)
	if(rend_desc().get_w()%tile_w_!=0)tw++;
	if(rend_desc().get_h()%tile_h_!=0)th++;
	
	x=(curr_tile_%tw)*tile_h_;
	y=(curr_tile_/tw)*tile_w_;
	
	curr_tile_++;
	return (tw*th)-curr_tile_+1;
}

bool
synfig::Target_Cairo_Tile::render_frame_(Context context,ProgressCallback *cb)
{
	if(tile_w_<=0||tile_h_<=0)
	{
		if(cb)cb->error(_("Bad Tile Size"));
		return false;
	}
	const RendDesc &rend_desc(desc);
#define total_tiles total_tiles()

	RendDesc tile_desc;
	int x,y,w,h;
	int i;

	while((i=next_tile(x,y)))
	{
		SuperCallback	super(cb,(total_tiles-i)*1000,(total_tiles-i+1)*1000,total_tiles*1000);
		if(!super.amount_complete(0,1000))
			return false;
		//			if(cb && !cb->amount_complete(total_tiles-i,total_tiles))
		//				return false;
		// Perform clipping on the tile
		if(clipping_)
		{
			w=x+tile_w_<rend_desc.get_w()?tile_w_:rend_desc.get_w()-x;
			h=y+tile_h_<rend_desc.get_h()?tile_h_:rend_desc.get_h()-y;
			if(w<=0||h<=0)continue;
		}
		else
		{
			w=tile_w_;
			h=tile_h_;
		}
		
		tile_desc=rend_desc;
		tile_desc.set_subwindow(x,y,w,h);
				
		cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		cairo_t* cr=cairo_create(surface);
		double tx=tile_desc.get_tl()[0];
		double ty=tile_desc.get_tl()[1];
		double sx=1.0/tile_desc.get_pw();
		double sy=1.0/tile_desc.get_ph();
		cairo_scale(cr, sx, sy);
		cairo_translate(cr, -tx, -ty);
		if(!context.accelerated_cairorender(cr,get_quality(),tile_desc,&super))
		{
			// For some reason, the accelerated renderer failed.
			if(cb)cb->error(_("Accelerated Renderer Failure"));
			cairo_destroy(cr);
			return false;
		}
		else
		{
			cairo_status_t status = cairo_surface_status(surface);
			if(status)
			{
				if(cb) cb->error(strprintf(_("Bad surface: %s"), cairo_status_to_string(status)));
				cairo_destroy(cr);
				return false;
			}
			// Add the tile to the target
			if(!add_tile(cairo_surface_reference(surface), x,y))
			{
				if(cb)cb->error(_("add_tile():Unable to put surface on target"));
				return false;
			}
			cairo_destroy(cr);
		}
		signal_progress()();
	}
	if(cb && !cb->amount_complete(total_tiles,total_tiles))
		return false;
	
#undef total_tiles
	return true;
}


bool
synfig::Target_Cairo_Tile::render(ProgressCallback *cb)
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

			if(!start_frame(cb))
			{
				if(cb)cb->error(_("Can't start frame"));
				return false;
			}
			Context context;
			// pass the Render Method to the context
			context=canvas->get_context(context_params);
			context.set_render_method(CAIRO);

			// Set the time that we wish to render
			if(!get_avoid_time_sync() || canvas->get_time()!=t) {
				canvas->set_time(t);
				canvas->load_resources(t);
			}

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
			if(!render_frame_(context,cb))
			{
				// For some reason, the accelerated renderer failed.
				if(cb)cb->error(_("Accelerated Renderer Failure"));
				return false;
			}
			end_frame();
		}while(frames);
	}
	catch(String str)
	{
		if(cb)cb->error(_("Caught string :")+str);
		return false;
	}
	catch(std::bad_alloc&)
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

void
Target_Cairo_Tile::gamma_filter(cairo_surface_t *surface, const synfig::Gamma &gamma)
{
	CairoSurface cairo_s;
	cairo_s.set_cairo_surface(surface);
	cairo_s.map_cairo_image();
	int w=cairo_s.get_w();
	int h=cairo_s.get_h();
	for(int y=0; y<h; y++)
		for(int x=0; x<w; x++)
		{
			CairoColor c=cairo_s[y][x];
			if (c.get_alpha()) {
				float a=c.get_alpha();
				c.set_a(gamma.b_U8_to_U8(c.get_alpha()));
				float aa=c.get_alpha();
				unsigned char r=(unsigned char)(aa*gamma.r_F32_to_F32(c.get_r()/a));
				unsigned char g=(unsigned char)(aa*gamma.g_F32_to_F32(c.get_g()/a));
				unsigned char b=(unsigned char)(aa*gamma.b_F32_to_F32(c.get_b()/a));
				c.set_r(r);
				c.set_g(g);
				c.set_b(b);
			}
			cairo_s[y][x]=c;
		}
	cairo_s.unmap_cairo_image();
}
