/* === S Y N F I G ========================================================= */
/*!	\file target_scanline.h
**	\brief Template Header for the Target Scanline class
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

#ifndef __SYNFIG_TARGET_SCANLINE_H
#define __SYNFIG_TARGET_SCANLINE_H

/* === H E A D E R S ======================================================= */

#include "target.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace rendering { class SurfaceResource; }

/*!	\class Target_Scanline
**	\brief This is a Target class that implements the render function
**     for a line by line render procedure
*/
class Target_Scanline : public Target
{
	//! Number of threads to use
	int threads_;

	String engine_;

	bool call_renderer(Context &context, const etl::handle<rendering::SurfaceResource> &surface, int quality, const RendDesc &renddesc, ProgressCallback *cb);

public:
	typedef etl::handle<Target_Scanline> Handle;
	typedef etl::loose_handle<Target_Scanline> LooseHandle;
	typedef etl::handle<const Target_Scanline> ConstHandle;
	//! Default constructor (threads = 2 current frame = 0)
	Target_Scanline();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);

	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	**	\see end_frame(), start_scanline()
	*/
	virtual bool start_frame(ProgressCallback *cb=NULL)=0;
	
	//! Returns the number of peniding frames to render. If it is zero it
	//! stops rendering frames.
	virtual int next_frame(Time& time);

	//! Marks the end of a frame
	/*! \see start_frame() */
	virtual void end_frame()=0;

	//! Marks the start of a scanline
	/*!	\param scanline Which scanline is going to be rendered.
	**	\return The address where the target wants the scanline
	**		to be written.
	**	\warning Must be called after start_frame()
	**	\see end_scanline(), start_frame()
	*/
	virtual Color * start_scanline(int scanline)=0;

	//! Marks the end of a scanline
	/*! Takes the data that was put at the address returned to by start_scanline()
	**	and does whatever it is supposed to do with it.
	**	\return \c true on success, \c false on failure.
	**	\see start_scanline()
	*/
	virtual bool end_scanline()=0;
	//! Sets the number of threads

	void set_threads(int x) { threads_=x; }
	//! Gets the number of threads
	int get_threads()const { return threads_; }
	//! Gets engine
	const String& get_engine()const { return engine_; }
	//! Sets engine
	void set_engine(const String &x) { engine_=x; }

	//! Puts the rendered surface onto the target.
	bool add_frame(const synfig::Surface *surface);
private:
}; // END of class Target_Scanline

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
