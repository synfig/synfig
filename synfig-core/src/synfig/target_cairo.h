/* === S Y N F I G ========================================================= */
/*!	\file target_cairo.h
**	\brief Template Header for the Target Cairo class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_TARGET_CAIRO_H
#define __SYNFIG_TARGET_CAIRO_H

/* === H E A D E R S ======================================================= */

#include "target.h"
#include "cairo.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class CairoSurface;

/*!	\class Target_Cairo
**	\brief This is a Target class that implements the render function
**     for a CairoSurface
*/
class Target_Cairo : public Target
{
	//! Number of threads to use
	// int threads_;

public:
	typedef etl::handle<Target_Cairo> Handle;
	typedef etl::loose_handle<Target_Cairo> LooseHandle;
	typedef etl::handle<const Target_Cairo> ConstHandle;
	//! Default constructor (current frame = 0)
	Target_Cairo();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);
	
	//! Obtain a surface pointer based on the render method
	//! this function has to be overrrided by the derived targets 
	//! to create the proper Cairo backend surface for each target type.
	virtual bool obtain_surface(cairo_surface_t*&)=0; 
	
	//! Returns the number of peniding frames to render. If it is zero it
	//! stops rendering frames.
	virtual int next_frame(Time& time);

	//! Puts the rendered surface onto the target.
	virtual bool put_surface(cairo_surface_t *surface, ProgressCallback *cb=NULL);
	//! Filters the cairo surface based on gamma (hardcored for the moment to 2.2)
	static void gamma_filter(cairo_surface_t* surface, const synfig::Gamma &gamma);

private:
}; // END of class Target_Cairo

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
