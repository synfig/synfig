/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskdistortsw.cpp
**	\brief TaskDistortSW interface base implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2024 Synfig Contributors
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
# include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "taskdistortsw.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig::rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskDistortSW::run_task(const rendering::TaskDistort& task) const
{
	LoopInfo info = get_loop_info(task);

	if (info.may_end)
		return true;
	if (info.should_abort)
		return false;

	rendering::TaskSW::LockWrite la(&task);
	if (!la)
		return false;

	const rendering::Task::Handle& sub_task = task.sub_task();

	rendering::TaskSW::LockRead sub_lock(sub_task);
	if (!sub_lock)
		return false;

	const synfig::Surface& sub_surface = sub_lock->get_surface();

	synfig::Surface::pen pen(la->get_surface().get_pen(task.target_rect.minx, task.target_rect.miny));

	for (int iy = task.target_rect.miny; iy < task.target_rect.maxy; ++iy, info.initial_p += info.p_dy, pen.move(info.pen_dy[0], info.pen_dy[1])) {
		for (int ix = task.target_rect.minx; ix < task.target_rect.maxx; ++ix, info.initial_p += info.p_dx, pen.inc_x()) {
			Point res = point_vfunc(info.initial_p);

			Point raster_p = info.sub_world_to_raster_transformation.get_transformed(res);

			if (!raster_p.is_valid()) {
				// problem! It shouldn't happen!! At least one of the coordinates is NaN
				pen.put_value(Color::cyan());
				continue;
			} else {
				Real u = raster_p[0];
				Real v = raster_p[1];
				if (u<0 || v<0 || u>=sub_surface.get_w() || v>=sub_surface.get_h()) {
					// problem! It shouldn't happen!! Out of sub_task surface bounds
					pen.put_value(Color::magenta());
					continue;
				}

				pen.put_value(sub_surface.cubic_sample(u,v));
			}
		}
	}

	return true;
}

TaskDistortSW::LoopInfo
TaskDistortSW::get_loop_info(const Task& task)
{
	LoopInfo info;

	if (!task.is_valid()) {
		info.may_end = true;
		return info;
	}

	if (task.sub_tasks.empty()) {
		info.may_end = true;
		return info;
	}

	{
		auto sub_task = task.sub_tasks.at(0);
		if (!sub_task->is_valid()) {
			info.may_end = true;
			return info;
		}

		const Vector sub_ppu = sub_task->get_pixels_per_unit();
		info.sub_world_to_raster_transformation.m00 = sub_ppu[0];
		info.sub_world_to_raster_transformation.m11 = sub_ppu[1];
		info.sub_world_to_raster_transformation.m20 = sub_task->target_rect.minx - sub_ppu[0]*sub_task->source_rect.minx;
		info.sub_world_to_raster_transformation.m21 = sub_task->target_rect.miny - sub_ppu[1]*sub_task->source_rect.miny;
	}

	const Vector upp = task.get_units_per_pixel();

	Matrix raster_to_world_transformation;
	raster_to_world_transformation.m00 = upp[0];
	raster_to_world_transformation.m11 = upp[1];
	raster_to_world_transformation.m20 = task.source_rect.minx - upp[0]*task.target_rect.minx;
	raster_to_world_transformation.m21 = task.source_rect.miny - upp[1]*task.target_rect.miny;

	const int tw = task.target_rect.get_width();
	info.p_dx = {upp[0] , 0};
	info.p_dy = {- info.p_dx[0] * tw, upp[1]};

	info.pen_dy = {-tw, 1};

	info.initial_p = raster_to_world_transformation.get_transformed( Vector(Real(task.target_rect.minx), Real(task.target_rect.miny)) );

	return info;
}

bool
TaskDistortOrColorSW::run_task(const rendering::TaskDistort& task) const
{
	TaskDistortSW::LoopInfo info = TaskDistortSW::get_loop_info(task);

	if (info.may_end)
		return true;
	if (info.should_abort)
		return false;

	rendering::TaskSW::LockWrite la(&task);
	if (!la)
		return false;

	const rendering::Task::Handle& sub_task = task.sub_task();

	rendering::TaskSW::LockRead sub_lock(sub_task);
	if (!sub_lock)
		return false;

	const synfig::Surface& sub_surface = sub_lock->get_surface();

	synfig::Surface::pen pen(la->get_surface().get_pen(task.target_rect.minx, task.target_rect.miny));

	for (int iy = task.target_rect.miny; iy < task.target_rect.maxy; ++iy, info.initial_p += info.p_dy, pen.move(info.pen_dy[0], info.pen_dy[1])) {
		for (int ix = task.target_rect.minx; ix < task.target_rect.maxx; ++ix, info.initial_p += info.p_dx, pen.inc_x()) {
			Result res = point_or_color_vfunc(info.initial_p);

			if (!res) {
				pen.put_value(res.color());
				continue;
			}

			Point raster_p = info.sub_world_to_raster_transformation.get_transformed(res.point());

			if (!raster_p.is_valid()) {
				// problem! It shouldn't happen!! At least one of the coordinates is NaN
				pen.put_value(Color::cyan());
				continue;
			} else {
				Real u = raster_p[0];
				Real v = raster_p[1];
				if (u<0 || v<0 || u>=sub_surface.get_w() || v>=sub_surface.get_h()) {
					// problem! It shouldn't happen!! Out of sub_task surface bounds
					pen.put_value(Color::magenta());
					continue;
				}

				pen.put_value(sub_surface.cubic_sample(u,v));
			}
		}
	}

	return true;
}

TaskDistortOrColorSW::Result::Result(const Point& p) noexcept
	: type_(Type::POINT)
{
	point_ = p;
}

TaskDistortOrColorSW::Result::Result(Point&& p) noexcept
	: type_(Type::POINT)
{
	point_ = p;
}

TaskDistortOrColorSW::Result::Result(const Color& color) noexcept
	: type_(Type::COLOR)
{
	color_ = color;
}

TaskDistortOrColorSW::Result::Result(Color&& color) noexcept
	: type_(Type::COLOR)
{
	color_ = color;
}

TaskDistortOrColorSW::Result::operator bool() const
{
	return type_ == Type::POINT;
}

synfig::Point
TaskDistortOrColorSW::Result::point() const
{
	if (type_ == Type::POINT)
		return point_;
	throw new std::logic_error("Internal error: trying to get an invalid point of Result");
}

synfig::Color
TaskDistortOrColorSW::Result::color() const
{
	if (type_ == Type::COLOR)
		return color_;
	throw new std::logic_error("Internal error: trying to get an invalid color of Result");
}
