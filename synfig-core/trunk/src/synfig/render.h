/* === S I N F G =========================================================== */
/*!	\file render.h
**	\brief Template Header
**
**	$Id: render.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_RENDER_H
#define __SINFG_RENDER_H

/* === H E A D E R S ======================================================= */

#include "target_scanline.h"
#include "vector.h"
#include "color.h"
#include "renddesc.h"
#include "general.h"
#include "layer.h"
#include "canvas.h"
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

//! Renders starting at \a context to \a target
/*! \warning \a Target::set_rend_desc() must have
**		already been called on \a target before
**		you call this function!
*/
extern bool render(Context context, Target_Scanline::Handle target, const RendDesc &desc,ProgressCallback *);

extern bool parametric_render(Context context, Surface &surface, const RendDesc &desc,ProgressCallback *);

extern bool render_threaded(	Context context,
	Target_Scanline::Handle target,
	const RendDesc &desc,
	ProgressCallback *callback,
	int threads);

}; /* end namespace sinfg */

/* -- E N D ----------------------------------------------------------------- */

#endif
