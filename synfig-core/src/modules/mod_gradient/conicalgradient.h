/* === S Y N F I G ========================================================= */
/*!	\file conicalgradient.h
**	\brief Header file for implementation of the "Conical Gradient" layer
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

#ifndef __SYNFIG_CONICALGRADIENT_H
#define __SYNFIG_CONICALGRADIENT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <synfig/gradient.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class ConicalGradient : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Gradient)
	ValueBase param_gradient;
	//! Parameter: (Point)
	ValueBase param_center;
	//! Parameter: (Angle)
	ValueBase param_angle;
	//! Parameter: (bool)
	ValueBase param_symmetric;

	Color color_func(const Point &x, float supersample=0)const;

	float calc_supersample(const Point &x, float pw,float ph)const;
	bool compile_mesh(cairo_pattern_t* pattern, Gradient gradient, Real radius)const;

public:

	ConicalGradient();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;

	virtual Vocab get_param_vocab()const;
}; // END of class ConicalGradient

/* === E N D =============================================================== */

#endif
