/* === S Y N F I G ========================================================= */
/*!	\file mandelbrot.h
**	\brief Header file for implementation of the "Mandelbrot Set" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MANDELBROT_H
#define __SYNFIG_MANDELBROT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/angle.h>
#include <synfig/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Mandelbrot : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (int)
	ValueBase param_iterations;
	//!Parameter: (Real)
	ValueBase param_bailout;
	Real lp;
	//!Parameter: (bool)
	ValueBase param_broken;

	//!Parameter: (bool)
	ValueBase param_distort_inside;
	//!Parameter: (bool)
	ValueBase param_shade_inside;
	//!Parameter: (bool)
	ValueBase param_solid_inside;
	//!Parameter: (bool)
	ValueBase param_invert_inside;
	//!Parameter: (Gradient)
	ValueBase param_gradient_inside;
	//!Parameter: (Real)
	ValueBase param_gradient_offset_inside;
	//!Parameter: (bool)
	ValueBase param_gradient_loop_inside;
	//!Parameter: (bool)
	ValueBase param_distort_outside;
	//!Parameter: (bool)
	ValueBase param_shade_outside;
	//!Parameter: (bool)
	ValueBase param_solid_outside;
	//!Parameter: (bool)
	ValueBase param_invert_outside;
	//!Parameter: (Gradient)
	ValueBase param_gradient_outside;
	//!Parameter: (bool)
	ValueBase param_smooth_outside;
	//!Parameter: (Real)
	ValueBase param_gradient_offset_outside;
	//!Parameter: (Real)
	ValueBase param_gradient_scale_outside;

public:
	Mandelbrot();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Vocab get_param_vocab()const;

protected:
	virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
