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
#include <synfig/guid.h>

#include <synfig/rendering/surface.h>
#include <synfig/rendering/software/function/packedsurface.h>

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

	GUID surface_modification_id;
public:
	typedef std::shared_ptr<Layer_Bitmap> Handle;

	ValueBase param_tl;
	ValueBase param_br;
	ValueBase param_c;
	ValueBase param_gamma_adjust;

	mutable std::mutex mutex;
	mutable rendering::software::PackedSurface::Reader reader;
	mutable rendering::SurfaceResource::Handle rendering_surface;
	mutable bool trimmed;
	mutable unsigned int left, top, width, height;


	Layer_Bitmap();

	GUID get_surface_modification_id() const
		{ return surface_modification_id; }
	bool is_surface_modified() const
		{ return (bool)get_surface_modification_id(); }
	void add_surface_modification_id(const GUID &modification_id)
		{ surface_modification_id ^= modification_id; }
	void reset_surface_modification_id()
		{ surface_modification_id = GUID::zero(); }

	virtual bool set_param(const String & param, const ValueBase & value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Vocab get_param_vocab()const;

	virtual Rect get_bounding_rect()const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	
protected:
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;
}; // END of class Layer_Bitmap

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
