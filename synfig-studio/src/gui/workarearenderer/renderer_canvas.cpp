/* === S Y N F I G ========================================================= */
/*!	\file renderer_canvas.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Nikita Kitaev
**  ......... ... 2018 Ivan Mahonin
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

#include <ctime>

#include <glib.h>

#include <gdkmm/general.h>

#include <ETL/misc>

#include <synfig/general.h>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/threadpool.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/renderer.h>

#include <gui/localization.h>
#include <gui/app.h>

#include "renderer_canvas.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static int
int_floor(int x, int base)
{
	int m = x % base;
	return m < 0 ? x - base - m
		 : m > 0 ? x - m : x;
}

static int
int_ceil(int x, int base)
{
	int m = x % base;
	return m > 0 ? x + base - m
		 : m < 0 ? x - m : x;
}

/* === M E T H O D S ======================================================= */

Renderer_Canvas::TimeMeasure
Renderer_Canvas::TimeMeasure::now() {
	const long long cpu_step_us = 1000000000ll/(long long)(CLOCKS_PER_SEC);
	assert(cpu_step_us*(long long)(CLOCKS_PER_SEC) == 1000000000ll);

	TimeMeasure t;
	t.time_us = g_get_monotonic_time()*1000;
	t.cpu_time_us = clock()*cpu_step_us;
	return t;
}


Renderer_Canvas::Renderer_Canvas():
	refresh_id(), render_queued() { }

Renderer_Canvas::~Renderer_Canvas()
	{ cancel_render(); }

void
Renderer_Canvas::enqueue_rendering_task_callback(
	synfig::rendering::Renderer::Handle renderer,
	synfig::rendering::Task::Handle task,
	synfig::rendering::TaskEvent::Handle event )
{
	// Handles will protect objects from deletion before this call
	renderer->enqueue(task, event);
}

void
Renderer_Canvas::free_pixbuf_data_callback(const guint8 *x)
	{ delete[] x; }

void
Renderer_Canvas::on_tile_finished_callback(bool success, Renderer_Canvas *obj, Tile::Handle tile)
{
	// Handle will protect 'tile' from deletion before this call
	// This callback will called only once for each tile
	// And will called before deletion of 'obj', by calling cancel_render() from destructor
	obj->on_tile_finished(success, tile);
}

void
Renderer_Canvas::post_tile_finished_callback(etl::handle<Renderer_Canvas> obj) {
	// this function should be called in main thread
	// Handle will protect 'obj' from deletion before this call
	// zero 'work_area' means that 'work_area' is destructed and here is a last Handle of 'obj'
	if (obj->get_work_area())
		obj->post_tile_finished();
}

void
Renderer_Canvas::inc_refresh_id()
{
	Glib::Threads::Mutex::Lock lock(mutex);
	++refresh_id;
}

