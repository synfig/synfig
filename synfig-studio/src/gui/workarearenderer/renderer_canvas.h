/* === S Y N F I G ========================================================= */
/*!	\file renderer_canvas.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERER_CANVAS_H
#define __SYNFIG_RENDERER_CANVAS_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <map>

#include <glibmm/threads.h>

#include "workarearenderer.h"
#include "workarea.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Canvas : public studio::WorkAreaRenderer
{
public:
	typedef synfig::rendering::TaskEvent Event;
	typedef std::vector<Event::Handle> EventList;

	class FrameDesc {
	public:
		synfig::Time time;
		synfig::ColorReal alpha;
		explicit FrameDesc(
			const synfig::Time &time = synfig::Time(),
			synfig::ColorReal alpha = synfig::ColorReal() ):
				time(time), alpha(alpha) { }
	};

	class Tile {
	public:
		long long refresh_id;
		synfig::Time time;
		synfig::RectInt rect;
		Glib::RefPtr<Gdk::Pixbuf> pixbuf;

		Tile(): refresh_id() { }
		Tile(
			int refresh_id,
			const synfig::Time &time,
			int left,
			int top,
			const Glib::RefPtr<Gdk::Pixbuf> &pixbuf
		):
			refresh_id(refresh_id),
			time(time),
			rect( left,
				  top,
				  left + (pixbuf ? pixbuf->get_width() : 0),
				  top + (pixbuf ? pixbuf->get_height() : 0) ),
			pixbuf(pixbuf)
		{ }

		bool operator< (const Tile &other) const {
			if (refresh_id < other.refresh_id) return true;
			if (other.refresh_id < refresh_id) return false;
			return time < other.time;
		}
	};

	typedef std::vector<FrameDesc> FrameList;
	typedef std::set<Tile> TileSet;
	typedef std::map<synfig::Time, TileSet> TileMap;

private:
	Glib::Threads::Mutex mutex;

	TileMap tiles;
	FrameList onion_frames;

	long long refresh_id;
	EventList events;
	bool render_after_current_task_complete;

	void apply_onion_skin(int past, int future);
	void enqueue_thread();
	void remove_old_tiles();

public:
	~Renderer_Canvas();

	int get_refresh_id() const
		{ return refresh_id; }
	void inc_refresh_id()
		{ ++refresh_id; }

	bool in_process() const
		{ return !events.empty(); }
	void cancel_render();
	void enqueue_render(bool after_current_task_complete);
	void wait_render();

	void render() {
		inc_refresh_id();
		enqueue_render(false);
		wait_render();
	}

	void draw_frame(const Cairo::RefPtr<Cairo::Context> &context, const FrameDesc &frame);
	void draw(const Cairo::RefPtr<Cairo::Context> &context);

	void render_vfunc(
		const Glib::RefPtr<Gdk::Window>& drawable,
		const Gdk::Rectangle& expose_area );
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
