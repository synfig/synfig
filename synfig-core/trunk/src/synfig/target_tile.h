/* === S I N F G =========================================================== */
/*!	\file target_tile.h
**	\brief Template Header
**
**	$Id: target_tile.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_TARGET_TILE_H
#define __SINFG_TARGET_TILE_H

/* === H E A D E R S ======================================================= */

#include "target.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/*!	\class Target_Tile
**	\brief Render-target
**	\todo writeme
*/
class Target_Tile : public Target
{
	int threads_;
	int tile_w_;
	int tile_h_;
	int curr_tile_;
	int curr_frame_;
	bool clipping_;
public:
	typedef etl::handle<Target_Tile> Handle;
	typedef etl::loose_handle<Target_Tile> LooseHandle;
	typedef etl::handle<const Target_Tile> ConstHandle;

	Target_Tile();

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

	virtual int next_frame(Time& time);

	//! Adds the tile at \a x , \a y contained in \a surface 
	virtual bool add_tile(const sinfg::Surface &surface, int x, int y)=0;

	virtual int total_tiles()const
	{
		// Width of the image(in tiles)
		const int tw(rend_desc().get_w()/tile_w_+(rend_desc().get_w()%tile_w_?1:0));
		const int th(rend_desc().get_h()/tile_h_+(rend_desc().get_h()%tile_h_?1:0));
		
		return tw*th;
	}

	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	**	\see end_frame(), start_scanline()
	*/
	virtual bool start_frame(ProgressCallback *cb=NULL)=0;

	//! Marks the end of a frame
	/*! \see start_frame() */
	virtual void end_frame()=0;
	
	void set_threads(int x) { threads_=x; }

	int get_threads()const { return threads_; }

	void set_tile_w(int w) { tile_w_=w; }

	int get_tile_w()const { return tile_w_; }

	void set_tile_h(int h) { tile_h_=h; }

	int get_tile_h()const { return tile_h_; }
	
	bool get_clipping()const { return clipping_; }

	void set_clipping(bool x) { clipping_=x; }
	
private:
	
	bool render_frame_(Context context,ProgressCallback *cb=0);
	
}; // END of class Target_Tile

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
