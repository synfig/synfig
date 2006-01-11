/* === S Y N F I G ========================================================= */
/*!	\file layer_bitmap.h
**	\brief Template Header
**
**	$Id: layer_bitmap.h,v 1.2 2005/01/24 03:08:18 darco Exp $
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

#ifndef __SYNFIG_LAYER_BITMAP_H
#define __SYNFIG_LAYER_BITMAP_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Bitmap
**	\todo writeme
*/
class Layer_Bitmap : public Layer_Composite, public Layer_NoDeform
{
	const Color& filter(const Color& c)const;
public:
	typedef etl::handle<Layer_Bitmap> Handle;	

	Point tl;
	Point br;
	int c;
	mutable Surface surface;
	
	Real gamma_adjust;
	
	Layer_Bitmap();
	
	virtual bool set_param(const String & param, ValueBase value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	
	virtual Vocab get_param_vocab()const;

	virtual Rect get_bounding_rect()const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &rend_desc, ProgressCallback *callback)const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
}; // END of class Layer_Bitmap

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
