/* === S Y N F I G ========================================================= */
/*!	\file target_multi.h
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
	virtual bool add_frame(const synfig::Surface *surface, ProgressCallback *cb);
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
