/* === S Y N F I G ========================================================= */
/*!	\file spiralgradient.h
**	\brief Header file for implementation of the "Spiral Gradient" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SPIRALGRADIENT_H
#define __SYNFIG_SPIRALGRADIENT_H

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

class SpiralGradient : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Gradient)
	ValueBase param_gradient;
	//! Parameter: (Point)
	ValueBase param_center;
	//! Parameter: (Real)
	ValueBase param_radius;
	//! Parameter: (Angle)
	ValueBase param_angle;
	//! Parameter: (bool)
	ValueBase param_clockwise;

	CompiledGradient compiled_gradient;

	void compile();
	Color color_func(const Point &x, Real supersample=0)const;
	Real calc_supersample(const Point &x, Real pw, Real ph)const;

public:

	SpiralGradient();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;

	virtual Vocab get_param_vocab()const;
}; // END of class SpiralGradient

/* === E N D =============================================================== */

#endif
