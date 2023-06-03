/* === S Y N F I G ========================================================= */
/*!	\file target_tile.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TARGET_TILE_H
#define __SYNFIG_TARGET_TILE_H

/* === H E A D E R S ======================================================= */

#include "target.h"

/* === M A C R O S ========================================================= */

#define TILE_SIZE 64

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace rendering { class SurfaceResource; }

/*!	\class Target_Tile
**	\brief Render-target
**	\todo writeme
*/
class Target_Tile : public Target
{
	//! Number of threads
	int threads_;
	//! Tile width in pixels
	int tile_w_;
	//! Tile height in pixles
	int tile_h_;
	//! The current tile being rendered
	int curr_tile_;
	//! Determines if the tiles should be clipped to the redener description
	//! or not
	bool clipping_;

	String engine_;

	struct TileGroup;

	bool call_renderer(
		const etl::handle<rendering::SurfaceResource> &surface,
		Canvas &canvas,
		const ContextParams &context_params,
		const RendDesc &renddesc );

public:
	typedef etl::handle<Target_Tile> Handle;
	typedef etl::loose_handle<Target_Tile> LooseHandle;
	typedef etl::handle<const Target_Tile> ConstHandle;

	Target_Tile();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback* cb = nullptr);

	virtual bool async_render_tile(
		Canvas::Handle canvas,
		ContextParams context_params,
		RectInt rect,
		RendDesc tile_desc,
		ProgressCallback *cb);
	virtual bool wait_render_tiles(ProgressCallback* cb = nullptr);

	//! Determines which tile needs to be rendered next.
	/*!	Most cases will not have to redefine this function.
	**	The default should be adequate in nearly all situations.
	**	\returns The number of tiles left to go <i>plus one</i>.
	**		This means that whenever this function returns zero,
	**		there are no more tiles to render and that any value
	**		in \a rect should be disregarded. */
	virtual int next_tile(RectInt& rect);
	
	//! Returns the number of peniding frames to render. If it is zero it
	//! stops rendering frames.
	virtual int next_frame(Time& time);

	//! Adds the tile at \a x , \a y contained in \a surface
	virtual bool add_tile(const synfig::Surface &surface, int x, int y)=0;

	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	**	\see end_frame(), start_scanline()
	*/
	virtual bool start_frame(ProgressCallback* cb = nullptr) = 0;

	//! Marks the end of a frame
	/*! \see start_frame() */
	virtual void end_frame()=0;
	//!Sets the number of threads
	void set_threads(int x) { threads_=x; }
	//!Gets the number of threads
	int get_threads()const { return threads_; }
	//!Sets the tile width
	void set_tile_w(int w) { tile_w_=w; }
	//!Gets the tile width
	int get_tile_w()const { return tile_w_; }
	//!Sets the tile height
	void set_tile_h(int h) { tile_h_=h; }
	//!Gets the tile height
	int get_tile_h()const { return tile_h_; }
	//! Gets clipping
	bool get_clipping()const { return clipping_; }
	//! Sets clipping
	void set_clipping(bool x) { clipping_=x; }
	//! Gets engine
	const String& get_engine()const { return engine_; }
	//! Sets engine
	void set_engine(const String &x) { engine_=x; }

private:
	//! Renders the context to the surface
	bool render_frame_(Canvas::Handle canvas, ContextParams context_params, ProgressCallback *cb);

}; // END of class Target_Tile

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
