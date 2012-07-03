/* === S Y N F I G ========================================================= */
/*!	\file layer_cairobitmap.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012 Carlos López
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

#ifndef __SYNFIG_LAYER_CAIROBITMAP_H
#define __SYNFIG_LAYER_CAIROBITMAP_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_CairoBitmap
**	\todo writeme
*/
class Layer_CairoBitmap : public Layer_Composite, public Layer_NoDeform
{
	const CairoColor& filter(CairoColor& c)const;
public:
	typedef etl::handle<Layer_CairoBitmap> Handle;

	Point tl;
	Point br;
	int c;
	mutable CairoSurface surface;
	mutable bool trimmed;
	mutable unsigned int width, height, top, left;

	Real gamma_adjust;

	Layer_CairoBitmap();

	virtual bool set_param(const String & param, const ValueBase & value);

	virtual ValueBase get_param(const String & param)const;

	virtual CairoColor get_cairocolor(Context context, const Point &pos)const;

	virtual Vocab get_param_vocab()const;

	virtual Rect get_bounding_rect()const;

	virtual bool accelerated_render(Context context,CairoSurface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
}; // END of class Layer_CairoBitmap

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
