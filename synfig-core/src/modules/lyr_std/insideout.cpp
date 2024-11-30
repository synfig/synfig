/* === S Y N F I G ========================================================= */
/*!	\file insideout.cpp
**	\brief Implementation of the "Inside Out" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "insideout.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/rendering/common/task/taskdistort.h>
#include <synfig/rendering/software/task/taskdistortsw.h>
#include <synfig/transform.h>

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(InsideOut);
SYNFIG_LAYER_SET_NAME(InsideOut,"inside_out");
SYNFIG_LAYER_SET_LOCAL_NAME(InsideOut,N_("Inside Out"));
SYNFIG_LAYER_SET_CATEGORY(InsideOut,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(InsideOut,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskInsideOut
	: public rendering::TaskDistort
{
public:
	typedef etl::handle<TaskInsideOut> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point origin;

	Rect
	compute_required_source_rect(const Rect& source_rect, const Matrix& /*inv_matrix*/) const override
	{
		Rect rect(source_rect.get_min() - Point(2,2), source_rect.get_max() + Point(2,2));
		Rect::value_type diff = rect.get_width() - rect.get_height();
		if (approximate_greater(diff, 0.)) {
			rect.expand_y(diff);
		} else if (approximate_less(diff, 0.)) {
			rect.expand_x(-diff);
		}
		return rect;
	}

};

class TaskInsideOutSW
	: public TaskInsideOut, public rendering::TaskDistortSW
{
public:
	typedef etl::handle<TaskInsideOutSW> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	TaskInsideOutSW()
	{
		rendering::TaskDistortSW::should_clamp_coordinates = true;
	}

	Point
	point_vfunc(const Point& point) const override
	{
		Point pos(point - origin);
		if (pos.mag_squared() == 0)
			return point;
		Point invpos(pos / pos.mag_squared());
		return invpos + origin;
	}

	bool run(Task::RunParams& /*params*/) const override
	{
		return run_task(*this);
	}
};

rendering::Task::Token TaskInsideOut::token(
	DescAbstract<TaskInsideOut>("InsideOut") );
rendering::Task::Token TaskInsideOutSW::token(
	DescReal<TaskInsideOutSW, TaskInsideOut>("InsideOutSW") );

InsideOut::InsideOut():
	param_origin(ValueBase(Point(0,0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
InsideOut::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	return false;
}

ValueBase
InsideOut::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Handle
InsideOut::hit_check(Context context, const Point &p)const
{
	Point origin=param_origin.get(Point());
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.hit_check(invpos+origin);
}

Color
InsideOut::get_color(Context context, const Point &p)const
{
	Point origin=param_origin.get(Point());
	Point pos(p-origin);
	Real inv_mag=pos.inv_mag();
	Point invpos(pos*inv_mag*inv_mag);
	return context.get_color(invpos+origin);
}


class lyr_std::InsideOut_Trans : public Transform
{
	etl::handle<const InsideOut> layer;
public:
	InsideOut_Trans(const InsideOut* x):Transform(x->get_guid()),layer(x) { }

	Vector perform(const Vector& x)const
	{
		Point origin=layer->param_origin.get(Point());
		Point pos(x-origin);
		Real inv_mag=pos.inv_mag();
		if(!std::isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+origin);
		return x;
	}

	Vector unperform(const Vector& x)const
	{
		Point origin=layer->param_origin.get(Point());
		Point pos(x-origin);
		Real inv_mag=pos.inv_mag();
		if(!std::isnan(inv_mag))
			return (pos*(inv_mag*inv_mag)+origin);
		return x;
	}

	String get_string()const
	{
		return "insideout";
	}
};
etl::handle<Transform>
InsideOut::get_transform()const
{
	return new InsideOut_Trans(this);
}

Layer::Vocab
InsideOut::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the distortion"))
		.set_is_distance()
	);

	return ret;
}

rendering::Task::Handle
InsideOut::build_rendering_task_vfunc(Context context) const
{
	rendering::Task::Handle task = context.build_rendering_task();

	TaskInsideOut::Handle task_insideout(new TaskInsideOut());
	task_insideout->origin = param_origin.get(Point());

	task_insideout->sub_task() = task;

	task = task_insideout;

	return task;
}
