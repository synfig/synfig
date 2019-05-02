/* === S Y N F I G ========================================================= */
/*!	\file target_cairo_tile.h
**	\brief Template Header for the Target Cairo class tile mode
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TARGET_CAIRO_TILE_H
#define __SYNFIG_TARGET_CAIRO_TILE_H

/* === H E A D E R S ======================================================= */

#include "target.h"
#include "cairo.h"


/* === M A C R O S ========================================================= */

#define TILE_SIZE 64

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class CairoSurface;

/*!	\class Target_Cairo_Tile
**	\brief This is a Target class that implements the render function
* for a CairoSurface using tile mode
*/
class Target_Cairo_Tile : public Target
{
	//! Number of threads to use
	int threads_;
	//! Tile width in pixels
	int tile_w_;
	//! Tile height in pixles
	int tile_h_;
	//! The current tile being rendered
	int curr_tile_;
	//! Determines if the tiles should be clipped to the render description
	//! or not
	bool clipping_;

public:
	typedef etl::handle<Target_Cairo_Tile> Handle;
	typedef etl::loose_handle<Target_Cairo_Tile> LooseHandle;
	typedef etl::handle<const Target_Cairo_Tile> ConstHandle;
	//! Default constructor (current frame = 0)
	Target_Cairo_Tile();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);

	//! Determines which tile needs to be rendered next.
	/*!	Most cases will not have to redefine this function.
	 **	The default should be adequate in nearly all situations.
	 **	\returns The number of tiles left to go <i>plus one</i>.
	 **		This means that whenever this function returns zero,
	 **		there are no more tiles to render and that any value
	 **		in \a x or \a y should be disregarded. */
	virtual int next_tile(int& x, int& y);
	
	//! Returns the number of peniding frames to render. If it is zero it
	//! stops rendering frames.
	virtual int next_frame(Time& time);
	
	//! Adds the tile at \a x , \a y contained in \a surface
	virtual bool add_tile(cairo_surface_t* surface, int x, int y)=0;
	//! Returns the total tiles of the imaged rounded to integer number of tiles
	virtual int total_tiles()const
	{
		// Width of the image(in tiles)
		const int tw(rend_desc().get_w()/tile_w_+(rend_desc().get_w()%tile_w_?1:0));
		const int th(rend_desc().get_h()/tile_h_+(rend_desc().get_h()%tile_h_?1:0));
		
		return tw*th;
	}
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
	
	
	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	 **	\see end_frame()*/
	virtual bool start_frame(ProgressCallback *cb=NULL)=0;
	
	//! Marks the end of a frame
	/*! \see start_frame() */
	virtual void end_frame()=0;
	
	//! Filters the cairo surface based on gamma (hardcored for the moment to 2.2)
	static void gamma_filter(cairo_surface_t* surface, const synfig::Gamma &gamma);

private:
	//! Renders the context to the surface
	bool render_frame_(Context context,ProgressCallback *cb=0);

}; // END of class Target_Cairo_Tile

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
