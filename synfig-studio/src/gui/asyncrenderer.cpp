/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include <synfig/general.h>
#include <synfig/context.h>
#include <ETL/clock>

#include <gui/localization.h>
#include <docks/dock_info.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

#define BOREDOM_TIMEOUT		1

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
	std::list<Glib::Thread*> threads;
	bool err;
	Glib::Mutex mutex;

#ifndef GLIB_DISPATCHER_BROKEN
	Glib::Dispatcher tile_ready_signal;
#endif
	Glib::Cond cond_tile_queue_empty;
	bool alive_flag;

	sigc::connection ready_connection;

public:
	AsyncTarget_Tile(etl::handle<synfig::Target_Tile> warm_target):
		warm_target(warm_target), err(false)
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_tile_w(warm_target->get_tile_w());
		set_tile_h(warm_target->get_tile_h());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_alpha_mode(warm_target->get_alpha_mode());
		set_threads(warm_target->get_threads());
		set_clipping(warm_target->get_clipping());
		set_allow_multithreading(warm_target->get_allow_multithreading());
		set_rend_desc(&warm_target->rend_desc());
		set_engine(warm_target->get_engine());
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

	virtual bool async_render_tile(synfig::RectInt rect, synfig::Context context, synfig::RendDesc tile_desc, synfig::ProgressCallback *cb=NULL)
	{
		if(!alive_flag)
			return false;

#ifdef SINGLE_THREADED
		if (App::single_threaded)
			return sync_render_tile(rect, context, tile_desc, cb);
#endif

		if (!get_allow_multithreading())
			return sync_render_tile(rect, context, tile_desc, cb);

		Glib::Thread *thread = Glib::Thread::create(
			sigc::hide_return(
				sigc::bind(
					sigc::mem_fun(*this, &AsyncTarget_Tile::sync_render_tile),
					rect, context, tile_desc, (synfig::ProgressCallback*)NULL )),
				true
			);
		assert(thread);

		{
			Glib::Mutex::Lock lock(mutex);
			threads.push_back(thread);
		}

		return true;
	}

	bool sync_render_tile(synfig::RectInt rect, synfig::Context context, synfig::RendDesc tile_desc, synfig::ProgressCallback *cb)
	{
		if(!alive_flag)
			return false;
		bool r = warm_target->async_render_tile(rect, context, tile_desc, cb);
		if (!r) { Glib::Mutex::Lock lock(mutex); err = true; }
		return r;
	}

	virtual bool wait_render_tiles(ProgressCallback *cb=NULL)
	{
		if(!alive_flag)
			return false;

		while(true)
		{
			Glib::Thread *thread;
			{
				Glib::Mutex::Lock lock(mutex);
				if (threads.empty()) break;
				thread = threads.front();
				threads.pop_front();

			}
			thread->join();
		}

		{ Glib::Mutex::Lock lock(mutex); if (err) return false; }
		return warm_target->wait_render_tiles(cb);
	}

	virtual int next_tile(RectInt& rect)
	{
		if(!alive_flag)
			return 0;

		return warm_target->next_tile(rect);
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
		{ Glib::Mutex::Lock lock(mutex); err = false; }
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
#ifdef SINGLE_THREADED
		if (!App::single_threaded)
		{
#endif
			while(alive_flag)
			{
				Glib::Mutex::Lock lock(mutex);
				Glib::TimeVal end_time;

				end_time.assign_current_time();
				end_time.add_microseconds(BOREDOM_TIMEOUT);

				if(!tile_queue.empty() && alive_flag)
				{
					if(cond_tile_queue_empty.timed_wait(mutex,end_time))
						break;
				}
				else
					break;
			}
#ifdef SINGLE_THREADED
		}
#endif
		Glib::Mutex::Lock lock(mutex);
		if(!alive_flag)
			return;
		return warm_target->end_frame();
	}
};

class AsyncTarget_Cairo_Tile : public synfig::Target_Cairo_Tile
{
public:
	etl::handle<synfig::Target_Cairo_Tile> warm_target;
	
	class tile_t
	{
	public:
		cairo_surface_t* surface;
		int x,y;
		tile_t(): surface(NULL), x(0), y(0)
		{
		}
		tile_t(cairo_surface_t*& surface_,int x_, int y_)
		{
			if(surface_)
				surface=cairo_surface_reference(surface_);
			else
				surface=surface_;
			x=x_;
			y=y_;
		}
		tile_t(const tile_t &other):
		surface(cairo_surface_reference(other.surface)),
		x(other.x),
		y(other.y)
		{
		}
		~tile_t()
		{
			if(surface)
				cairo_surface_destroy(surface);
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
	AsyncTarget_Cairo_Tile(etl::handle<synfig::Target_Cairo_Tile> warm_target):
	warm_target(warm_target)
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_tile_w(warm_target->get_tile_w());
		set_tile_h(warm_target->get_tile_h());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_alpha_mode(warm_target->get_alpha_mode());
		set_threads(warm_target->get_threads());
		set_clipping(warm_target->get_clipping());
		set_rend_desc(&warm_target->rend_desc());
		alive_flag=true;
#ifndef GLIB_DISPATCHER_BROKEN
		ready_connection=tile_ready_signal.connect(sigc::mem_fun(*this,&AsyncTarget_Cairo_Tile::tile_ready));
#endif
	}
	
