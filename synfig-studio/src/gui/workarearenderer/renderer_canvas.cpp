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
#include <cstring>
#include <valarray>

#include <glib.h>
#include <gdkmm/general.h>

#include <ETL/misc>

#include <synfig/general.h>
#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/threadpool.h>
#include <synfig/rendering/renderer.h>
#include <synfig/rendering/common/task/tasktransformation.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/timemodel.h>

#include "renderer_canvas.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef NDEBUG
//#define DEBUG_TILES
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static int
int_floor(int x, int base)
	{ int m = x % base; return m < 0 ? x - base - m : m > 0 ? x - m : x; }

static int
int_ceil(int x, int base)
	{ int m = x % base; return m > 0 ? x + base - m : m < 0 ? x - m : x; }

static long long
image_rect_size(const RectInt &rect)
	{ return 4ll*rect.get_width()*rect.get_height(); }

/* === M E T H O D S ======================================================= */

Renderer_Canvas::Renderer_Canvas():
	max_tiles_size_soft(512*1024*1024),
	max_tiles_size_hard(max_tiles_size_soft + 128*1024*1024),
	weight_future      (   1.0), // high priority
	weight_past        (   2.0), // low priority
	weight_future_extra(  16.0),
	weight_past_extra  (  32.0),
	weight_zoom_in     (1024.0), // very very low priority
	weight_zoom_out    (1024.0),
	max_enqueued_tasks (6),
	enqueued_tasks(),
	tiles_size(),
	pixel_format()
{
	// check endianness
    union { int i; char c[4]; } checker = {0x01020304};
    bool big_endian = checker.c[0] == 1;

    pixel_format = big_endian
		         ? (PF_A_START | PF_RGB | PF_A_PREMULT)
		         : (PF_BGR | PF_A | PF_A_PREMULT);

	alpha_src_surface = Cairo::ImageSurface::create(
		Cairo::FORMAT_ARGB32, 1, 1);
	alpha_dst_surface = Cairo::ImageSurface::create(
		Cairo::FORMAT_ARGB32, 1, 1);

	//! fill alpha_src_surface with white color
	//! alpha premulted - so all four channels have the same value
	unsigned char *data = alpha_src_surface->get_data();
	data[0] = data[1] = data[2] = data[3] = 255;
	alpha_src_surface->mark_dirty();
	alpha_src_surface->flush();

	alpha_context = Cairo::Context::create(alpha_dst_surface);
}

Renderer_Canvas::~Renderer_Canvas()
	{ clear_render(); }

void
Renderer_Canvas::on_tile_finished_callback(bool success, Renderer_Canvas *obj, Tile::Handle tile)
{
	// This method may be called from the other threads
	// Handle will protect 'tile' from deletion before this call
	// This callback will called only once for each tile
	// And will called before deletion of 'obj', by calling cancel_render() from destructor
	obj->on_tile_finished(success, tile);
}

void
Renderer_Canvas::on_post_tile_finished_callback(etl::handle<Renderer_Canvas> obj, Tile::Handle tile) {
	// this function should be called in main thread
	// Handle will protect 'obj' from deletion before this call
	// zero 'work_area' means that 'work_area' is destructed and here is a last Handle of 'obj'
	if (obj->get_work_area())
		obj->on_post_tile_finished(tile);
}

