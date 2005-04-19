/* === S I N F G =========================================================== */
/*!	\file savecanvas.h
**	\brief writeme
**
**	$Id: savecanvas.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_SAVECANVAS_H
#define __SINFG_SAVECANVAS_H

/* === H E A D E R S ======================================================= */

#include "string.h"
#include "canvas.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/* === E X T E R N S ======================================================= */

//!	Saves a canvas to \a filename
/*!	\return	\c true on success, \c false on error. */
bool save_canvas(const String &filename, Canvas::ConstHandle canvas);

//! Stores a Canvas in a string in XML format
String canvas_to_string(Canvas::ConstHandle canvas);

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
