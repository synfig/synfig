/* === S Y N F I G ========================================================= */
/*!	\file sphere_distort.h
**	\brief Header file for implementation of the "Spherize" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_SPHERE_DISTORT_H
#define __SYNFIG_SPHERE_DISTORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/vector.h>
#include <synfig/rect.h>
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

class Spherize_Trans;

class Layer_SphereDistort : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Spherize_Trans;

private:
	
	//!Parameter (Vector)
	ValueBase param_center;
	//!Parameter (double)
	ValueBase param_radius;
	//!Parameter (double)
	ValueBase param_amount;
	//!Parameter (int)
	ValueBase param_type;
	//!Parameter (bool)
	ValueBase param_clip;
	//!Parameter (bool)
	ValueBase param_cobra;

	Rect bounds;

	void sync();

public:

	Layer_SphereDistort();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;

	virtual Rect get_bounding_rect()const;

	virtual Vocab get_param_vocab()const;
	virtual etl::handle<Transform> get_transform()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;

public:
	struct Internal
	{
		Vector center = Vector(0,0);
		Real radius = 1.;
		Real percent = 1.;
		int type = 0;
		bool clip = false;

		Point transform(const Point& point) const;
		Point transform(const Point& point, bool& clipped) const;
	};
}; // END of class Layer_SphereDistort

class TaskSphereDistort
	: public rendering::TaskDistort
{
public:
	typedef etl::handle<TaskSphereDistort> Handle;
	static Token token;
	Token::Handle get_token() const override;

	Layer_SphereDistort::Internal internal;

	int get_pass_subtask_index() const override;

	Rect compute_required_source_rect(const Rect& source_rect, const Matrix& /*raster_to_world_transformation*/) const override;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
