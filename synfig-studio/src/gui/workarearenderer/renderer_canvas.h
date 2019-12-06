/* === S Y N F I G ========================================================= */
/*!	\file renderer_canvas.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERER_CANVAS_H
#define __SYNFIG_RENDERER_CANVAS_H

/* === H E A D E R S ======================================================= */

#include <climits>

#include <vector>
#include <map>

#include <synfig/time.h>
#include <synfig/rendering/task.h>
#include <synfig/rendering/renderer.h>

#include "../workarea.h"
#include "workarearenderer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Canvas : public studio::WorkAreaRenderer
{
public:
	typedef etl::handle<Renderer_Canvas> Handle;
	typedef etl::loose_handle<Renderer_Canvas> LooseHandle;

	enum FrameStatus {
		FS_None,
		FS_PartiallyDone,
		FS_InProcess,
		FS_Done
	};
	enum { FS_Count = FS_Done + 1 };

	class FrameId {
	public:
		synfig::Time time;
		int width;
		int height;

		explicit FrameId(
			const synfig::Time &time = synfig::Time(),
			int width = 0,
			int height = 0
		):
			time(time), width(width), height(height) { }

		bool operator< (const FrameId &other) const {
			if (time < other.time) return true;
			if (other.time < time) return false;
			if (width < other.width) return true;
			if (other.width < width) return false;
			return height < other.height;
		}

		bool operator== (const FrameId &other) const
			{ return time == other.time && width == other.width && height == other.height; }

		bool operator!= (const FrameId &other) const
			{ return !(*this == other); }

		FrameId with_time(const synfig::Time &time) const
			{ return FrameId(time, width, height); }
		synfig::RectInt rect() const
			{ return synfig::RectInt(0, 0, width, height); }
	};

	class FrameDesc {
	public:
		FrameId id;
		synfig::ColorReal alpha;
		FrameDesc(): alpha() { }
		FrameDesc(
			const FrameId &id,
			synfig::ColorReal alpha ):
				id(id), alpha(alpha) { }
		FrameDesc(
			const synfig::Time &time,
			int width,
			int height,
			synfig::ColorReal alpha ):
				id(time, width, height), alpha(alpha) { }
	};

	class Tile: public etl::shared_object {
	public:
		typedef etl::handle<Tile> Handle;

		const FrameId frame_id;
		const synfig::RectInt rect;

		synfig::rendering::TaskEvent::Handle event;
		synfig::rendering::SurfaceResource::Handle surface;
		Cairo::RefPtr<Cairo::ImageSurface> cairo_surface;

		Tile() { }
		Tile(const FrameId &frame_id, synfig::RectInt &rect):
			frame_id(frame_id), rect(rect) { }
	};

	typedef std::map<synfig::Time, FrameStatus> StatusMap;
	typedef std::set<FrameId> FrameSet;
	typedef std::vector<FrameDesc> FrameList;
	typedef std::vector<Tile::Handle> TileList;
	typedef std::map<FrameId, TileList> TileMap;

private:
	// cache options
	const long long max_tiles_size_soft; //!< threshold for creation of new tiles
	const long long max_tiles_size_hard; //!< threshold for removing already created tiles
	const synfig::Real weight_future;    //!< will multiply to frames count
	const synfig::Real weight_past;
	const synfig::Real weight_future_extra;
	const synfig::Real weight_past_extra;
	const synfig::Real weight_zoom_in;   //!< will multiply to log(zoom)
	const synfig::Real weight_zoom_out;
	const int max_enqueued_tasks;

	//! controls access to fields: enqueued_tasks, tiles, onion_frames, visible_frames, current_frame, frame_duration, tiles_size
	std::mutex mutex;

	int enqueued_tasks;

	//! stored tiles may be actual/outdated and rendered/not-rendered
	TileMap tiles;

	//! all currently visible frames (onion skin feature allows to see more than one frame)
	FrameList onion_frames;
	FrameSet visible_frames;
	FrameId current_thumb;
	FrameId current_frame;
	synfig::Time frame_duration;

	//! increment of this field makes all tiles outdated
	long long tiles_size;

	synfig::PixelFormat pixel_format;

	//! uses to normalize alpha value after blending of onion surfaces
	Cairo::RefPtr<Cairo::ImageSurface> alpha_src_surface;
	Cairo::RefPtr<Cairo::ImageSurface> alpha_dst_surface;
	Cairo::RefPtr<Cairo::Context> alpha_context;

	synfig::Vector previous_tl;
	synfig::Vector previous_br;
	Cairo::RefPtr<Cairo::ImageSurface> previous_surface;

	// don't try to pass arguments to callbacks by reference, it cannot be properly saved in signal
	// Renderer_Canvas is non-thread-safe sigc::trackable, so use static callback methods in signals
	static void on_tile_finished_callback(bool success, Renderer_Canvas *obj, Tile::Handle tile);
	static void on_post_tile_finished_callback(etl::handle<Renderer_Canvas> obj, Tile::Handle tile);

	//! this method may be called from the other threads
	void on_tile_finished(bool success, const Tile::Handle &tile);

	//! this method may be called from the main thread only
	void on_post_tile_finished(const Tile::Handle &tile);

	//! this method may be called from the other threads
	Cairo::RefPtr<Cairo::ImageSurface> convert(
		const synfig::rendering::SurfaceResource::Handle &surface,
		int width, int height ) const;

	//! mutex must be locked before call
	void insert_tile(TileList &list, const Tile::Handle &tile);

	//! mutex must be locked before call
	TileList::iterator erase_tile(TileList &list, TileList::iterator i, synfig::rendering::Task::List &events);

	//! mutex must be locked before call
	void remove_extra_tiles(synfig::rendering::Task::List &events);

	//! mutex must be locked before call
	void build_onion_frames();

	//! mutex must be locked before call
	FrameStatus calc_frame_status(const FrameId &id, const synfig::RectInt &window_rect);

	//! mutex must be locked before call
	//! returns true if rendering task actually enqueued
	//! function can change the canvas time
	bool enqueue_render_frame(
		const synfig::rendering::Renderer::Handle &renderer,
		const synfig::Canvas::Handle &canvas,
		const synfig::RectInt &window_rect,
		const FrameId &id );

public:
	Renderer_Canvas();
	~Renderer_Canvas();

	// functions to render canvas in background

	void enqueue_render();
	void wait_render();
	void clear_render();

	void get_render_status(StatusMap &out_map);

	// just paint already rendered tiles at window
	void render_vfunc(
		const Glib::RefPtr<Gdk::Window>& drawable,
		const Gdk::Rectangle& expose_area );

	Cairo::RefPtr<Cairo::ImageSurface> get_thumb(const synfig::Time &time);

	static FrameStatus merge_status(FrameStatus a, FrameStatus b);
	static FrameStatus& merge_status_to(FrameStatus &dst, FrameStatus src)
		{ return dst = merge_status(dst, src); }
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
