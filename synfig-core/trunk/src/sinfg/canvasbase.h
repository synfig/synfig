/* === S I N F G =========================================================== */
/*!	\file canvasbase.h
**	\brief Template Header
**
**	$Id: canvasbase.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_CANVASBASE_H
#define __SINFG_CANVASBASE_H

/* === H E A D E R S ======================================================= */

#include <deque>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

namespace sinfg {

class Layer;
	
typedef std::deque< etl::handle< Layer > > CanvasBase;
	
}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
