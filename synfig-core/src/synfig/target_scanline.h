/* === S Y N F I G ========================================================= */
/*!	\file target_scanline.h
**	\brief Template Header for the Target Scanline class
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

#ifndef __SYNFIG_TARGET_SCANLINE_H
#define __SYNFIG_TARGET_SCANLINE_H

/* === H E A D E R S ======================================================= */

#include "target.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Scanline
**	\brief This is a Target class that implements the render fucntion
* for a line by line render procedure
*/
class Target_Scanline : public Target
{
	//! Number of threads to use
	int threads_;
	//! Current frame being rendered
	int curr_frame_;

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
	/*! \todo Fix the calculation of frames to really render the last frame
	** When start frame= 1f and end frame = 1f it just render one frame (at 1f)
	** When start frame= 1f and end frame = 2f it just render one frame (at 1f)
	** which is a bug.
	* */
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

	//! Marks the start of a scanline (using RGBA data)
	/*!	\param scanline Which scanline is going to be rendered.
	**	\return The address where the target wants the scanline
	**		to be written.
	**	\warning Must be called after start_frame()
	**	\see end_scanline(), start_frame()
	*/
	virtual unsigned char* start_scanline_rgba(int scanline)=0;

	//! Marks the end of a scanline
	/*! Takes the data that was put at the address returned to by start_scanline()
	**	and does whatever it is supposed to do with it.
	**	\return \c true on success, \c false on failure.
	**	\see start_scanline()
	*/
	virtual bool end_scanline()=0;

	//! Marks the end of a scanline (using RGBA data)
	/*! Takes the data that was put at the address returned to by start_scanline_rgba()
	**	and does whatever it is supposed to do with it.
	**	\return \c true on success, \c false on failure.
	**	\see start_scanline()
	*/
	virtual bool end_scanline_rgba()=0;

	//! Sets the number of threads
	void set_threads(int x) { threads_=x; }
	//! Gets the number of threads
	int get_threads()const { return threads_; }
	//! Puts the rendered surface onto the target.
	bool add_frame(const synfig::Surface *surface);
	bool add_frame(const unsigned char *data, const unsigned int width, const unsigned int height);

private:
	bool render_frame_(int quality, ProgressCallback *cb=0);
}; // END of class Target_Scanline

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