	~AsyncTarget_Cairo_Tile()
	{
		ready_connection.disconnect();
	}
	void set_dead()
	{
		Glib::Mutex::Lock lock(mutex);
		alive_flag=false;
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
	
	virtual bool add_tile(cairo_surface_t* surface, int gx, int gy)
	{
		if(cairo_surface_status(surface))
			return false;
		if(!alive_flag)
			return false;
		Glib::Mutex::Lock lock(mutex);
		tile_queue.push_back(tile_t(surface,gx,gy));
		if(tile_queue.size()==1)
		{
#ifdef GLIB_DISPATCHER_BROKEN
			ready_connection=Glib::signal_timeout().connect(
					sigc::bind_return(
							sigc::mem_fun(*this,&AsyncTarget_Cairo_Tile::tile_ready),
							false
							)
							,0
						);
#else
			tile_ready_signal();
#endif
		}
		//cairo_surface_destroy(surface);
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
			
//			if (getenv("SYNFIG_SHOW_TILE_OUTLINES"))
//			{
//				Color red(1,0,0);
//				tile.surface.fill(red, 0, 0, 1, tile.surface.get_h());
//				tile.surface.fill(red, 0, 0, tile.surface.get_w(), 1);
//			}
			
			alive_flag=warm_target->add_tile(tile.surface,tile.x,tile.y);
			
			tile_queue.pop_front();
		}
		cond_tile_queue_empty.signal();
	}
	
	virtual void end_frame()
	{
#ifdef SINGLE_THREADED
		if (!App::single_threaded)
		{
#endif
			while(alive_flag)
			{
				Glib::Mutex::Lock lock(mutex);
				Glib::TimeVal end_time;
				
				end_time.assign_current_time();
				end_time.add_microseconds(BOREDOM_TIMEOUT);
				
				if(!tile_queue.empty() && alive_flag)
				{
					if(cond_tile_queue_empty.timed_wait(mutex,end_time))
						break;
				}
				else
					break;
			}
#ifdef SINGLE_THREADED
		}
#endif
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
		warm_target(warm_target),
		scanline_(),
		alive_flag(),
		ready_next()
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_alpha_mode(warm_target->get_alpha_mode());
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

#ifdef SINGLE_THREADED
		if (App::single_threaded)
			signal_progress()();
		else
#endif
		{

			Glib::TimeVal end_time;

			end_time.assign_current_time();
			end_time.add_microseconds(BOREDOM_TIMEOUT);

			while(alive_flag && !ready_next)
			{
				Glib::Mutex::Lock lock(mutex);
				if(cond_frame_queue_empty.timed_wait(mutex, end_time))
					break;
			}
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
#ifdef SINGLE_THREADED
		if (!App::single_threaded)
#endif
			cond_frame_queue_empty.signal();
		ready_next=true;
		
		int n_total_frames_to_render = warm_target->desc.get_frame_end()        //120
		                             - warm_target->desc.get_frame_start()      //0
		                             + 1;                                       //->121
		int current_rendered_frames_count = warm_target->curr_frame_
		                                  - warm_target->desc.get_frame_start();
		float r = (float) current_rendered_frames_count 
		        / (float) n_total_frames_to_render;
		App::dock_info_->set_render_progress(r);		
	}
};

class AsyncTarget_Cairo : public synfig::Target_Cairo
{
public:
	etl::handle<synfig::Target_Cairo> warm_target;
	
	cairo_surface_t* surface;
	ProgressCallback *callback;
	
	Glib::Mutex mutex;
	
#ifndef GLIB_DISPATCHER_BROKEN
	Glib::Dispatcher frame_ready_signal;
#endif
	Glib::Cond cond_frame_queue_empty;
	bool alive_flag;
	bool ready_next;
	sigc::connection ready_connection;
	
	
public:
	AsyncTarget_Cairo(etl::handle<synfig::Target_Cairo> warm_target):
		warm_target(warm_target),
		surface(),
		callback(),
		alive_flag(),
		ready_next()
	{
		set_avoid_time_sync(warm_target->get_avoid_time_sync());
		set_canvas(warm_target->get_canvas());
		set_quality(warm_target->get_quality());
		set_alpha_mode(warm_target->get_alpha_mode());
		set_rend_desc(&warm_target->rend_desc());
		alive_flag=true;
#ifndef GLIB_DISPATCHER_BROKEN
		ready_connection=frame_ready_signal.connect(sigc::mem_fun(*this,&AsyncTarget_Cairo::frame_ready));
#endif
	}
	
	~AsyncTarget_Cairo()
	{
		ready_connection.disconnect();
	}
	