Cairo::RefPtr<Cairo::ImageSurface>
Renderer_Canvas::convert(
	const rendering::SurfaceResource::Handle &surface,
	int width, int height ) const
{
	// this method may be called from the other threads
	assert(width > 0 && height > 0);

	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface =
		Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);

	bool success = false;

	rendering::SurfaceResource::LockReadBase surface_lock(surface);
	if (surface_lock.get_resource() && surface_lock.get_resource()->is_blank()) {
		success = true;
	} else
	if (surface_lock.convert(rendering::Surface::Token::Handle(), false, true)) {
		const rendering::Surface &s = *surface_lock.get_surface();
		int w = s.get_width();
		int h = s.get_height();
		if (w == width && h == height) {
			const Color *pixels = s.get_pixels_pointer();
			std::vector<Color> pixels_copy;
			if (!pixels) {
				pixels_copy.resize(w*h);
				if (s.get_pixels(&pixels_copy.front()))
					pixels = &pixels_copy.front();
			}
			if (pixels) {
				// do conversion
				cairo_surface->flush();
				color_to_pixelformat(
					cairo_surface->get_data(),
					pixels,
					pixel_format,
					&App::gamma,
					cairo_surface->get_width(),
					cairo_surface->get_height(),
					cairo_surface->get_stride() );
				cairo_surface->mark_dirty();
				cairo_surface->flush();
				success = true;
			} else error("Renderer_Canvas::convert: cannot access surface pixels - that really strange");
		} else error("Renderer_Canvas::convert: surface with wrong size");
	} else error("Renderer_Canvas::convert: surface not exists");


	#ifdef DEBUG_TILES
	const bool debug_tiles = true;
	#else
	const bool debug_tiles = false;
	#endif

	// paint tile
	if (debug_tiles || !success) {
		Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(cairo_surface);

		if (!success) {
			// draw cross
			context->move_to(0.0, 0.0);
			context->line_to((double)width, (double)height);
			context->move_to((double)width, 0.0);
			context->line_to(0.0, (double)height);
			context->stroke();
		}

		// draw border
		context->rectangle(0, 0, width, height);
		context->stroke();
		std::valarray<double> dash(2); dash[0] = 2.0; dash[1] = 2.0;
		context->set_dash(dash, 0.0);
		context->rectangle(4, 4, width-8, height-8);
		context->stroke();

		cairo_surface->flush();
	}
	return cairo_surface;
}

void
Renderer_Canvas::on_tile_finished(bool success, const Tile::Handle &tile)
{
	// this method must be called from on_tile_finished_callback()
	// this method may be called from other threads

	// 'tiles', 'onion_frames', 'refresh_id', and 'tiles_size' are controlled by mutex

	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface;
	if (success && tile->surface)
		cairo_surface = convert(tile->surface, tile->rect.get_width(), tile->rect.get_height());

	Glib::Threads::Mutex::Lock lock(mutex);

	--enqueued_tasks;

	if (!tile->event && !tile->surface && !tile->cairo_surface)
		return; // tile is already removed

	tile->event.reset();
	tile->cairo_surface = cairo_surface;
	tile->surface.reset();

	// don't create handle if ref-count is zero
	// it means that object was nether had a handles and will removed with handle
	// or object is already in destruction phase
	if (shared_object::count())
		Glib::signal_idle().connect_once(
			sigc::bind(sigc::ptr_fun(&on_post_tile_finished_callback), etl::handle<Renderer_Canvas>(this), tile),
			visible_frames.count(tile->frame_id) ? Glib::PRIORITY_DEFAULT : Glib::PRIORITY_DEFAULT_IDLE );
}

void
Renderer_Canvas::on_post_tile_finished(const Tile::Handle &tile)
{
	// this method must be called from on_post_tile_finished_callback()
	// check if rendering is finished
	bool tile_visible = false;
	int local_enqueued_tasks;
	Time time;
	{
		Glib::Threads::Mutex::Lock lock(mutex);
		time = tile->frame_id.time;
		if (visible_frames.count(tile->frame_id))
			tile_visible = true;
		local_enqueued_tasks = enqueued_tasks; // field should be protected by mutex
	}

	if (get_work_area()) {
		get_work_area()->signal_rendering()();
		get_work_area()->signal_rendering_tile_finished()(time);
		if (tile_visible)
			get_work_area()->queue_draw(); // enqueue_render will called while draw
		else
		if (!local_enqueued_tasks)
			enqueue_render();
	}
}

void
Renderer_Canvas::insert_tile(TileList &list, const Tile::Handle &tile)
{
	// this method may be called from other threads
	// mutex must be already locked
	list.push_back(tile);
	tiles_size += image_rect_size(tile->rect);
}

void
Renderer_Canvas::erase_tile(TileList &list, TileList::iterator i, rendering::Task::List &events)
{
	// this method may be called from other threads
	// mutex must be already locked
	if ((*i)->event) events.push_back((*i)->event);
	tiles_size -= image_rect_size((*i)->rect);
	(*i)->event.reset();
	(*i)->surface.reset();
	(*i)->cairo_surface.clear();
	list.erase(i);
}