void
Renderer_Canvas::cancel_render(long long keep_refresh_id)
{
	rendering::Task::List list;
	while(true) {
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			render_queued = false;
			for(TileMap::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
				for(TileSet::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
					if (*j && (*j)->event && !(*j)->event->is_finished() && (*j)->refresh_id < keep_refresh_id)
						list.push_back((*j)->event);
		}
		if (list.empty()) return;
		rendering::Renderer::cancel(list);
		list.clear();
		Glib::usleep(1);
	}
}

void
Renderer_Canvas::on_tile_finished(bool success, const Tile::Handle &tile)
{
	// this method must be called from on_tile_finished_callback()
	// only this non-static method may be called in other threads

	// 'tiles', 'onion_frames' and 'refresh_id' are controlled by mutex

	Glib::Threads::Mutex::Lock lock(mutex);

	// release event
	tile->event.reset();

	// convert surface
	if (success && tile->surface) {
		rendering::SurfaceResource::LockReadBase surface_lock(tile->surface);
		if (surface_lock) {
			const rendering::Surface &surface = *surface_lock.get_surface();
			int w = surface.get_width();
			int h = surface.get_height();
			int count = w*h;

			if ( w > 0 && w == tile->rect.get_width()
			  && h > 0 && h == tile->rect.get_height() )
			{
				const Color *pixels = surface.get_pixels_pointer();
				std::vector<Color> pixels_copy;
				if (!pixels) {
					pixels_copy.resize(count);
					if (surface.get_pixels(&pixels_copy.front()))
						pixels = &pixels_copy.front();
				}

				if (pixels) {
					// convert to RGBA32
					PixelFormat pf(PF_RGB | PF_A);
					int pixel_size = synfig::channels(pf);
					unsigned char *buffer = new unsigned char[count*pixel_size];
					unsigned char *dest(buffer);
					const Color *src = pixels;
					const Color *end = src + surface.get_pixels_count();
					while(src < end)
						dest = Color2PixelFormat(
								(src++)->clamped(),
								pf,
								dest,
								App::gamma );

					// create Gdk::PixBuf
					tile->pixbuf = Gdk::Pixbuf::create_from_data(
							buffer, Gdk::COLORSPACE_RGB, pf & PF_A,
							8, w, h, count*pixel_size,
							sigc::ptr_fun(&free_pixbuf_data_callback) );
				}
			}
		}
	}

	// release surface
	tile->surface.reset();

	// new tile is ready
	// let's now remove old tiles
	for(TileMap::iterator i = tiles.begin(); i != tiles.end(); ++i) {
		bool is_frame_visible = false;
		for(FrameList::const_iterator j = onion_frames.begin(); j != onion_frames.end(); ++j)
			if (j->time == i->first) { is_frame_visible = true; break; }

		// remove bad tiles
		for(TileSet::iterator j = i->second.begin(); j != i->second.end(); )
			if ( !*j                                                    // remove nulls (possible bug)
			  || (!(*j)->event && !(*j)->surface)                       // remove tiles which was fail to render
			  || (!is_frame_visible && (*j)->refresh_id < refresh_id) ) // remove outdated tiles at invisible frame
					i->second.erase(j++); else ++j;

		// remove overlapped and partially overlapped tiles
		// note: tiles are sorted by refresh_id - older first
		for(TileSet::iterator j = i->second.begin(); j != i->second.end(); ) {
			bool overlapped = false;
			TileSet::iterator k = j;
			while(++k != i->second.end())
				if ((*k)->pixbuf && etl::intersect((*j)->rect, (*k)->rect))
					{ overlapped = true; break; }
			if (overlapped)
				i->second.erase(j++); else ++j;
		}
	}

	// don't create handle if ref-count is zero
	// it means that object was nether had a handles and will removed with handle
	// or object is already in destruction phase
	if (shared_object::count())
		Glib::signal_timeout().connect_once(
			sigc::bind(sigc::ptr_fun(&post_tile_finished_callback), etl::handle<Renderer_Canvas>(this)), 0);
}

void
Renderer_Canvas::pre_tile_started()
{
	// remember time update statusbar
	if (rendering_start_time.time_us == 0) {
		rendering_start_time = TimeMeasure::now();
		if (ProgressCallback *cb = get_work_area()->get_progress_callback())
			cb->task(_("Rendering..."));
	}
}

void
Renderer_Canvas::post_tile_finished()
{
	// NB: this function locks the mutex (but pre_tile_started() don't)

	if (!render_queued && rendering_start_time.time_us != 0) {
		// check if rendering finished
		bool all_finished = true;
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			for(TileMap::const_iterator i = tiles.begin(); all_finished && i != tiles.end(); ++i)
				for(TileSet::const_iterator j = i->second.begin(); all_finished && j != i->second.end(); ++j)
					if (*j && (*j)->event && !(*j)->event->is_finished())
						all_finished = false;
		}

		if (all_finished) {
			// measure rendering time and update statusbar
			TimeMeasure time = TimeMeasure::now();
			if (ProgressCallback *cb = get_work_area()->get_progress_callback())
				cb->task( strprintf("%s %f (%f) %s",
					_("Rendered:"),
					1e-9*(time.time_us - rendering_start_time.time_us),
					1e-9*(time.cpu_time_us - rendering_start_time.cpu_time_us),
					_("sec") ));
			rendering_start_time = TimeMeasure();
		}
	}

	get_work_area()->queue_draw();
	if (render_queued) enqueue_render();
}

