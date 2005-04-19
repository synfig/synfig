/* === S Y N F I G ========================================================= */
/*!	\file target_multi.h
**	\brief Template Header
**
**	$Id: target_multi.h,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#ifndef __SYNFIG_TARGET_MULTI_H
#define __SYNFIG_TARGET_MULTI_H

/* === H E A D E R S ======================================================= */

#include "target_scanline.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Multi
**	\brief Render-target
**	\todo writeme
*/
class Target_Multi : public Target_Scanline
{
	Target_Scanline::Handle a,b;
	Color *buffer_a;
	Color *buffer_b;
public:

	Target_Multi(Target_Scanline::Handle a,Target_Scanline::Handle b);
	virtual ~Target_Multi();
	virtual bool add_frame(const synfig::Surface *surface);
	virtual bool start_frame(ProgressCallback *cb=NULL);
	virtual void end_frame();
	virtual Color * start_scanline(int scanline);
	virtual bool end_scanline();
	
	virtual void set_canvas(etl::handle<Canvas> c);
	virtual bool set_rend_desc(RendDesc *d);
	virtual bool init();
}; // END of class Target_Multi

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