void
Renderer_Canvas::remove_extra_tiles(rendering::Task::List &events)
{
	// mutex must be already locked

	typedef std::multimap<Real, TileMap::iterator> WeightMap;
	WeightMap sorted_frames;

	Real current_zoom = sqrt((Real)(current_frame.width * current_frame.height));

	// calc weight
	for(TileMap::iterator i = tiles.begin(); i != tiles.end(); ++i) {
		if (!visible_frames.count(i->first) && tiles_size > max_tiles_size_hard) {
			Real weight = 0.0;
			if (frame_duration) {
				Time dt = i->first.time - current_frame.time;
				Real df = ((double)dt)/(double)frame_duration;
				weight += df*(df > 0.0 ? weight_future : weight_past);
			}
			if (current_zoom) {
				Real zoom = sqrt((Real)(i->first.width * i->first.height));
				Real zoom_step = log(zoom/current_zoom);
				weight += zoom_step*(zoom_step > 0.0 ? weight_zoom_in : weight_zoom_out);
			}
			sorted_frames.insert( WeightMap::value_type(weight, i) );
		}
	}

	// remove some extra tiles to free the memory
	for(WeightMap::reverse_iterator ri = sorted_frames.rbegin(); ri != sorted_frames.rend() && tiles_size > max_tiles_size_hard; ++ri)
		for(TileList::iterator j = ri->second->second.begin(); j != ri->second->second.end() && tiles_size > max_tiles_size_hard; )
			erase_tile(ri->second->second, j++, events);

	// remove empty entries from tiles map
	for(TileMap::iterator i = tiles.begin(); i != tiles.end(); )
		if (i->second.empty()) tiles.erase(i++); else ++i;
}

void
Renderer_Canvas::build_onion_frames()
{
	// mutex must be already locked

	Canvas::Handle canvas    = get_work_area()->get_canvas();
	int            w         = get_work_area()->get_w();
	int            h         = get_work_area()->get_h();
	int            thumb_w   = get_work_area()->get_thumb_w();
	int            thumb_h   = get_work_area()->get_thumb_h();
	int            past      = std::max(0, get_work_area()->get_onion_skins()[0]);
	int            future    = std::max(0, get_work_area()->get_onion_skins()[1]);

	Time base_time;
	if (CanvasView::Handle canvas_view = get_work_area()->get_canvas_view())
		base_time = Time(canvas_view->get_time());

	RendDesc       rend_desc = canvas->rend_desc();
	float          fps       = rend_desc.get_frame_rate();

	current_frame = FrameId(base_time, w, h);
	current_thumb = FrameId(base_time, thumb_w, thumb_h);
	frame_duration = Time(approximate_greater_lp(fps, 0.f) ? 1.0/(double)fps : 0.0);

	// set onion_frames
	onion_frames.clear();
	if ( get_work_area()->get_onion_skin()
	  && frame_duration
	  && (past > 0 || future > 0) )
	{
		const Color color_past  (1.f, 0.f, 0.f, 0.2f);
		const Color color_future(0.f, 1.f, 0.f, 0.2f);
		const ColorReal base_alpha = 1.f;
		const ColorReal current_alpha = 0.5f;
		// make onion levels
		for(int i = past; i > 0; --i) {
			Time time = base_time - frame_duration*i;
			ColorReal alpha = base_alpha + (ColorReal)(past - i + 1)/(ColorReal)(past + 1);
			if (time >= rend_desc.get_time_start() && time <= rend_desc.get_time_end())
				onion_frames.push_back(FrameDesc(time, w, h, alpha));
		}
		for(int i = future; i > 0; --i) {
			Time time = base_time + frame_duration*i;
			ColorReal alpha = base_alpha + (ColorReal)(future - i + 1)/(ColorReal)(future + 1);
			if (time >= rend_desc.get_time_start() && time <= rend_desc.get_time_end())
				onion_frames.push_back(FrameDesc(time, w, h, alpha));
		}
		onion_frames.push_back(FrameDesc(current_frame, base_alpha + 1.f + current_alpha));

		// normalize
		ColorReal summary = 0.f;
		for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i)
			summary += i->alpha;
		ColorReal k = approximate_greater(summary, ColorReal(1.f)) ? 1.f/summary : 1.f;
		for(FrameList::iterator i = onion_frames.begin(); i != onion_frames.end(); ++i)
			i->alpha *= k;
	} else {
		onion_frames.push_back(FrameDesc(current_frame, 1.f));
	}

	// set visible_frames
	visible_frames.clear();
	for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i)
		visible_frames.insert(i->id);
}

