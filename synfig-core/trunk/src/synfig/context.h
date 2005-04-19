/* === S I N F G =========================================================== */
/*!	\file context.h
**	\brief Template Header
**
**	$Id: context.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#ifndef __SINFG_CONTEXT_H
#define __SINFG_CONTEXT_H

/* === H E A D E R S ======================================================= */

#include "canvasbase.h"
#include "rect.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Vector;
typedef Vector Point;
class Color;
class Surface;
class RendDesc;
class ProgressCallback;
class Layer;
class Time;
class Rect;
	
/*!	\class Context
**	\todo writeme
**	\see Layer, Canvas */
class Context : public CanvasBase::const_iterator
{
public:
	Context() { }

	Context(const CanvasBase::const_iterator &x):CanvasBase::const_iterator(x) { }

	Context operator=(const CanvasBase::const_iterator &x)
	{ return CanvasBase::const_iterator::operator=(x); }
	
	/*!	\todo write me */
	Color get_color(const Point &pos)const;

	/*!	\todo write me */
	bool accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;

	/*!	\todo write me */
   	void set_time(Time time)const;

	/*!	\writeme */
   	void set_time(Time time,const Vector &pos)const;

	Rect get_full_bounding_rect()const;

	/*! \writeme */
	virtual etl::handle<Layer> hit_check(const Point &point)const;

}; // END of class Context
	
}; // END of namespace sinfg
	
/* === E N D =============================================================== */

#endif
