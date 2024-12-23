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

#ifndef SYNFIG_CURVEWARP_H
#define SYNFIG_CURVEWARP_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

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

	void sync();

public:
	CurveWarp();
	~CurveWarp();

	bool set_param(const String& param, const ValueBase& value) override;
	ValueBase get_param(const String& param) const override;
	Color get_color(Context context, const Point& pos) const override;
	Layer::Handle hit_check(Context context, const Point& point) const override;

	Vocab get_param_vocab() const override;

protected:
	rendering::Task::Handle build_rendering_task_vfunc(Context context) const override;

public:
	struct Internal;

private:
	Internal* internal;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