bool
Renderer_Canvas::enqueue_render_frame(
	const rendering::Renderer::Handle &renderer,
	const Canvas::Handle &canvas,
	const RectInt &window_rect,
	const FrameId &id )
{
	// mutex must be already locked

	const int tile_grid_step = 64;

	RendDesc rend_desc = canvas->rend_desc();
	int      w         = id.width;
	int      h         = id.height;

	rend_desc.clear_flags();
	rend_desc.set_wh(w, h);
	rend_desc.set_render_excluded_contexts(true);
	ContextParams context_params(rend_desc.get_render_excluded_contexts());
	TileList &frame_tiles = tiles[id];

	// create transformation matrix to flip result if needed
	bool transform = false;
	Matrix matrix;
	Vector p0 = rend_desc.get_tl();
	Vector p1 = rend_desc.get_br();
	if (p0[0] > p1[0] || p0[1] > p1[1]) {
		if (p0[0] > p1[0]) { matrix.m00 = -1.0; matrix.m20 = p0[0] + p1[0]; std::swap(p0[0], p1[0]); }
		if (p0[1] > p1[1]) { matrix.m11 = -1.0; matrix.m21 = p0[1] + p1[1]; std::swap(p0[1], p1[1]); }
		rend_desc.set_tl_br(p0, p1);
		transform = true;
	}

	// find not actual regions
	std::vector<RectInt> rects;
	rects.reserve(20);
	rects.push_back(window_rect);
	for(TileList::const_iterator j = frame_tiles.begin(); j != frame_tiles.end(); ++j)
		if (*j) etl::rects_subtract(rects, (*j)->rect);
	etl::rects_merge(rects);

	if (rects.empty()) return false;

	// build rendering task
	canvas->set_time(id.time);
	canvas->load_resources(id.time);
	canvas->set_outline_grow(rend_desc.get_outline_grow());
	CanvasBase sub_queue;
	Context context = canvas->get_context_sorted(context_params, sub_queue);
	rendering::Task::Handle task = context.build_rendering_task();
	sub_queue.clear();

	// add transformation task to flip result if needed
	if (task && transform) {
		rendering::TaskTransformationAffine::Handle t = new rendering::TaskTransformationAffine();
		t->transformation->matrix = matrix;
		t->sub_task() = task;
		task = t;
	}

	// TaskSurface assumed as valid non-trivial task by renderer
	// and TaskTransformationAffine of TaskSurface will not be optimized.
	// To avoid this construction place creation of dummy TaskSurface here.
	if (!task) task = new rendering::TaskSurface();

	for(std::vector<RectInt>::iterator j = rects.begin(); j != rects.end(); ++j) {
		// snap rect corners to tile grid
		RectInt &rect = *j;
		rect.minx = int_floor(rect.minx, tile_grid_step);
		rect.miny = int_floor(rect.miny, tile_grid_step);
		rect.maxx = int_ceil (rect.maxx, tile_grid_step);
		rect.maxy = int_ceil (rect.maxy, tile_grid_step);
		rect &= id.rect();

		RendDesc tile_desc=rend_desc;
		tile_desc.set_subwindow(rect.minx, rect.miny, rect.get_width(), rect.get_height());

		rendering::Task::Handle tile_task = task->clone_recursive();
		tile_task->target_surface = new rendering::SurfaceResource();
		tile_task->target_surface->create(tile_desc.get_w(), tile_desc.get_h());
		tile_task->target_rect = RectInt( VectorInt(), tile_task->target_surface->get_size() );
		tile_task->source_rect = Rect(tile_desc.get_tl(), tile_desc.get_br());

		Tile::Handle tile = new Tile(id, *j);
		tile->surface = tile_task->target_surface;

		tile->event = new rendering::TaskEvent();
		tile->event->signal_finished.connect( sigc::bind(
			sigc::ptr_fun(&on_tile_finished_callback), this, tile ));

		insert_tile(frame_tiles, tile);

		++enqueued_tasks;

		// Renderer::enqueue contains the expensive 'optimization' stage, so call it async
		ThreadPool::instance.enqueue( sigc::bind(
			sigc::ptr_fun(&rendering::Renderer::enqueue_task_func),
			renderer, tile_task, tile->event, false ));
	}

	return true;
}

