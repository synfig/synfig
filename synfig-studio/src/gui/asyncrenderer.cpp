/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include <ETL/clock>

#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/target_scanline.h>
#include <synfig/target_tile.h>

#include <gui/app.h>
#include <gui/docks/dock_info.h>
#include <gui/localization.h>
#include <gui/progresslogger.h>

#endif

/* === U S I N G =========================================================== */

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

	virtual bool async_render_tile(
		etl::handle<Canvas> canvas,
		ContextParams context_params,
		RectInt rect,
		RendDesc tile_desc,
		ProgressCallback */*cb*/ )
	{
		if(!alive_flag)
			return false;

		Glib::Thread *thread = Glib::Thread::create(
			sigc::hide_return(
				sigc::bind(
					sigc::mem_fun(*this, &AsyncTarget_Tile::sync_render_tile),
					canvas, context_params, rect, tile_desc, (synfig::ProgressCallback*)NULL )),
				true
			);
		assert(thread);

		{
			Glib::Mutex::Lock lock(mutex);
			threads.push_back(thread);
		}

		return true;
	}

	bool sync_render_tile(
		etl::handle<Canvas> canvas,
		ContextParams context_params,
		RectInt rect,
		RendDesc tile_desc,
		ProgressCallback *cb )
	{
		if(!alive_flag)
			return false;
		bool r = warm_target->async_render_tile(canvas, context_params, rect, tile_desc, cb);
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

	ProgressCallback *cb;

public:
	AsyncTarget_Scanline(etl::handle<synfig::Target_Scanline> warm_target):
		warm_target(warm_target),
		scanline_(),
		alive_flag(),
		ready_next(),
		cb(nullptr)
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

	virtual bool start_frame(synfig::ProgressCallback *cb)
	{
		this->cb = cb;
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
			alive_flag=warm_target->add_frame(&surface, cb);
		cond_frame_queue_empty.signal();
		ready_next=true;
		
		int n_total_frames_to_render = warm_target->desc.get_frame_end()        //120
		                             - warm_target->desc.get_frame_start()      //0
		                             + 1;                                       //->121
		int current_rendered_frames_count = warm_target->curr_frame_;
		
		float r = (float) current_rendered_frames_count 
		        / (float) n_total_frames_to_render;
		
		App::dock_info_->set_render_progress(r);		
	}
};

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

AsyncRenderer::AsyncRenderer(etl::handle<synfig::Target> target_,synfig::ProgressCallback *cb):
	status(RENDERING_UNDEFINED),
	cb(cb),
	start_clock(0),
	finish_clock(0),
	start_time(0, 0),
	finish_time(0, 0)
{
	render_thread=0;
	if(auto cast_target = synfig::Target_Tile::Handle::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Tile> wrap_target(
			new AsyncTarget_Tile(cast_target)
		);

		signal_stop_.connect(sigc::mem_fun(*wrap_target,&AsyncTarget_Tile::set_dead));

		target=wrap_target;
	}
	else if(auto cast_target = etl::handle<synfig::Target_Scanline>::cast_dynamic(target_))
	{
		etl::handle<AsyncTarget_Scanline> wrap_target(
			new AsyncTarget_Scanline(cast_target)
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
AsyncRenderer::stop(AsyncRenderer::Interaction interaction)
{
	if(target)
	{
		Glib::Mutex::Lock lock(mutex);
		done_connection.disconnect();

		if(render_thread)
		{
			signal_stop_();

#if REJOIN_ON_STOP
			render_thread->join();
#endif
			finish_time.assign_current_time();
			finish_clock = ::clock();


			// Make sure all the dispatch crap is cleared out
			//Glib::MainContext::get_default()->iteration(false);

			std::string error_message;

			if(interaction == INTERACTION_UNDEFINED)
			{
				if(status == RENDERING_SUCCESS)
					signal_success_();
				else
					error_message = _("Animation couldn't be rendered");

				target=0;
				render_thread=0;
				lock.release();

				if(status == RENDERING_ERROR)
				{
					if(ProgressLogger *logger = dynamic_cast<ProgressLogger*>(cb))
						error_message += "\n" + logger->get_error_message();
				}
			}
			else
			{
				target=0;
				render_thread=0;
				lock.release();
			}

			signal_finished_(error_message);
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

void
AsyncRenderer::start_()
{
	status = RENDERING_UNDEFINED;
	if(target)
	{
#ifndef GLIB_DISPATCHER_BROKEN
		done_connection=signal_done_.connect(mem_fun(*this,&AsyncRenderer::stop));
#endif
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
	else
	{
		stop();
	}
}

void
AsyncRenderer::render_target()
{
	etl::handle<Target> target(AsyncRenderer::target);

	std::string error_str;
	try {
		if(target && target->render(cb))
		{
			status = RENDERING_SUCCESS;
		}
		else
		{
			status = RENDERING_ERROR;
#ifndef REJOIN_ON_STOP
			return;
#endif
		}
	} catch (std::runtime_error &err) {
		error_str = std::string(_("AsyncRenderer: ")) + _("runtime error: ") + err.what();
	} catch (std::exception &ex) {
		error_str = std::string(_("AsyncRenderer: ")) + _("exception: ") + ex.what();
	} catch (std::string &str) {
		error_str = std::string(_("AsyncRenderer: ")) + _("string exception: ") + str;
	} catch (...) {
		error_str = std::string(_("AsyncRenderer: ")) + _("internal error: ") + _("some exception has been thrown while rendering");
	}

	if (!error_str.empty()) {
		status = RENDERING_ERROR;
		synfig::error(error_str);
		if (cb)
			cb->error(error_str);
#ifndef REJOIN_ON_STOP
		return;
#endif
	}

	if(mutex.trylock())
	{
#ifdef GLIB_DISPATCHER_BROKEN
		done_connection=Glib::signal_timeout().connect(
			sigc::bind_return(
				sigc::bind(mem_fun(*this,&AsyncRenderer::stop),
						 AsyncRenderer::INTERACTION_UNDEFINED
				),
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
