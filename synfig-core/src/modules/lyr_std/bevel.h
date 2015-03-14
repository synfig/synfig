/* === S Y N F I G ========================================================= */
/*!	\file bevel.h
**	\brief Header file for implementation of the "Bevel" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_BEVEL_H__
#define __SYNFIG_LAYER_BEVEL_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>
#include <synfig/angle.h>

using namespace synfig;
using namespace std;
using namespace etl;

class Layer_Bevel : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (int) type of blur to use
	ValueBase param_type;
	//!Parameter: (synfig::Real) amount of blur
	ValueBase param_softness;
	//!Parameter: (synfig::Color) light color
	ValueBase param_color1;
	//!Parameter: (synfig::Color) dark color
	ValueBase param_color2;
	//!Parameter: (synfig::Angle) angle of the light source
	ValueBase param_angle;
	//!Parameter: (synfig::Real) depth of the bevel
	ValueBase param_depth;
	//!Parameter: (bool) use luma
	ValueBase param_use_luma;
	//!Parameter: (bool) solid
	ValueBase param_solid;
	
	synfig::Vector	offset;
	synfig::Vector	offset45;


	void calc_offset();
public:
	Layer_Bevel();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual synfig::Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }
}; // END of class Layer_Bevel

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
