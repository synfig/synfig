/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "asyncrenderer.h"
#include "app.h"
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include <synfig/general.h>
#include <ETL/clock>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

#define BOREDOM_TIMEOUT		50

#define REJOIN_ON_STOP	1

// The Glib::Dispatcher class is broken as of Glibmm 2.4.5.
// Defining this macro enables the workaround.
#define GLIB_DISPATCHER_BROKEN 1

/* === C L A S S E S ======================================================= */

class AsyncTarget_Tile : public synfig::Target_Tile
{
public:
	etl::handle<synfig::Target_Tile> warm_target;

	struct tile_t
	{
		Surface surface;
		int x,y;
		tile_t(const Surface& surface,int x, int y):
			surface(surface),
			x(x),y(y)
		{
		}
	};
	std::list<tile_t> tile_queue;
	Glib::Mutex mutex;

#ifndef GLIB_DISPATCHER_BROKEN
	Glib::Dispatcher tile_ready_signal;
#endif
	Glib::Cond cond_tile_queue_empty;
	bool alive_flag;

	sigc::connection ready_connection;

public:
	AsyncTarget_Tile(etl::handle<synfig::Target_Tile> warm_target):
		warm_target(warm_target)
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_tile_w(warm_target->get_tile_w());
		set_tile_h(warm_target->get_tile_h());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_remove_alpha(warm_target->get_remove_alpha());
		set_threads(warm_target->get_threads());
		set_clipping(warm_target->get_clipping());
		set_rend_desc(&warm_target->rend_desc());
		alive_flag=true;
#ifndef GLIB_DISPATCHER_BROKEN
		ready_connection=tile_ready_signal.connect(sigc::mem_fun(*this,&AsyncTarget_Tile::tile_ready));
#endif
	}

	~AsyncTarget_Tile()
	{
		ready_connection.disconnect();
	}
	void set_dead()
	{
		Glib::Mutex::Lock lock(mutex);
		alive_flag=false;
	}

	virtual int total_tiles()const
	{
		return warm_target->total_tiles();
	}

	virtual int next_tile(int& x, int& y)
	{
		if(!alive_flag)
			return 0;

		return warm_target->next_tile(x,y);
	}

	virtual int next_frame(Time& time)
	{
		if(!alive_flag)
			return 0;
		return warm_target->next_frame(time);
	}

	virtual bool start_frame(synfig::ProgressCallback *cb=0)
	{
		if(!alive_flag)
			return false;
		return warm_target->start_frame(cb);
	}

	virtual bool add_tile(const synfig::Surface &surface, int gx, int gy)
	{
		assert(surface);
		if(!alive_flag)
			return false;
		Glib::Mutex::Lock lock(mutex);
		tile_queue.push_back(tile_t(surface,gx,gy));
		if(tile_queue.size()==1)
		{
#ifdef GLIB_DISPATCHER_BROKEN
		ready_connection=Glib::signal_timeout().connect(
			sigc::bind_return(
				sigc::mem_fun(*this,&AsyncTarget_Tile::tile_ready),
				false
			)
			,0
		);
#else
		tile_ready_signal();
#endif
		}

		return alive_flag;
	}

	void tile_ready()
	{
		Glib::Mutex::Lock lock(mutex);
		if(!alive_flag)
		{
			tile_queue.clear();
			cond_tile_queue_empty.signal();
			return;
		}
		while(!tile_queue.empty() && alive_flag)
		{
			tile_t& tile(tile_queue.front());

			if (getenv("SYNFIG_SHOW_TILE_OUTLINES"))
			{
				Color red(1,0,0);
				tile.surface.fill(red, 0, 0, 1, tile.surface.get_h());
				tile.surface.fill(red, 0, 0, tile.surface.get_w(), 1);
			}

			alive_flag=warm_target->add_tile(tile.surface,tile.x,tile.y);

			tile_queue.pop_front();
		}
		cond_tile_queue_empty.signal();
	}

	virtual void end_frame()
	{
		if (!single_threaded())
		{
			while(alive_flag)
			{
				Glib::Mutex::Lock lock(mutex);
				if(!tile_queue.empty() && alive_flag)
				{
					if(cond_tile_queue_empty.timed_wait(mutex,Glib::TimeVal(0,BOREDOM_TIMEOUT)))
						break;
				}
				else
					break;
			}
		}
		Glib::Mutex::Lock lock(mutex);
		if(!alive_flag)
			return;
		return warm_target->end_frame();
	}
};



class AsyncTarget_Scanline : public synfig::Target_Scanline
{
public:
	etl::handle<synfig::Target_Scanline> warm_target;

	int scanline_;
	Surface surface;

