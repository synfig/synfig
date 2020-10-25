/* === S Y N F I G ========================================================= */
/*!	\file julia.h
**	\brief Header file for implementation of the "Julia Set" layer
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

#ifndef __SYNFIG_JULIA_H
#define __SYNFIG_JULIA_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Julia : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (Color)
	ValueBase param_icolor;
	//!Parameter: (Color)
	ValueBase param_ocolor;
	//!Parameter: (Angle)
	ValueBase param_color_shift;
	//!Parameter: (int)
	ValueBase param_iterations;
	//!Parameter: (Point)
	ValueBase param_seed;
	//!Parameter: (Real)
	ValueBase param_bailout;
	//!Parameter: (bool)
	ValueBase param_distort_inside;
	//!Parameter: (bool)
	ValueBase param_shade_inside;
	//!Parameter: (bool)
	ValueBase param_solid_inside;
	//!Parameter: (bool)
	ValueBase param_invert_inside;
	//!Parameter: (bool)
	ValueBase param_color_inside;
	//!Parameter: (bool)
	ValueBase param_distort_outside;
	//!Parameter: (bool)
	ValueBase param_shade_outside;
	//!Parameter: (bool)
	ValueBase param_solid_outside;
	//!Parameter: (bool)
	ValueBase param_invert_outside;
	//!Parameter: (bool)
	ValueBase param_color_outside;
	//!Parameter: (bool)
	ValueBase param_color_cycle;
	//!Parameter: (bool)
	ValueBase param_smooth_outside;
	//!Parameter: (bool)
	ValueBase param_broken;
	Real lp;



public:
	Julia();

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
