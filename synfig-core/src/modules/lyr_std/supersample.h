/* === S Y N F I G ========================================================= */
/*!	\file supersample.h
**	\brief Header file for implementation of the "Super Sample" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SUPERSAMPLE_H
#define __SYNFIG_SUPERSAMPLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class SuperSample : public synfig::Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter (int)
	ValueBase param_width, param_height;
	//!Parameter (bool)
	ValueBase param_scanline, param_alpha_aware;
public:
	SuperSample();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	using Layer::get_bounding_rect;
	virtual synfig::Rect get_bounding_rect(Context context)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