	virtual int next_frame(Time& time)
	{
		Glib::Mutex::Lock lock(mutex);
		if(!alive_flag)
			return 0;
		return warm_target->next_frame(time);
		
	}
	
	void set_dead()
	{
		Glib::Mutex::Lock lock(mutex);
		alive_flag=false;
	}
		
	virtual bool put_surface(cairo_surface_t* s, ProgressCallback *cb)
	{
		{
			Glib::Mutex::Lock lock(mutex);
			surface=s;
			callback=cb;
			if(!alive_flag)
				return false;
			ready_next=false;
			
#ifdef GLIB_DISPATCHER_BROKEN
			ready_connection=Glib::signal_timeout().connect(
							sigc::bind_return(sigc::mem_fun(*this,&AsyncTarget_Cairo::frame_ready),false)
											,0
											);
#else
			frame_ready_signal();
#endif
		}
		
#ifdef SINGLE_THREADED
		if (App::single_threaded)
			signal_progress()();
		else
#endif
		{
			
			Glib::TimeVal end_time;
			
			end_time.assign_current_time();
			end_time.add_microseconds(BOREDOM_TIMEOUT);
			
			while(alive_flag && !ready_next)
			{
				Glib::Mutex::Lock lock(mutex);
				if(cond_frame_queue_empty.timed_wait(mutex, end_time))
					break;
			}
		}
		return true;
	}
	
	void frame_ready()
	{
		Glib::Mutex::Lock lock(mutex);
		if(alive_flag)
			alive_flag=warm_target->put_surface(surface, callback);
#ifdef SINGLE_THREADED
		if (!App::single_threaded)
#endif
			cond_frame_queue_empty.signal();
		ready_next=true;
		
		int n_total_frames_to_render = warm_target->desc.get_frame_end()        //120
		                             - warm_target->desc.get_frame_start()      //0
		                             + 1;                                       //->121
		int current_rendered_frames_count = warm_target->curr_frame_
		                                  - warm_target->desc.get_frame_start();
		float r = (float) current_rendered_frames_count 
		        / (float) n_total_frames_to_render;
		App::dock_info_->set_render_progress(r);
	}

	virtual bool obtain_surface(cairo_surface_t*& s)
	{
		Glib::Mutex::Lock lock(mutex);
		if(!alive_flag)
			return false;
		return warm_target->obtain_surface(s);
	}

};

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

AsyncRenderer::AsyncRenderer(etl::handle<synfig::Target> target_,synfig::ProgressCallback *cb):
	error(false),
	success(false),
	cb(cb),
#ifdef SINGLE_THREADED
	updating(false),
#endif
	start_clock(0),
	finish_clock(0),
	start_time(0, 0),
	finish_time(0, 0)
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
	else if(etl::handle<synfig::Target_Cairo_Tile>::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Cairo_Tile> wrap_target(
			new AsyncTarget_Cairo_Tile(
					etl::handle<synfig::Target_Cairo_Tile>::cast_dynamic(target_)
			)
		);
		
		signal_stop_.connect(sigc::mem_fun(*wrap_target,&AsyncTarget_Cairo_Tile::set_dead));
		
		target=wrap_target;
	}
	else if(etl::handle<synfig::Target_Cairo>::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Cairo> wrap_target(
			new AsyncTarget_Cairo(
				etl::handle<synfig::Target_Cairo>::cast_dynamic(target_)
			)
		);
		
		signal_stop_.connect(sigc::mem_fun(*wrap_target,&AsyncTarget_Cairo::set_dead));
		
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
#ifdef SINGLE_THREADED
			if (!App::single_threaded)
#endif
				render_thread->join();
#endif
			finish_time.assign_current_time();
			finish_clock = ::clock();


			// Make sure all the dispatch crap is cleared out
			//Glib::MainContext::get_default()->iteration(false);

			if(success)
				signal_success_();

			target=0;
			render_thread=0;
			
			lock.release();
			
			signal_finished_();
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
	App::dock_info_->set_render_progress(0.0);
	start_time.assign_current_time();
	finish_time = start_time;
	start_clock = ::clock();
	finish_clock = start_clock;
	done_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			mem_fun(*this,&AsyncRenderer::start_),
			false
		)
		, 0
	);
}

#ifdef SINGLE_THREADED
void
AsyncRenderer::rendering_progress()
{
	updating = true;
	App::process_all_events();
	updating = false;
}
#endif

void
AsyncRenderer::start_()
{
	error=false;success=false;
	if(target)
	{
#ifndef GLIB_DISPATCHER_BROKEN
		done_connection=signal_done_.connect(mem_fun(*this,&AsyncRenderer::stop));
#endif

#ifdef SINGLE_THREADED
		if (App::single_threaded)
		{
			//synfig::info("%s:%d rendering in the same thread", __FILE__, __LINE__);
			target->signal_progress().connect(sigc::mem_fun(this,&AsyncRenderer::rendering_progress));
			render_thread = (Glib::Thread*)1;
			render_target();
		}
		else
#endif
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
