/* === S Y N F I G ========================================================= */
/*!	\file layer_bitmap.h
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

#ifndef __SYNFIG_LAYER_BITMAP_H
#define __SYNFIG_LAYER_BITMAP_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/surface.h>
#include <synfig/target.h> // for RenderMethod

#include <synfig/rendering/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Bitmap
**	\todo writeme
*/
class Layer_Bitmap : public Layer_Composite, public Layer_NoDeform
{
	const Color& filter(Color& c)const;
	const CairoColor& filter(CairoColor& c)const;
	RenderMethod method;
public:
	typedef etl::handle<Layer_Bitmap> Handle;

	ValueBase param_tl;
	ValueBase param_br;
	ValueBase param_c;
	ValueBase param_gamma_adjust;

	mutable synfig::Mutex mutex;
	mutable Surface surface;
	mutable CairoSurface csurface;
	mutable rendering::Surface::Handle rendering_surface;
	mutable bool trimmed;
	mutable unsigned int width, height, top, left;


	Layer_Bitmap();
	~Layer_Bitmap()	{ 
	if(csurface.is_mapped()) csurface.unmap_cairo_image(); }

	virtual bool set_param(const String & param, const ValueBase & value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual CairoColor get_cairocolor(Context context, const Point &pos)const;

	virtual Vocab get_param_vocab()const;

	virtual Rect get_bounding_rect()const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	
	virtual void set_render_method(Context context, RenderMethod x);
	void set_method(RenderMethod x) { method=x;}
	RenderMethod get_method()const { return method;}
	
	void set_cairo_surface(cairo_surface_t* cs);

protected:
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;
}; // END of class Layer_Bitmap

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