void
Renderer_Canvas::enqueue_render()
{
	assert(get_work_area());

	rendering::Task::List events;

	{
		Glib::Threads::Mutex::Lock lock(mutex);

		String         renderer_name  = get_work_area()->get_renderer();
		RectInt        window_rect    = get_work_area()->get_window_rect();
		bool           bg_rendering   = get_work_area()->get_background_rendering();
		Canvas::Handle canvas         = get_work_area()->get_canvas();
		etl::handle<CanvasView> canvas_view = get_work_area()->get_canvas_view();
		etl::handle<TimeModel> time_model = canvas_view->time_model();
		bool			is_playing = canvas_view->is_playing();

		build_onion_frames();

		rendering::Renderer::Handle renderer = rendering::Renderer::get_renderer(renderer_name);
		
		int max_tasks = max_enqueued_tasks;
		if (is_playing)
			max_tasks = 2;
		
		if (renderer && enqueued_tasks < max_tasks) {
			if (canvas && window_rect.is_valid()) {
				Time orig_time = canvas->get_time();
				int enqueued = 0;

				// generate rendering task for thumbnail
				// do it first to be sure that thmubnails will always fully covered by the single tile
				if (enqueue_render_frame(renderer, canvas, current_thumb.rect(), current_thumb))
					++enqueued;

				// generate rendering tasks for visible areas
				for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i)
					if (enqueue_render_frame(renderer, canvas, window_rect, i->id))
						++enqueued;

				remove_extra_tiles(events);

				// generate rendering tasks for future or past frames
				// render only one frame in background
				int future = 0, past = 0;
				long long frame_size = image_rect_size(window_rect);
				bool time_in_repeat_range = time_model->get_time() >= time_model->get_play_bounds_lower()
						                 && time_model->get_time() <= time_model->get_play_bounds_upper();
				
				while(bg_rendering && enqueued_tasks < max_tasks && tiles_size + frame_size < max_tiles_size_soft)
				{
					Time future_time = current_frame.time + frame_duration*future;
					bool future_exists = future_time >= time_model->get_lower()
									  && future_time <= time_model->get_upper();
					Real weight_future_current = !time_in_repeat_range
							                  || ( future_time >= time_model->get_play_bounds_lower()
							                    && future_time <= time_model->get_play_bounds_upper() )
											   ? weight_future : weight_future_extra;

					Time past_time = current_frame.time - frame_duration*past;
					bool past_exists = false;
					if (!is_playing)
						past_exists = past_time >= time_model->get_lower()
									&& past_time <= time_model->get_upper();
					Real weight_past_current = !time_in_repeat_range
							                || ( past_time >= time_model->get_play_bounds_lower()
							                  && past_time <= time_model->get_play_bounds_upper() )
											 ? weight_past : weight_past_extra;

					if (!future_exists && !past_exists) break;

					bool future_priority = weight_future_current*future < weight_past_current*past;

					if (future_exists && (!past_exists || future_priority)) {
						// queue future
						if (enqueue_render_frame(renderer, canvas, current_thumb.rect(), current_thumb.with_time(future_time)))
							++enqueued;
						if (enqueue_render_frame(renderer, canvas, window_rect, current_frame.with_time(future_time)))
							++enqueued;
						++future;
					} else {
						// queue past
						if (enqueue_render_frame(renderer, canvas, current_thumb.rect(), current_thumb.with_time(past_time)))
							++enqueued;
						if (enqueue_render_frame(renderer, canvas, window_rect, current_frame.with_time(past_time)))
							++enqueued;
						++past;
					}
				}

				// restore canvas time
				if (!is_playing)
					canvas->set_time(orig_time);

				if (enqueued)
					get_work_area()->signal_rendering()();
			}
		}
	}

	rendering::Renderer::cancel(events);
}

