/* === S Y N F I G ========================================================= */
/*!	\file twirl.h
**	\brief Header file for implementation of the "Twirl" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TWIRL_H
#define __SYNFIG_TWIRL_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite_fork.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <synfig/gradient.h>
#include <synfig/angle.h>
#include <synfig/rendering/common/task/taskdistort.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Twirl_Trans;

class Twirl : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Twirl_Trans;

private:
	//! Parameter: (Point)
	ValueBase param_center;
	//! Parameter: (Real)
	ValueBase param_radius;
	//! Parameter: (Angle)
	ValueBase param_rotations;
	//! Parameter: (bool)
	ValueBase param_distort_inside;
	//! Parameter: (bool)
	ValueBase param_distort_outside;
	//! Parameter: (bool)
	ValueBase param_cobra;

	Point distort(const Point &pos, bool reverse=false)const;
public:

	Twirl();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	//virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	Layer::Handle hit_check(Context context, const Point &point)const;

	virtual Vocab get_param_vocab()const;
	virtual etl::handle<Transform> get_transform()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;

public:
	struct Internal
	{
		Point center;
		Real radius;
		Angle rotations;
		bool distort_inside;
		bool distort_outside;

		Point transform(const Point& point) const;
		Point transform(const Point& point, bool reverse) const;
	};
}; // END of class Twirl

class TaskTwirl
	: public rendering::TaskDistort
{
public:
	typedef etl::handle<TaskTwirl> Handle;
	static Token token;
	Token::Handle get_token() const override;

	Twirl::Internal internal;

	// virtual bool get_mode_allow_source_as_target() const {
	// 	if (const Mode *mode = dynamic_cast<const Mode*>(this))
	// 		return mode->get_mode_allow_source_as_target();
	// 	return false;
	// }

	Rect compute_required_source_rect(const Rect& source_rect, const Matrix& raster_to_world_transformation) const override;

	Rect calc_bounds() const override;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