	Glib::Mutex mutex;

#ifndef GLIB_DISPATCHER_BROKEN
	Glib::Dispatcher frame_ready_signal;
#endif
	Glib::Cond cond_frame_queue_empty;
	bool alive_flag;
	bool ready_next;
	sigc::connection ready_connection;


public:
	AsyncTarget_Scanline(etl::handle<synfig::Target_Scanline> warm_target):
		warm_target(warm_target)
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_remove_alpha(warm_target->get_remove_alpha());
		set_threads(warm_target->get_threads());
		set_rend_desc(&warm_target->rend_desc());
		alive_flag=true;
#ifndef GLIB_DISPATCHER_BROKEN
		ready_connection=frame_ready_signal.connect(sigc::mem_fun(*this,&AsyncTarget_Scanline::frame_ready));
#endif
		surface.set_wh(warm_target->rend_desc().get_w(),warm_target->rend_desc().get_h());
	}

	~AsyncTarget_Scanline()
	{
		ready_connection.disconnect();
	}

	virtual int next_frame(Time& time)
	{
		if(!alive_flag)
			return 0;
		return warm_target->next_frame(time);

	}

	void set_dead()
	{
		Glib::Mutex::Lock lock(mutex);
		alive_flag=false;
	}

	virtual bool start_frame(synfig::ProgressCallback */*cb*/=0)
	{
		return alive_flag;
	}

	virtual void end_frame()
	{
		{
			Glib::Mutex::Lock lock(mutex);

			if(!alive_flag)
				return;
			ready_next=false;

#ifdef GLIB_DISPATCHER_BROKEN
		ready_connection=Glib::signal_timeout().connect(
			sigc::bind_return(
				sigc::mem_fun(*this,&AsyncTarget_Scanline::frame_ready),
				false
			)
			,0
		);
#else
			frame_ready_signal();
#endif
		}

		if (single_threaded())
			signal_progress()();
		else
			while(alive_flag && !ready_next)
			{
				Glib::Mutex::Lock lock(mutex);
				if(cond_frame_queue_empty.timed_wait(mutex,Glib::TimeVal(0,BOREDOM_TIMEOUT)))
					break;
			}
	}


	virtual Color * start_scanline(int scanline)
	{
		Glib::Mutex::Lock lock(mutex);

		return surface[scanline];
	}

	virtual bool end_scanline()
	{
		return alive_flag;
	}

	void frame_ready()
	{
		Glib::Mutex::Lock lock(mutex);
		if(alive_flag)
			alive_flag=warm_target->add_frame(&surface);
		if (!single_threaded()) cond_frame_queue_empty.signal();
		ready_next=true;
	}
};

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

AsyncRenderer::AsyncRenderer(etl::handle<synfig::Target> target_,synfig::ProgressCallback *cb):
	error(false),
	success(false),
	cb(cb),
	updating(false)
{
	render_thread=0;
	if(etl::handle<synfig::Target_Tile>::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Tile> wrap_target(
			new AsyncTarget_Tile(etl::handle<synfig::Target_Tile>::cast_dynamic(target_))
		);

		signal_stop_.connect(sigc::mem_fun(*wrap_target,&AsyncTarget_Tile::set_dead));

		target=wrap_target;
	}
	else if(etl::handle<synfig::Target_Scanline>::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Scanline> wrap_target(
			new AsyncTarget_Scanline(
				etl::handle<synfig::Target_Scanline>::cast_dynamic(target_)
			)
		);

		signal_stop_.connect(sigc::mem_fun(*wrap_target,&AsyncTarget_Scanline::set_dead));

		target=wrap_target;
	}
}

AsyncRenderer::~AsyncRenderer()
{
	stop();
}

void
AsyncRenderer::stop()
{
	if(target)
	{
		Glib::Mutex::Lock lock(mutex);
		done_connection.disconnect();

		if(render_thread)
		{
			signal_stop_();

#if REJOIN_ON_STOP
			if (!single_threaded()) render_thread->join();
#endif

			// Make sure all the dispatch crap is cleared out
			//Glib::MainContext::get_default()->iteration(false);

			if(success)
				signal_success_();

			signal_finished_();

			target=0;
			render_thread=0;
		}
	}
}

void
AsyncRenderer::pause()
{
}

void
AsyncRenderer::resume()
{
}

void
AsyncRenderer::start()
{
	done_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			mem_fun(*this,&AsyncRenderer::start_),
			false
		)
		,50
	);
}

void
AsyncRenderer::rendering_progress()
{
	updating = true;
	while(studio::App::events_pending()) studio::App::iteration(false);
	updating = false;
}

void
AsyncRenderer::start_()
{
	error=false;success=false;
	if(target)
	{
#ifndef GLIB_DISPATCHER_BROKEN
		done_connection=signal_done_.connect(mem_fun(*this,&AsyncRenderer::stop));
#endif

		if (single_threaded())
		{
			synfig::info("%s:%d rendering in the same thread", __FILE__, __LINE__);
			target->signal_progress().connect(sigc::mem_fun(this,&AsyncRenderer::rendering_progress));
			render_thread = (Glib::Thread*)1;
			render_target();
		}
		else
		{
			render_thread=Glib::Thread::create(
				sigc::mem_fun(*this,&AsyncRenderer::render_target),
#if REJOIN_ON_STOP
				true
#else
				false
#endif
				);
			assert(render_thread);
		}
	}
	else
	{
		stop();
	}
}

void
AsyncRenderer::render_target()
{
	etl::handle<Target> target(AsyncRenderer::target);

	if(target && target->render())
	{
		success=true;
	}
	else
	{
		error=true;
#ifndef REJOIN_ON_STOP
		return;
#endif
	}

	if(mutex.trylock())
	{
#ifdef GLIB_DISPATCHER_BROKEN
		done_connection=Glib::signal_timeout().connect(
			sigc::bind_return(
				mem_fun(*this,&AsyncRenderer::stop),
				false
			)
			,0
		);
#else
		signal_done_.emit();
#endif
		mutex.unlock();
	}
}