void
Renderer_Canvas::enqueue_render(bool force)
{
	ColorReal base_onion_alpha = 0.75;
	const int tile_grid_step = 64;

	assert(get_work_area());
	rendering::Task::List tasks_to_cancel;
	long long current_refresh_id;

	{
		Glib::Threads::Mutex::Lock lock(mutex);
		current_refresh_id = refresh_id;

		if (!force) {
			render_queued = true;
			// if any outdated tile still in process, then return
			for(TileMap::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
				for(TileSet::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
					if (*j && (*j)->refresh_id < current_refresh_id && (*j)->event && !(*j)->event->is_finished())
						return;
		}
		render_queued = false;

		String renderer_name = get_work_area()->get_renderer();
		rendering::Renderer::Handle renderer = rendering::Renderer::get_renderer(renderer_name);
		if (!renderer) return;

		Canvas::Handle canvas         = get_work_area()->get_canvas();
		RendDesc       rend_desc      = get_work_area()->get_rend_desc();
		Time           base_time      = get_work_area()->get_time();
		VectorInt      window_offset  = get_work_area()->get_windows_offset();
		RectInt        window_rect    = get_work_area()->get_window_rect();
		RectInt        tile_rect      = window_rect + window_offset;
		float          fps            = rend_desc.get_frame_rate();

		ContextParams context_params(rend_desc.get_render_excluded_contexts());

		// apply onion skin
		onion_frames.clear();
		if (get_work_area()->get_onion_skin() && approximate_not_equal_lp(fps, 0.f)) {
			Time frame_duration(1.0/(double)fps);
			int past   = get_work_area()->get_onion_skins()[0];
			int future = get_work_area()->get_onion_skins()[1];
			for(int i = 0; i < past; ++i) {
				Time time = base_time - frame_duration*(past - i);
				ColorReal alpha = base_onion_alpha*(double)(i + 1)/(double)(past + 1);
				if (time >= rend_desc.get_time_start() && time <= rend_desc.get_time_end())
					onion_frames.push_back(FrameDesc(base_time, alpha));
			}
			for(int i = 0; i < future; ++i) {
				Time time = base_time + frame_duration*(future - i);
				ColorReal alpha = base_onion_alpha*(double)(i + 1)/(double)(future + 1);
				if (time >= rend_desc.get_time_start() && time <= rend_desc.get_time_end())
					onion_frames.push_back(FrameDesc(base_time, alpha));
			}
			onion_frames.push_back(FrameDesc(base_time, base_onion_alpha));
		} else {
			onion_frames.push_back(FrameDesc(base_time, 1.f));
		}

		// generate rendering tasks
		if (canvas && tile_rect.is_valid()) {
			Time orig_time = canvas->get_time();

			CanvasBase sub_queue;
			std::vector<synfig::RectInt> rects;
			for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i) {
				Time frame_time = i->time;
				TileSet &frame_tiles = tiles[frame_time];

				// find not actual regions
				rects.clear();
				rects.push_back(tile_rect);
				for(TileSet::const_iterator j = frame_tiles.begin(); j != frame_tiles.end(); ++j)
					if (*j && (*j)->refresh_id == current_refresh_id)
						etl::rects_subtract(rects, (*j)->rect);
				rects_merge(rects);

				if (!rects.empty()) {
					// this function don't locks mutex, so we may use it here with locked mutex
					pre_tile_started();

					// build rendering task
					canvas->set_time(frame_time);
					canvas->set_outline_grow(rend_desc.get_outline_grow());
					Context context = canvas->get_context_sorted(context_params, sub_queue);
					rendering::Task::Handle task = context.build_rendering_task();
					sub_queue.clear();

					// add transformation task to flip result if needed
					Vector p0 = rend_desc.get_tl();
					Vector p1 = rend_desc.get_br();
					if (p0[0] > p1[0] || p0[1] > p1[1]) {
						Matrix m;
						if (p0[0] > p1[0]) { m.m00 = -1.0; m.m20 = p0[0] + p1[0]; std::swap(p0[0], p1[0]); }
						if (p0[1] > p1[1]) { m.m11 = -1.0; m.m21 = p0[1] + p1[1]; std::swap(p0[1], p1[1]); }
						rendering::TaskTransformationAffine::Handle t = new rendering::TaskTransformationAffine();
						t->transformation->matrix = m;
						t->sub_task() = task;
						task = t;
					}

					rendering::Task::List list;
					list.push_back(task);

					for(std::vector<synfig::RectInt>::iterator j = rects.begin(); j != rects.end(); ++j) {
						// snap rect corners to tile grid
						RectInt &rect = *j;
						rect.minx = int_floor(rect.minx, tile_grid_step);
						rect.miny = int_floor(rect.miny, tile_grid_step);
						rect.maxx = int_ceil (rect.maxx, tile_grid_step);
						rect.maxy = int_ceil (rect.maxy, tile_grid_step);

						RendDesc tile_desc=rend_desc;
						tile_desc.set_subwindow(rect.minx, rect.miny, rect.maxx - rect.minx, rect.maxy - rect.miny);

						rendering::Task::Handle tile_task = task->clone_recursive();
						tile_task->target_surface = new rendering::SurfaceResource();
						tile_task->target_rect = RectInt( 0, 0, tile_desc.get_w(), tile_desc.get_h() );
						tile_task->source_rect = Rect(p0, p1);

						Tile::Handle tile = new Tile(current_refresh_id, frame_time, *j);
						tile->surface = tile_task->target_surface;

						rendering::TaskEvent::Handle event = new rendering::TaskEvent();
						event->signal_finished.connect( sigc::bind(
							sigc::ptr_fun(&on_tile_finished_callback), this, tile ));

						frame_tiles.insert(tile);

						// Renderer::enqueue contains the expensive 'optimization' stage, so call it async
						ThreadPool::instance.enqueue( sigc::bind(
							sigc::ptr_fun(&enqueue_rendering_task_callback),
							renderer, task, event ));
					}
				}
			}
			canvas->set_time(orig_time);
		}
	}

	// cancel all previous tasks
	cancel_render(current_refresh_id);
}

void
Renderer_Canvas::wait_render()
{
	rendering::TaskEvent::List list;
	while(true) {
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			for(TileMap::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
				for(TileSet::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
					if (*j && (*j)->event && !(*j)->event->is_finished())
						list.push_back((*j)->event);
		}
		if (list.empty()) return;
		for(rendering::TaskEvent::List::iterator i = list.begin(); i != list.end(); ++i)
			(*i)->wait();
		list.clear();
		Glib::usleep(1);
	}
}

void
Renderer_Canvas::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& expose_area )
{
	VectorInt window_offset = get_work_area()->get_windows_offset();
	RectInt   window_rect   = get_work_area()->get_window_rect();
	RectInt   expose_rect   = RectInt( expose_area.get_x(),
			                           expose_area.get_y(),
									   expose_area.get_x() + expose_area.get_width(),
									   expose_area.get_y() + expose_area.get_height() );
	expose_rect &= window_rect;
	if (!expose_rect.is_valid()) return;

	// enqueue rendering if not all of visible tiles are exists and actual
	enqueue_render();

	Glib::Threads::Mutex::Lock lock(mutex);

	ColorReal summary_alpha = 0.f;
	if (onion_frames.empty()) return;
	for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i)
		summary_alpha = summary_alpha*(1.f - i->alpha) + i->alpha;
	if (approximate_less_or_equal_lp(summary_alpha, 0.f)) return;

	Cairo::RefPtr<Cairo::Context> context = drawable->create_cairo_context();

	// context for tiles
	Cairo::RefPtr<Cairo::ImageSurface> onion_surface;
	Cairo::RefPtr<Cairo::Context> onion_context = context;
	if (approximate_less(summary_alpha, 1.f)) {
		// create surface to merge onion skin
		onion_surface = Cairo::ImageSurface::create(
				Cairo::FORMAT_ARGB32, expose_rect.get_width(), expose_rect.get_height() );
		onion_context = Cairo::Context::create(onion_surface);
		onion_context->translate(-(double)expose_rect.minx, -(double)expose_rect.miny);
	}

	// draw tiles
	onion_context->save();
	onion_context->translate((double)window_offset[0], (double)window_offset[1]);
	for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i) {
		TileMap::const_iterator ii = tiles.find(i->time);
		if (ii == tiles.end()) continue;
		for(TileSet::const_iterator j = ii->second.begin(); j != ii->second.end(); ++j) {
			if (*j && (*j)->pixbuf) {
				Gdk::Cairo::set_source_pixbuf(
					onion_context,
					(*j)->pixbuf,
					(*j)->rect.minx,
					(*j)->rect.miny );
				onion_context->paint_with_alpha(i->alpha);
			}
		}
	}
	onion_context->restore();

	// finish with onion skin
	if (onion_surface) {
		assert(onion_context != context);
		onion_context.clear(); // release onion context
		onion_surface->flush();

		// normalize alpha
		ColorReal k = 255.f/summary_alpha;
		unsigned char *data = onion_surface->get_data();
		int stride = onion_surface->get_stride();
		int image_size = stride*onion_surface->get_height();
		for(unsigned char *row = data, *image_end = data + image_size; row < image_end; row += stride)
			for(unsigned char *pixel = row, *row_end = row + stride; pixel < row_end; pixel += 4)
				*pixel = (unsigned char)round(max(0.f, min(255.f, ((ColorReal)(*pixel)*k))));

		// put merged onion to context
		context->save();
		context->set_source(onion_surface, (double)expose_rect.minx, (double)expose_rect.miny);
		context->paint();
		context->restore();
	}

	// draw the border around the rendered region
	context->save();
	context->set_line_cap(Cairo::LINE_CAP_BUTT);
	context->set_line_join(Cairo::LINE_JOIN_MITER);
	context->set_antialias(Cairo::ANTIALIAS_NONE);
	context->set_line_width(1.0);
	context->set_source_rgb(0,0,0);
	context->rectangle(
		(double)window_offset[0],
		(double)window_offset[1],
		(double)get_w(),
		(double)get_h() );
	context->stroke();
	context->restore();
}