void
Renderer_Canvas::wait_render()
{
	rendering::TaskEvent::List events;
	{
		Glib::Threads::Mutex::Lock lock(mutex);
		for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i) {
			TileMap::const_iterator ii = tiles.find(i->id);
			if (ii != tiles.end())
				for(TileList::const_iterator j = ii->second.begin(); j != ii->second.end(); ++j)
					if (*j && (*j)->event)
						events.push_back((*j)->event);
		}
	}
	for(rendering::TaskEvent::List::const_iterator i = events.begin(); i != events.end(); ++i)
		(*i)->wait();
}

void
Renderer_Canvas::clear_render()
{
	rendering::Task::List events;
	bool cleared = false;
	{
		Glib::Threads::Mutex::Lock lock(mutex);
		cleared = !tiles.empty();
		for(TileMap::iterator i = tiles.begin(); i != tiles.end(); ++i)
			while(!i->second.empty()) {
				TileList::iterator j = i->second.end(); --j;
				erase_tile(i->second, j++, events);
			}
		tiles.clear();
	}
	rendering::Renderer::cancel(events);
	if (cleared && get_work_area())
		get_work_area()->signal_rendering()();
}

Renderer_Canvas::FrameStatus
Renderer_Canvas::merge_status(FrameStatus a, FrameStatus b) {
	static FrameStatus map[FS_Count][FS_Count] = {
	// FS_None          | FS_PartiallyDone | FS_InProcess     | FS_Done              //
	// -----------------|------------------|------------------|-----------------     //
	 { FS_None          , FS_PartiallyDone , FS_InProcess     , FS_PartiallyDone },  // FS_None
	 { FS_PartiallyDone , FS_PartiallyDone , FS_InProcess     , FS_PartiallyDone },  // FS_PartiallyDone
	 { FS_InProcess     , FS_InProcess     , FS_InProcess     , FS_InProcess     },  // FS_InProcess
	 { FS_PartiallyDone , FS_PartiallyDone , FS_InProcess     , FS_Done          }}; // FS_Done

	if ((int)a < 0 || (int)a > (int)FS_Count) a = FS_None;
	if ((int)b < 0 || (int)b > (int)FS_Count) b = FS_None;
	return map[a][b];
}


