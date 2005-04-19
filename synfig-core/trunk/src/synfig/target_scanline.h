/* === S I N F G =========================================================== */
/*!	\file target_scanline.h
**	\brief Template Header
**
**	$Id: target_scanline.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SINFG_TARGET_SCANLINE_H
#define __SINFG_TARGET_SCANLINE_H

/* === H E A D E R S ======================================================= */

#include "target.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/*!	\class Target_Scanline
**	\brief Render-target
**	\todo writeme
*/
class Target_Scanline : public Target
{
	int threads_;
	int curr_frame_;

public:
	typedef etl::handle<Target_Scanline> Handle;
	typedef etl::loose_handle<Target_Scanline> LooseHandle;
	typedef etl::handle<const Target_Scanline> ConstHandle;

	Target_Scanline();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);

	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	**	\see end_frame(), start_scanline()
	*/
	virtual bool start_frame(ProgressCallback *cb=NULL)=0;

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
	**	and does whatever it is supose to do with it.
	**	\return \c true on success, \c false on failure.
	**	\see start_scanline()
	*/
	virtual bool end_scanline()=0;
	
	void set_threads(int x) { threads_=x; }

	int get_threads()const { return threads_; }

	bool add_frame(const sinfg::Surface *surface);
private:
}; // END of class Target_Scanline

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
