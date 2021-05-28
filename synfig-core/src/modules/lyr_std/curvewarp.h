/* === S Y N F I G ========================================================= */
/*!	\file curvewarp.h
**	\brief Header file for implementation of the "Curve Warp" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_CURVEWARP_H
#define __SYNFIG_CURVEWARP_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <synfig/vector.h>
#include <synfig/layer.h>
#include <synfig/blinepoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class CurveWarp : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (Point) origin of the warp
	ValueBase param_origin;
	//!Parameter: (Real) perpendicular expansion
	ValueBase param_perp_width;
	//!Parameter: (Point) start point of source
	ValueBase param_start_point;
	//!Parameter: (Point) end point of source
	ValueBase param_end_point;
	//!Parameter: (std::vector<BlinePoint>) spline of the warp
	ValueBase param_bline;
	//!Parameter: (bool)
	ValueBase param_fast;

	Vector perp_;
	Real curve_length_;

	void sync();

public:
	CurveWarp();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Point transform(const Point &point_, Real *dist=NULL, Real *along=0, int quality=10)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;

	virtual Vocab get_param_vocab()const;

protected:
	virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