Renderer_Canvas::FrameStatus
Renderer_Canvas::calc_frame_status(const FrameId &id, const RectInt &window_rect)
{
	// mutex must be already locked

	TileMap::const_iterator i = tiles.find(id);
	if (i == tiles.end() || i->second.empty())
		return FS_None;

	std::vector<RectInt> rects;
	rects.reserve(20);
	rects.push_back(window_rect);
	for(TileList::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		if (*j) {
			if ((*j)->event)
				return FS_InProcess;
			if ((*j)->cairo_surface)
				etl::rects_subtract(rects, (*j)->rect);
		}
	etl::rects_merge(rects);

	if (rects.size() == 1 && rects.front() == window_rect)
		return FS_None;
	if (rects.empty())
		return FS_Done;
	return FS_PartiallyDone;
}

void
Renderer_Canvas::get_render_status(StatusMap &out_map)
{
	Glib::Threads::Mutex::Lock lock(mutex);

	RectInt window_rect = get_work_area()->get_window_rect();

	out_map.clear();
	for(TileMap::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
		if ( !i->second.empty()
		  && ( (i->first.width == current_thumb.width && i->first.height == current_thumb.height)
			|| (i->first.width == current_frame.width && i->first.height == current_frame.height) ))
				out_map[i->first.time] = FS_None;

	for(StatusMap::iterator i = out_map.begin(); i != out_map.end(); ) {
		i->second = merge_status(
			calc_frame_status(current_frame.with_time(i->first), window_rect),
			calc_frame_status(current_thumb.with_time(i->first), current_thumb.rect()) );
		if (i->second == FS_None) out_map.erase(i++); else ++i;
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
	expose_rect -= window_offset;
	expose_rect &= window_rect;
	if (!expose_rect.is_valid()) return;

	// calculate world coordinates of expose_rect corners
	if (!get_work_area()->get_canvas()) return;
	const RendDesc &rend_desc = get_work_area()->get_canvas()->rend_desc();
	int w = get_work_area()->get_w();
	int h = get_work_area()->get_h();
	Vector tl = rend_desc.get_tl();
	Vector br = rend_desc.get_br();
	if ( w <= 0 || h <= 0
	  || approximate_equal(tl[0], br[0])
	  || approximate_equal(tl[1], br[1]) ) return;

	Real pw = (br[0] - tl[0])/(Real)w;
	Real ph = (br[1] - tl[1])/(Real)h;
	tl[0] += pw*(double)expose_rect.minx;
	tl[1] += ph*(double)expose_rect.miny;
	br[0] += pw*(double)(expose_rect.maxx - w);
	br[1] += ph*(double)(expose_rect.maxy - h);

	// calculate pixel coordinates of previous surface
	RectInt previous_rect;
	if ( previous_surface
	  && previous_surface->get_width() > 0
	  && previous_surface->get_height() > 0
	  && approximate_not_equal(previous_tl[0], previous_br[0])
	  && approximate_not_equal(previous_tl[1], previous_br[1]) )
	{
		previous_rect.minx = (int)round((previous_tl[0] - tl[0])/pw);
		previous_rect.miny = (int)round((previous_tl[1] - tl[1])/ph);
		previous_rect.maxx = (int)round((previous_br[0] - tl[0])/pw);
		previous_rect.maxy = (int)round((previous_br[1] - tl[1])/ph);
		previous_rect = expose_rect;
	}

	// enqueue rendering if not all of visible tiles are exists and actual
	enqueue_render();

	Cairo::RefPtr<Cairo::Context> canvas_context;
	Cairo::RefPtr<Cairo::ImageSurface> canvas_surface;
	std::vector<RectInt> empty_rects;
	empty_rects.reserve(20);
	empty_rects.push_back(previous_rect);

	{ // merge all tiles into single surface
		Glib::Threads::Mutex::Lock lock(mutex);

		if (onion_frames.empty()) return;

		// create surface and context to merge tiles
		canvas_surface = Cairo::ImageSurface::create(
			Cairo::FORMAT_ARGB32, expose_rect.get_width(), expose_rect.get_height() );
		canvas_context = Cairo::Context::create(canvas_surface);
		canvas_context->translate(-(double)expose_rect.minx, -(double)expose_rect.miny);
		canvas_context->set_operator(Cairo::OPERATOR_SOURCE);

		if ( onion_frames.size() > 1
		  || !approximate_equal_lp(onion_frames.front().alpha, ColorReal(1.f)) )
		{
			canvas_context->set_operator(Cairo::OPERATOR_ADD);

			// prepare background to tune alpha
			alpha_context->set_operator(canvas_context->get_operator());
			alpha_context->set_source(alpha_src_surface, 0, 0);
			int alpha_offset = FLAGS(pixel_format, PF_A_START) ? 0 : 3;
			unsigned char base[] = {0, 0, 0, 0};
			memcpy(alpha_dst_surface->get_data(), base, sizeof(base));
			alpha_dst_surface->mark_dirty();
			alpha_dst_surface->flush();
			for(FrameList::const_iterator j = onion_frames.begin(), i = j++; j != onion_frames.end(); i = j++)
				alpha_context->paint_with_alpha(i->alpha);
			alpha_dst_surface->flush();
			memcpy(base, alpha_dst_surface->get_data(), sizeof(base));

			// tune alpha
			while(true) {
				memcpy(alpha_dst_surface->get_data(), base, sizeof(base));
				alpha_dst_surface->mark_dirty();
				alpha_dst_surface->flush();
				alpha_context->paint_with_alpha(onion_frames.back().alpha);
				int alpha = alpha_dst_surface->get_data()[alpha_offset];
				if (alpha >= 255) break;
				onion_frames.back().alpha += (ColorReal)(255 - alpha)/ColorReal(128.f);
			}
		}

		// draw tiles
		canvas_context->save();
		for(FrameList::const_iterator i = onion_frames.begin(); i != onion_frames.end(); ++i) {
			TileMap::const_iterator ii = tiles.find(i->id);
			if (ii == tiles.end()) continue;
			for(TileList::const_iterator j = ii->second.begin(); j != ii->second.end(); ++j) {
				if (!*j) continue;
				if ((*j)->cairo_surface) {
					etl::rects_subtract(empty_rects, (*j)->rect); // mark area as not empty
					canvas_context->save();
					canvas_context->rectangle((*j)->rect.minx, (*j)->rect.miny, (*j)->rect.get_width(), (*j)->rect.get_height());
					canvas_context->clip();
					canvas_context->set_source((*j)->cairo_surface, (*j)->rect.minx, (*j)->rect.miny);
					if (canvas_surface)
						canvas_context->paint_with_alpha(i->alpha);
					else
						canvas_context->paint();
					canvas_context->restore();
				}
			}
		}
		canvas_context->restore();
		canvas_surface->flush();
	}

	// fill empty areas with previous surface
	etl::rects_merge(empty_rects);
	if (!empty_rects.empty()) {
		canvas_context->save();

		for(std::vector<RectInt>::const_iterator i = empty_rects.begin(); i != empty_rects.end(); ++i)
			canvas_context->rectangle(i->minx, i->miny, i->get_width(), i->get_height());
		canvas_context->clip();

		Real previous_pw = (previous_br[0] - previous_tl[0])/(Real)previous_surface->get_width();
		Real previous_ph = (previous_br[1] - previous_tl[1])/(Real)previous_surface->get_height();
		canvas_context->translate(
			(previous_tl[0] - rend_desc.get_tl()[0])/pw,
			(previous_tl[1] - rend_desc.get_tl()[1])/ph );
		canvas_context->scale(
			previous_pw/pw,
			previous_ph/ph );
		canvas_context->set_operator(Cairo::OPERATOR_SOURCE);
		canvas_context->set_source(previous_surface, 0.0, 0.0);
		canvas_context->paint();

		canvas_context->restore();
		canvas_surface->flush();
	}

	// remember surface with merged tiles for the future
	previous_tl = tl;
	previous_br = br;
	previous_surface = canvas_surface;

	Cairo::RefPtr<Cairo::Context> context = drawable->create_cairo_context();
	context->save();
	context->translate((double)window_offset[0], (double)window_offset[1]);

	// put merged tiles to context
	context->save();
	context->rectangle(expose_rect.minx, expose_rect.miny, expose_rect.get_width(), expose_rect.get_height());
	context->clip();
	context->set_source(canvas_surface, (double)expose_rect.minx, (double)expose_rect.miny);
	context->paint();
	context->restore();

	// draw the border around the rendered region
	context->save();
	context->set_line_cap(Cairo::LINE_CAP_BUTT);
	context->set_line_join(Cairo::LINE_JOIN_MITER);
	context->set_antialias(Cairo::ANTIALIAS_NONE);
	context->set_line_width(1.0);
	context->set_source_rgba(0.0, 0.0, 0.0, 1.0);
	context->rectangle(0.0, 0.0, (double)current_frame.width, (double)current_frame.height);
	context->stroke();
	context->restore();

	context->restore();
}

Cairo::RefPtr<Cairo::ImageSurface>
Renderer_Canvas::get_thumb(const Time &time)
{
	Glib::Threads::Mutex::Lock lock(mutex);
	TileMap::const_iterator i = tiles.find( current_thumb.with_time(time) );
	return i == tiles.end() || i->second.empty() || !*(i->second.begin())
		 ? Cairo::RefPtr<Cairo::ImageSurface>()
		 : (*(i->second.begin()))->cairo_surface;
}
