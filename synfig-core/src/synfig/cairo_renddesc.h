/* === S Y N F I G ========================================================= */
/*!	\file cairo_renddesc.h
 **	\brief Cairo helper functions with RendDesc
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2013 Carlos López
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

#ifndef __SYNFIG_CAIRO_RENDDESC_H
#define __SYNFIG_CAIRO_RENDDESC_H

/* === H E A D E R S ======================================================= */
#include "renddesc.h"
#include "cairo.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

// Helper function to untransform the RendDesc by the inverse of the current
// Cairo matrix from the Cairo context.
// It doesn't count the initial scale and translation performed to
// convert the user coordinates to pixels.
// See implementation comments for more details
bool cairo_renddesc_untransform(cairo_t* cr, synfig::RendDesc &renddesc);

/* === E N D =============================================================== */

#endif
