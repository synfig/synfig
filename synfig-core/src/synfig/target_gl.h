/* === S Y N F I G ========================================================= */
/*!	\file target_gl.h
**	\brief Template Header for the Target GL class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_TARGET_GL_H
#define __SYNFIG_TARGET_GL_H

/* === H E A D E R S ======================================================= */

#include "target.h"

#include <synfig/rendering/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

// TODO: This name is temporary

/*!	\class Target_GL
**	\brief This is a Target class that implements the render function
* for a new rendering engine
*/
class Target_GL : public Target
{
private:
	// TODO: reorganize work with surfaces, it's quickhack
	std::map<int, rendering::Surface::Handle> surfaces;

public:
	typedef etl::handle<Target_GL> Handle;
	typedef etl::loose_handle<Target_GL> LooseHandle;
	typedef etl::handle<const Target_GL> ConstHandle;

	//! Default constructor (current frame = 0)
	Target_GL();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);
	
	//! Returns the number of peniding frames to render. If it is zero it
	//! stops rendering frames.
	virtual int next_frame(Time& time);

	rendering::Surface::Handle get_surface(int frame) const;
	const std::map<int, rendering::Surface::Handle>& get_surfaces() const
		{ return surfaces; }

	// TODO: reorganize work with surfaces
	void clear_surfaces() { surfaces.clear(); }
}; // END of class Target_Cairo

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
