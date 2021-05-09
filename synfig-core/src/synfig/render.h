/* === S Y N F I G ========================================================= */
/*!	\file synfig/render.h
**	\brief Template Header
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

#ifndef __SYNFIG_RENDER_H
#define __SYNFIG_RENDER_H

/* === H E A D E R S ======================================================= */

#include "target_scanline.h"
#include "renddesc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! Renders starting at \a context to \a target
/*! \warning \a Target::set_rend_desc() must have
**		already been called on \a target before
**		you call this function!
*/
extern bool render(Context context, Target_Scanline::Handle target, const RendDesc &desc,ProgressCallback *);

}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
