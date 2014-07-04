/* === S Y N F I G ========================================================= */
/*!	\file target_tile.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "target_tile.h"
#include "string.h"
#include "surface.h"
#include "render.h"
#include "canvas.h"
#include "context.h"
#include "general.h"
#include <ETL/clock>

#include <vector>
#include <algorithm>

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

#ifdef _DEBUG
// #define SYNFIG_DISPLAY_EFFICIENCY
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

struct Target_Tile::TileGroup
{
	struct TileInfo {
		int tile_index;
		int x, y;
		TileInfo(): tile_index(), x(), y() { }
		bool operator < (const TileInfo &other) const
		{
			return y < other.y ? true
				 : other.y < y ? false
				 : x < other.x ? false
				 : other.x < x ? false
				 : tile_index < other.tile_index;
		}
	};

	int x0, y0, x1, y1;
	std::vector<TileInfo> tiles;

	TileGroup(): x0(), y0(), x1(), y1() { }

	static bool can_fill_rectangle(int x0, int y0, int x1, int y1, const std::vector<TileInfo> &tiles)
	{
		for(int x = x0; x < x1; ++x)
		{
			for(int y = y0; y < y1; ++y)
			{
				bool found = false;
				for(std::vector<TileInfo>::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
					if (i->x == x && i->y == y) { found = true; break; }
				if (!found) return false;
			}
		}
		return true;
	}

	void take_tiles(std::vector<TileInfo> &tiles)
	{
		for(int x = x0; x < x1; ++x)
			for(int y = y0; y < y1; ++y)
				for(std::vector<TileInfo>::iterator i = tiles.begin(); i != tiles.end(); ++i)
					if (i->x == x && i->y == y)
						{ this->tiles.push_back(*i); tiles.erase(i); break; }
	}

	static void group_tiles(std::vector<TileGroup> &out_groups, std::vector<TileInfo> &in_tiles)
	{
		while(!in_tiles.empty())
		{
			out_groups.push_back(TileGroup());
			TileGroup &group = out_groups.back();
			group.x0 = in_tiles.front().x;
			group.y0 = in_tiles.front().y;
			group.x1 = group.x0 + 1;
			group.y1 = group.y0 + 1;
			group.take_tiles(in_tiles);
			while(can_fill_rectangle(group.x0 - 1, group.y0, group.x0, group.y1, in_tiles))
				{ --group.x0; group.take_tiles(in_tiles); }
			while(can_fill_rectangle(group.x0, group.y0 - 1, group.x1, group.y0, in_tiles))
				{ --group.y0; group.take_tiles(in_tiles); }
			while(can_fill_rectangle(group.x1, group.y0, group.x1 + 1, group.y1, in_tiles))
				{ ++group.x1; group.take_tiles(in_tiles); }
			while(can_fill_rectangle(group.x0, group.y1, group.x1, group.y1 + 1, in_tiles))
				{ ++group.y1; group.take_tiles(in_tiles); }
			std::sort(group.tiles.begin(), group.tiles.end());
		}
	}
};

Target_Tile::Target_Tile():
	threads_(2),
	tile_w_(DEF_TILE_WIDTH),
	tile_h_(DEF_TILE_HEIGHT),
	curr_tile_(0),
	clipping_(true)
{
	curr_frame_=0;
}

int
Target_Tile::next_frame(Time& time)
{
	return Target::next_frame(time);
}

int
Target_Tile::next_tile(int& x, int& y)
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
synfig::Target_Tile::render_frame_(Context context,ProgressCallback *cb)
{
	if(tile_w_<=0||tile_h_<=0)
	{
		if(cb)cb->error(_("Bad Tile Size"));
		return false;
	}
	const RendDesc &rend_desc(desc);
#define total_tiles total_tiles()

	etl::clock total_time;
	etl::clock::value_type work_time(0);
	etl::clock::value_type find_tile_time(0);
	etl::clock::value_type add_tile_time(0);
	total_time.reset();

	// If the quality is set to zero, then we
	// use the parametric scanline-renderer.
	if(get_quality()==0)
	{
		Surface surface;

		RendDesc tile_desc;
		int x,y,w,h;
		int i;
		etl::clock tile_timer;
		tile_timer.reset();
		while((i=next_tile(x,y)))
		{
			find_tile_time+=tile_timer();
			SuperCallback	super(cb,(total_tiles-i+1)*1000,(total_tiles-i+2)*1000,total_tiles*1000);
			if(!super.amount_complete(0,1000))
				return false;
			//if(cb && !cb->amount_complete(total_tiles-i,total_tiles))
			//	return false;

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
			if(!parametric_render(context, surface, tile_desc,&super))
			{
				// For some reason, the parametric renderer failed.
				if(cb)cb->error(_("Parametric Renderer Failure"));
				return false;
			}
			else
			{
				if(!surface)
				{
					if(cb)cb->error(_("Bad surface"));
					return false;
				}
				switch(get_alpha_mode())
				{
					case TARGET_ALPHA_MODE_FILL:
						for(int i=0;i<surface.get_w()*surface.get_h();i++)
							surface[0][i]=Color::blend(surface[0][i],desc.get_bg_color(),1.0f);
						break;
					case TARGET_ALPHA_MODE_EXTRACT:
						for(int i=0;i<surface.get_w()*surface.get_h();i++)
						{
							float a=surface[0][i].get_a();
							surface[0][i] = Color(a,a,a,a);
						}
						break;
					case TARGET_ALPHA_MODE_REDUCE:
						for(int i=0;i<surface.get_w()*surface.get_h();i++)
							surface[0][i].set_a(1.0f);
						break;
				}

				// Add the tile to the target
				if(!add_tile(surface,x,y))
				{
					if(cb)cb->error(_("add_tile():Unable to put surface on target"));
					return false;
				}
			}
		tile_timer.reset();
		}
	}
	else // If quality is set otherwise, then we use the accelerated renderer
	{
		etl::clock tile_timer;
		tile_timer.reset();

		// Gather tiles
		std::vector<TileGroup::TileInfo> tiles;
		TileGroup::TileInfo tile_info;
		while((tile_info.tile_index = next_tile(tile_info.x, tile_info.y)) != 0) {
			if (clipping_)
				if (tile_info.x >= rend_desc.get_w() || tile_info.y >= rend_desc.get_h())
					continue;
			tile_info.x /= tile_w_;
			tile_info.y /= tile_h_;
			tiles.push_back(tile_info);
		}
		find_tile_time += tile_timer();

		// Group tiles
		std::vector<TileGroup> groups;
		TileGroup::group_tiles(groups, tiles);

		// Render groups
		for(std::vector<TileGroup>::iterator i = groups.begin(); i != groups.end(); ++i)
		{
			// Progress callback
			int group_index = i - groups.begin();
			int groups_count = (int)groups.size();
			SuperCallback super(cb, (groups_count-group_index)*1000, (groups_count-group_index+1)*1000, groups_count*1000);
			if(!super.amount_complete(0,1000))
				return false;

			// Render group
			tile_timer.reset();

			int x0 = i->x0 * tile_w_;
			int y0 = i->y0 * tile_w_;
			int x1 = i->x1 * tile_w_;
			int y1 = i->y1 * tile_w_;

			if (clipping_)
			{
				x1 = std::min(x1, rend_desc.get_w());
				y1 = std::min(y1, rend_desc.get_h());
			}

			RendDesc group_desc=rend_desc;
			group_desc.set_subwindow(x0,y0,x1-x0,y1-y0);

			Surface surface;
			if (!context.accelerated_render(&surface, get_quality(), group_desc, &super))
			{
				// For some reason, the accelerated renderer failed.
				if(cb)cb->error(_("Accelerated Renderer Failure"));
				return false;
			}

			if(!surface)
			{
				if(cb)cb->error(_("Bad surface"));
				return false;
			}
			switch(get_alpha_mode())
			{
				case TARGET_ALPHA_MODE_FILL:
					for(int i=0; i<surface.get_w()*surface.get_h(); ++i)
						surface[0][i] = Color::blend(surface[0][i], desc.get_bg_color(), 1.0f);
					break;
				case TARGET_ALPHA_MODE_EXTRACT:
					for(int i=0; i<surface.get_w()*surface.get_h(); ++i)
					{
						float a=surface[0][i].get_a();
						surface[0][i] = Color(a,a,a,a);
					}
					break;
				case TARGET_ALPHA_MODE_REDUCE:
					for(int i=0;i<surface.get_w()*surface.get_h(); ++i)
						surface[0][i].set_a(1.0f);
					break;
				}

			work_time += tile_timer();

			// Split group by tiles
			for(std::vector<TileGroup::TileInfo>::iterator j = i->tiles.begin(); j != i->tiles.end(); ++j)
			{
				int tx0 = j->x * tile_w_;
				int ty0 = j->y * tile_w_;
				int tx1 = std::min(tx0 + tile_w_, x1);
				int ty1 = std::min(ty0 + tile_w_, y1);

				Surface tile_surface(Surface::size_type(tx1-tx0, ty1-ty0));
				Surface::pen pen = tile_surface.get_pen(0, 0);
				surface.blit_to(
					pen,
					tx0-x0, ty0-y0,
					tile_surface.get_w(), tile_surface.get_h() );

				// Add the tile to the target
				tile_timer.reset();
				if(!add_tile(tile_surface, tx0, ty0))
				{
					if(cb)cb->error(_("add_tile():Unable to put surface on target"));
					return false;
				}
				add_tile_time+=tile_timer();
			}

			signal_progress()();
		}
	}
	if(cb && !cb->amount_complete(total_tiles,total_tiles))
		return false;

#ifdef SYNFIG_DISPLAY_EFFICIENCY
	synfig::info(">>>>>> Render Time: %fsec, Find Tile Time: %fsec, Add Tile Time: %fsec, Total Time: %fsec",work_time,find_tile_time,add_tile_time,total_time());
	synfig::info(">>>>>> FRAME EFFICIENCY: %f%%",(100.0f*work_time/total_time()));
#endif
#undef total_tiles
	return true;
}

bool
synfig::Target_Tile::render(ProgressCallback *cb)
{
	SuperCallback super_cb;
	int
		frames=0,
		total_frames,
		frame_start,
		frame_end;
	Time
		t=0;

	assert(canvas);
	curr_frame_=0;
	init();
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

		if(total_frames>=1)
		{
			do
			{		
				// Grab the time
				frames=next_frame(t);

				curr_tile_=0;

				// If we have a callback, and it returns
				// false, go ahead and bail. (maybe a use cancel)
				if(cb && !cb->amount_complete(total_frames-frames,total_frames))
					return false;

				if(!start_frame(cb))
					return false;
				Context context;
				// pass the Render Method to the context
				context=canvas->get_context(context_params);
				context.set_render_method(SOFTWARE);

				// Set the time that we wish to render
				//if(!get_avoid_time_sync() || canvas->get_time()!=t)
				// Why the above line was commented here and not in TargetScaline?
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

	/*
				#ifdef SYNFIG_OPTIMIZE_LAYER_TREE
				Context context;
				Canvas::Handle op_canvas(Canvas::create());
				op_canvas->set_file_name(canvas->get_file_name());
				// Set the time that we wish to render
				canvas->set_time(t);
				optimize_layers(canvas->get_time(), canvas->get_context(context_params), op_canvas);
				context=op_canvas->get_context();
				#else
				Context context;
				// Set the time that we wish to render
				canvas->set_time(t);
				context=canvas->get_context(context_params);
				#endif
	*/

				if(!render_frame_(context,0))
					return false;
				end_frame();
			}while(frames);
			//synfig::info("tilerenderer: i=%d, t=%s",i,t.get_string().c_str());
		}
		else
		{
			curr_tile_=0;

			if(!start_frame(cb))
				return false;

			// Set the time that we wish to render
			//if(!get_avoid_time_sync() || canvas->get_time()!=t)
				canvas->set_time(t);

			//synfig::info("2time_set_to %s",t.get_string().c_str());

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

			if(!render_frame_(context, cb))
				return false;
			end_frame();
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
