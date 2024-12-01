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

#include <synfig/rendering/software/task/tasksw.h>
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
	: public rendering::Task
{
public:
	typedef etl::handle<TaskInsideOut> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point origin;
	bool draft = false;

	Rect
	get_expanded_square(const Rect& source_rect, Vector expansion) const
	{
		Rect rect(source_rect.get_min() - expansion, source_rect.get_max() + expansion);
		Rect::value_type diff = rect.get_width() - rect.get_height();
		if (approximate_greater(diff, 0.)) {
			rect.expand_y(diff);
		} else if (approximate_less(diff, 0.)) {
			rect.expand_x(-diff);
		}
		return rect;
	}

	void
	set_coords_sub_tasks() override
	{
		if (!sub_task(0)) {
			trunc_to_zero();
			return;
		}
		if (!is_valid_coords()) {
			for (const auto& sub_task : sub_tasks)
				sub_task->set_coords_zero();
			return;
		}

		const Vector ppu = get_pixels_per_unit();

		Matrix bounds_transformation;
		bounds_transformation.m00 = ppu[0];
		bounds_transformation.m11 = ppu[1];
		bounds_transformation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transformation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Rect required_source_rect = get_expanded_square(source_rect, Point(3,3));

		sub_task(0)->set_coords(required_source_rect, target_rect.get_size());
		if (!draft) {
			sub_tasks[1]->set_coords(Rect(origin - Point(1.5,1.5), origin + Point(1.5,1.5)), target_rect.get_size()*5);
			sub_tasks[2]->set_coords(get_expanded_square(source_rect, Vector(20,20)), target_rect.get_size()/4);
		}
	}

};

class TaskInsideOutSW
	: public TaskInsideOut, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskInsideOutSW> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point
	point_vfunc(const Point& point) const
	{
		Point pos(point - origin);
		if (pos.mag_squared() == 0)
			return point;
		Point invpos(pos / pos.mag_squared());
		return invpos + origin;
	}

	bool run(Task::RunParams& /*params*/) const override
	{
		if (!sub_task(0))
			return true;
		if (!is_valid())
			return true;

		rendering::TaskSW::LockWrite la(this);
		if (!la)
			return false;

		rendering::TaskSW::LockRead lb(sub_task(0));
		if (!lb)
			return false;

		rendering::TaskSW::LockRead lc(!draft && sub_tasks.size() > 1 ? sub_tasks[1] : nullptr);
		if (!draft && !lc)
			return false;

		rendering::TaskSW::LockRead ld(!draft && sub_tasks.size() > 1 ? sub_tasks[2] : nullptr);
		if (!draft && !ld)
			return false;

		const int tw = target_rect.get_width();

		synfig::Surface::pen pen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));

		const Vector ppu = get_pixels_per_unit();

		Matrix bounds_transformation;
		bounds_transformation.m00 = ppu[0];
		bounds_transformation.m11 = ppu[1];
		bounds_transformation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transformation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix inv_matrix = bounds_transformation.get_inverted();

		Vector dx = inv_matrix.axis_x();
		Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
		Vector p = inv_matrix.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );


		std::vector<Vector> sub_ppu{sub_task(0)->get_pixels_per_unit()};
		std::vector<const synfig::Surface*> sub_surfaces{&lb->get_surface()};
		if (!draft) {
			sub_ppu.push_back(sub_tasks[1]->get_pixels_per_unit());
			sub_ppu.push_back(sub_tasks[2]->get_pixels_per_unit());

			sub_surfaces.push_back(&lc->get_surface());
			sub_surfaces.push_back(&ld->get_surface());
		}

		for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
			for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
				Point pos(p - origin);
				int sub_task_idx = draft ? 0 : (pos.mag_squared() <= 0.05 ? 2 : (pos.mag_squared() < 0.5 ? 0 : 1));
				const synfig::Surface* sub_surface = sub_surfaces[sub_task_idx];
				pos = point_vfunc(p);


				Real u = (pos[0]-sub_tasks[sub_task_idx]->source_rect.minx)*sub_ppu[sub_task_idx][0];
				Real v = (pos[1]-sub_tasks[sub_task_idx]->source_rect.miny)*sub_ppu[sub_task_idx][1];

				u = synfig::clamp(u, 0., Real(sub_surface->get_w()-1));
				v = synfig::clamp(v, 0., Real(sub_surface->get_h()-1));

				if (u<0 || v<0 || u>=sub_surface->get_w() || v>=sub_surface->get_h() || std::isnan(u) || std::isnan(v)) {
					// problem! It shouldn't happen!!
					pen.put_value(Color::magenta());
					continue;
				} else {
					pen.put_value(sub_surface->cubic_sample(u,v));
				}
			}
		}

		return true;
	}
};

rendering::Task::Token TaskInsideOut::token(
	DescAbstract<TaskInsideOut>("InsideOut") );
rendering::Task::Token TaskInsideOutSW::token(
	DescReal<TaskInsideOutSW, TaskInsideOut>("InsideOutSW") );

InsideOut::InsideOut():
	param_origin(ValueBase(Point(0,0))),
	param_cobra(ValueBase(true)),
	param_draft(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
InsideOut::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_cobra);
	IMPORT_VALUE(param_draft);
	return false;
}

ValueBase
InsideOut::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_cobra);
	EXPORT_VALUE(param_draft);

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

	ret.push_back(ParamDesc("cobra")
		.set_local_name(_("Cobra"))
		.set_description(_("Use Cobra Renderer"))
	);

	ret.push_back(ParamDesc("draft")
		.set_local_name(_("Draft"))
		.set_description(_("Renders faster, but inaccurately"))
	);

	return ret;
}

rendering::Task::Handle
InsideOut::build_rendering_task_vfunc(Context context) const
{
	if (!param_cobra.get(bool()))
		return Layer::build_rendering_task_vfunc(context);

	rendering::Task::Handle task = context.build_rendering_task();

	TaskInsideOut::Handle task_insideout(new TaskInsideOut());
	task_insideout->origin = param_origin.get(Point());
	task_insideout->draft = param_draft.get(bool());

	task_insideout->sub_task(0) = task;
	if (!task_insideout->draft) {
		task_insideout->sub_tasks.push_back(task->clone_recursive());
		task_insideout->sub_tasks.push_back(task->clone_recursive());
	}

	task = task_insideout;

	return task;
}
